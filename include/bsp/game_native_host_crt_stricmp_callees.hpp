#pragma once

#include "bsp/native_crt_stricmp.hpp"

namespace bsp::game {

// Stateless services in the current MSVC Win32 /MD host CRT domain. The Root
// owns this provider through every borrower/callback; each endpoint uses the
// reached caller thread. No errno, locale, handler or Native owner is cached.
// This is not an Original static-CRT/TLS/private-ABI equivalence claim.
class GameNativeHostCrtStricmpCallees final : public NativeCrtStricmpCallees {
public:
    GameNativeHostCrtStricmpCallees() = default;
    ~GameNativeHostCrtStricmpCallees() override = default;
    GameNativeHostCrtStricmpCallees(const GameNativeHostCrtStricmpCallees&) = delete;
    GameNativeHostCrtStricmpCallees& operator=(const GameNativeHostCrtStricmpCallees&) = delete;

    // Forward the genuine current API result, including its no-PTD policy.
    volatile std::int32_t* errno_cell_00bffb8b() override;
    // Only the existing parent's five-zero domain is admitted. Preserve the
    // actual returning-handler/default policy; no catch or forced return.
    void invalid_parameter_00bf66ef(std::uint32_t expression, std::uint32_t function,
        std::uint32_t file, std::uint32_t line, std::uint32_t reserved) override;
    // Only a null opaque locale is admitted. Both string pointers, including
    // null strings, pass unchanged to the real host CRT validation/policy.
    std::int32_t compare_locale_00bf7eec(
        const char* left, const char* right, void* actual_locale) override;
};

} // namespace bsp::game
