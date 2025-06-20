# Garbage Drop - Jogo de Reciclagem 

**Um jogo educativo em OpenGL/C++ sobre coleta seletiva de resíduos**

---

### Compilação e Execução
```bash
# Instalar dependências (Linux)
make install-deps

# Compilar
make

# Compilar e executar
make run

# Ver informações do projeto
make info
```

---

### Comandos Disponíveis
```bash
make              # Compilar o projeto
make run          # Compilar e executar
make clean        # Limpar arquivos temporários
make rebuild      # Limpar e recompilar
make install-deps # Instalar dependências
make info         # Informações do projeto
```

---

## Como Jogar

1. **Tela Inicial**: Digite seu nome e pressione "Iniciar" ou Enter  
2. **Jogo**: 
   - Use as setas ← → para mover a cesta  
   - Use as teclas 1-5 para mudar o tipo da cesta  
   - Colete o lixo correto na cesta da cor correspondente  
   - Pressione 'P' para pausar, ESC para sair  
3. **Tipos de Lixo**:
   - 🟦 **Azul** - Papel  
   - 🟥 **Vermelho** - Plástico  
   - 🟨 **Amarelo** - Metal  
   - 🟩 **Verde** - Vidro  
   - 🟫 **Marrom** - Orgânico  

---

## Requisitos do Sistema

### Dependências Obrigatórias
- **Compilador C++**: g++ ou clang++ com suporte a C++11  
- **OpenGL**: Bibliotecas de renderização gráfica  
- **GLUT**: Biblioteca para interface gráfica e entrada  
- **SDL2**: Sistema de áudio (música e efeitos sonoros)  

---

## Instalação por Sistema Operacional

### Ubuntu/Debian
```bash
sudo apt-get update
sudo apt-get install build-essential freeglut3-dev libglu1-mesa-dev libgl1-mesa-dev libsdl2-dev libsdl2-mixer-dev
```

### Red Hat/CentOS/Fedora
```bash
# Red Hat/CentOS
sudo yum install gcc-c++ freeglut-devel mesa-libGL-devel mesa-libGLU-devel SDL2-devel SDL2_mixer-devel

# Fedora
sudo dnf install gcc-c++ freeglut-devel mesa-libGL-devel mesa-libGLU-devel SDL2-devel SDL2_mixer-devel
```

### Windows (com MSYS2)

> Para rodar o jogo no Windows, é recomendado usar o ambiente MSYS2 com MinGW.

1. **Instale o [MSYS2](https://www.msys2.org/)**  
   Siga as instruções do site oficial para baixar e instalar.

2. **Abra o terminal `MSYS2 MinGW 64-bit`**

3. **Atualize os pacotes do MSYS2**:
   ```bash
   pacman -Syu
   # reinicie o terminal, depois:
   pacman -Su
   ```

4. **Instale as dependências**:
   ```bash
   pacman -S make mingw-w64-x86_64-gcc mingw-w64-x86_64-freeglut mingw-w64-x86_64-SDL2 mingw-w64-x86_64-SDL2_mixer
   ```

5. **Compile e execute o jogo**:
   ```bash
   make run
   ```

> Obs.: Use sempre o terminal `MSYS2 MinGW 64-bit` para compilar e rodar o jogo corretamente.

---

## Arquitetura do Projeto

```
src/
├── main.cpp              # Ponto de entrada do programa
├── AudioManager.h/cpp    # Sistema de áudio (música e efeitos)
├── GameConstants.h       # Constantes globais e configurações
├── GameState.h/cpp       # Gerenciamento do estado global
├── GameLoop.h/cpp        # Loop principal e callbacks OpenGL
├── InputHandler.h/cpp    # Processamento de entrada (teclado/mouse)
├── GameObjects.h/cpp     # Objetos do jogo (cesta, lixo)
├── Screens.h/cpp         # Telas (menu, jogo, pausa, game over)
├── TextRenderer.h/cpp    # Renderização de texto
├── Scenery.h/cpp         # Cenário urbano
├── RankingSystem.h/cpp   # Sistema de pontuação persistente
└── assets/audio/         # Arquivos de música e efeitos sonoros
```

---

### Módulos e Responsabilidades

#### Core (Núcleo)
- **main.cpp**: Inicialização e configuração da janela OpenGL  
- **AudioManager.h/cpp**: Sistema de áudio com SDL2/SDL_mixer  
- **GameConstants.h**: Enums, constantes e tabelas de cores  
- **GameState.h/cpp**: Estado global do jogo e variáveis compartilhadas  

#### Jogo (Mecânicas)
- **GameLoop.h/cpp**: Callbacks principais (display, update, reshape)  
- **InputHandler.h/cpp**: Processamento de input do usuário  
- **GameObjects.h/cpp**: Lógica de objetos (cesta, lixo, física)  

#### Interface
- **Screens.h/cpp**: Renderização de todas as telas  
- **TextRenderer.h/cpp**: Sistema de renderização de texto  

#### World (Mundo)
- **Scenery.h/cpp**: Geração e renderização do cenário urbano  
- **RankingSystem.h/cpp**: Persistência de pontuações  

---

## Sistema de Áudio

O jogo inclui trilha sonora e efeitos sonoros usando SDL2:
- **Música de fundo**: Menu, gameplay e game over  
- **Efeitos sonoros**: Feedback para coletas e cliques  
- **Funcionamento**: O jogo funciona normalmente mesmo sem arquivos de áudio
