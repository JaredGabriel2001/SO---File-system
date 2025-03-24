# Variáveis
CXX = g++
TARGET = filesystem
SOURCES = Main.cpp BootRecord.cpp FAT.cpp RootDirectory.cpp DataArea.cpp FileSystem.cpp

# Regra padrão
$(TARGET): $(SOURCES)
	$(CXX) -o $(TARGET) $(SOURCES)

# Limpar arquivos gerados
clean:
	rm -f $(TARGET)
