#pragma once

#include "bsp/native_resource_extra_item_parsers.hpp"
#include "bsp/native_resource_value_reads.hpp"

namespace bsp {

// Complete B8AF30..B8B04C (285 bytes). Original: ECX actual 2Ch Bone item,
// stacked one-pointer structured-node handle, RET4; EDX is not an argument.
// Read a native string into an uninitialized temporary, copy it into item+08,
// and return the temporary before reading seven floats. Every ST0 result is
// spilled to float32; all seven item+10..+28 stores follow the seventh read.
// The item and its copied name survive a later read failure. No defaults or
// item ownership rollback are introduced. Services operate on actual storage.
void read_native_bone_resource_item_00b8af30(void* actual_item,
    void* actual_handle, NativeResourceStreamReadContext&);

// Resolve only the exact captured Bone slot20 target. Every other captured
// target is forwarded unchanged to the required remaining reader service.
class NativeBoneResourceItemReaderCalls final : public NativeResourceExtraItemReaderCalls {
public:
    NativeBoneResourceItemReaderCalls(NativeResourceStreamReadContext& context,
        NativeResourceExtraItemReaderCalls& remaining) noexcept
        : context_(context), remaining_(remaining) {}

    void read_item(std::uintptr_t captured_target, void* actual_item,
        void* actual_handle) override;

private:
    NativeResourceStreamReadContext& context_;
    NativeResourceExtraItemReaderCalls& remaining_;
};

// New MSVC Win32 source ABI. Original FH3/SEH, private stack addresses,
// hardware-fault delivery, register/EFLAGS identity and gameplay are unproven.
} // namespace bsp
