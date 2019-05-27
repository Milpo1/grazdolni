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
export PATH=/c/MinGW/bin/:/c/MinGW/mingw32/bin/:$PATH
```

Ściągnij bibliotekę SDL2 w wersji developerskiej dla MinGW [z tej strony](https://www.libsdl.org/download-2.0.php)

Skopiuj ją do katalogu w którym masz grę.

W GitBash w katalogu w którym jest nasza gra wykonaj:

```bash
tar -xvf *.tar.gz
```

Teraz przenieś z tego katalogu wszystko co jest w i686-w64-mingw32 do katalogu z grą (będzie tam katalog bin, include, lib oraz share)

Kompilacja w GitBash:

```bash
g++ -std=c++17 -I include -L lib src/main.cpp -o gra -lSDL2
```
