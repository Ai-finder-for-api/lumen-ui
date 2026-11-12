#pragma once
#include <cstddef>
#include <functional>
#include <memory>
#include <unordered_map>
#include <utility>
#include <vector>

namespace lumen {
// A lightweight typed multicast signal. Connections persist until explicitly disconnected
// or until the signal is destroyed; discarding the returned handle is safe.
class Connection {
public:
    Connection() = default;
    void disconnect() { if (disconnect_) { disconnect_(); disconnect_ = {}; } }
    [[nodiscard]] bool connected() const { return static_cast<bool>(disconnect_) && alive_ && alive_(); }
private:
    template<class...> friend class Signal;
    Connection(std::function<void()> d, std::function<bool()> a) : disconnect_(std::move(d)), alive_(std::move(a)) {}
    std::function<void()> disconnect_;
    std::function<bool()> alive_;
};

template<class... Args>
class Signal {
    struct State { std::size_t next{1}; std::unordered_map<std::size_t, std::function<void(Args...)>> slots; };
public:
    Signal() : state_(std::make_shared<State>()) {}
    template<class F> Connection connect(F&& callback) {
        const auto id = state_->next++;
        state_->slots.emplace(id, std::forward<F>(callback));
        std::weak_ptr<State> weak = state_;
        return Connection([weak,id] { if (auto s=weak.lock()) s->slots.erase(id); },
                          [weak,id] { auto s=weak.lock(); return s && s->slots.contains(id); });
    }
    void emit(Args... args) const {
        // Snapshot permits slots to connect/disconnect while an emission is in progress.
        std::vector<std::function<void(Args...)>> callbacks;
        callbacks.reserve(state_->slots.size());
        for (const auto& [_, fn] : state_->slots) callbacks.push_back(fn);
        for (auto& fn : callbacks) fn(args...);
    }
    [[nodiscard]] std::size_t size() const noexcept { return state_->slots.size(); }
private:
    std::shared_ptr<State> state_;
};
}
