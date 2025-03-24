#ifndef BOOT_RECORD_H
#define BOOT_RECORD_H

#include <cstdint>
#include <cstdio>

// Estrutura do Boot Record (12 bytes)
struct BootRecord {
    uint16_t bytesPerSector;    // Bytes por setor (2 bytes)
    uint8_t sectorsPerCluster;  // Setores por cluster (1 byte)
    uint8_t numberOfFATs;       // Número de FATs (1 byte)
    uint16_t rootEntryCount;    // Número de entradas no diretório raiz (2 bytes)
    uint16_t sectorsPerFAT;     // Setores por FAT (2 bytes)
    char volumeLabel[4];        // Rótulo do volume (4 bytes para totalizar 12 bytes)
};

class BootRecordManager {
public:
    // Construtor
    BootRecordManager();

    // Função para formatar o sistema de arquivos
    void format(uint32_t totalSectors, uint16_t rootEntryCount, uint8_t sectorsPerCluster);

    // Obter o Boot Record
    BootRecord getBootRecord() const;

    // Salvar o Boot Record no disco
    void saveToDisk(FILE* disk);

    // Carregar o Boot Record do disco
    void loadFromDisk(FILE* disk);

private:
    BootRecord bootRecord;
    static const uint16_t BYTES_PER_SECTOR_DEFAULT = 512; // Valor padrão para bytes por setor
};

#endif // BOOT_RECORD_H