#include "DataArea.h"
#include <cstring>

// Construtor: inicializa a Área de Dados com o tamanho do cluster e o número de clusters
DataAreaManager::DataAreaManager(uint32_t clusterSize, uint32_t clusterCount) {
    this->clusterSize = clusterSize;
    this->clusterCount = clusterCount;
    dataArea.resize(clusterSize * clusterCount, 0); // Inicializa com zeros
}

// Escreve dados em um cluster específico
void DataAreaManager::writeData(uint16_t cluster, const char* data, uint32_t size) {
    if (cluster >= clusterCount) {
        return; // Cluster inválido
    }
    uint32_t offset = cluster * clusterSize;
    size = std::min(size, clusterSize); // Não escrever além do tamanho do cluster
    memcpy(dataArea.data() + offset, data, size);
}

// Lê dados de um cluster específico
void DataAreaManager::readData(uint16_t cluster, char* buffer, uint32_t size) const {
    if (cluster >= clusterCount) {
        memset(buffer, 0, size); // Cluster inválido, preenche com zeros
        return;
    }
    uint32_t offset = cluster * clusterSize;
    size = std::min(size, clusterSize); // Não ler além do tamanho do cluster
    memcpy(buffer, dataArea.data() + offset, size);
}

// Obtém o tamanho de um cluster
uint32_t DataAreaManager::getClusterSize() const {
    return clusterSize;
}

// Obtém o número total de clusters
uint32_t DataAreaManager::getClusterCount() const {
    return clusterCount;
}

// Salva a Área de Dados no disco a partir de um offset
void DataAreaManager::saveToDisk(FILE* disk, uint32_t offset) {
    fseek(disk, offset, SEEK_SET);
    fwrite(dataArea.data(), sizeof(char), dataArea.size(), disk);
}

// Carrega a Área de Dados do disco a partir de um offset
void DataAreaManager::loadFromDisk(FILE* disk, uint32_t offset) {
    fseek(disk, offset, SEEK_SET);
    fread(dataArea.data(), sizeof(char), dataArea.size(), disk);
}