# Matrix Server - Serwer HTTP dla matrycy LED RGB

Serwer HTTP do kontrolowania matrycy LED RGB z kolejką FIFO. Obsługuje wyświetlanie przewijającego się tekstu i animowanych GIFów.

## Kompilacja

Najpierw zainstaluj wymagane biblioteki:
```bash
sudo apt-get update
sudo apt-get install libgraphicsmagick++-dev libwebp-dev
```

Następnie skompiluj:
```bash
make
```

## Uruchomienie

```bash
sudo ./matrix-server
```

Serwer nasłuchuje na porcie 8080.

## Użycie

### Wyświetlanie tekstu:
```bash
curl "http://raspberrypi:8080/text?msg=Hello World"
```

### Wyświetlanie GIFa:
```bash
curl "http://raspberrypi:8080/gif?file=/path/to/animation.gif"
```

## Konfiguracja matrycy

- Wymiary: 32 wiersze x 128 kolumn (2 panele 32x64)
- Mapping GPIO: adafruit-hat
- GPIO slowdown: 3

## Pliki

- `main.cpp`: Główna pętla i inicjalizacja
- `command_queue.hpp/cpp`: Kolejka komend
- `handlers.hpp/cpp`: Funkcje wyświetlania tekstu i GIFów
- `simple_http_server.hpp`: Prosty serwer HTTP
- `Makefile`: Skrypt kompilacji

## Wymagania

- Raspberry Pi z biblioteką RGBMatrix
- GraphicsMagick++ dla obsługi obrazów/GIFów
- Kompilator C++11