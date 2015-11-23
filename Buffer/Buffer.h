#include <array>


namespace Buffer {

    template <typename Record_, std::size_t Size_>
    struct Buffer
    {
        using Record = Record_;
        using Size = Size_ * sizeof(Record);

        // emplace_back

      private:
        bool _isFull;
        std::size_t _current;
        std::array<Record, Size> _buffer;
    };

}
