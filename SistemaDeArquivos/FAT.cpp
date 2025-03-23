#include "FAT.h"
using namespace std;

// Construtor: inicializa a FAT com o número de clusters
FATManager::FATManager(uint32_t clusterCount) {
    fatTable.resize(clusterCount, CLUSTER_FREE);
}

// Inicializa a FAT (todos os clusters livres)
void FATManager::initialize() {
    for (auto& entry : fatTable) {
        entry = CLUSTER_FREE;
    }
}

// Aloca um número de clusters para um arquivo
vector<uint16_t> FATManager::allocateClusters(uint32_t clusterCount) {
    vector<uint16_t> allocatedClusters;

    // Procurar clusters livres
    for (uint32_t i = 0; i < fatTable.size() && allocatedClusters.size() < clusterCount; ++i) {
        if (fatTable[i] == CLUSTER_FREE) {
            allocatedClusters.push_back(i);
        }
    }

    // Verificar se há clusters suficientes
    if (allocatedClusters.size() < clusterCount) {
        allocatedClusters.clear(); // Não há espaço suficiente
        return allocatedClusters;
    }

    // Criar a cadeia de clusters
    for (size_t i = 0; i < allocatedClusters.size() - 1; ++i) {
        fatTable[allocatedClusters[i]] = allocatedClusters[i + 1];
    }
    // Último cluster da cadeia recebe o marcador de fim de arquivo
    if (!allocatedClusters.empty()) {
        fatTable[allocatedClusters.back()] = CLUSTER_EOF;
    }

    return allocatedClusters;
}

// Libera os clusters de um arquivo a partir do cluster inicial
void FATManager::freeClusters(uint16_t startCluster) {
    uint16_t currentCluster = startCluster;
    while (currentCluster != CLUSTER_EOF && currentCluster < fatTable.size()) {
        uint16_t nextCluster = fatTable[currentCluster];
        fatTable[currentCluster] = CLUSTER_FREE;
        currentCluster = nextCluster;
    }
}

// Obtém o próximo cluster na cadeia
uint16_t FATManager::getNextCluster(uint16_t cluster) const {
    if (cluster >= fatTable.size()) {
        return CLUSTER_EOF; // Cluster inválido
    }
    return fatTable[cluster];
}

// Define o próximo cluster na cadeia
void FATManager::setNextCluster(uint16_t cluster, uint16_t nextCluster) {
    if (cluster < fatTable.size()) {
        fatTable[cluster] = nextCluster;
    }
}

// Salva a FAT no disco a partir de um offset
void FATManager::saveToDisk(FILE* disk, uint32_t offset) {
    fseek(disk, offset, SEEK_SET);
    fwrite(fatTable.data(), sizeof(uint16_t), fatTable.size(), disk);
}

// Carrega a FAT do disco a partir de um offset
void FATManager::loadFromDisk(FILE* disk, uint32_t offset) {
    fseek(disk, offset, SEEK_SET);
    fread(fatTable.data(), sizeof(uint16_t), fatTable.size(), disk);
}

// Obtém o número total de clusters
uint32_t FATManager::getClusterCount() const {
    return fatTable.size();
}