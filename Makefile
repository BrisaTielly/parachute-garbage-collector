# Makefile para o jogo Coleta Seletiva
# Compilador e flags
CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++11 -O2
LIBS = -lglut -lGL -lGLU

# Nome do executável
TARGET = coleta_seletiva

# Arquivos fonte
SOURCES = main.cpp \
          GameObjects.cpp \
          GameState.cpp \
          GameLoop.cpp \
          InputHandler.cpp \
          RankingSystem.cpp \
          Scenery.cpp \
          Screens.cpp \
          TextRenderer.cpp

# Arquivos objeto (gerados automaticamente a partir dos .cpp)
OBJECTS = $(SOURCES:.cpp=.o)

# Arquivos de cabeçalho
HEADERS = GameConstants.h \
          GameObjects.h \
          GameState.h \
          GameLoop.h \
          InputHandler.h \
          RankingSystem.h \
          Scenery.h \
          Screens.h \
          TextRenderer.h

# Regra principal
all: $(TARGET)

# Compilação do executável
$(TARGET): $(OBJECTS)
	@echo "Linkando executável $(TARGET)..."
	$(CXX) $(OBJECTS) -o $(TARGET) $(LIBS)
	@echo "Compilação concluída com sucesso!"

# Regra genérica para compilar arquivos .cpp em .o
%.o: %.cpp $(HEADERS)
	@echo "Compilando $<..."
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Limpeza dos arquivos objeto e executável
clean:
	@echo "Removendo arquivos temporários..."
	rm -f $(OBJECTS) $(TARGET)
	@echo "Limpeza concluída!"

# Compilação e execução
run: $(TARGET)
	@echo "Executando $(TARGET)..."
	./$(TARGET)

# Força recompilação completa
rebuild: clean all

# Instalação das dependências (multi-sistema)
install-deps:
	@echo "Detectando sistema operacional..."
	@if command -v apt-get >/dev/null 2>&1; then \
		echo "Sistema Ubuntu/Debian detectado"; \
		echo "Instalando dependências..."; \
		sudo apt-get update && sudo apt-get install -y build-essential freeglut3-dev libglu1-mesa-dev libgl1-mesa-dev; \
	elif command -v yum >/dev/null 2>&1; then \
		echo "Sistema Red Hat/CentOS detectado"; \
		echo "Instalando dependências..."; \
		sudo yum install -y gcc-c++ freeglut-devel mesa-libGL-devel mesa-libGLU-devel; \
	elif command -v dnf >/dev/null 2>&1; then \
		echo "Sistema Fedora detectado"; \
		echo "Instalando dependências..."; \
		sudo dnf install -y gcc-c++ freeglut-devel mesa-libGL-devel mesa-libGLU-devel; \
	elif command -v pacman >/dev/null 2>&1; then \
		echo "Sistema Arch Linux detectado"; \
		echo "Instalando dependências..."; \
		sudo pacman -S --needed base-devel freeglut mesa; \
	else \
		echo "Sistema não reconhecido automaticamente"; \
		echo "Consulte o README.md para instruções de instalação manual"; \
	fi


# Informações sobre o projeto
info:
	@echo "=== Informações do Projeto ==="
	@echo "Nome: Coleta Seletiva"
	@echo "Linguagem: C++ com OpenGL/GLUT"
	@echo ""
	@echo "=== Comandos disponíveis ==="
	@echo "make          - Compila o projeto"
	@echo "make run      - Compila e executa"
	@echo "make clean    - Remove arquivos temporários"
	@echo "make rebuild  - Limpa e recompila tudo"
	@echo "make install-deps - Instala dependências"
	@echo "make info     - Mostra estas informações"

