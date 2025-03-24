#include "RootDirectory.h"
#include <cstring>
#include <iostream>
#include <ctime>

// Construtor: inicializa o Root Directory com o número de entradas
RootDirectoryManager::RootDirectoryManager(uint16_t entryCount) {
    entries.resize(entryCount);
    initialize();
}

// Inicializa o Root Directory (todas as entradas vazias)
void RootDirectoryManager::initialize() {
    for (auto& entry : entries) {
        memset(&entry, 0, sizeof(RootEntry));
    }
}

// Adiciona um arquivo ao Root Directory
bool RootDirectoryManager::addFile(const std::string& fileName, uint32_t fileSize, uint16_t startCluster) {
    // Procurar uma entrada vazia
    for (auto& entry : entries) {
        if (entry.fileName[0] == 0) { // Entrada vazia
            // Preencher os campos
            strncpy(entry.fileName, fileName.c_str(), 16);
            entry.fileName[15] = '\0'; // Garantir terminação nula
            entry.fileSize = fileSize;
            entry.startCluster = startCluster;
            entry.attributes = 0x20; // Atributo de arquivo comum
            entry.creationTime = static_cast<uint32_t>(time(nullptr));
            entry.modificationTime = entry.creationTime;
            return true;
        }
    }
    return false; // Sem espaço no Root Directory
}

// Remove um arquivo do Root Directory
bool RootDirectoryManager::removeFile(const std::string& fileName) {
    for (auto& entry : entries) {
        if (strncmp(entry.fileName, fileName.c_str(), 16) == 0) {
            // Marcar a entrada como vazia
            memset(&entry, 0, sizeof(RootEntry));
            return true;
        }
    }
    return false; // Arquivo não encontrado
}

// Lista todos os arquivos no Root Directory
void RootDirectoryManager::listFiles() const {
    bool hasFiles = false;
    for (const auto& entry : entries) {
        if (entry.fileName[0] != 0) { // Entrada não vazia
            hasFiles = true;
            std::cout << "File: " << entry.fileName
                      << ", Size: " << entry.fileSize << " bytes"
                      << ", Start Cluster: " << entry.startCluster
                      << std::endl;
        }
    }
    if (!hasFiles) {
        std::cout << "Nenhum arquivo encontrado no sistema." << std::endl;
    }
}

// Encontra um arquivo pelo nome
RootEntry* RootDirectoryManager::findFile(const std::string& fileName) {
    for (auto& entry : entries) {
        if (strncmp(entry.fileName, fileName.c_str(), 16) == 0) {
            return &entry;
        }
    }
    return nullptr; // Arquivo não encontrado
}

// Salva o Root Directory no disco a partir de um offset
void RootDirectoryManager::saveToDisk(FILE* disk, uint32_t offset) {
    fseek(disk, offset, SEEK_SET);
    fwrite(entries.data(), sizeof(RootEntry), entries.size(), disk);
}

// Carrega o Root Directory do disco a partir de um offset
void RootDirectoryManager::loadFromDisk(FILE* disk, uint32_t offset) {
    fseek(disk, offset, SEEK_SET);
    fread(entries.data(), sizeof(RootEntry), entries.size(), disk);
}