#ifndef DATA_AREA_H
#define DATA_AREA_H

#include <cstdint>
#include <vector>
#include <cstdio>

class DataAreaManager {
public:
    // Construtor: inicializa a Área de Dados com o tamanho do cluster e o número de clusters
    DataAreaManager(uint32_t clusterSize, uint32_t clusterCount);

    // Escreve dados em um cluster específico
    void writeData(uint16_t cluster, const char* data, uint32_t size);

    // Lê dados de um cluster específico
    void readData(uint16_t cluster, char* buffer, uint32_t size) const;

    // Obtém o tamanho de um cluster
    uint32_t getClusterSize() const;

    // Obtém o número total de clusters
    uint32_t getClusterCount() const;

    // Salva a Área de Dados no disco a partir de um offset
    void saveToDisk(FILE* disk, uint32_t offset);

    // Carrega a Área de Dados do disco a partir de um offset
    void loadFromDisk(FILE* disk, uint32_t offset);

private:
    std::vector<char> dataArea;  // Vetor que representa a Área de Dados
    uint32_t clusterSize;        // Tamanho de um cluster em bytes
    uint32_t clusterCount;       // Número total de clusters
};

#endif // DATA_AREA_H