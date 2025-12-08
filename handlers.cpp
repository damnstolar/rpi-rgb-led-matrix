#include "handlers.hpp"
#include "graphics.h"
#include <GraphicsMagick/Magick++.h>
#include <magick/image.h>
#include <vector>
#include <unistd.h>
#include <algorithm>
#include <string>
#include <fstream>
#include <streambuf>
#include <sys/stat.h>
#include <time.h>
#include <math.h>
#include <stdint.h>
#include <atomic>

using namespace rgb_matrix;

// External interrupt flag
extern std::atomic<bool> interrupt;

// Simplified ScrollText based on scrolling-text-example.cc
void ShowText(RGBMatrix *matrix, const std::string &text) {
    Font font;
    if (!font.LoadFont("fonts/7x13.bdf")) {
        fprintf(stderr, "Couldn't load font\n");
        return;
    }

    Color color(255, 255, 0);
    Color bg_color(0, 0, 0);

    FrameCanvas *offscreen_canvas = matrix->CreateFrameCanvas();

    const int x_default_start = matrix->width() + 5;
    int x_orig = x_default_start;
    int y_orig = 0;
    int letter_spacing = 0;
    float speed = 7.0f;
    int loops = -1;  // endless

    int x = x_orig;
    int y = y_orig;
    int length = 0;

    int delay_speed_usec = 1000000;
    if (speed > 0) {
        delay_speed_usec = 1000000 / speed / font.CharacterWidth('W');
    } else if (x_orig == x_default_start) {
        x_orig = 0;
        x = 0;
    }

    while (!interrupt && loops != 0) {
        offscreen_canvas->Fill(bg_color.r, bg_color.g, bg_color.b);
        length = DrawText(offscreen_canvas, font, x, y + font.baseline(), color, nullptr, text.c_str(), letter_spacing);

        if (speed > 0 && x + length < 0) {
            x = x_orig;
            if (loops > 0) --loops;
        }

        offscreen_canvas = matrix->SwapOnVSync(offscreen_canvas);
        if (speed > 0) {
            x -= 1;
            usleep(delay_speed_usec);
        } else {
            pause();
        }
    }
}

// Simplified PlayGif based on led-image-viewer.cc
void PlayGif(RGBMatrix *matrix, const std::string &filename) {
    std::vector<Magick::Image> frames;
    try {
        readImages(&frames, filename.c_str());
    } catch (std::exception& e) {
        fprintf(stderr, "Failed to load GIF: %s\n", e.what());
        return;
    }

    if (frames.empty()) return;

    // Scale frames
    for (auto &img : frames) {
        img.scale(Magick::Geometry(matrix->width(), matrix->height()));
    }

    FrameCanvas *offscreen_canvas = matrix->CreateFrameCanvas();

    int loop = 0;
    while (!interrupt) {
        for (size_t i = 0; i < frames.size() && !interrupt; ++i) {
            const Magick::Image &img = frames[i];

            offscreen_canvas->Clear();
            for (size_t y = 0; y < img.rows(); ++y) {
                for (size_t x = 0; x < img.columns(); ++x) {
                    const Magick::Color &c = img.pixelColor(x, y);
                    if (c.alphaQuantum() < 255) {
                        offscreen_canvas->SetPixel(x, y,
                                                   ScaleQuantumToChar(c.redQuantum()),
                                                   ScaleQuantumToChar(c.greenQuantum()),
                                                   ScaleQuantumToChar(c.blueQuantum()));
                    }
                }
            }

            offscreen_canvas = matrix->SwapOnVSync(offscreen_canvas);
            usleep(100000);  // 0.1 sec per frame
        }
        ++loop;
    }
}