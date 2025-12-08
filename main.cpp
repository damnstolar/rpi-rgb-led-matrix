#include "led-matrix.h"
#include "command_queue.hpp"
#include "handlers.hpp"
#include "simple_http_server.hpp"
#include <map>

using namespace rgb_matrix;

CommandQueue queue;

int main(int argc, char *argv[]) {
    RGBMatrix::Options opt;
    RuntimeOptions rt;

    opt.rows = 32;
    opt.cols = 64;
    opt.chain_length = 2;
    opt.hardware_mapping = "adafruit-hat";

    rt.gpio_slowdown = 3;

    RGBMatrix *matrix = RGBMatrix::CreateFromOptions(opt, rt);
    if (!matrix) {
        fprintf(stderr, "Failed to create matrix\n");
        return 1;
    }

    // HTTP server
    SimpleHTTPServer server(8080);

    server.on("/text", [&](const std::map<std::string,std::string>& params) {
        if (params.count("msg")) {
            queue.push(Command{CommandType::SHOW_TEXT, params.at("msg")});
        }
    });

    server.on("/gif", [&](const std::map<std::string,std::string>& params) {
        if (params.count("file")) {
            queue.push(Command{CommandType::PLAY_GIF, params.at("file")});
        }
    });

    printf("Starting HTTP server...\n");
    server.start();

    // Main loop
    while (true) {
        Command cmd;
        if (queue.pop(cmd)) {
            switch (cmd.type) {
                case CommandType::SHOW_TEXT:
                    ShowText(matrix, cmd.arg);
                    break;
                case CommandType::PLAY_GIF:
                    PlayGif(matrix, cmd.arg);
                    break;
            }
        }
        usleep(5000);
    }

    delete matrix;
    return 0;
}