#pragma once
#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
namespace lumen {
class Application {
public:
    Application();
    ~Application();
    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;
    static Application* instance() noexcept;
    void post(std::function<void()> task);
    int exec();
    void quit(int exit_code = 0);
    // Run queued work currently available; useful for embedding and deterministic tests.
    std::size_t process_events();
private:
    static Application* instance_;
    std::mutex mutex_;
    std::condition_variable ready_;
    std::queue<std::function<void()>> queue_;
    bool stopping_{false};
    int exit_code_{0};
};
}
