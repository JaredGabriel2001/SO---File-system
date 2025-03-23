#ifndef FILE_SYSTEM_H
#define FILE_SYSTEM_H

#include "BootRecord.h"
#include "FAT.h"
#include "RootDirectory.h"
#include "DataArea.h"
#include <string>

class FileSystem {
public:
    // Construtor: inicializa o sistema de arquivos com o caminho do disco
    FileSystem(const std::string& diskPath);

    // Destrutor: fecha o disco
    ~FileSystem();

    // Formata o sistema de arquivos
    bool format(uint32_t totalSectors, uint16_t rootEntryCount, uint8_t sectorsPerCluster);

    // Copia um arquivo do disco rígido para o sistema de arquivos
    bool copyToSystem(const std::string& sourcePath, const std::string& destFileName);

    // Copia um arquivo do sistema de arquivos para o disco rígido
    bool copyFromSystem(const std::string& fileName, const std::string& destPath);

    // Lista os arquivos armazenados no sistema de arquivos
    void listFiles() const;

    // Remove um arquivo do sistema de arquivos
    bool removeFile(const std::string& fileName);

private:
    FILE* disk;                    // Arquivo que simula o disco
    BootRecordManager bootRecord;  // Gerenciador do Boot Record
    FATManager* fat;               // Gerenciador da FAT
    RootDirectoryManager* rootDir; // Gerenciador do Root Directory
    DataAreaManager* dataArea;     // Gerenciador da Área de Dados
};

#endif // FILE_SYSTEM_H