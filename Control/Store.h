#ifndef CONTROL_RECORD_H
#define CONTROL_RECORD_H

#include <iostream>

namespace Control
{

    // Heavily inspired by:
    // https://raw.githubusercontent.com/wiki/sean-parent/sean-parent.github.io/presentations/2013-03-06-value_semantics/value-semantics.cpp

    template <typename T_>
    void store(const T_& storable_, std::ostream& out_)
    {
        // TODO: ...
    }

    struct StorableHolder
    {
        template <typename T_>
        StorableHolder(const T_* storable_)
            : _self(new Model<T_>(storable_))
        { }

        friend void store(const StorableHolder& storableHolder_, std::ostream& out_)
        {
            storableHolder_._self->storeImpl(out_);
        }

      private:
        struct Concept
        {
            virtual ~Concept() = default;
            virtual void storeImpl(std::ostream& out_) const = 0;
        };

        template <typename T_>
        struct Model: Concept
        {
            using Type = T_;
            Model(const Type* storable_)
                : storable(storable_)
            { }

            void storeImpl(std::ostream& out_) const override
            {
                store(storable, out_);
            }

            const Type* storable;
        };

        const Concept* _self;
    };
}

#endif
