# Makefile para o jogo Coleta Seletiva
# Compilador e flags
CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++11 -O2
LIBS = -lglut -lGL -lGLU -lSDL2 -lSDL2_mixer

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
          TextRenderer.cpp \
          AudioManager.cpp

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
          TextRenderer.h \
          AudioManager.h

# Regra principal
all: $(TARGET)

# Cria diretório de assets se não existir
assets-dir:
	@mkdir -p assets/audio
	@if [ ! -f assets/audio/README.md ]; then \
		echo "Criando README de áudio..."; \
		echo "# Arquivos de Áudio" > assets/audio/README.md; \
		echo "Coloque aqui os arquivos de música (.ogg) e efeitos sonoros (.wav)" >> assets/audio/README.md; \
	fi

# Compilação do executável
$(TARGET): assets-dir $(OBJECTS)
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
		sudo apt-get update && sudo apt-get install -y build-essential freeglut3-dev libglu1-mesa-dev libgl1-mesa-dev libsdl2-dev libsdl2-mixer-dev; \
	elif command -v yum >/dev/null 2>&1; then \
		echo "Sistema Red Hat/CentOS detectado"; \
		echo "Instalando dependências..."; \
		sudo yum install -y gcc-c++ freeglut-devel mesa-libGL-devel mesa-libGLU-devel SDL2-devel SDL2_mixer-devel; \
	elif command -v dnf >/dev/null 2>&1; then \
		echo "Sistema Fedora detectado"; \
		echo "Instalando dependências..."; \
		sudo dnf install -y gcc-c++ freeglut-devel mesa-libGL-devel mesa-libGLU-devel SDL2-devel SDL2_mixer-devel; \
	elif command -v pacman >/dev/null 2>&1; then \
		echo "Sistema Arch Linux detectado"; \
		echo "Instalando dependências..."; \
		sudo pacman -S --needed base-devel freeglut mesa sdl2 sdl2_mixer; \
	else \
		echo "Sistema não reconhecido automaticamente"; \
		echo "Consulte o README.md para instruções de instalação manual"; \
		echo ""; \
		echo "Dependências necessárias:"; \
		echo "- OpenGL/GLUT: freeglut3-dev (Ubuntu) | freeglut-devel (Fedora) | freeglut (Arch)"; \
		echo "- SDL2: libsdl2-dev (Ubuntu) | SDL2-devel (Fedora) | sdl2 (Arch)"; \
		echo "- SDL2_mixer: libsdl2-mixer-dev (Ubuntu) | SDL2_mixer-devel (Fedora) | sdl2_mixer (Arch)"; \
	fi

# Verificação de dependências
check-deps:
	@echo "=== Verificando Dependências ==="
	@echo -n "OpenGL/GLUT: "
	@if pkg-config --exists glut 2>/dev/null; then \
		echo "✓ Instalado"; \
	else \
		echo "✗ Não encontrado"; \
	fi
	@echo -n "SDL2: "
	@if pkg-config --exists sdl2 2>/dev/null; then \
		echo "✓ Instalado (versão: $$(pkg-config --modversion sdl2))"; \
	else \
		echo "✗ Não encontrado"; \
	fi
	@echo -n "SDL2_mixer: "
	@if pkg-config --exists SDL2_mixer 2>/dev/null; then \
		echo "✓ Instalado (versão: $$(pkg-config --modversion SDL2_mixer))"; \
	else \
		echo "✗ Não encontrado"; \
	fi
	@echo ""
	@echo "Se alguma dependência estiver faltando, execute: make install-deps"

# Configuração completa (instalar dependências + compilar)
setup: install-deps all
	@echo ""
	@echo "=== Configuração Completa ==="
	@echo "✓ Dependências instaladas"
	@echo "✓ Projeto compilado"
	@echo ""
	@echo "Para executar: make run"
	@echo "Para testar áudio: coloque arquivos em assets/audio/ e execute"

# Informações sobre o projeto
info:
	@echo "=== Informações do Projeto ==="
	@echo "Nome: Coleta Seletiva"
	@echo "Linguagem: C++ com OpenGL/GLUT + SDL2 para áudio"
	@echo ""
	@echo "=== Comandos disponíveis ==="
	@echo "make              - Compila o projeto"
	@echo "make run          - Compila e executa"
	@echo "make clean        - Remove arquivos temporários"
	@echo "make rebuild      - Limpa e recompila tudo"
	@echo "make install-deps - Instala dependências do sistema"
	@echo "make check-deps   - Verifica se dependências estão instaladas"
	@echo "make setup        - Instalação completa (deps + compilação)"
	@echo "make info         - Mostra esta informação"
	@echo ""
	@echo "• PS: O jogo funciona sem áudio se arquivos não estiverem presentes"

# Phony targets (alvos que não representam arquivos)
.PHONY: all clean run rebuild install-deps check-deps setup info assets-dir

