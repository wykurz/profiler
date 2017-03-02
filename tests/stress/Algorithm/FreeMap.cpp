#include <Profiler/Algorithm/FreeMap.h>
#include <Profiler/Queue/Queue.h>
#include <atomic>
#include <boost/test/unit_test.hpp>
#include <chrono>
#include <thread>

namespace Profiler {
namespace Algorithm {
namespace Test {

namespace {

struct PatternsFixture {
  PatternsFixture() {
    std::srand(0);
    for (int i = 0; i < test.size(); ++i)
      bits.getFree();
  }
  void setupTest() {
    int free = 0;
    for (auto &&itest : test) {
      if (itest) {
        itest = false;
        ++free;
      }
    }
    for (int i = 0; i < free; ++i)
      bits.getFree();
    for (int i = 0; i < test.size(); ++i) {
      bool flip = (std::rand() & 1) != 0;
      if (flip) {
        test[i] = true;
        bits.setFree(i);
      }
    }
  }
  bool isFree(std::size_t index_) { return test[index_]; }
  std::vector<bool> test = std::vector<bool>(1024 + 3, false);
  FreeMap bits{test.size()};
};
} // namespace

BOOST_FIXTURE_TEST_SUITE(FreeMapStressTests, PatternsFixture)

BOOST_AUTO_TEST_CASE(Patterns) {
  for (int i = 0; i < 1024; ++i) {
    setupTest();
    for (int i = 0; i < test.size(); ++i)
      BOOST_CHECK_EQUAL(isFree(i), bits.isFree(i));
  }
}

BOOST_AUTO_TEST_SUITE_END()

namespace {

struct ThreadsFixture {
  ThreadsFixture() {
    for (auto &node : nodes)
      free.push(&node);
  }
  using Queue = Queue::Queue<int>;
  using Node = Queue::Node;
  std::vector<Node> nodes{FreeMap::MaxSize};
  Queue acquired{nodes.data()};
  Queue free{nodes.data()};
  std::atomic<bool> done{false};
  FreeMap bits{FreeMap::MaxSize};
};
} // namespace

BOOST_FIXTURE_TEST_SUITE(FreeMapThreadsTests, ThreadsFixture)

BOOST_AUTO_TEST_CASE(OpposingThreads) {
  std::thread taker([this]() {
    while (!this->done.load(std::memory_order_acquire)) {
      Node *node = free.pull();
      if (node == nullptr)
        continue;
      auto index = this->bits.getFree();
      BOOST_REQUIRE(0 <= index);
      node->value = index;
      acquired.push(node);
    }
  });
  std::thread giver([this]() {
    while (!this->done.load(std::memory_order_acquire)) {
      Node *node = acquired.pull();
      if (node == nullptr)
        continue;
      this->bits.setFree(node->value);
      free.push(node);
    }
  });
  std::this_thread::sleep_for(std::chrono::seconds(3));
  done.store(true, std::memory_order_release);
  taker.join();
  giver.join();
}

BOOST_AUTO_TEST_SUITE_END()
} // namespace Test
} // namespace Algorithm
} // namespace Profiler
