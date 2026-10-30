#pragma once

#include <cstdint>

namespace bsp {
// Borrowed typed Source service, not a Native vtable or an owning/erased
// callback. Only the real retained locale owner implements this interface.
class NativeLocaleManagerDeletionLifetime {
public:
    NativeLocaleManagerDeletionLifetime(const NativeLocaleManagerDeletionLifetime&) = delete;
    NativeLocaleManagerDeletionLifetime& operator=(const NativeLocaleManagerDeletionLifetime&) = delete;
    virtual void* delete_registered_current_profile(void* captured_receiver,
        std::uint32_t captured_profile, const volatile std::uint32_t& actual_flags) = 0;
protected:
    NativeLocaleManagerDeletionLifetime() = default;
    ~NativeLocaleManagerDeletionLifetime() = default;
};
} // namespace bsp
