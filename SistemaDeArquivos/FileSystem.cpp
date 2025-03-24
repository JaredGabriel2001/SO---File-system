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

    if (disk) {
        fclose(disk);
    }
}

bool FileSystem::format(uint32_t totalSectors, uint16_t rootEntryCount, uint8_t sectorsPerCluster) {
    // Formatar o Boot Record
    bootRecord.format(totalSectors, rootEntryCount, sectorsPerCluster);
    bootRecord.saveToDisk(disk);

    // Calcular o número de clusters
    uint32_t rootDirSectors = (rootEntryCount * 32 + 511) / 512; // Cada entrada do Root Directory ocupa 32 bytes, e o resultado é arredondado para o número de setores (dividindo por 512 bytes por setor)
    uint32_t reservedSectors = 1; // Boot Record
    uint32_t dataSectors = totalSectors - reservedSectors - rootDirSectors; //Calcula o número de setores disponíveis para dados
    uint32_t clusterCount = dataSectors / sectorsPerCluster; //Calcula o número total de clusters

    // Inicializar a FAT
    delete fat; // Liberar memória, se já existir
    fat = new FATManager(clusterCount);
    fat->initialize();
    uint32_t fatOffset = 512; // Após o Boot Record (setor 1)
    fat->saveToDisk(disk, fatOffset);

    // Inicializar o Root Directory
    delete rootDir;
    rootDir = new RootDirectoryManager(rootEntryCount);
    rootDir->initialize();
    uint32_t rootDirOffset = (reservedSectors + (bootRecord.getBootRecord().numberOfFATs * bootRecord.getBootRecord().sectorsPerFAT)) * 512;
    rootDir->saveToDisk(disk, rootDirOffset);

    // Inicializar a Área de Dados
    delete dataArea;
    uint32_t clusterSize = 512 * sectorsPerCluster; // 512 bytes por setor
    dataArea = new DataAreaManager(clusterSize, clusterCount);
    uint32_t dataAreaOffset = rootDirOffset + (rootEntryCount * 32);
    dataArea->saveToDisk(disk, dataAreaOffset);

    return true;
}

//Cópia de um arquivo do disco rígido para o sistema de arquivos 
bool FileSystem::copyToSystem(const std::string& sourcePath, const std::string& destFileName) {
    // Abrir o arquivo de origem
    std::ifstream inFile(sourcePath, std::ios::binary);
    if (!inFile) {
        std::cerr << "Erro ao abrir o arquivo de origem: " << sourcePath << std::endl;
        return false;
    }

    // Obter o tamanho do arquivo
    inFile.seekg(0, std::ios::end);
    uint32_t fileSize = inFile.tellg();
    inFile.seekg(0, std::ios::beg);

    // Ler o conteúdo do arquivo
    std::vector<char> fileData(fileSize);
    inFile.read(fileData.data(), fileSize);
    inFile.close();

    // Calcular o número de clusters necessários
    uint32_t clusterSize = dataArea->getClusterSize();
    uint32_t clustersNeeded = (fileSize + clusterSize - 1) / clusterSize; //Divide o tamanho do arquivo pelo tamanho do cluster e arredonda para cima

    // Alocar clusters na FAT
    auto clusters = fat->allocateClusters(clustersNeeded);
    if (clusters.empty()) {
        std::cerr << "Sem espaço para alocar clusters!" << std::endl;
        return false;
    }

    // Escrever os dados na Área de Dados
    for (size_t i = 0; i < clusters.size(); ++i) {
        uint32_t sizeToWrite = (i == clusters.size() - 1) ? (fileSize % clusterSize ? fileSize % clusterSize : clusterSize) : clusterSize; //Para o último cluster, escreve apenas o restante do arquivo (fileSize % clusterSize) || Para os outros clusters, escreve o tamanho total do cluster (clusterSize).
        dataArea->writeData(clusters[i], fileData.data() + i * clusterSize, sizeToWrite);  
    }

    // Adicionar entrada no Root Directory
    if (!rootDir->addFile(destFileName, fileSize, clusters[0])) {
        std::cerr << "Sem espaço no Root Directory!" << std::endl;
        fat->freeClusters(clusters[0]); // Liberar clusters alocados
        return false;
    }

    // Salvar as alterações no disco
    uint32_t fatOffset = 512;
    fat->saveToDisk(disk, fatOffset);
    uint32_t rootDirOffset = (1 + (bootRecord.getBootRecord().numberOfFATs * bootRecord.getBootRecord().sectorsPerFAT)) * 512;
    rootDir->saveToDisk(disk, rootDirOffset);
    uint32_t dataAreaOffset = rootDirOffset + (bootRecord.getBootRecord().rootEntryCount * 32);
    dataArea->saveToDisk(disk, dataAreaOffset);

    return true;
}

//Cópia de um arquivo do sistema de arquivos para o disco rígido 
bool FileSystem::copyFromSystem(const std::string& fileName, const std::string& destPath) {
    // Encontrar o arquivo no Root Directory
    RootEntry* entry = rootDir->findFile(fileName);
    if (!entry) {
        std::cerr << "Arquivo não encontrado: " << fileName << std::endl;
        return false;
    }

    // Abrir o arquivo de destino
    std::ofstream outFile(destPath, std::ios::binary);
    if (!outFile) {
        std::cerr << "Erro ao criar o arquivo de destino: " << destPath << std::endl;
        return false;
    }

    // Ler os dados da Área de Dados
    std::vector<char> buffer(entry->fileSize);
    uint16_t cluster = entry->startCluster;
    size_t bytesRead = 0;
    while (cluster != CLUSTER_EOF) {
        uint32_t sizeToRead = std::min<uint32_t>(dataArea->getClusterSize(), entry->fileSize - bytesRead);
        dataArea->readData(cluster, buffer.data() + bytesRead, sizeToRead);
        bytesRead += sizeToRead;
        cluster = fat->getNextCluster(cluster);
    }

    // Escrever os dados no arquivo de destino
    outFile.write(buffer.data(), entry->fileSize);
    outFile.close();

    return true;
}

//Listagem dos arquivos armazenados no sistema de arquivos 
void FileSystem::listFiles() const {
    rootDir->listFiles();
}


//Remoção de arquivos 
bool FileSystem::removeFile(const std::string& fileName) {
    // Encontrar o arquivo no Root Directory
    RootEntry* entry = rootDir->findFile(fileName);
    if (!entry) {
        std::cerr << "Arquivo não encontrado: " << fileName << std::endl;
        return false;
    }

    // Liberar os clusters na FAT
    fat->freeClusters(entry->startCluster);

    // Remover a entrada do Root Directory
    rootDir->removeFile(fileName);

    // Salvar as alterações no disco
    uint32_t fatOffset = 512;
    fat->saveToDisk(disk, fatOffset);
    uint32_t rootDirOffset = (1 + (bootRecord.getBootRecord().numberOfFATs * bootRecord.getBootRecord().sectorsPerFAT)) * 512;
    rootDir->saveToDisk(disk, rootDirOffset);

    return true;
}