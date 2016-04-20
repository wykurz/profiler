#include <Instrumentation/StatsScope.h>
#include <Queue/Queue.h>
#include <boost/test/unit_test.hpp>
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
        QueueTest queueTest(1000);
        std::thread t1([&queueTest]() { int count = 1000000; while (count--) queueTest.shuffle(); });
        std::thread t2([&queueTest]() { int count = 1000000; while (count--) queueTest.shuffle(); });
        t1.join();
        t2.join();
    }

}
}
