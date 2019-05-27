# Nasza gra

```bash
mkdir build
cmake ..
make
./sdldemoapp
```

```bash
make
./sdldemoapp
```


## Na windows

W GitBash

```bash
export PATH=$PATH:/c/MinGW/bin/:/c/MinGW/mingw32/bin/
```

Ściągnij bibliotekę SDL2 w wersji developerskiej dla MinGW [z tej strony](https://www.libsdl.org/download-2.0.php)

Skopiuj ją do katalogu w którym masz grę.

W GitBash w katalogu w którym jest nasza gra wykonaj:

```bash
tar -xvf *.tar.bz2
```

Teraz przenieś z tego katalogu wszystko co jest w i686-w64-mingw32 do katalogu z grą (będzie tam katalog bin, include, lib oraz share)

Kompilacja w GitBash:

```bash
g++ -std=c++17 -I include -L lib -lSDL2 src/main.cpp -o gra
```
