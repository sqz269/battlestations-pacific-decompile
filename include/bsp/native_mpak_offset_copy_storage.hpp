#pragma once
#include "bsp/native_mpak_record_copy.hpp"

namespace bsp {
// Source Win32 adapter for BB6180's actual 10h allocator/vector header.
// This is not a binary-compatible implementation of the original FH3 ABI.
class NativeMpakOffsetCopyStorage final : public NativeMpakOffsetVectorCopyLibrary {
public:
    // ECX destination, stack source; EAX destination, RET4 in the original.
    // Destination+0 remains untouched. The allocated backing belongs to the
    // caller's offset-vector lifetime, including after a returning diagnostic.
    void* copy_00bb6180(void* actual_destination,
        const void* actual_source) override;
};
} // namespace bsp
