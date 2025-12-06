# Projekt Girlprojekt - Wyświetlanie GIF-ów i tekstu na panelach LED

Ten projekt pozwala na wyświetlanie GIF-ów i tekstu na panelach LED Raspberry Pi, sterowane przez interfejs przeglądarki z serwerem HTTP w Pythonie (Flask).

## Wymagania

- Raspberry Pi z zainstalowanym rpi-rgb-led-matrix
- GraphicsMagick: `sudo apt-get install libgraphicsmagick++-dev libwebp-dev` (na RPi) lub `brew install graphicsmagick` (na macOS)
- Python 3 z Flask: `pip install flask`
- Kompilator C++ z obsługą C++11

## Kompilacja

1. Zbuduj bibliotekę główną: `make -C ..`
2. W katalogu girlprojekt: `make`

## Uruchomienie

1. Uruchom aplikację C++: `sudo ./girlprojekt`
2. W osobnym terminalu uruchom serwer Flask: `python app.py`

Serwer HTTP działa na porcie 8080.

Otwórz przeglądarkę i przejdź do `http://<IP_RPI>:8080`

## Użycie

- Wpisz tekst i kliknij "Wyświetl tekst"
- Wybierz GIF z listy i kliknij "Wyświetl GIF"
- Prześlij nowy GIF przez formularz uploadu

Komunikacja między serwerem a aplikacją C++ odbywa się przez plik `/tmp/display_mode.txt`.

## Pliki

- `main.cc`: Główny kod aplikacji C++ (wyświetlanie)
- `app.py`: Serwer HTTP w Pythonie (Flask)
- `index.html`: Interfejs webowy
- `Makefile`: Skrypt kompilacji
- `gifs/`: Katalog na pliki GIF
