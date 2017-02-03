#include <Profiler/Control/ThreadRecords.h>
#include <Profiler/Control/Writer.h>
#include <Profiler/Instrumentation/StatsScope.h>
#include <boost/test/unit_test.hpp>
#include <cassert>
#include <thread>
#include <unordered_map>

namespace Profiler { namespace Control { namespace Test
{

namespace
{

    using BufferMap = std::unordered_map<std::string, std::string>;

    struct MemoryOut : Output
    {
        MemoryOut(BufferMap& buffers_, const char* name_)
          : _buffers(buffers_), _name(name_)
        { }
        ~MemoryOut()
        {
            _buffers[_name] = _out.str();
        }
        virtual std::ostream& get()
        {
            return _out;
        }
        virtual void flush()
        { }
      private:
        BufferMap& _buffers;
        const char* _name;
        std::stringstream _out;
    };

    struct MockOutputs : OutputFactory
    {
        virtual Output::Ptr newOutput(std::size_t extractorId_) const override
        {
            return std::make_unique<MemoryOut>(buffers, "test");
        }
        mutable BufferMap buffers;
    };

}

    BOOST_AUTO_TEST_SUITE(WriterTests)

    BOOST_AUTO_TEST_CASE(Basic)
    {
        Arena arena{100000};
        HolderArray holderArray{1};
        MockOutputs outputs;
        {
            holderArray[0].setOut(outputs.newOutput(0));
            ThreadRecords<Record::TimeRecord> threadRecords(Allocation({}, arena, holderArray[0]));
            {
                Record::TimeRecord record("test");
                record.finish();
                Scope::record(threadRecords.getRecordManager(), std::move(record));
            }
        }
        Writer writer(holderArray, std::chrono::microseconds(100000));
        std::thread writerThread{[&writer](){ writer.run(); }};
        writer.stop();
        writerThread.join();
        BOOST_REQUIRE(outputs.buffers.find("test") != outputs.buffers.end());
        BOOST_CHECK(0 < outputs.buffers["test"].size());
    }

    BOOST_AUTO_TEST_SUITE_END()
}
}
}
