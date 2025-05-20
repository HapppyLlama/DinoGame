# 🦖 DinoGame

**DinoGame** este o reinterpretare a clasicului joc offline cu dinozaur, cu scopul de a recrea experiența originală și de a adăuga funcționalități și elemente de design noi, într-o aplicație interactivă, cu animații și elemente vizuale proprii.

## 🎯 Obiective principale

- ✅ **Crearea personajului principal**  
  Dinozaurul are abilitatea de a sări pentru a evita obstacolele. Pe parcursul jocului, poziția sa orizontală rămâne fixă.

- ✅ **Implementarea solului**  
  Solul este reprezentat printr-un fișier `.png` cu coliziune, care se va deplasa ciclic pe orizontală pentru a crea iluzia de mișcare.

- ✅ **Generarea obstacolelor**  
  Obstacolele constau în cactuși și păsări, generate aleator cu un interval minim de timp între apariții, pentru a evita secvențele imposibile. Coliziunea cu obstacolele încheie jocul și redirecționează jucătorul către ecranul de start.

- ✅ **Crearea fundalului**  
  Fundal personalizat pentru o experiență vizuală plăcută.

- ✅ **Design pentru elementele jocului**  
  Elemente grafice pentru o estetică coerentă.

- ✅ **Execuția jocului ca aplicație**  
  Jocul rulează într-o fereastră grafică, nu prin terminal.

- ✅ **Animări**  
  Personajul, obstacolele și fundalul beneficiază de animații pentru a adăuga dinamism.

## ⭐ Funcționalități opționale

- ✅ **Boss Fight / Modul Campanie**  
  La atingerea unui scor prestabilit, jucătorul intră într-un boss fight. Bossul atacă prin aruncarea de meteoriți, iar lupta este împărțită în mai multe etape (faze). După fiecare etapă finalizată cu succes de către jucător, bossul primește damage. La finalul confruntării, odată cu înfrângerea bossului, se activează scena de sfârșit: personajul găsește o peșteră unde poate supraviețui.

- ✅ **Ciclu zi-noapte**  
  Alternanța zi-noapte afectează vizibilitatea în timpul nopții, crescând dificultatea.

  

## 🧰 Librării utilizate

- Raylib

## How to install:

Required tools:
```bash
sudo apt install build-essential git
```
Required libraries:

Ubuntu
```bash
sudo apt install libasound2-dev libx11-dev libxrandr-dev libxi-dev libgl1-mesa-dev libglu1-mesa-dev libxcursor-dev libxinerama-dev libwayland-dev libxkbcommon-dev
```
Fedora
```bash
sudo dnf install alsa-lib-devel mesa-libGL-devel libX11-devel libXrandr-devel libXi-devel libXcursor-devel libXinerama-devel libatomic
```
Arch Linux
```bash
sudo pacman -S alsa-lib mesa libx11 libxrandr libxi libxcursor libxinerama
```


Compile command:
```bash
./build.bash
```


Info: submodule

