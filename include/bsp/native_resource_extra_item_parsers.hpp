#pragma once

#include "bsp/native_resource_root_dispatch.hpp"
#include <cstddef>
#include <cstdint>

namespace bsp {

inline constexpr std::uint32_t kNativeAnimationChannelsItemProfile = 0x00d6328cu;
inline constexpr std::uint32_t kNativeBoneItemProfile = 0x00d632b8u;

// Complete native allocation extents. AnimationChannels initializes every
// payload DWORD. Bone initializes only +08/+0C; +10..+2B remain allocator data
// until the current D632B8 slot20 reader writes them.
struct alignas(4) NativeAnimationChannelsItemStorage {
    std::uint32_t native_profile_00;
    std::int32_t reference_count_04;
    std::uint32_t field_08;
    std::uint32_t field_0c;
    std::uint32_t field_10;
};
static_assert(sizeof(NativeAnimationChannelsItemStorage) == 0x14);

struct alignas(4) NativeBoneItemStorage {
    std::uint32_t native_profile_00;
    std::int32_t reference_count_04;
    std::uint32_t field_08;
    std::uint32_t field_0c;
    std::byte reader_owned_10[0x1c];
};
static_assert(sizeof(NativeBoneItemStorage) == 0x2c);

// Required concrete dependency for the original current-table slot20 call.
// The implementation must execute the supplied captured target with native
// semantics; there is no fallback reader or default field initialization here.
class NativeResourceExtraItemReaderCalls {
public:
    virtual ~NativeResourceExtraItemReaderCalls() = default;
    virtual void read_item(std::uintptr_t captured_target, void* actual_item,
        void* actual_handle) = 0;
};

// Complete B8A910/B8A990 outer bodies with a new explicit-service C++ ABI.
// Original ABI for each is parser ECX (unused), one stacked handle, EAX item,
// RET4. Allocation cleanup is disarmed before the current result-table slot20
// call; a reader failure retains the allocated, reference-counted item.
void* parse_native_animation_channels_item_00b8a910(
    void* actual_handle, NativeResourceExtraItemReaderCalls&);
void* parse_native_bone_item_00b8a990(
    void* actual_handle, NativeResourceExtraItemReaderCalls&);

// Extend the existing resource-root parser dispatch for the two exact parser
// targets. Renderer, append and every other parser target forward unchanged.
class NativeResourceExtraItemParserCalls final : public NativeResourceDispatchCalls {
public:
    NativeResourceExtraItemParserCalls(NativeResourceDispatchCalls& remaining,
        NativeResourceExtraItemReaderCalls& readers) noexcept
        : remaining_(remaining), readers_(readers) {}

    void renderer_hook(std::uintptr_t captured_target, void* renderer) override;
    void* parse_item(std::uintptr_t captured_target, void* actual_parser,
        void* actual_handle) override;
    void append_item(std::uintptr_t captured_target, void* actual_resource,
        void* actual_item) override;

private:
    NativeResourceDispatchCalls& remaining_;
    NativeResourceExtraItemReaderCalls& readers_;
};

// Requires the application's mapped D6328C/D632B8 profiles and source CRT
// allocation. The item readers B8AD80/B8AF30, deleting paths, original FH3/SEH,
// hardware-fault behavior and gameplay are separate evidence boundaries.
} // namespace bsp
