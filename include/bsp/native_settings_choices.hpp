#pragma once
#include "bsp/native_string.hpp"
#include <cstddef>
#include <cstdint>

namespace bsp {
struct NativeSettingsChoiceCalls {
    virtual ~NativeSettingsChoiceCalls()=default;
    virtual void resize_dwords_0086a430(void*,std::int32_t);
    virtual void reserve_dwords_0086a220(void*,std::int32_t);
    virtual void reserve_pairs_008d4750(void*,std::int32_t);
    // Direct native BF7FBF boundary. Default uses linked host CRT _stricmp;
    // original CRT locale/invalid-input behavior is not established here.
    virtual int compare_language_00bf7fbf(const char*,const char*);
};
// Full 8D4DF0/8D4EA0 normal bodies: ECX actual 12-byte target header, stack
// source header, EAX target, RET4. These source interfaces add explicit calls.
// Headers are data/count/capacity; rows have stride4/8. The existing complete
// reserves use the actual shared CRT allocator. Self-assignment clears count.
// Source rows are captured before growth; current headers/counts are reloaded
// at native points. Pair word copies remain ordered even for overlapping rows.
void* assign_native_settings_dwords_008d4df0(void*,const void*,NativeSettingsChoiceCalls&);
void* assign_native_settings_pairs_008d4ea0(void*,const void*,NativeSettingsChoiceCalls&);

struct NativeSettingsLanguageContext {
    NativeStringRawPoolContext& strings;
    NativeSettingsChoiceCalls& calls;
    void* const volatile& actual_catalog_00f88974;
    const volatile std::uint32_t& actual_count_00f88978;
};
// Caller-owned storage for the native temporary string and retained failure
// state. Its raw string header has no C++ destructor. No implicit rollback.
struct NativeSettingsLanguageOperation final {
    enum class Phase { fresh,running,complete,failed,diagnostic_retired };
    Phase phase{Phase::fresh};
    alignas(4) std::byte temporary_string[8];
    char* captured_data{};
    std::uint32_t captured_length{};
    std::uint32_t native_site{};
    NativeSettingsLanguageOperation()=default;
    ~NativeSettingsLanguageOperation();
    NativeSettingsLanguageOperation(const NativeSettingsLanguageOperation&)=delete;
    NativeSettingsLanguageOperation& operator=(const NativeSettingsLanguageOperation&)=delete;
    // Caller must resolve retained allocation before acknowledging; no release.
    void acknowledge_diagnostic_cleanup() noexcept;
};
// Full 8D56C0: ECX actual settings owner, stack nonnull C string, RET4, native
// void. Construct an actual pooled temporary; compare first string of 20h rows
// only when lengths match. First hit writes owner+04, miss writes0. Capture
// table once then reload after EACH CRT compare; reread signed count at loop
// tests. Finally return captured temporary data/length through actual raw pool.
void select_native_settings_language_008d56c0(void*,const char*,NativeSettingsLanguageContext&,NativeSettingsLanguageOperation&);
} // namespace bsp
