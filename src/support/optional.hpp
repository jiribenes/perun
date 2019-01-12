#ifndef PERUN_SUPPORT_OPTIONAL_HPP
#define PERUN_SUPPORT_OPTIONAL_HPP

namespace perun {
namespace support {

#include <cassert>

template <typename T>
/// Basic optional storage
class Optional {
public:
    constexpr explicit Optional(T&& value)
        : value(std::move(value)), _hasValue(true) {}
    constexpr explicit Optional() : _hasValue(false) {}

    /// Returns a const pointer to the value inside or nullptr if it is empty
    const T* get() const {
        if (hasValue()) {
            return &value;
        }

        return nullptr;
    }

    /// Asserts there is a value and returns a const reference to the value
    /// inside
    const T& getValue() const {
        assert(hasValue());
        return value;
    }

    /// Asserts there is a value and moves it out
    T&& moveValue() {
        assert(hasValue());
        return std::move(value);
    }

    bool hasValue() const { return _hasValue; }

private:
    T value;
    bool _hasValue = false;
};

} // namespace support
} // namespace perun

#endif // PERUN_SUPPORT_OPTIONAL_HPP
