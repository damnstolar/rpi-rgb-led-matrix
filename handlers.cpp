#include "handlers.hpp"
#include "graphics.h"
#include <GraphicsMagick/Magick++.h>
#include <magick/image.h>
#include <vector>
#include <unistd.h>

using namespace rgb_matrix;

void ShowText(RGBMatrix *matrix, const std::string &text) {
    Font font;
    if (!font.LoadFont("fonts/7x13.bdf")) {
        fprintf(stderr, "Couldn't load font\n");
        return;
    }

    Color color(255, 255, 0);
    Color bg_color(0, 0, 0);

    FrameCanvas *offscreen_canvas = matrix->CreateFrameCanvas();

    int x_orig = matrix->width();
    int x = x_orig;
    int y = 10;
    int length = 0;
    int delay_speed_usec = 100000;  // 0.1 sec

    while (true) {
        offscreen_canvas->Fill(bg_color.r, bg_color.g, bg_color.b);
        length = DrawText(offscreen_canvas, font, x, y + font.baseline(), color, nullptr, text.c_str(), 0);

        if (x + length < 0) {
            x = x_orig;
        }

        offscreen_canvas = matrix->SwapOnVSync(offscreen_canvas);
        x -= 1;
        usleep(delay_speed_usec);

        // Simple exit condition - scroll once
        if (x < -length) break;
    }

    delete offscreen_canvas;
}

void PlayGif(RGBMatrix *matrix, const std::string &filename) {
    std::vector<Magick::Image> frames;
    try {
        readImages(&frames, filename.c_str());
    } catch (std::exception& e) {
        fprintf(stderr, "Failed to load GIF: %s\n", e.what());
        return;
    }

    if (frames.empty()) return;

    FrameCanvas *offscreen_canvas = matrix->CreateFrameCanvas();

    for (auto &img : frames) {
        img.scale(Magick::Geometry(matrix->width(), matrix->height()));

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

    delete offscreen_canvas;
}