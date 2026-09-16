#pragma once

#include "bsp/native_animation_channels_resource_reader.hpp"

namespace bsp {
// Actual 28h channel. +10 is not initialized by B8AAD0; its meaning is unknown.
// Key +8 drives group +1C; the remaining scalar meanings are not established.
struct NativeAnimationChannelStorage {
    std::uint32_t profile_00;
    std::int32_t references_04;
    std::uint32_t name_length_08, name_data_0c, unknown_10;
    std::uint32_t field_14, field_18, keys_data_1c;
    std::int32_t keys_count_20, keys_capacity_24;
};
struct NativeAnimationKeyStorage {
    std::uint32_t field_00;
    float fields_04_to_24[9];
};
static_assert(sizeof(NativeAnimationChannelStorage) == 0x28);
static_assert(sizeof(NativeAnimationKeyStorage) == 0x28);

// Header is actual data/count/capacity0Ch, usually channel+1C. Growth copies
// ten DWORDs per key, then frees old data before publishing data/capacity.
void reserve_native_animation_keys_00b76680(void* header, std::int32_t requested);
void append_native_animation_key_00b77870(void* actual_channel, const void* key);
// Original ECX item (unused), stacked handle/channel, RET8. The source omits
// unused item; nine scalar reads each have exactly one following FSTP32.
void read_native_animation_key_00b8a0a0(void* handle, void* actual_channel,
    NativeResourceStreamReadContext&);
// Original ECX group, stack index/channel, RET8; returns last-key+8 address.
// Exact FLD/FSTP32/FCOMIP/JBE/MOVSS schedule, including NaNs and signed zero.
void* __cdecl publish_native_animation_channel_00b771c0(void* actual_group,
    std::uint32_t index, void* actual_channel);
void read_native_animation_channel_00b8aad0(void* actual_item, void* handle,
    void* captured_group, NativeResourceStreamReadContext&);

class NativeAnimationChannelBodyReader final : public NativeAnimationChannelBodyCalls {
public:
    explicit NativeAnimationChannelBodyReader(NativeResourceStreamReadContext& context) noexcept
        : context_(context) {}
    void read_channel_00b8aad0(void* actual_item, void* child_handle,
        void* captured_group) override;
private:
    NativeResourceStreamReadContext& context_;
};
// Explicit-service MSVC Win32 source interface, not original ABI/FH3/SEH.
// No name/index/group validation or channel rollback is added. Live storage,
// native current stream targets, x87 stack space and clear DF are required.
} // namespace bsp
