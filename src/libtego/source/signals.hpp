#pragma once

namespace tego
{
    // allows us to marshall lambda functions and their data to
    // call them later
    class type_erased_callback
    {
    public:
        type_erased_callback() = default;
        type_erased_callback(type_erased_callback&&);
        type_erased_callback& operator=(type_erased_callback&&);

        template<typename FUNC>
        type_erased_callback(FUNC&& func)
        {
            // heap allocated a moved copy of the passed in function
            data_ = {
                new FUNC(std::move(func)),
                // deleter function casts raw ptr back to FUNC and deletes
                [](void* data) {
                    auto* lambda = static_cast<FUNC*>(data);
                    delete lambda;
                }
            };

            // save of a function for invoking func
            callback_ = [](void* data) {
                auto* lambda = static_cast<FUNC*>(data);
                (*lambda)();
            };
        }
        void invoke();
    private:
        std::unique_ptr<void, void(*)(void*)> data_ = {nullptr, nullptr};
        void (*callback_)(void* data) = nullptr;
    };

    /*
     * The callback_register class keeps track of provided user callbacks
     * and lets us register them via register_X functions. Libtego internals
     * trigger callbacks by way of the emit_EVENT functions. The callback
     * registry is per-tego_context
     */

    class callback_registry
    {
    public:
        callback_registry(tego_context* context);

        /*
         * Each callback X has a register_X function, an emit_X function, and
         * a cleanup_X_args function
         *
         * It is assumed that a callback always sends over the tego_context_t* as
         * the first argument
         */
        #define TEGO_IMPLEMENT_CALLBACK_FUNCTIONS(EVENT, ...)\
        private:\
            tego_##EVENT##_callback_t EVENT##_ = nullptr;\
            static void cleanup_##EVENT##_args(__VA_ARGS__);\
        public:\
            void register_##EVENT(tego_##EVENT##_callback_t cb)\
            {\
                EVENT##_ = cb;\
            }\
            template<typename... ARGS>\
            void emit_##EVENT(ARGS&&... args)\
            {\
                if (EVENT##_ != nullptr) {\
                    push_back(\
                        [=, context=context_, callback=EVENT##_]() mutable -> void\
                        {\
                            callback(context, std::forward<ARGS>(args)...);\
                            cleanup_##EVENT##_args(std::forward<ARGS>(args)...);\
                        }\
                    );\
                }\
            }

        TEGO_IMPLEMENT_CALLBACK_FUNCTIONS(tor_state_changed);
        TEGO_IMPLEMENT_CALLBACK_FUNCTIONS(tor_log_received);
        TEGO_IMPLEMENT_CALLBACK_FUNCTIONS(chat_request_received);
        TEGO_IMPLEMENT_CALLBACK_FUNCTIONS(chat_request_response_received);
        TEGO_IMPLEMENT_CALLBACK_FUNCTIONS(message_received);
        TEGO_IMPLEMENT_CALLBACK_FUNCTIONS(user_status_changed, tego_user_id_t*, tego_user_status_t);
        TEGO_IMPLEMENT_CALLBACK_FUNCTIONS(new_identity_created, tego_ed25519_private_key_t*);

    private:
        void push_back(type_erased_callback&&);
        tego_context* context_ = nullptr;
    };

    /*
     * callback_queue holds onto a queue of callbacks. Libtego internals
     * enqueue callbacks and the callback queue executes them on a
     * background worker thread
     */
    class callback_queue
    {
    public:
        callback_queue(tego_context* context);
        ~callback_queue();

        void push_back(type_erased_callback&&);
    private:
        tego_context* context_;

        std::atomic_bool terminating_;
        std::mutex mutex_;
        std::thread worker_;
        // this queue is protected by mutex_ within worker_ thread and callback_queue methods
        std::vector<type_erased_callback> pending_callbacks_;
    };
}