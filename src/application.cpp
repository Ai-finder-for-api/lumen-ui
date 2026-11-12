#include "lumen/application.hpp"
#include <stdexcept>
namespace lumen {
Application* Application::instance_ = nullptr;
Application::Application() {
    if (instance_) throw std::logic_error("Only one Lumen Application may be active");
    instance_=this;
}
Application::~Application() { if(instance_==this) instance_=nullptr; }
Application* Application::instance() noexcept { return instance_; }
void Application::post(std::function<void()> task) {
    if(!task) return;
    { std::lock_guard lock(mutex_); queue_.push(std::move(task)); }
    ready_.notify_one();
}
std::size_t Application::process_events() {
    std::size_t count=0;
    for(;;) {
        std::function<void()> task;
        { std::lock_guard lock(mutex_); if(queue_.empty()) break; task=std::move(queue_.front()); queue_.pop(); }
        task(); ++count;
    }
    return count;
}
int Application::exec() {
    for(;;) {
        std::function<void()> task;
        { std::unique_lock lock(mutex_); ready_.wait(lock,[this]{return stopping_ || !queue_.empty();});
          if(queue_.empty() && stopping_) return exit_code_;
          task=std::move(queue_.front()); queue_.pop(); }
        task();
    }
}
void Application::quit(int code) {
    { std::lock_guard lock(mutex_); stopping_=true; exit_code_=code; }
    ready_.notify_all();
}
}
