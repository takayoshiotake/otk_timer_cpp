//
//  main.cpp
//
//  Copyright © 2017 OTAKE Takayoshi. All rights reserved.
//

#include <cstdio>
#include <vector>
#include "otk_timer.hpp"

void test_1_normal() {
    static char const* name = __FUNCTION__;
    auto start = std::chrono::system_clock::now();
    auto end = start;
    otk::timer timer([&end](bool cancelled) {
        if (cancelled) {
            printf("Error: %s\n", name);
        }
        end = std::chrono::system_clock::now();
    }, std::chrono::milliseconds(300));
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    if (elapsed_ms >= std::chrono::milliseconds(300)) {
        printf("%s: passed (%ldms)\n", name, (long) elapsed_ms.count());
    }
    else {
        printf("%s: failed\n", name);
    }
}

void test_2_wait() {
    static char const* name = __FUNCTION__;
    auto start = std::chrono::system_clock::now();
    auto end = start;
    otk::timer timer([&end](bool cancelled) {
        if (cancelled) {
            printf("Error: %s\n", name);
        }
    }, std::chrono::milliseconds(300));
    timer.wait();
    end = std::chrono::system_clock::now();
    
    timer.wait();
    timer.cancel();
    
    if (timer.ended()) {
    }
    else {
        printf("%s: failed (timer.ended())\n", name);
        return;
    }
    if (!timer.cancelled()) {
    }
    else {
        printf("%s: failed (!timer.cancelled())\n", name);
        return;
    }
    
    auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    if (elapsed_ms >= std::chrono::milliseconds(300)) {
        printf("%s: passed (%ldms)\n", name, (long) elapsed_ms.count());
    }
    else {
        printf("%s: failed\n", name);
    }
}

void test_3_cancel_raii() {
    static char const* name = __FUNCTION__;
    auto start = std::chrono::system_clock::now();
    auto end = start;
    {
        otk::timer timer([&end](bool cancelled) {
            if (cancelled) {
            }
            else {
                printf("Error: %s\n", name);
            }
            end = std::chrono::system_clock::now();
        }, std::chrono::milliseconds(300));
    }
    
    auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    if (elapsed_ms < std::chrono::milliseconds(1)) {
        printf("%s: passed (%ldms)\n", name, (long) elapsed_ms.count());
    }
    else {
        printf("%s: failed (%ldms)\n", name, (long) elapsed_ms.count());
    }
}

void test_4_cancel() {
    static char const* name = __FUNCTION__;
    auto start = std::chrono::system_clock::now();
    auto end = start;
    otk::timer timer([&end](bool cancelled) {
        if (cancelled) {
        }
        else {
            printf("Error: %s\n", name);
        }
    }, std::chrono::milliseconds(300));
    timer.cancel();
    timer.cancel();
    timer.wait();
    end = std::chrono::system_clock::now();
    
    timer.cancel();
    timer.wait();
    
    if (timer.ended()) {
    }
    else {
        printf("%s: failed (timer.ended())\n", name);
        return;
    }
    if (timer.cancelled()) {
    }
    else {
        printf("%s: failed (timer.cancelled())\n", name);
        return;
    }
    
    auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    if (elapsed_ms < std::chrono::milliseconds(1)) {
        printf("%s: passed (%ldms)\n", name, (long) elapsed_ms.count());
    }
    else {
        printf("%s: failed\n", name);
    }
}

void test_5_multi() {
    static char const* name = __FUNCTION__;
    int x = 0;
    
    std::vector<std::shared_ptr<otk::timer>> timers;
    auto timer1 = std::make_shared<otk::timer>([&x, &timers](bool cancelled) {
        if (x == 0 && !cancelled) {
            ++x;
            // timer3
            timers.push_back(std::make_shared<otk::timer>([&x, &timers](bool cancelled) {
                if (x == 2 && cancelled) {
                    ++x;
                    // !!!: Destructing itself is forbidden
                    //timers.erase(timers.begin());
                }
                else {
                    printf("Error: %s timer3\n", name);
                }
            }, std::chrono::milliseconds(10), "timer3"));
        }
        else {
            printf("Error: %s timer1\n", name);
        }
    }, std::chrono::nanoseconds(0), "timer1");
    
    auto timer2 = std::make_unique<otk::timer>([&x, &timers](bool cancelled) {
        if (x == 1 && !cancelled) {
            ++x;
            timers.front()->cancel();
        }
        else {
            printf("Error: %s timer2\n", name);
        }
    }, std::chrono::milliseconds(5), "timer2");
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    if (x == 3 && timers.size() == 1 && timers.front()->cancelled()) {
        printf("%s: passed\n", name);
    }
    else {
        printf("%s: failed\n", name);
    }
}


int main(int argc, char const* argv[]) {
    test_1_normal();
    test_2_wait();
    test_3_cancel_raii();
    test_4_cancel();
    test_5_multi();
    return 0;
}

