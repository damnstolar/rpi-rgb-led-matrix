// -*- mode: c++; c-basic-offset: 2; indent-tabs-mode: nil; -*-
// Projekt wyświetlania GIF-ów i tekstu na panelach LED, sterowany przez HTTP.
// Na podstawie przykładów z examples-api-use.

#include "led-matrix.h"
#include "graphics.h"

#include <Magick++.h>
#include <magick/image.h>


#include <atomic>
#include <string>
#include <vector>

#include <unistd.h>
#include <signal.h>
#include <fstream>

using namespace rgb_matrix;
using ImageVector = std::vector<Magick::Image>;

// Stan globalny
std::atomic<bool> interrupt_received(false);

// Funkcje pomocnicze
static void InterruptHandler(int signo) {
  interrupt_received = true;
}

// Ładowanie GIF-a, na podstawie image-example.cc
bool LoadGif(const char *filename, ImageVector *result) {
  try {
    readImages(result, filename);
  } catch (std::exception &e) {
    fprintf(stderr, "Błąd ładowania GIF-a: %s\n", e.what());
    return false;
  }
  if (result->empty()) {
    fprintf(stderr, "Brak obrazów w pliku.\n");
    return false;
  }
  return true;
}

// Wyświetlanie GIF-a
void DisplayGif(RGBMatrix *matrix, const ImageVector &images) {
  FrameCanvas *offscreen_canvas = matrix->CreateFrameCanvas();
  while (!interrupt_received) {
    for (const auto &image : images) {
      if (interrupt_received) break;
      // Skalowanie do rozmiaru matrix
      Magick::Image scaled = image;
      scaled.scale(Magick::Geometry(matrix->width(), matrix->height()));
      // Kopiowanie do canvas
      for (size_t y = 0; y < scaled.rows(); ++y) {
        for (size_t x = 0; x < scaled.columns(); ++x) {
          const Magick::Color &c = scaled.pixelColor(x, y);
          offscreen_canvas->SetPixel(x, y,
                                     ScaleQuantumToChar(c.redQuantum()),
                                     ScaleQuantumToChar(c.greenQuantum()),
                                     ScaleQuantumToChar(c.blueQuantum()));
        }
      }
      offscreen_canvas = matrix->SwapOnVSync(offscreen_canvas);
      usleep(image.animationDelay() * 10000); // Opóźnienie animacji
    }
  }
}

// Wyświetlanie tekstu, na podstawie text-example.cc
void DisplayText(RGBMatrix *matrix, const std::string &text, Font &font) {
  FrameCanvas *offscreen_canvas = matrix->CreateFrameCanvas();
  Color color(255, 255, 255);
  Color bg_color(0, 0, 0);
  int x = 0;
  int y = font.baseline();
  while (!interrupt_received) {
    offscreen_canvas->Fill(bg_color.r, bg_color.g, bg_color.b);
    DrawText(offscreen_canvas, font, x, y, color, &bg_color, text.c_str(), 0);
    offscreen_canvas = matrix->SwapOnVSync(offscreen_canvas);
    usleep(100000); // Odświeżanie co 100ms
  }
}


int main(int argc, char *argv[]) {
  Magick::InitializeMagick(*argv);

  RGBMatrix::Options defaults;
  defaults.hardware_mapping = "regular";
  defaults.rows = 32;
  defaults.chain_length = 1;
  defaults.parallel = 1;
  Canvas *canvas = RGBMatrix::CreateFromFlags(&argc, &argv, &defaults);
  if (canvas == NULL) return 1;

  signal(SIGTERM, InterruptHandler);
  signal(SIGINT, InterruptHandler);

  // Ładowanie fontu
  Font font;
  if (!font.LoadFont("../fonts/7x13.bdf")) {
    fprintf(stderr, "Nie można załadować fontu.\n");
    return 1;
  }

  // Główna pętla wyświetlania
  std::string current_mode = "text:Witaj w girlprojekt!";
  ImageVector current_gif;
  bool gif_loaded = false;

  while (!interrupt_received) {
    // Czytaj tryb z pliku
    std::ifstream mode_file("/tmp/display_mode.txt");
    if (mode_file.is_open()) {
      std::string line;
      if (std::getline(mode_file, line)) {
        current_mode = line;
        gif_loaded = false; // Wymuś przeładowanie GIF-a jeśli zmieniony
      }
      mode_file.close();
    }

    // Parsuj tryb
    size_t colon_pos = current_mode.find(':');
    if (colon_pos != std::string::npos) {
      std::string type = current_mode.substr(0, colon_pos);
      std::string value = current_mode.substr(colon_pos + 1);

      if (type == "text") {
        DisplayText((RGBMatrix*)canvas, value, font);
      } else if (type == "gif") {
        if (!gif_loaded || current_gif.empty()) {
          current_gif.clear();
          if (LoadGif(value.c_str(), &current_gif)) {
            gif_loaded = true;
          } else {
            // Jeśli błąd, wyświetl tekst błędu
            DisplayText((RGBMatrix*)canvas, "Blad GIF-a", font);
            usleep(2000000); // 2 sekundy
            continue;
          }
        }
        DisplayGif((RGBMatrix*)canvas, current_gif);
      }
    } else {
      // Domyślny tekst
      DisplayText((RGBMatrix*)canvas, "Witaj w girlprojekt!", font);
    }
  }
  canvas->Clear();
  delete canvas;
  return 0;
}