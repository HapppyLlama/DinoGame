#!/bin/bash

# Variabile
EXECUTABLE="mmm"
ICON_FILE="resources/icon.png"
DESKTOP_FILE="jocul_meu.desktop"
INSTALL_DIR="$HOME/.local/share/applications"

# 1. Compilează jocul
echo "Compilare joc..."
gcc -o $EXECUTABLE main.c -Isrc -Lsrc -l:libraylib.a -lGL -lm -lpthread -ldl -lrt -lX11

# Verifică dacă compilarea a reușit
if [ $? -ne 0 ]; then
    echo "Eroare la compilare!"
    exit 1
fi

# 2. Creează fișierul .desktop
echo "Creare fișier .desktop..."
cat > $DESKTOP_FILE <<EOL
[Desktop Entry]
Name=Jocul Meu
Exec=$(pwd)/$EXECUTABLE
Icon=$(pwd)/$ICON_FILE
Type=Application
Categories=Game;
EOL

# 3. Dă permisiuni de executare fișierului .desktop
chmod +x $DESKTOP_FILE

# 4. Copiază fișierul .desktop în locația corectă
echo "Instalare fișier .desktop în $INSTALL_DIR..."
mkdir -p $INSTALL_DIR
cp $DESKTOP_FILE $INSTALL_DIR/

# 5. Verifică dacă totul a funcționat
if [ $? -eq 0 ]; then
    echo "Jocul a fost compilat și instalat cu succes!"
    echo "Poți găsi jocul în meniul de aplicații."
else
    echo "Eroare la instalare!"
    exit 1
fi
