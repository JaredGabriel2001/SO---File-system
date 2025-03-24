#include "FileSystem.h"
#include <iostream>
#include <string>
#include <cstring>

using namespace std;

int main() {
    try {
        // 1. Definir parâmetros do sistema de arquivos
        uint32_t TOTAL_SECTORS;
        const uint16_t ROOT_ENTRY_COUNT = 16; // Número de entradas no Root Directory
        const uint8_t SECTORS_PER_CLUSTER = 1; // Setores por cluster
        const uint32_t BYTES_PER_SECTOR = 512; // Bytes por setor (definido no BootRecord)
        string DISK_PATH; // Nome do arquivo .img, será definido pelo usuário

        // Pedir ao usuário o nome do arquivo .img
        cout << "Digite o nome do arquivo .img (ex.: filesystem.img): ";
        getline(cin, DISK_PATH);

        // Validar entrada (opcional, para evitar entradas vazias)
        if (DISK_PATH.empty()) {
            cerr << "Nome do arquivo não pode ser vazio! Usando 'filesystem.img' como padrão." << endl;
            DISK_PATH = "filesystem.img";
        }

        // Pedir ao usuário o tamanho em setores
        cout << "Digite o tamanho da partição em setores (ex.: 1000): ";
        cin >> TOTAL_SECTORS;
        cin.ignore(); // Limpar o buffer do \n

        // Validar entrada
        if (TOTAL_SECTORS < 10) { // Valor mínimo para garantir espaço para as estruturas
            cerr << "Tamanho muito pequeno! Deve ser pelo menos 10 setores." << endl;
            return 1;
        }

        // 2. Criar o arquivo .img com o tamanho apropriado usando FILE*
        cout << "Criando arquivo " << DISK_PATH << "..." << endl;
        FILE* imgFile = fopen(DISK_PATH.c_str(), "wb");
        if (!imgFile) {
            cerr << "Erro ao criar o arquivo " << DISK_PATH << "!" << endl;
            return 1;
        }

        // Preencher o arquivo com zeros para reservar o espaço
        fseek(imgFile, TOTAL_SECTORS * BYTES_PER_SECTOR - 1, SEEK_SET);
        fputc(0, imgFile); // Escrever um byte no final para definir o tamanho
        fclose(imgFile);

        // 3. Criar o sistema de arquivos
        FileSystem fs(DISK_PATH);

        // 4. Formatar o sistema
        cout << "Formatando o sistema de arquivos..." << endl;
        if (fs.format(TOTAL_SECTORS, ROOT_ENTRY_COUNT, SECTORS_PER_CLUSTER)) {
            cout << "Formatação concluída com sucesso!" << endl;
        } else {
            cout << "Falha na formatação." << endl;
            return 1;
        }

        // 5. Menu interativo
        int choice;
        string sourcePath, destFileName, destPath, fileName;

        while (true) {
            cout << "\n=== Sistema de Arquivos FAT16 Simples ===" << endl;
            cout << "1. Copiar arquivo para o sistema" << endl;
            cout << "2. Copiar arquivo do sistema para o disco" << endl;
            cout << "3. Listar arquivos" << endl;
            cout << "4. Remover arquivo" << endl;
            cout << "5. Sair" << endl;
            cout << "Escolha uma opção (1-5): ";
            cin >> choice;
            cin.ignore(); // Limpar o buffer do \n

            if (choice == 5) {
                cout << "Saindo..." << endl;
                break;
            }

            switch (choice) {
                case 1: // Copiar arquivo para o sistema
                    cout << "Digite o caminho do arquivo de origem (ex.: test.txt): ";
                    getline(cin, sourcePath);
                    cout << "Digite o nome do arquivo no sistema (ex.: test.txt): ";
                    getline(cin, destFileName);
                    if (fs.copyToSystem(sourcePath, destFileName)) {
                        cout << "Arquivo copiado com sucesso!" << endl;
                    } else {
                        cout << "Falha ao copiar o arquivo." << endl;
                    }
                    break;

                case 2: // Copiar arquivo do sistema para o disco
                    cout << "Digite o nome do arquivo no sistema (ex.: test.txt): ";
                    getline(cin, fileName);
                    cout << "Digite o caminho de destino no disco (ex.: test_copy.txt): ";
                    getline(cin, destPath);
                    if (fs.copyFromSystem(fileName, destPath)) {
                        cout << "Arquivo copiado com sucesso!" << endl;
                    } else {
                        cout << "Falha ao copiar o arquivo." << endl;
                    }
                    break;

                case 3: // Listar arquivos
                    cout << "\nListando arquivos no sistema:" << endl;
                    fs.listFiles();
                    break;

                case 4: // Remover arquivo
                    cout << "Digite o nome do arquivo a remover (ex.: test.txt): ";
                    getline(cin, fileName);
                    if (fs.removeFile(fileName)) {
                        cout << "Arquivo removido com sucesso!" << endl;
                    } else {
                        cout << "Falha ao remover o arquivo." << endl;
                    }
                    break;

                default:
                    cout << "Opção inválida! Tente novamente." << endl;
            }
        }
    } catch (const exception& e) {
        cerr << "Erro: " << e.what() << endl;
        return 1;
    }

    return 0;
}