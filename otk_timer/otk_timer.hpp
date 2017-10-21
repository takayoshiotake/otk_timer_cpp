//
//  otk_timer.hpp
//
//  Copyright © 2017 OTAKE Takayoshi. All rights reserved.
//

// C++14

#pragma once

#include <cassert>
#include <thread>
#include <mutex>
#include <future>
#include <functional>
#include <chrono>
#include <string>

namespace otk {
    // otk::timer
    //
    // `timer` creates a new thread, and waits until the time comes, then calls `callback` given by constructor.
    // `timer` is cancellable, see also `void cancel()`.
    //
    // - version: 1.0
    // - last_updated: 2017-10-22
    struct timer {
        std::string name_;
        std::thread thread_;
        std::mutex wait_mutex_;
        std::promise<void> cancel_promise_;
        std::mutex cancel_mutex_;
        bool cancel_request_;
        
        bool cancelled_;
        bool ended_;
        
        // Constructs a new timer.
        //
        // - template parameter Rep: ...
        // - template parameter Period: ...
        // - parameter callback: called when timer ends
        //   - parameter cancelled: indicates this times was cancelled
        // - parameter timeout: ...
        // - parameter name: name for debugging
        //
        // - returns: a new timer
        template <class Rep, class Period>
        timer(std::function<void(bool cancelled)> callback, std::chrono::duration<Rep, Period> timeout, std::string name = "")
        : name_(name)
        , cancel_request_(false)
        , ended_(false)
        , cancelled_(false) {
            thread_ = std::thread([=]() {
                auto future = cancel_promise_.get_future();
                auto status = future.wait_for(timeout);
                
                // NOTE: In normal case, status will be `std::future_status::timeout`,
                //       and `std::future_status::ready` indicates this timer was cancelled.
                cancelled_ = !(status == std::future_status::timeout);
                callback(cancelled_);
                ended_ = true;
            });
        }
        
        // Destructs this timer.
        //
        // - notice: you can not call me from in `callback`.
        ~timer() {
            assert(std::this_thread::get_id() != thread_.get_id());
            
            cancel();
            wait();
        }
        
        // Waits timer end.
        // If already this timer was ended, this returns as soon as possible.
        //
        // - notice: you can not call me from in `callback`.
        void wait() {
            assert(std::this_thread::get_id() != thread_.get_id());
            
            std::lock_guard<std::mutex> lock(wait_mutex_);
            if (thread_.joinable()) {
                thread_.join();
            }
        }
        
        // Cancels this timer, but this timer does not end immidietly.
        // If cancel succeeded, `callback` given by constructor will be called with parameter `cancelled` that is `true`, see also constructor.
        void cancel() {
            if (std::this_thread::get_id() == thread_.get_id()) {
                // You are already in `callback`...
                return;
            }
            
            std::lock_guard<std::mutex> lock(cancel_mutex_);
            if (!cancel_request_) {
                cancel_promise_.set_value();
                cancel_request_ = true;
            }
        }
        
        // Indicates this timer was ended.
        //
        // - returns: true or false
        bool ended() const {
            return ended_;
        }
        
        // Indicates this timer was ended by cancellation.
        //
        // - notice: This does not indicate whether this timer was ended or not, so you need check return of `bool ended() const` to know it.
        //
        // - returns: true or false
        bool cancelled() const {
            return cancelled_;
        }
        
        timer() = delete;
        timer(timer const&) = delete;
        timer(timer&&) = delete;
    };
}
