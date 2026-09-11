#include "bsp/message_record_resolver.hpp"

#include "bsp/native_pooled_string_substring.hpp"

#include <cstring>
#include <new>
#include <stdexcept>
#include <utility>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Message record reconstruction requires MSVC Win32 x87 operations.
#endif

namespace bsp {
namespace {
void set_word(float& value, std::uint32_t bits) noexcept {
    std::memcpy(&value, &bits, sizeof bits);
}
void copy_float(float& destination, const float& source) noexcept {
    auto* target = &destination;
    auto* input = &source;
    __asm {
        mov eax, input
        fld dword ptr [eax]
        mov eax, target
        fstp dword ptr [eax]
    }
}
float message_duration(std::uint32_t count, float per_character, float base) noexcept {
    const float unsigned_bias = 4294967296.0f; //00CE3978
    float result;
    __asm {
        fld per_character
        fild count
        test count, 80000000h
        jz nonnegative_count
        fadd unsigned_bias
    nonnegative_count:
        fmulp st(1), st(0)
        fadd base
        fstp result
    }
    return result;
}
void assign_string(NativeString& to, const NativeString& from, NativeStringStorage& strings) {
    if (&to == &from) return;
    to.resize_0041dd40(strings, from.length(), true);
    if (from.length()) std::memcpy(to.data(), from.data(), to.length());
}
struct StringOwner {
    NativeString value;
    NativeStringStorage& strings;
    explicit StringOwner(NativeStringStorage& s) : strings(s) {}
    ~StringOwner() { destroy_native_string_header_0041dd20(&value, strings); }
};
struct KeyOwner {
    VoiceTimedKey value;
    NativeStringStorage& strings;
    explicit KeyOwner(NativeStringStorage& s) : strings(s) {}
    KeyOwner(VoiceTimedKey&& from, NativeStringStorage& s)
        : value(std::move(from)), strings(s) {}
    ~KeyOwner() { destroy_message_key_007029b0(value, strings); }
};
struct RecordOwner {
    VoiceClipRecord value;
    MessageRecordResolverContext& context;
    explicit RecordOwner(MessageRecordResolverContext& c) : context(c) {}
    ~RecordOwner() { destroy_message_record_00704060(value, context); }
};
void copy_keys(VoiceTimedKeys& to, const VoiceTimedKeys& from, NativeStringStorage& strings) {
    to.reserve(from.size()); //00703C20 allocates exact count; STL capacity is projected.
    for (const auto& source : from) {
        KeyOwner key(strings);
        copy_construct_message_key_00703170(key.value, source, strings);
        to.push_back(std::move(key.value));
    }
}
} // namespace

MessageRecordStore::iterator lower_bound_message_record_00702300(
    MessageRecordStore& store, const NativeString& name)
{
    return store.lower_bound(name);
}
MessageRecordIterator find_message_record_00702910(MessageRecordStore& store,
    const NativeString& name)
{
    auto where = lower_bound_message_record_00702300(store, name);
    if (where != store.end()
        && native_string_less_case_insensitive_00443d00(name, where->first))
        where = store.end();
    return {&store, where};
}
VoiceClipRecord& construct_message_record_00703ff0(VoiceClipRecord& record) {
    // Constructor use only. Native likewise discards pre-existing headers.
    new (&record.text_00) NativeString;
    new (&record.auxiliary_0c) NativeString;
    new (&record.alternate_name_18) NativeString;
    record.resources_20.assign(4, nullptr);
    record.timed_keys_34.clear();
    return record;
}
void destroy_message_key_007029b0(VoiceTimedKey& key, NativeStringStorage& strings) {
    destroy_native_string_header_0041dd20(&key.callback_08, strings);
    destroy_native_string_header_0041dd20(&key.text_00, strings);
}
VoiceTimedKey& copy_construct_message_key_00703170(VoiceTimedKey& to,
    const VoiceTimedKey& from, NativeStringStorage& strings)
{
    copy_construct_native_string_header_00426060(&to.text_00, &from.text_00, strings);
    copy_construct_native_string_header_00426060(&to.callback_08, &from.callback_08, strings);
    copy_float(to.start_10, from.start_10);
    copy_float(to.end_14, from.end_14);
    to.flag_18 = from.flag_18;
    return to;
}
void destroy_message_keys_00703b90(VoiceTimedKeys& keys, NativeStringStorage& strings) {
    auto* first = keys.data();
    const auto count = keys.size();
    for (std::size_t i = 0; i < count; ++i) destroy_message_key_007029b0(first[i], strings);
    VoiceTimedKeys{}.swap(keys);
}
void destroy_message_record_00704060(VoiceClipRecord& record,
    MessageRecordResolverContext& context)
{
    destroy_message_keys_00703b90(record.timed_keys_34, context.strings);
    // Fresh partial C++ constructors may have no projected slots yet. Fully
    // constructed records always expose exactly the native four cells.
    const auto slots = record.resources_20.size() < 4 ? record.resources_20.size() : 4;
    for (std::size_t i = slots; i != 0; --i) {
        auto& cell = record.resources_20[i - 1];
        if (void* captured = cell) {
            context.playback.release_reference(captured);
            cell = nullptr; //004C3810 clears AFTER callback
        }
    }
    destroy_native_string_header_0041dd20(&record.alternate_name_18, context.strings);
    destroy_native_string_header_0041dd20(&record.auxiliary_0c, context.strings);
    destroy_native_string_header_0041dd20(&record.text_00, context.strings);
}
VoiceClipRecord& copy_construct_message_record_007042b0(VoiceClipRecord& to,
    const VoiceClipRecord& from, MessageRecordResolverContext& context)
{
    copy_construct_native_string_header_00426060(&to.text_00, &from.text_00, context.strings);
    to.sound_id_08 = from.sound_id_08;
    copy_construct_native_string_header_00426060(&to.auxiliary_0c, &from.auxiliary_0c, context.strings);
    to.alternate_14 = from.alternate_14;
    copy_construct_native_string_header_00426060(&to.alternate_name_18,
        &from.alternate_name_18, context.strings);
    to.resources_20.resize(4);
    for (std::size_t i = 0; i != 4; ++i) {
        copy_message_resource_reference_005b7dc0(to.resources_20[i],
            from.resources_20.at(i), context.playback);
    }
    copy_keys(to.timed_keys_34, from.timed_keys_34, context.strings);
    return to;
}
void*& copy_message_resource_reference_005b7dc0(void*& destination,
    void* const& source, VoiceLineHost& host)
{
    destination = nullptr;
    if (void* captured = source) {
        destination = captured;
        host.retain_reference(captured);
    }
    return destination;
}
void resize_message_keys_00704190(VoiceTimedKeys& keys, std::uint32_t count,
    VoiceTimedKey fill, NativeStringStorage& strings)
{
    KeyOwner argument(std::move(fill), strings);
    const auto old_count = keys.size();
    if (old_count < count) {
        KeyOwner local(strings); //00703CE0 copies fill before length/capacity checks
        copy_construct_message_key_00703170(local.value, argument.value, strings);
        constexpr std::size_t maximum = 0x09249249;
        if (count > maximum) throw std::length_error("vector<T> too long");
        if (keys.capacity() < count) {
            VoiceTimedKeys replacement;
            auto capacity = keys.capacity();
            capacity = maximum - capacity / 2 < capacity ? 0 : capacity + capacity / 2;
            if (capacity < count) capacity = count;
            replacement.reserve(capacity);
            try {
                copy_keys(replacement, keys, strings);
                for (auto i = old_count; i != count; ++i) {
                    KeyOwner key(strings);
                    copy_construct_message_key_00703170(key.value, local.value, strings);
                    replacement.push_back(std::move(key.value));
                }
            } catch (...) {
                destroy_message_keys_00703b90(replacement, strings);
                throw;
            }
            destroy_message_keys_00703b90(keys, strings);
            keys.swap(replacement);
        } else {
            for (auto i = old_count; i != count; ++i) {
                KeyOwner key(strings);
                copy_construct_message_key_00703170(key.value, local.value, strings);
                keys.push_back(std::move(key.value));
            }
        }
    } else if (count < old_count) {
        //00703B10 receives [begin+count,end); there are no elements to shift.
        auto* captured = keys.data();
        for (auto i = count; i != old_count; ++i)
            destroy_message_key_007029b0(captured[i], strings);
        while (keys.size() != count) keys.pop_back();
    }
}
NativeString& concatenate_message_string_004261a0(const NativeString& left,
    NativeString& destination, const NativeString& right, NativeStringStorage& strings)
{
    new (&destination) NativeString; // intentionally clears even when aliasing left
    try {
        assign_string(destination, left, strings);
        const auto right_length = right.length();
        if (right_length) {
            const auto old_length = destination.length();
            destination.resize_0041dd40(strings, old_length + right_length, true);
            std::memcpy(destination.data() + old_length, right.data(), right_length);
        }
    } catch (...) {
        destroy_native_string_header_0041dd20(&destination, strings);
        // The C++ wrapper can have an outer owner. Disarm its cleanup after
        // consuming this constructor's cleanup; native SEH storage is dying.
        new (&destination) NativeString;
        throw;
    }
    return destination;
}
VoiceClipRecord& lookup_message_record_00705c50(MessageRecordStore& store,
    const NativeString& name, MessageRecordResolverContext& context)
{
    const auto lower = lower_bound_message_record_00702300(store, name);
    if (lower != store.end()
        && !native_string_less_case_insensitive_00443d00(name, lower->first))
        return lower->second;
    RecordOwner temporary(context);
    std::memcpy(&temporary.value.sound_id_08, &context.scratch.sound_word_08, 4);
    temporary.value.alternate_14 = context.scratch.alternate_14;
    construct_message_record_00703ff0(temporary.value);
    StringOwner pair_key(context.strings);
    copy_construct_native_string_header_00426060(&pair_key.value, &name, context.strings);
    RecordOwner pair_record(context);
    copy_construct_message_record_007042b0(pair_record.value, temporary.value, context);
    StringOwner node_key(context.strings);
    copy_construct_native_string_header_00426060(&node_key.value, &pair_key.value, context.strings);
    RecordOwner node_record(context);
    copy_construct_message_record_007042b0(node_record.value, pair_record.value, context);
    auto where = store.emplace_hint(lower, std::move(node_key.value), std::move(node_record.value));
    return where->second; // pair record/key, then temporary destroy after publication
}
VoiceClipRecord& resolve_message_record_00705e00(MessageRecordStore& store,
    const NativeString& name, std::uint32_t unused, MessageRecordResolverContext& context)
{
    (void)unused;
    const auto found = find_message_record_00702910(store, name);
    if (found.position != store.end()) return found.position->second;
    auto& record = lookup_message_record_00705c50(store, name, context);
    assign_string(record.text_00, name, context.strings);
    record.sound_id_08 = -1;
    VoiceTimedKey fill;
    set_word(fill.start_10, context.scratch.key_start_word_10);
    set_word(fill.end_14, context.scratch.key_end_word_14);
    fill.flag_18 = context.scratch.key_flag_18;
    resize_message_keys_00704190(record.timed_keys_34, 1, std::move(fill), context.strings);
    auto& key = record.timed_keys_34.front();
    {
        StringOwner suffix(context.strings);
        suffix.value.assign_0041e870(context.strings, ">");
        StringOwner prefix(context.strings);
        prefix.value.assign_0041e870(context.strings, "<");
        StringOwner intermediate(context.strings);
        concatenate_message_string_004261a0(prefix.value, intermediate.value, name, context.strings);
        StringOwner final(context.strings);
        concatenate_message_string_004261a0(intermediate.value, final.value, suffix.value, context.strings);
        assign_string(key.text_00, final.value, context.strings);
    } // final, intermediate, prefix, suffix, before manager or scalar writes
    const auto length = key.text_00.length();
    key.flag_18 = 1;
    key.start_10 = 0.0f;
    auto& manager = context.playback.current_voice_manager_00e198c4_a4();
    key.end_14 = message_duration(length, manager.per_character_7c, manager.base_80);
    return record;
}
void clear_message_record_store(MessageRecordStore& store, MessageRecordResolverContext& context) {
    while (!store.empty()) {
        auto node = store.extract(store.begin());
        destroy_message_record_00704060(node.mapped(), context);
        destroy_native_string_header_0041dd20(&node.key(), context.strings);
    }
}
} // namespace bsp
