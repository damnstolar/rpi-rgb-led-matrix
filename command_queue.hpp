#pragma once
#include <string>
#include <queue>
#include <mutex>

enum class CommandType {
    SHOW_TEXT,
    PLAY_GIF
};

struct Command {
    CommandType type;
    std::string arg;      // tekst lub ścieżka do GIF-a
};

class CommandQueue {
public:
    void push(const Command &cmd) {
        std::lock_guard<std::mutex> lock(mutex_);
        q_.push(cmd);
    }

    bool pop(Command &cmd) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (q_.empty()) return false;
        cmd = q_.front();
        q_.pop();
        return true;
    }

private:
    std::queue<Command> q_;
    std::mutex mutex_;
};