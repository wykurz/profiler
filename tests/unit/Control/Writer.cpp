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

    struct MockOutputs : OutputFactory
    {
        virtual Output::Ptr newOutput(Holder::Id extractorId_, Record::TypeId recordTypeId_) const override
        {
            return Output::Ptr();
        }
    };

    struct MockManager
    {
        Allocation addThreadRecords()
        {
            return {{}, arena, _scratchHolder, _outputs};
        }
        Arena arena{100000};
      private:
        MockOutputs _outputs;
        Holder _scratchHolder;
    };

    using BufferMap = std::unordered_map<std::string, std::stringstream*>;

    struct MemoryOut : Output
    {
        MemoryOut(BufferMap& buffers_, const char* name_)
        {
            buffers_[name_] = &_out;
        }
        virtual std::ostream& get()
        {
            return _out;
        }
        virtual void flush()
        { }
      private:
        std::stringstream _out;
    };

}

    BOOST_AUTO_TEST_SUITE(WriterTests)

    BOOST_AUTO_TEST_CASE(Basic)
    {
        MockManager manager;
        ThreadRecords<Record::TimeRecord> threadRecords(manager.addThreadRecords());
        {
            std::chrono::duration<double> timeDelta{0};
            Scope::record(threadRecords.getRecordManager(), Record::TimeRecord("test", timeDelta));
        }
        HolderArray threadArray(1);
        auto& holder = threadArray[0];
        BufferMap buffers;
        {
            auto lk = holder.lock();
            holder.setup(threadRecords.getRecordManager(), Output::Ptr(new MemoryOut(buffers, "test")));
        }
        Writer writer(threadArray, std::chrono::microseconds(100000));
        std::thread writerThread{[&writer](){ writer.run(); }};
        writer.stop();
        writerThread.join();
        BOOST_REQUIRE(buffers["test"]);
        BOOST_CHECK(0 < buffers["test"]->str().size());
    }

    BOOST_AUTO_TEST_SUITE_END()
}
}
}
