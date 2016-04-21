#include <Instrumentation/StatsScope.h>
#include <Queue/Queue.h>
#include <boost/test/unit_test.hpp>
#include <array>
#include <chrono>
#include <condition_variable>
#include <memory>
#include <thread>

namespace Queue { namespace Tests
{

    BOOST_AUTO_TEST_CASE(Basic)
    {
        std::size_t n = 1000;
        std::vector<Node<int>> data;
        for (int i = 0; i < n; ++i) data.push_back(std::move(i));
        Queue<int> queue;
        for (int i = 0; i < n; ++i) queue.push(&data[i]);
        std::vector<Node<int>> data2;
        for (int i = 0; i < n; ++i) {
            auto iptr = queue.pull();
            BOOST_REQUIRE(iptr);
            data2.push_back(*iptr);
        }
        for (int i = 0; i < n; ++i) {
            BOOST_CHECK_EQUAL(data[i].value, data2[n - i - 1].value);
        }
    }

    struct QueueTest
    {
        using QueueType = Queue<int>;
        static constexpr std::size_t numQueues = 2;

        QueueTest(std::size_t size_)
        {
            for (int i = 0; i < size_; ++i) data.push_back(std::move(i));
            for (int i = 0; i < size_; ++i) queues[0].push(&data[i]);
        }

        void shuffle()
        {
            std::size_t idx = 0;
            auto nodePtr = queues[idx].pull();
            while (!nodePtr) {
                idx = (idx + 1) % numQueues;
                nodePtr = queues[idx].pull();
            }
            idx = (idx + 1) % numQueues;
            queues[idx].push(nodePtr);
        }

        std::vector<QueueType::NodeType> data;
        std::array<QueueType, numQueues> queues;
    };

    BOOST_AUTO_TEST_CASE(Shuffle)
    {
        constexpr std::size_t numThreads = 2;
        std::mutex lock;
        std::condition_variable control, workers;
        std::size_t waitCount = 0;
        std::atomic<bool> done{false};
        QueueTest queueTest(100);
        auto func = [&]() {
            std::unique_lock<std::mutex> lk(lock);
            ++waitCount;
            control.notify_one();
            workers.wait(lk);
            while (not done.load()) queueTest.shuffle();
        };
        std::vector<std::unique_ptr<std::thread>> threads;
        for (int i = 0; i < numThreads; ++i)
            threads.push_back(std::move(std::unique_ptr<std::thread>(new std::thread(func))));
        {
            std::unique_lock<std::mutex> lk(lock);
            control.wait(lk, [&waitCount]{ return waitCount == numThreads; });
        }
        std::this_thread::sleep_for(std::chrono::microseconds(10000)); // 10ms
        workers.notify_all();
        std::this_thread::sleep_for(std::chrono::seconds(1));
        done.store(true);
        for (auto& thread : threads) thread->join();
    }

}
}
