#ifndef AMMOPROTOCOL_MIDDLEWARE_HPP
#define AMMOPROTOCOL_MIDDLEWARE_HPP

#include <vector>
#include <functional>
#include <memory>

using next_t = std::function<void()>;
using next_ptr_t = std::shared_ptr<next_t>;

template <typename T>
class middleware {
protected:
    using T_ptr = std::shared_ptr<T>;
    using callback_t = std::function<void(T_ptr, next_ptr_t)>;
public:
    virtual callback_t handler() = 0;
};

template <typename T>
class middleware_manager {
protected:
    using T_ptr = std::shared_ptr<T>;
    using callback_t = std::function<void(T_ptr, next_ptr_t)>;
    using middleware_ptr_t = std::shared_ptr<middleware<T>>;
public:
    explicit middleware_manager(std::function<void(T&)> callback):
            callback_(std::move(callback)) {}

    void use(callback_t middleware) {
        middlewares_.emplace_back(std::move(middleware));
    }

    void use(middleware_ptr_t middleware) {
        middleware_storage_.emplace_back(middleware);
        use(middleware->handler());
    }

    void process(T_ptr payload) {
        auto it_ptr = std::make_shared<typename std::vector<callback_t>::iterator>(middlewares_.begin());
        auto next = std::make_shared<next_t>();
        auto weak_next = std::weak_ptr<next_t>(next);

        // setup next callback
        *next = [this, it_ptr, weak_next, payload] {
            auto& it = *it_ptr;

            // if there's something to do
            if (it != middlewares_.end()) {
                auto& func = *it;
                // advance the iterator to the next middleware
                ++it;
                auto next = weak_next.lock();
                func(payload, next);
                // proceed with post-middleware logic
            } else {
                callback_(*payload);
            }
        };
        // bootstrap the whole thing
        (*next)();
    }

private:
    std::vector<callback_t> middlewares_;
    std::vector<middleware_ptr_t> middleware_storage_;
    std::function<void(T&)> callback_;
};

#endif //AMMOPROTOCOL_MIDDLEWARE_HPP
