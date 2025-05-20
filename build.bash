#!/bin/bash

# Configurare
EXECUTABLE="DinoGame"
ICON_FILE="resources/icon.png"
DESKTOP_FILE="DinoGame.desktop"
INSTALL_DIR="$HOME/.local/share/applications"
BUILD_DIR="build"

# 1. Compilează Raylib (dacă nu e deja construit)
echo "Compilare Raylib..."
cd raylib/src
make PLATFORM=PLATFORM_DESKTOP  # Linux (X11) implicit
cd ../..

# 2. Compilează jocul (link-uit la biblioteca Raylib)
echo "Compilare joc..."
mkdir -p $BUILD_DIR

SRC_FILES="main.c src/game.c src/window.c src/menu.c src/draw.c src/utils.c src/sound.c"
INCLUDE_DIRS="-I. -I./src -I./raylib/src"
LIBS="./raylib/src/libraylib.a -lm -lpthread -ldl -lrt -lX11"

# Compilează toate fișierele sursă
cc $SRC_FILES $INCLUDE_DIRS $LIBS -o $BUILD_DIR/$EXECUTABLE

# Verifică erori
if [ $? -ne 0 ]; then
    echo "Eroare la compilare!"
    exit 1
fi

# 3. Creează fișier .desktop
echo "Creare $DESKTOP_FILE..."
cat > $BUILD_DIR/$DESKTOP_FILE <<EOL
[Desktop Entry]
Name=DinoGame
Exec=$(pwd)/$BUILD_DIR/$EXECUTABLE
Icon=$(pwd)/$ICON_FILE
Type=Application
Categories=Game;
Terminal=false
EOL

chmod +x $BUILD_DIR/$DESKTOP_FILE

# 4. Instalare
echo "Instalare în $INSTALL_DIR..."
mkdir -p $INSTALL_DIR
cp $BUILD_DIR/$DESKTOP_FILE $INSTALL_DIR/

echo "Gata! Jocul apare în meniul de aplicații."