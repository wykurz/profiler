#include <Control/Thread.h>
#include <Control/Writer.h>
#include <Instrumentation/StatsScope.h>
#include <boost/test/unit_test.hpp>
#include <cassert>
#include <thread>
#include <unordered_map>

namespace Control { namespace Test
{

namespace
{

    struct MockManager
    {
        Arena& addThread(Thread& thread_)
        {
            return arena;
        }
        Arena arena{100000};
    };

    using MemBuffer = std::shared_ptr<std::stringstream>;
    using BufferMap = std::unordered_map<std::string, MemBuffer>;

    struct MemoryOut : Output
    {
        MemoryOut(BufferMap& buffers_, const char* name_)
          : _out(std::make_shared<std::stringstream>())
        {
            buffers_[name_] = _out;
        }
        virtual std::ostream& get()
        {
            assert(_out.get());
            return *_out;
        }
      private:
        MemBuffer _out;
    };

}

    BOOST_AUTO_TEST_SUITE(WriterTests)

    BOOST_AUTO_TEST_CASE(Basic)
    {
        MockManager manager;
        Thread thread(manager);
        {
            Scope::StatsScope scope(thread.template getRecordManager<Record::Record>(), "test");
        }
        ThreadArray threadArray(1);
        auto& holder = threadArray[0];
        {
            auto lk = holder.lock();
            BOOST_REQUIRE(!holder.thread);
            holder.thread = &thread;
        }
        BufferMap buffers;
        Writer writer(Output::Ptr(new MemoryOut(buffers, "test")), threadArray, std::chrono::microseconds(100000));
        std::thread writerThread{[&writer](){ writer.run(); }};
        writer.stop();
        writerThread.join();
        BOOST_REQUIRE(buffers["test"].get());
        BOOST_CHECK(0 < buffers["test"]->str().size());
    }

    BOOST_AUTO_TEST_SUITE_END()
}
}
