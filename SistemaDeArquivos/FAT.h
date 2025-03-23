#ifndef FAT_H
#define FAT_H

#include <cstdint>
#include <vector>
#include <cstdio>

// Marcadores da FAT
const uint16_t CLUSTER_FREE = 0x0000;  // Cluster livre
const uint16_t CLUSTER_EOF = 0xFFFF;   // Fim de arquivo
const uint16_t CLUSTER_BAD = 0xFFF7;   // Cluster defeituoso

class FATManager {
public:
    // Construtor: inicializa a FAT com o número de clusters
    FATManager(uint32_t clusterCount);

    // Inicializa a FAT (todos os clusters livres)
    void initialize();

    // Aloca um número de clusters para um arquivo
    std::vector<uint16_t> allocateClusters(uint32_t clusterCount);

    // Libera os clusters de um arquivo a partir do cluster inicial
    void freeClusters(uint16_t startCluster);

    // Obtém o próximo cluster na cadeia
    uint16_t getNextCluster(uint16_t cluster) const;

    // Define o próximo cluster na cadeia
    void setNextCluster(uint16_t cluster, uint16_t nextCluster);

    // Salva a FAT no disco a partir de um offset
    void saveToDisk(FILE* disk, uint32_t offset);

    // Carrega a FAT do disco a partir de um offset
    void loadFromDisk(FILE* disk, uint32_t offset);

    // Obtém o número total de clusters
    uint32_t getClusterCount() const;

private:
    std::vector<uint16_t> fatTable;  // Tabela FAT (vetor de entradas de 16 bits)
};

#endif // FAT_H