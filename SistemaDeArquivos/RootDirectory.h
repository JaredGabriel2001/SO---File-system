#ifndef ROOT_DIRECTORY_H
#define ROOT_DIRECTORY_H

#include <cstdint>
#include <vector>
#include <string>
#include <cstdio>

// Estrutura de uma entrada no Root Directory (32 bytes)
struct RootEntry {
    char fileName[16];          // Nome do arquivo (16 bytes)
    uint32_t fileSize;          // Tamanho do arquivo em bytes (4 bytes)
    uint16_t startCluster;      // Primeiro cluster do arquivo (2 bytes)
    uint8_t attributes;         // Atributos do arquivo (1 byte)
    uint32_t creationTime;      // Data e hora de criação (4 bytes)
    uint32_t modificationTime;  // Data e hora de modificação (4 bytes)
};

class RootDirectoryManager {
public:
    // Construtor: inicializa o Root Directory com o número de entradas
    RootDirectoryManager(uint16_t entryCount);

    // Inicializa o Root Directory (todas as entradas vazias)
    void initialize();

    // Adiciona um arquivo ao Root Directory
    bool addFile(const std::string& fileName, uint32_t fileSize, uint16_t startCluster);

    // Remove um arquivo do Root Directory
    bool removeFile(const std::string& fileName);

    // Lista todos os arquivos no Root Directory
    void listFiles() const;

    // Encontra um arquivo pelo nome
    RootEntry* findFile(const std::string& fileName);

    // Salva o Root Directory no disco a partir de um offset
    void saveToDisk(FILE* disk, uint32_t offset);

    // Carrega o Root Directory do disco a partir de um offset
    void loadFromDisk(FILE* disk, uint32_t offset);

private:
    std::vector<RootEntry> entries;  // Vetor de entradas do Root Directory
};

#endif // ROOT_DIRECTORY_H