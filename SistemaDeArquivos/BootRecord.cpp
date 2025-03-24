#include "BootRecord.h"
#include <cstring>

BootRecordManager::BootRecordManager() {
    // Inicializa com valores padrão
    bootRecord.bytesPerSector = BYTES_PER_SECTOR_DEFAULT;
    bootRecord.sectorsPerCluster = 1;
    bootRecord.numberOfFATs = 1;
    bootRecord.rootEntryCount = 0;
    bootRecord.sectorsPerFAT = 0;
    strncpy(bootRecord.volumeLabel, "FAT", 4);
}

// Função para formatar o sistema de arquivos
void BootRecordManager::format(uint32_t totalSectors, uint16_t rootEntryCount, uint8_t sectorsPerCluster) {
    // Configura os campos do Boot Record
    bootRecord.bytesPerSector = BYTES_PER_SECTOR_DEFAULT;
    bootRecord.sectorsPerCluster = sectorsPerCluster;
    bootRecord.numberOfFATs = 1; // Simplificação: apenas uma FAT
    bootRecord.rootEntryCount = rootEntryCount;

    // Calcular o número de setores ocupados pelo Root Directory
    uint32_t rootDirSectors = (rootEntryCount * 32 + BYTES_PER_SECTOR_DEFAULT - 1) / BYTES_PER_SECTOR_DEFAULT;

    // Calcular o número de setores disponíveis para a Área de Dados
    uint32_t reservedSectors = 1; // Boot Record ocupa 1 setor
    uint32_t dataSectors = totalSectors - reservedSectors - rootDirSectors;

    // Calcular o número de clusters
    uint32_t clusters = dataSectors / sectorsPerCluster;

    // Calcular o tamanho da FAT (cada entrada da FAT tem 2 bytes)
    uint32_t fatSizeBytes = clusters * 2; // 2 bytes por cluster
    bootRecord.sectorsPerFAT = (fatSizeBytes + BYTES_PER_SECTOR_DEFAULT - 1) / BYTES_PER_SECTOR_DEFAULT;

    // Definir o rótulo do volume
    strncpy(bootRecord.volumeLabel, "FAT", 4);
}

// Retorna o Boot Record
BootRecord BootRecordManager::getBootRecord() const {
    return bootRecord;
}

// Salva o Boot Record no disco (setor 0)
void BootRecordManager::saveToDisk(FILE* disk) {
    fseek(disk, 0, SEEK_SET); // Posiciona no início do disco
    fwrite(&bootRecord, sizeof(BootRecord), 1, disk);
}

// Carrega o Boot Record do disco (setor 0)
void BootRecordManager::loadFromDisk(FILE* disk) {
    fseek(disk, 0, SEEK_SET); // Posiciona no início do disco
    fread(&bootRecord, sizeof(BootRecord), 1, disk);
}