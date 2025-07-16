 #pragma once

#ifndef MG_PIPELINEEXECUTOR_HPP
#define MG_PIPELINEEXECUTOR_HPP

namespace MobileGL {
    namespace MG_Util {
        namespace Pipeline {
            using next_t = std::function<void()>;
            using next_ptr_t = SharedPtr<next_t>;

            template<typename T>
            class Stage {
            protected:
                using T_ptr = SharedPtr<T>;
                using callback_t = std::function<void(T_ptr, next_ptr_t)>;
            public:
                virtual callback_t handler() = 0;
            };

            template<typename T>
            class PipelineExecutor {
            protected:
                using T_ptr = SharedPtr<T>;
                using callback_t = std::function<void(T_ptr, next_ptr_t)>;
                using middleware_ptr_t = SharedPtr<Stage<T>>;
            public:
                explicit PipelineExecutor(std::function<void(T &)> callback) :
                        callback_(Move(callback)) {}

                PipelineExecutor &Register(callback_t middleware) {
                    middlewares_.emplace_back(Move(middleware));
                    return *this;
                }

                PipelineExecutor &Register(middleware_ptr_t middleware) {
                    middleware_storage_.emplace_back(middleware);
                    Register(middleware->handler());
                    return *this;
                }

                void Process(T_ptr payload) {
                    auto it_ptr = MakeShared<typename Vector<callback_t>::iterator>(
                            middlewares_.begin());
                    auto next = MakeShared<next_t>();
                    auto weak_next = std::weak_ptr<next_t>(next);

                    // setup next callback
                    *next = [this, it_ptr, weak_next, payload] {
                        auto &it = *it_ptr;

                        // if there's something to do
                        if (it != middlewares_.end()) {
                            auto &func = *it;
                            // advance the iterator to the next Stage
                            ++it;
                            auto next = weak_next.lock();
                            func(payload, next);
                            // proceed with post-Stage logic
                        } else {
                            callback_(*payload);
                        }
                    };
                    // bootstrap the whole thing
                    (*next)();
                }

            private:
                Vector<callback_t> middlewares_;
                Vector<middleware_ptr_t> middleware_storage_;
                std::function<void(T &)> callback_;
            };
        }
    }
}

#endif //MG_PIPELINEEXECUTOR_HPP
