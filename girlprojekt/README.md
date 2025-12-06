# Projekt Girlprojekt - Wyświetlanie GIF-ów i tekstu na panelach LED

Ten projekt pozwala na wyświetlanie GIF-ów i tekstu na panelach LED Raspberry Pi, sterowane przez interfejs przeglądarki poprzez serwer HTTP.

## Wymagania

- Raspberry Pi z zainstalowanym rpi-rgb-led-matrix
- GraphicsMagick: `sudo apt-get install libgraphicsmagick++-dev libwebp-dev` (na RPi) lub `brew install graphicsmagick` (na macOS)
- Kompilator C++ z obsługą C++11

## Kompilacja

1. Zbuduj bibliotekę główną: `make -C ..`
2. W katalogu girlprojekt: `make`

## Uruchomienie

`sudo ./girlprojekt`

Serwer HTTP działa na porcie 8080.

Otwórz przeglądarkę i przejdź do `http://<IP_RPI>:8080/index.html`

## Użycie

- Wpisz tekst i kliknij "Wyświetl tekst"
- Wpisz ścieżkę do pliku GIF i kliknij "Wyświetl GIF"

## Pliki

- `main.cc`: Główny kod aplikacji
- `index.html`: Interfejs webowy
- `Makefile`: Skrypt kompilacji
- `httplib.h`: Biblioteka HTTP (header-only)