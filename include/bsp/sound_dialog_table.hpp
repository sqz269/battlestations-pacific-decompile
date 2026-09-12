#pragma once
#include "bsp/native_string.hpp"
#include <array>
#include <cstdint>

namespace bsp {
class SoundSampleHost;
// Actual20h table layout is produced by A79230: reference header0/4,
// actual14h record vector8/C/10, volume14, total channels18, loop byte1C.
// Records contain a native string0/4, count8, first-channelC, format10.
struct SoundDialogTableContext {
    NativeStringStorage& strings;
    SoundSampleHost& scanner;
    const std::array<std::uint32_t, 4>& format_counts_00e12ef0;
    // Native temporary+C is an unwritten stack word. It is copied and then
    // overwritten in the appended row before any subsequent external call.
    std::uint32_t temporary_first_channel_word;
};

// ECX actual14h; RET. Release only the captured name; leave header unchanged.
void destroy_sound_dialog_record_00a77ea0(void*, NativeStringStorage&) noexcept;
// ECX destination14h, stack source14h; EAX destination, RET4. Self-copy clears
// the name before comparing and does not release its previous allocation.
void* copy_sound_dialog_record_00a77ef0(void*, const void*, NativeStringStorage&,
    const std::array<std::uint32_t, 4>& format_counts);
// ECX actual0Ch vector header, stack signed capacity; RET4. Growth deep-copies
// forward then releases old names forward. Failed copies leak the new prefix,
// as the native reserve EH map has only a no-op placement delete.
void reserve_sound_dialog_records_00a78dc0(void*, std::int32_t,
    NativeStringStorage&, const std::array<std::uint32_t, 4>& format_counts);
// ECX actual20h table, stack filename8h; RET4. Append, do not reset. Unknown
// top-level input makes no progress, matching native behavior. Requires a
// terminating, well-formed scanner input and valid native storage/format0..3.
// Scanner remains a value projection over actual VFS bytes, not an828h ABI.
void load_sound_dialog_table_00a87060(void*, const NativeString&,
    SoundDialogTableContext&);
} // namespace bsp
