#include <Profiler/Algorithm/FoldLeft.h>
#include <Profiler/Instrumentation/StatsScope.h>
#include <Profiler/Queue/Queue.h>
#include <array>
#include <boost/test/unit_test.hpp>
#include <chrono>
#include <condition_variable>
#include <deque>
#include <memory>
#include <stdexcept>
#include <thread>

namespace Profiler {
namespace Queue {
namespace Tests {

BOOST_AUTO_TEST_SUITE(WriterTests)

BOOST_AUTO_TEST_CASE(Basic) {
  int n = 1000;
  std::vector<Queue<int>::Node> data(n);
  for (int i = 0; i < n; ++i)
    data[i].value = i;
  Queue<int> queue(&data[0]);
  for (int i = 0; i < n; ++i)
    queue.push(&data[i]);
  std::vector<int> dataVals;
  for (int i = 0; i < n; ++i) {
    auto iptr = queue.pull();
    BOOST_REQUIRE(iptr);
    dataVals.push_back(iptr->value);
  }
  for (int i = 0; i < n; ++i) {
    BOOST_CHECK_EQUAL(data[i].value, dataVals[n - i - 1]);
  }
}

struct QueueTest {
  using QueueType = Queue<int>;
  using NodeType = QueueType::Node;

  static constexpr int numQueues = 10;

  explicit QueueTest(int size_) : data(size_) {
    for (int i = 0; i < size_; ++i) {
      data[i].value = i;
      queues.emplace_back(&data[0]);
      queues[0].push(&data[i]);
    }
  }

  void shuffle() {
    int idx = 0;
    auto nodePtr = queues[idx].pull();
    while (!nodePtr) {
      idx = (idx + 1) % numQueues;
      nodePtr = queues[idx].pull();
    }
    idx = (idx + 1) % numQueues;
    queues[idx].push(nodePtr);
    ++shuffles;
  }

  void exchange() {
    int idx = 0;
    auto nodePtr = queues[idx].extract();
    while (!nodePtr) {
      idx = (idx + 1) % numQueues;
      nodePtr = queues[idx].extract();
    }
    idx = (idx + 1) % numQueues;
    while (nodePtr) {
      auto head = nodePtr;
      nodePtr = nodePtr->getNext();
      head->setNext(nullptr);
      queues[idx].push(head);
    }
    ++exchanges;
  }

  void check() const {
    int size = Algorithm::foldLeft(
        queues, 0, [](int v, const QueueType &q) { return v + q.size(); });
    BOOST_CHECK_EQUAL(data.size(), size);
    BOOST_CHECK_GT(shuffles, 0);
    BOOST_CHECK_GT(exchanges, 0);
  }

  std::vector<NodeType> data;
  std::deque<QueueType> queues;
  std::atomic<int> shuffles = {0};
  std::atomic<int> exchanges = {0};
};

BOOST_AUTO_TEST_CASE(Shuffle) {
  constexpr int numThreads = 4;
  std::mutex lock;
  std::condition_variable control, workers;
  int waitCount = 0;
  std::atomic<bool> done{false};
  QueueTest queueTest(1000);
  auto setup = [&]() {
    std::unique_lock<std::mutex> lk(lock);
    ++waitCount;
    control.notify_one();
    workers.wait(lk);
  };
  auto func1 = [&]() {
    setup();
    int loops = 0;
    while (not done.load()) {
      queueTest.shuffle();
      ++loops;
    }
    assert(0 < loops);
  };
  auto func2 = [&]() {
    setup();
    int loops = 0;
    while (not done.load()) {
      queueTest.exchange();
      ++loops;
    }
    assert(0 < loops);
  };
  std::vector<std::unique_ptr<std::thread>> threads;
  threads.push_back(std::unique_ptr<std::thread>(new std::thread(func2)));
  for (int i = 0; i < numThreads - 1; ++i)
    threads.push_back(std::unique_ptr<std::thread>(new std::thread(func1)));
  {
    std::unique_lock<std::mutex> lk(lock);
    control.wait(lk, [&waitCount] { return waitCount == numThreads; });
  }
  std::this_thread::sleep_for(std::chrono::microseconds(1000000)); // 1s
  workers.notify_all();
  std::this_thread::sleep_for(std::chrono::seconds(3));
  done.store(true);
  for (auto &thread : threads)
    thread->join();
  queueTest.check();
}

BOOST_AUTO_TEST_SUITE_END()
} // namespace Tests
} // namespace Queue
} // namespace Profiler
