#pragma once

#include "bsp/voice_playback.hpp"

namespace bsp {
// Semantic container: native owner+4 has a checked MSVC tree with key at
// node+0C and the canonical 40h record at node+14. References stay borrowed
// from this store. This does not project the native node or iterator ABI.
using MessageRecordStore = std::map<NativeString, VoiceClipRecord, PanelSequenceNameLess>;
struct MessageRecordIterator {
    MessageRecordStore* owner;
    MessageRecordStore::iterator position;
};
// Native constructors leave these bytes untouched. The integration supplies
// their actual scratch values; zero/default colors or flags are not inferred.
struct MessageRecordScratch {
    std::uint32_t sound_word_08;
    std::uint8_t alternate_14;
    std::uint32_t key_start_word_10;
    std::uint32_t key_end_word_14;
    std::uint8_t key_flag_18;
};
struct MessageRecordResolverContext {
    NativeStringStorage& strings;
    VoiceLineHost& playback;
    MessageRecordScratch scratch;
};

// Native ECX=map, name* stack, RET4; and ECX=map, result*/name*, RET8.
MessageRecordStore::iterator lower_bound_message_record_00702300(
    MessageRecordStore&, const NativeString&);
MessageRecordIterator find_message_record_00702910(MessageRecordStore&, const NativeString&);
// Native constructor ECX=fresh40h record, RET; preserves sound08/alternate14.
VoiceClipRecord& construct_message_record_00703ff0(VoiceClipRecord&);
// ECX=fresh record, source* stack, RET4. Exactly four intrusive slots.
VoiceClipRecord& copy_construct_message_record_007042b0(VoiceClipRecord&,
    const VoiceClipRecord&, MessageRecordResolverContext&);
// ECX=fresh reference cell, source-cell* stack, RET4; returns destination cell.
void*& copy_message_resource_reference_005b7dc0(void*& destination,
    void* const& source, VoiceLineHost&);
// Native plain destructor, RET: timed keys, four resources backwards, three
// strings backwards. No record allocation is freed and no string is reset.
void destroy_message_record_00704060(VoiceClipRecord&, MessageRecordResolverContext&);
void destroy_message_key_007029b0(VoiceTimedKey&, NativeStringStorage&);
VoiceTimedKey& copy_construct_message_key_00703170(VoiceTimedKey&,
    const VoiceTimedKey&, NativeStringStorage&);
// Native ECX=vector, count and an owned BY-VALUE 1Ch key, RET20h. The C++
// argument is moved in; it is always destroyed, including unchanged size.
void resize_message_keys_00704190(VoiceTimedKeys&, std::uint32_t,
    VoiceTimedKey fill, NativeStringStorage&);
void destroy_message_keys_00703b90(VoiceTimedKeys&, NativeStringStorage&);
// ECX=left, destination*/right* stack, RET8. Constructor, even on alias.
NativeString& concatenate_message_string_004261a0(const NativeString&,
    NativeString& destination, const NativeString&, NativeStringStorage&);
// Native map operator[] and owning resolver, respectively RET4 and RET8.
// The second resolver word is unused. Existing entries are never rewritten.
VoiceClipRecord& lookup_message_record_00705c50(MessageRecordStore&,
    const NativeString&, MessageRecordResolverContext&);
VoiceClipRecord& resolve_message_record_00705e00(MessageRecordStore&,
    const NativeString&, std::uint32_t unused, MessageRecordResolverContext&);
// Host lifetime convenience, not an additional recovered native routine.
void clear_message_record_store(MessageRecordStore&, MessageRecordResolverContext&);
} // namespace bsp
