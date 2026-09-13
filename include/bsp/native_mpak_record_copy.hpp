#pragma once

#include "bsp/native_string.hpp"

namespace bsp {

// Original STL vector<DWORD> copy-constructor boundary, not reconstructed STL.
// ECX destination10h; stack source10h; EAX destination; RET4. Operate on actual
// allocator/begin/end/capacity storage, preserve allocator+0, and propagate
// failures. Implementations must supply the compatible library operation;
// there is no successful fallback. Evidence: NATIVE_MPAK_CONTAINER_BA_DISCOVERY.
class NativeMpakOffsetVectorCopyLibrary {
public:
    virtual ~NativeMpakOffsetVectorCopyLibrary() = default;
    virtual void* copy_00bb6180(void* actual_destination,
        const void* actual_source) = 0;
};

// Full BB65A0..BB662F[144]. Original ECX destination24h, stack source24h;
// EAX destination, RET4. Clear name before self-alias test, copy current name
// fields, DWORDs8/C and byte10, preserve padding11..13 and allocator14..17,
// then call the explicit offset-vector library copy on both records+14h.
// Only the name is armed for cleanup, immediately before that nested call.
void* copy_construct_native_mpak_file_00bb65a0(void* actual_destination,
    const void* actual_source, NativeStringStorage&,
    NativeMpakOffsetVectorCopyLibrary&);

// Full BB6630..BB66AC[125]. Same original ABI for 14h records. Clear/copy name,
// arm name-only cleanup, zero the custom0Ch member vector at +8 and perform
// actual543E50. No nested-vector rollback on failure; initial name-copy
// failure has no armed owner. These C++ APIs are not native FH3/SEH shims.
void* copy_construct_native_mpak_directory_00bb6630(void* actual_destination,
    const void* actual_source, NativeStringStorage&);

} // namespace bsp
