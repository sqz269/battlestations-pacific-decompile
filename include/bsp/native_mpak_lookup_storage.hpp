#pragma once
#include "bsp/native_mpak_open.hpp"
#include "bsp/native_mpak_enumeration.hpp"
#include "bsp/singleton_lifetime.hpp"
namespace bsp {
// Borrow actual vector storage and the returning invalid-parameter callback.
class NativeMpakLookupStorage final : public NativeMpakOpenLibrary,
                                      public NativeMpakDirectorySearchLibrary {
public:
    explicit NativeMpakLookupStorage(const SingletonLifetimeCallbacks& invalid_parameters) noexcept
        : invalid_parameters_(invalid_parameters) {}
    void* file_at_00bb4140(void* actual_vector, std::uint32_t index) override;
    void* find_member_directory_00bb4f40(void* captured_first, void* captured_end,
        const void* actual_temporary_directory) override;
private:
    const SingletonLifetimeCallbacks& invalid_parameters_;
};
} // namespace bsp
