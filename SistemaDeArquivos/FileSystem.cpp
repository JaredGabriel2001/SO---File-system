#include "FileSystem.h"
#include <iostream>
#include <fstream>
#include <cstring>

FileSystem::FileSystem(const std::string& diskPath) {
    // Abrir o arquivo que simula o disco
    disk = fopen(diskPath.c_str(), "wb+");
    if (!disk) {
        throw std::runtime_error("Erro ao abrir o disco!");
    }
    fat = nullptr;
    rootDir = nullptr;
    dataArea = nullptr;
}

FileSystem::~FileSystem() {
    // Liberar memória
    delete fat;
    delete rootDir;
    delete dataArea;
    // Fechar o disco
    if (disk) {
        fclose(disk);
    }
}

bool FileSystem::format(uint32_t totalSectors, uint16_t rootEntryCount, uint8_t sectorsPerCluster) {
    // 1. Formatar o Boot Record
    bootRecord.format(totalSectors, rootEntryCount, sectorsPerCluster);
    bootRecord.saveToDisk(disk);

    // 2. Calcular o número de clusters
    uint32_t rootDirSectors = (rootEntryCount * 32 + 511) / 512; // 512 é o valor padrão de bytes por setor
    uint32_t reservedSectors = 1; // Boot Record
    uint32_t dataSectors = totalSectors - reservedSectors - rootDirSectors;
    uint32_t clusterCount = dataSectors / sectorsPerCluster;

    // 3. Inicializar a FAT
    delete fat; // Liberar memória, se já existir
    fat = new FATManager(clusterCount);
    fat->initialize();
    uint32_t fatOffset = 512; // Após o Boot Record (setor 1)
    fat->saveToDisk(disk, fatOffset);

    // 4. Inicializar o Root Directory
    delete rootDir;
    rootDir = new RootDirectoryManager(rootEntryCount);
    rootDir->initialize();
    uint32_t rootDirOffset = (reservedSectors + (bootRecord.getBootRecord().numberOfFATs * bootRecord.getBootRecord().sectorsPerFAT)) * 512;
    rootDir->saveToDisk(disk, rootDirOffset);

    // 5. Inicializar a Área de Dados
    delete dataArea;
    uint32_t clusterSize = 512 * sectorsPerCluster; // 512 bytes por setor
    dataArea = new DataAreaManager(clusterSize, clusterCount);
    uint32_t dataAreaOffset = rootDirOffset + (rootEntryCount * 32);
    dataArea->saveToDisk(disk, dataAreaOffset);

    return true;
}

bool FileSystem::copyToSystem(const std::string& sourcePath, const std::string& destFileName) {
    // 1. Abrir o arquivo de origem
    std::ifstream inFile(sourcePath, std::ios::binary);
    if (!inFile) {
        std::cerr << "Erro ao abrir o arquivo de origem: " << sourcePath << std::endl;
        return false;
    }

    // 2. Obter o tamanho do arquivo
    inFile.seekg(0, std::ios::end);
    uint32_t fileSize = inFile.tellg();
    inFile.seekg(0, std::ios::beg);

    // 3. Ler o conteúdo do arquivo
    std::vector<char> fileData(fileSize);
    inFile.read(fileData.data(), fileSize);
    inFile.close();

    // 4. Calcular o número de clusters necessários
    uint32_t clusterSize = dataArea->getClusterSize();
    uint32_t clustersNeeded = (fileSize + clusterSize - 1) / clusterSize;

    // 5. Alocar clusters na FAT
    auto clusters = fat->allocateClusters(clustersNeeded);
    if (clusters.empty()) {
        std::cerr << "Sem espaço para alocar clusters!" << std::endl;
        return false;
    }

    // 6. Escrever os dados na Área de Dados
    for (size_t i = 0; i < clusters.size(); ++i) {
        uint32_t sizeToWrite = (i == clusters.size() - 1) ? (fileSize % clusterSize ? fileSize % clusterSize : clusterSize) : clusterSize;
        dataArea->writeData(clusters[i], fileData.data() + i * clusterSize, sizeToWrite);
    }

    // 7. Adicionar entrada no Root Directory
    if (!rootDir->addFile(destFileName, fileSize, clusters[0])) {
        std::cerr << "Sem espaço no Root Directory!" << std::endl;
        fat->freeClusters(clusters[0]); // Liberar clusters alocados
        return false;
    }

    // 8. Salvar as alterações no disco
    uint32_t fatOffset = 512;
    fat->saveToDisk(disk, fatOffset);
    uint32_t rootDirOffset = (1 + (bootRecord.getBootRecord().numberOfFATs * bootRecord.getBootRecord().sectorsPerFAT)) * 512;
    rootDir->saveToDisk(disk, rootDirOffset);
    uint32_t dataAreaOffset = rootDirOffset + (bootRecord.getBootRecord().rootEntryCount * 32);
    dataArea->saveToDisk(disk, dataAreaOffset);

    return true;
}

bool FileSystem::copyFromSystem(const std::string& fileName, const std::string& destPath) {
    // 1. Encontrar o arquivo no Root Directory
    RootEntry* entry = rootDir->findFile(fileName);
    if (!entry) {
        std::cerr << "Arquivo não encontrado: " << fileName << std::endl;
        return false;
    }

    // 2. Abrir o arquivo de destino
    std::ofstream outFile(destPath, std::ios::binary);
    if (!outFile) {
        std::cerr << "Erro ao criar o arquivo de destino: " << destPath << std::endl;
        return false;
    }

    // 3. Ler os dados da Área de Dados
    std::vector<char> buffer(entry->fileSize);
    uint16_t cluster = entry->startCluster;
    size_t bytesRead = 0;
    while (cluster != CLUSTER_EOF) {
        uint32_t sizeToRead = std::min<uint32_t>(dataArea->getClusterSize(), entry->fileSize - bytesRead);
        dataArea->readData(cluster, buffer.data() + bytesRead, sizeToRead);
        bytesRead += sizeToRead;
        cluster = fat->getNextCluster(cluster);
    }

    // 4. Escrever os dados no arquivo de destino
    outFile.write(buffer.data(), entry->fileSize);
    outFile.close();

    return true;
}

void FileSystem::listFiles() const {
    rootDir->listFiles();
}

bool FileSystem::removeFile(const std::string& fileName) {
    // 1. Encontrar o arquivo no Root Directory
    RootEntry* entry = rootDir->findFile(fileName);
    if (!entry) {
        std::cerr << "Arquivo não encontrado: " << fileName << std::endl;
        return false;
    }

    // 2. Liberar os clusters na FAT
    fat->freeClusters(entry->startCluster);

    // 3. Remover a entrada do Root Directory
    rootDir->removeFile(fileName);

    // 4. Salvar as alterações no disco
    uint32_t fatOffset = 512;
    fat->saveToDisk(disk, fatOffset);
    uint32_t rootDirOffset = (1 + (bootRecord.getBootRecord().numberOfFATs * bootRecord.getBootRecord().sectorsPerFAT)) * 512;
    rootDir->saveToDisk(disk, rootDirOffset);

    return true;
}