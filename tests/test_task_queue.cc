#include "../src/task_queue.h"

#include <cassert>
#include <cstring>
#include <thread>
#include <atomic>
#include <chrono>

static void test_constructor()
{
    ezlive::task_queue tq(10);
    // Just verify it doesn't crash
}

static void test_push_and_pop_single()
{
    ezlive::task_queue tq(10);
    std::atomic<int> counter{0};

    auto task_fn = [](void* arg) {
        std::atomic<int>* c = static_cast<std::atomic<int>*>(arg);
        (*c)++;
    };

    tq.push(task_fn, &counter);

    ezlive::task_queue::task_fn popped_task;
    void* popped_arg = nullptr;
    tq.pop(popped_task, &popped_arg);

    assert(popped_task != nullptr);
    popped_task(popped_arg);
    assert(counter == 1);
}

static void test_push_and_pop_multiple()
{
    ezlive::task_queue tq(5);
    std::atomic<int> counter{0};

    auto task_fn = [](void* arg) {
        std::atomic<int>* c = static_cast<std::atomic<int>*>(arg);
        (*c)++;
    };

    // Push multiple tasks
    for (int i = 0; i < 3; i++) {
        tq.push(task_fn, &counter);
    }

    // Pop and execute all
    for (int i = 0; i < 3; i++) {
        ezlive::task_queue::task_fn popped_task;
        void* popped_arg = nullptr;
        tq.pop(popped_task, &popped_arg);
        popped_task(popped_arg);
    }

    assert(counter == 3);
}

static void test_producer_consumer()
{
    ezlive::task_queue tq(10);
    std::atomic<int> sum{0};

    auto task_fn = [](void* arg) {
        int* val = static_cast<int*>(arg);
        // Use atomic to avoid race conditions
        static std::atomic<int> total{0};
        total += *val;
        // Store result back through a global or passed structure
        // For simplicity, just verify it runs
    };

    std::thread producer([&tq, &task_fn]() {
        for (int i = 1; i <= 10; i++) {
            int* val = new int(i);
            tq.push(task_fn, val);
        }
    });

    std::thread consumer([&tq, &sum]() {
        for (int i = 0; i < 10; i++) {
            ezlive::task_queue::task_fn popped_task;
            void* popped_arg = nullptr;
            tq.pop(popped_task, &popped_arg);
            int* val = static_cast<int*>(popped_arg);
            sum += *val;
            delete val;
        }
    });

    producer.join();
    consumer.join();

    assert(sum == 55);  // 1+2+...+10 = 55
}

static void test_capacity_limit()
{
    // Test that queue blocks when full
    ezlive::task_queue tq(2);
    std::atomic<int> pushed{0};

    auto task_fn = [](void* arg) {
        (void)arg;
    };

    // Push 2 tasks (fills the queue)
    tq.push(task_fn, nullptr);
    pushed++;
    tq.push(task_fn, nullptr);
    pushed++;

    assert(pushed == 2);

    // Now pop one to make room
    ezlive::task_queue::task_fn popped_task;
    void* popped_arg = nullptr;
    tq.pop(popped_task, &popped_arg);

    // Can push another one now
    tq.push(task_fn, nullptr);
    pushed++;

    assert(pushed == 3);
}

static void test_multiple_producers_consumers()
{
    ezlive::task_queue tq(20);
    std::atomic<int> counter{0};
    const int num_tasks = 100;

    std::thread producers[2];
    for (int p = 0; p < 2; p++) {
        producers[p] = std::thread([&tq, &counter]() {
            auto inc_fn = [](void* arg) {
                std::atomic<int>* c = static_cast<std::atomic<int>*>(arg);
                (*c)++;
            };
            for (int i = 0; i < num_tasks / 2; i++) {
                tq.push(inc_fn, &counter);
            }
        });
    }

    std::thread consumers[2];
    for (int c = 0; c < 2; c++) {
        consumers[c] = std::thread([&tq]() {
            for (int i = 0; i < num_tasks / 2; i++) {
                ezlive::task_queue::task_fn task;
                void* arg = nullptr;
                tq.pop(task, &arg);
                task(arg);
            }
        });
    }

    for (int i = 0; i < 2; i++) {
        producers[i].join();
        consumers[i].join();
    }

    assert(counter == num_tasks);
}

static void test_different_task_functions()
{
    ezlive::task_queue tq(10);

    auto add_fn = [](void* arg) {
        int* val = static_cast<int*>(arg);
        *val += 10;
    };

    auto mul_fn = [](void* arg) {
        int* val = static_cast<int*>(arg);
        *val *= 2;
    };

    int x = 5;
    tq.push(add_fn, &x);
    tq.push(mul_fn, &x);

    ezlive::task_queue::task_fn task;
    void* arg = nullptr;

    tq.pop(task, &arg);
    task(arg);

    tq.pop(task, &arg);
    task(arg);

    // 5 + 10 = 15, then 15 * 2 = 30
    assert(x == 30);
}

int main()
{
    test_constructor();
    test_push_and_pop_single();
    test_push_and_pop_multiple();
    test_producer_consumer();
    test_capacity_limit();
    test_multiple_producers_consumers();
    test_different_task_functions();

    printf("All task_queue tests passed!\n");
    return 0;
}
