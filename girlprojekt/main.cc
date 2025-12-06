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
std::atomic<bool> center_gif(true);
std::atomic<int> brightness(100);

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

// Wyświetlanie jednej klatki GIF-a
void DisplayGifFrame(RGBMatrix *matrix, const ImageVector &images, int &frame_index, FrameCanvas *offscreen_canvas) {
  if (images.empty()) return;
  const auto &image = images[frame_index % images.size()];
  // Skalowanie do rozmiaru matrix
  Magick::Image scaled = image;
  scaled.scale(Magick::Geometry(matrix->width(), matrix->height()));
  // Wyczyść canvas
  offscreen_canvas->Fill(0, 0, 0);
  // Obliczenie offsetu dla centrowania
  int offset_x = center_gif ? (matrix->width() - scaled.columns()) / 2 : 0;
  int offset_y = center_gif ? (matrix->height() - scaled.rows()) / 2 : 0;
  // Kopiowanie do canvas
  for (size_t y = 0; y < scaled.rows(); ++y) {
    for (size_t x = 0; x < scaled.columns(); ++x) {
      const Magick::Color &c = scaled.pixelColor(x, y);
      if (c.alphaQuantum() < 256) {  // Sprawdź kanał alpha
        offscreen_canvas->SetPixel(x + offset_x, y + offset_y,
                                   ScaleQuantumToChar(c.redQuantum()),
                                   ScaleQuantumToChar(c.greenQuantum()),
                                   ScaleQuantumToChar(c.blueQuantum()));
      }
    }
  }
  offscreen_canvas = matrix->SwapOnVSync(offscreen_canvas);
  frame_index++;
}

// Wyświetlanie tekstu, na podstawie text-example.cc
void DisplayTextFrame(RGBMatrix *matrix, const std::string &text, Font &font, FrameCanvas *offscreen_canvas) {
  Color color(255, 255, 255);
  Color bg_color(0, 0, 0);
  int x = 0;
  int y = font.baseline();
  offscreen_canvas->Fill(bg_color.r, bg_color.g, bg_color.b);
  DrawText(offscreen_canvas, font, x, y, color, &bg_color, text.c_str(), 0);
  offscreen_canvas = matrix->SwapOnVSync(offscreen_canvas);
}


int main(int argc, char *argv[]) {
  Magick::InitializeMagick(*argv);

  RGBMatrix::Options defaults;
  defaults.hardware_mapping = "adafruit-hat";
  defaults.rows = 32;
  defaults.cols = 128;
  defaults.chain_length = 1;
  defaults.parallel = 1;
  defaults.gpio_slowdown = 3;
  rgb_matrix::RuntimeOptions runtime_opt;
  if (!rgb_matrix::ParseOptionsFromFlags(&argc, &argv, &defaults, &runtime_opt)) {
    return 1;
  }
  Canvas *canvas = RGBMatrix::CreateFromOptions(defaults, runtime_opt);
  if (canvas == NULL) return 1;
  ((RGBMatrix*)canvas)->SetBrightness(brightness);

  signal(SIGTERM, InterruptHandler);
  signal(SIGINT, InterruptHandler);

  // Ładowanie fontu
  Font font;
  if (!font.LoadFont("../fonts/7x13.bdf")) {
    fprintf(stderr, "Nie można załadować fontu.\n");
    return 1;
  }

  // Główna pętla wyświetlania
  FrameCanvas *offscreen_canvas = ((RGBMatrix*)canvas)->CreateFrameCanvas();
  std::string current_mode = "text:Witaj w girlprojekt!";
  ImageVector current_gif;
  bool gif_loaded = false;
  int gif_frame_index = 0;

  while (!interrupt_received) {
    // Czytaj tryb z pliku
    std::ifstream mode_file("/tmp/display_mode.txt");
    if (mode_file.is_open()) {
      std::string line;
      if (std::getline(mode_file, line)) {
        if (line != current_mode) {
          current_mode = line;
          gif_loaded = false; // Wymuś przeładowanie GIF-a jeśli zmieniony
          gif_frame_index = 0;
        }
      }
      mode_file.close();
    }

    // Parsuj tryb
    size_t colon_pos = current_mode.find(':');
    if (colon_pos != std::string::npos) {
      std::string type = current_mode.substr(0, colon_pos);
      std::string value = current_mode.substr(colon_pos + 1);

      if (type == "center") {
        center_gif = (value == "true");
      } else if (type == "brightness") {
        brightness = std::stoi(value);
        ((RGBMatrix*)canvas)->SetBrightness(brightness);
      } else if (type == "text") {
        DisplayTextFrame((RGBMatrix*)canvas, value, font, offscreen_canvas);
      } else if (type == "gif") {
        if (!gif_loaded) {
          current_gif.clear();
          if (LoadGif(value.c_str(), &current_gif)) {
            gif_loaded = true;
          } else {
            // Jeśli błąd, wyświetl tekst błędu
            DisplayTextFrame((RGBMatrix*)canvas, "Blad GIF-a", font, offscreen_canvas);
            usleep(2000000); // 2 sekundy
            continue;
          }
        }
        DisplayGifFrame((RGBMatrix*)canvas, current_gif, gif_frame_index, offscreen_canvas);
        // Opóźnienie animacji
        if (!current_gif.empty()) {
          int delay = current_gif[gif_frame_index % current_gif.size()].animationDelay();
          usleep(delay * 10000);
        }
      }
    } else {
      // Domyślny tekst
      DisplayTextFrame((RGBMatrix*)canvas, "Witaj w girlprojekt!", font, offscreen_canvas);
    }

    // Małe opóźnienie, aby nie obciążać CPU
    usleep(10000); // 10ms
  }
  canvas->Clear();
  delete canvas;
  return 0;
}