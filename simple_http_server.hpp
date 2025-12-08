#pragma once
#include <string>
#include <thread>
#include <functional>
#include <map>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

class SimpleHTTPServer {
public:
    using HandlerFn = std::function<void(const std::map<std::string,std::string>&)>;

    SimpleHTTPServer(int port) : port_(port) {}

    void on(const std::string &path, HandlerFn fn) {
        handlers_[path] = fn;
    }

    void start() {
        std::thread([this]() { run(); }).detach();
    }

private:
    int port_;
    std::map<std::string, HandlerFn> handlers_;

    std::map<std::string,std::string> parseQuery(const std::string &req) {
        std::map<std::string,std::string> out;
        auto pos = req.find("?");
        if (pos == std::string::npos) return out;

        auto query = req.substr(pos + 1);
        while (true) {
            auto eq = query.find("=");
            if (eq == std::string::npos) break;
            auto amp = query.find("&");
            std::string key = query.substr(0, eq);
            std::string val = (amp == std::string::npos
                ? query.substr(eq + 1)
                : query.substr(eq + 1, amp - eq - 1));
            out[key] = val;
            if (amp == std::string::npos) break;
            query = query.substr(amp + 1);
        }
        return out;
    }

    void run() {
        int server_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (server_fd < 0) {
            perror("socket failed");
            return;
        }
        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port_);
        addr.sin_addr.s_addr = INADDR_ANY;

        if (bind(server_fd, (sockaddr*)&addr, sizeof(addr)) < 0) {
            perror("bind failed");
            close(server_fd);
            return;
        }
        if (listen(server_fd, 10) < 0) {
            perror("listen failed");
            close(server_fd);
            return;
        }
        printf("HTTP server listening on port %d\n", port_);

        while (true) {
            int client = accept(server_fd, nullptr, nullptr);
            char buffer[2048] = {0};
            read(client, buffer, sizeof(buffer));

            std::string req(buffer);
            auto space = req.find(" ");
            auto pathFull = req.substr(space + 1);
            pathFull = pathFull.substr(0, pathFull.find(" "));

            auto path = pathFull.substr(0, pathFull.find("?"));

            auto params = parseQuery(pathFull);

            if (handlers_.count(path)) {
                handlers_[path](params);
            }

            std::string msg = "HTTP/1.1 200 OK\r\n\r\nOK";
            write(client, msg.c_str(), msg.size());
            close(client);
        }
    }
};