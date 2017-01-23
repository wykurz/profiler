#ifndef ALGORITHMS_STREAMS_H
#define ALGORITHMS_STREAMS_H

#include <Profiler/Exception/Exception.h>
#include <Profiler/Log/Log.h>
#include <cstring>
#include <iostream>
#include <vector>

namespace Profiler { namespace Algorithm
{

    template <typename T_>
    T_ decode(std::istream& in_)
    {
        T_ val;
        in_.read(reinterpret_cast<char*>(&val), sizeof(T_));
        return val;
    }

    template <typename T_>
    void encode(std::ostream& out_, T_ value_)
    {
        out_.write(reinterpret_cast<const char*>(&value_), sizeof(value_));
    }

    inline std::string decodeString(std::istream& in_)
    {
        auto length = decode<std::size_t>(in_);
        DLOG("Read length: " << length << ", currently at: " << in_.tellg());
        std::vector<char> bytes(length);
        in_.read(bytes.data(), length);
        return std::string(bytes.data(), bytes.data() + length);
    }

    inline void encodeString(std::ostream& out_, const char* str_)
    {
        encode(out_, strlen(str_));
        out_ << str_;
    }

}
}

#endif
