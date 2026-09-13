#pragma once

#include "bsp/native_mpak_provider.hpp"
#include "bsp/native_string_vector.hpp"

namespace bsp {

// BB4F40 is retained as an explicit original container/search boundary.
// ECX first14h row, EDX captured end, stack temporary14h key, EAX row/end,
// RET4. Walk rows in order; 5EFBA0 searches each row's actual0Ch member
// vector at +8 for key's name at +0. Match requires equal recorded lengths,
// then empty/empty or CRT _stricmp==0. Return first nonnegative member index.
// No comparison with the directory's own name is made. No fallback is supplied.
class NativeMpakDirectorySearchLibrary {
public:
    virtual ~NativeMpakDirectorySearchLibrary() = default;
    virtual void* find_member_directory_00bb4f40(void* captured_first,
        void* captured_end, const void* actual_temporary_directory) = 0;
};

// Full BEE340[80]: ECX original prefix8h, EDX extension8h, stack flags,
// candidate8h; AL Boolean, RET8. Reuses the existing actual string helpers.
// No bounded string_view conversion, normalization or input validation.
bool matches_native_provider_enumeration_00bee340(const void* actual_prefix,
    const void* actual_extension, std::uint32_t flags,
    const void* actual_candidate, const char* null_pattern_00e17bf0);

// Full BB5F40[390]: ECX provider44h, stack original prefix8h/extension8h/
// flags/output0Ch, RET10h. Build and discard a slash-appended prefix copy,
// but filter with the ORIGINAL prefix. Scan current24h file rows, append full
// names to existing output. Keep the original diagnostic and cleanup order.
void append_native_mpak_names_00bb5f40(void* actual_provider,
    const void* actual_prefix, const void* actual_extension, std::uint32_t flags,
    NativeStringVectorStorage& actual_output, NativeMpakDirectoryContext&,
    const char* null_pattern_00e17bf0);

// Full BB68F0[265]: ECX provider44h, stack input8h/output8h, AL Boolean,
// RET8. Copy input into temporary14h directory, search member lists through
// the above library boundary, destroy temporary, then validate current vector.
// Success publishes found row at provider+3C BEFORE copying row name to output.
// Failure preserves provider+3C and output. Original has no EH cleanup frame;
// a throwing search does not clean up the temporary name.
bool select_native_mpak_member_directory_00bb68f0(void* actual_provider,
    const void* actual_input, void* actual_output, NativeMpakDirectoryContext&,
    NativeMpakDirectorySearchLibrary&);

// Genuine default bodies shared through D641F8; ECX is unused.
// BB40C0[5]: AL=0, RET4 (one ignored stack argument).
bool reject_native_mpak_probe_00bb40c0(const void* ignored_name) noexcept;
// BB79E0[5]: AL=0, RET10h (four ignored stack words).
bool reject_native_mpak_operation_00bb79e0(std::uint32_t, std::uint32_t,
    std::uint32_t, std::uint32_t) noexcept;
// BB79F0[1]: RET, no arguments and no stable return value.
void noop_native_mpak_default_00bb79f0() noexcept;
// BB7A00[23]: stack output and ignored word; EAX output, RET8.
// Zero exactly14h (20 DECIMAL) bytes in descending DWORD order +10h..+0.
void* clear_native_mpak_result_00bb7a00(void* actual_output,
    std::uint32_t ignored) noexcept;

// All interfaces are new source APIs. The caller supplies actual compatible
// Win32 storage and one canonical string/pool lifetime domain. Original FH3,
// binary ABI, search-library implementation and installed/game paths are not
// established by this source reconstruction.
} // namespace bsp
