#include <Control/Thread.h>
#include <Control/Writer.h>
#include <boost/test/unit_test.hpp>
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
            return *_out;
        }
      private:
        MemBuffer _out;
    };

}

    BOOST_AUTO_TEST_CASE(Basic)
    {
        constexpr std::size_t numThreads = 3;
        MockManager manager;
        Thread thread(manager);
        ThreadArray threadArray(numThreads);
        // TODO: temp dir!
        BufferMap buffers;
        Writer writer(Output::Ptr(new MemoryOut(buffers, "test")));
        writer.run(threadArray);
        // BOOST_CHECK_EQUAL(3, size);
    }

}
}
