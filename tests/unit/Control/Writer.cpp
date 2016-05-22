#include <Control/Thread.h>
#include <Control/Writer.h>
#include <Instrumentation/StatsScope.h>
#include <boost/test/unit_test.hpp>
#include <cassert>
#include <unordered_map>

namespace Control { namespace Test
{

namespace
{

    using RecordStorageType = Record::RecordStorage<Record::Record>;

    struct MockManager
    {
        static constexpr std::size_t NumRecords = 10;
        ThreadHolder* addThread(Thread& thread_)
        {
            return nullptr;
        }
        RecordStorageType& getRecordStorage()
        {
            return _recordStorage;
        }
      private:
        RecordStorageType _recordStorage{NumRecords};
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
        Writer writer(Output::Ptr(new MemoryOut(buffers, "test")), threadArray);
        writer.run();
        BOOST_REQUIRE(buffers["test"].get());
        BOOST_CHECK(0 < buffers["test"]->str().size());
    }

}
}
