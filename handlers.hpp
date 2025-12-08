#pragma once
#include <string>
#include "led-matrix.h"

// funkcje które będą wołać istniejący kod z examples
void ShowText(rgb_matrix::RGBMatrix *matrix, const std::string &text);
void PlayGif(rgb_matrix::RGBMatrix *matrix, const std::string &filename);