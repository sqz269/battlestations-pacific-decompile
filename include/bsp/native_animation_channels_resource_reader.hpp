#pragma once

#include "bsp/native_resource_extra_item_parsers.hpp"
#include "bsp/native_resource_stream_reads.hpp"

namespace bsp {

// Actual group20h. Numeric D62ED4 profile requires the application's mapped
// profile domain; no host vtable or second owner is installed in these bytes.
struct NativeAnimationChannelGroupStorage {
    std::uint32_t profile_00;
    std::int32_t references_04;
    std::uint32_t name_length_08;
    std::uint32_t name_data_0c;
    std::uint32_t channels_data_10;
    std::int32_t channels_count_14;
    std::int32_t channels_capacity_18;
    float field_1c;
};
static_assert(sizeof(NativeAnimationChannelGroupStorage) == 0x20);

// Actual raw pointer-vector header: data0/count4/capacity8. No retain/release
// of pointed channels. Signed comparisons and wrapping DWORD addressing.
void reserve_native_animation_channel_pointers_00b76710(void*, std::int32_t);
void resize_native_animation_channel_pointers_00b774e0(void*, std::int32_t);
void destroy_native_animation_channel_pointers_00b77a10(void*);
void* construct_native_animation_channel_group_00b78c40(
    void* actual_group, const void* actual_name, NativeStringRawPoolContext&);

// Required concrete B8AAD0 body. Original ECX is the actual AnimationChannels
// item; stacked arguments are actual child HANDLE ADDRESS and captured group
// (possibly null, including a ChannelAnimation before any AnimationGroupName).
// This operation must parse/publish the channel with native ownership and
// exception behavior. There is no synthetic channel or missing-group fallback.
class NativeAnimationChannelBodyCalls {
public:
    virtual ~NativeAnimationChannelBodyCalls() = default;
    virtual void read_channel_00b8aad0(void* actual_item, void* child_handle,
        void* captured_group) = 0;
};

// Complete B8AD80 control/storage/lifetime schedule, explicit-service source
// ABI. Native ECX item14h, stacked handle, RET4. Reuses actual node/string/
// vector services. Item+8/+C/+10 are borrowed data/count/capacity words.
void read_native_animation_channels_00b8ad80(void* actual_item,
    void* actual_handle, NativeResourceStreamReadContext&,
    NativeAnimationChannelBodyCalls&);

// Intercepts precisely the captured B8AD80 target; every other current slot20
// entry forwards unchanged through the required remaining reader service.
class NativeAnimationChannelsReaderCalls final : public NativeResourceExtraItemReaderCalls {
public:
    NativeAnimationChannelsReaderCalls(NativeResourceExtraItemReaderCalls& remaining,
        NativeResourceStreamReadContext& context, NativeAnimationChannelBodyCalls& channels) noexcept
        : remaining_(remaining), context_(context), channels_(channels) {}
    void read_item(std::uintptr_t target, void* item, void* handle) override;
private:
    NativeResourceExtraItemReaderCalls& remaining_;
    NativeResourceStreamReadContext& context_;
    NativeAnimationChannelBodyCalls& channels_;
};

// Requires live native spans, valid vector extents and concrete current stream/
// reference targets. Original ABI/FH3/SEH and game execution are not established.
} // namespace bsp
