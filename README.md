# 🦖 DinoGame

**DinoGame** este o reinterpretare a clasicului joc offline cu dinozaur, cu scopul de a recrea experiența originală și de a adăuga funcționalități și elemente de design noi, într-o aplicație interactivă, cu animații și elemente vizuale proprii.

## 🎯 Obiective principale

- ❌ **Crearea personajului principal**  
  Dinozaurul are abilitatea de a sări pentru a evita obstacolele. Pe parcursul jocului, poziția sa orizontală rămâne fixă.

- ❌ **Implementarea solului**  
  Solul este reprezentat printr-un fișier `.png` cu coliziune, care se va deplasa ciclic pe orizontală pentru a crea iluzia de mișcare.

- ❌ **Generarea obstacolelor**  
  Obstacolele constau în cactuși și păsări, generate aleator cu un interval minim de timp între apariții, pentru a evita secvențele imposibile. Coliziunea cu obstacolele încheie jocul și redirecționează jucătorul către ecranul de start.

- ❌ **Crearea fundalului**  
  Fundal personalizat pentru o experiență vizuală plăcută.

- ❌ **Design pentru elementele jocului**  
  Elemente grafice originale sau prelucrate pentru o estetică coerentă.

- ✅ **Execuția jocului ca aplicație**  
  Jocul rulează într-o fereastră grafică, nu prin terminal.

- ❌ **Animări fluide**  
  Personajul, obstacolele și fundalul beneficiază de animații pentru a adăuga dinamism.

## ⭐ Funcționalități opționale

- 🦕 **Boss Fight / Modul Campanie**  
  La atingerea unui scor prestabilit, jucătorul intră într-un boss fight. Bossul atacă prin aruncarea de meteoriți, iar lupta este împărțită în mai multe etape (faze). După fiecare etapă finalizată cu succes de către jucător, bossul primește damage. La finalul confruntării, odată cu înfrângerea bossului, se activează scena de sfârșit: personajul găsește o peșteră unde poate supraviețui.

- 🌙 **Ciclu zi-noapte**  
  Alternanța zi-noapte afectează vizibilitatea în timpul nopții, crescând dificultatea.

  

## 🧰 Librării utilizate

- Raylib

Info: submodule

