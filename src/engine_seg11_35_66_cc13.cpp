#define _CRT_SECURE_NO_WARNINGS  // The image's own strcpy / sprintf / fopen calls are reproduced.
// Segments 11, 35 and 66 leaves (cc13-sys3). See include/bsp/engine_seg11_35_66_cc13.hpp and
// docs/CC13_SYS_3.md for the evidence, the original ABIs and what is not claimed.

#include "bsp/engine_seg11_35_66_cc13.hpp"

#include <windows.h>
#include <intrin.h>

#include <cctype>
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <new>
#include <string>
#include <utility>
#include <vector>

namespace bsp::engine_seg11_35_66_cc13 {

namespace {

// Assign a C++ string into a native string the way the inline copies do: resize with preserve,
// then copy the bytes over the terminator resize wrote.
void assign_native(NativeString& to, NativeStringStorage& strings, const std::string& text) {
    to.resize_0041dd40(strings, static_cast<std::uint32_t>(text.size()), true);
    if (!text.empty() && to.data() != nullptr) {
        std::memcpy(to.data(), text.data(), text.size());
    }
}

std::string text_of(const NativeString& s) {
    return s.data() == nullptr ? std::string() : std::string(s.data(), s.length());
}

// The inline null-aware compare of 007055A0's descent and 00706160's lower-bound test: an empty
// left string is "less" exactly when the right one is not empty; an empty right string is less
// than any non-empty left; otherwise __stricmp (00BF7FBF).
int compare_ci_null_aware(const NativeString& left, const NativeString& right) {
    if (left.length() == 0) {
        return right.length() != 0 ? -1 : 0;
    }
    if (right.length() == 0) {
        return 1;
    }
    return _stricmp(left.data(), right.data());
}

}  // namespace

GuiLuaVariant variant_name(const char* text) noexcept {
    GuiLuaVariant v;
    v.tag = 0;
    v.value.text = text;
    return v;
}

GuiLuaVariant variant_index(std::int32_t index) noexcept {
    GuiLuaVariant v;
    v.tag = 1;
    v.value.integer = index;
    return v;
}

GuiLuaVariant variant_field(std::int32_t type, void* address) noexcept {
    GuiLuaVariant v;
    v.tag = type;
    v.value.pointer = address;
    return v;
}

// ===================================================================================================
// Segment 35: message manager.
// ===================================================================================================

void init_message_filter_007021c0(MessageCategoryFilter& filter, MessageGroupFlags& flags,
                                  const NativeIntArray& ids, bool enabled) noexcept {
    // 007021C0..007021D9: four dwords, a word and a byte of zeros (13h bytes).
    filter.category.fill(0);
    flags = MessageGroupFlags{};
    if (!enabled) {
        return;
    }
    for (std::int32_t i = 0; i < ids.count; ++i) {
        const std::int32_t id = ids.data[i];
        // The image stores filter[id] before classifying, with no bounds test.
        reinterpret_cast<std::uint8_t*>(&filter)[id] = 1;
        if ((id >= 0 && id <= 6) || id == 0x0D) {
            flags.group_0_6_13 = 1;
        } else if ((id >= 7 && id <= 0x0C)) {
            flags.group_7_12 = 1;
        } else if (id >= 0x0E && id <= 0x11) {
            flags.group_14_17 = 1;
        }
    }
}

void construct_record_tree_007026b0(NativeTreeHeader& tree, MessageManagerCalls& calls) {
    NativeTreeLinks* head = calls.buy_record_head_00702480();
    tree.head = head;
    reinterpret_cast<std::uint8_t*>(head)[0x55] = 1;  // nil
    head->parent = head;
    head->left = head;
    head->right = head;
    tree.size = 0;
}

NativeTreeHeader& construct_record_tree_00702a30(NativeTreeHeader& tree, MessageManagerCalls& calls) {
    construct_record_tree_007026b0(tree, calls);
    return tree;
}

std::string message_map_path_00702a80(const std::string& name) {
    // 00702A80: the extension (00CFD2C8 ".lua") is built first, then the directory; the result is
    // directory + name, then + extension.
    return std::string("Scripts/datatables/messagemaps/") + name + ".lua";
}

void mark_record_unloaded_00702bd0(VoiceClipRecord& record, MessageManagerCalls& calls) {
    NativeStringStorage& strings = calls.strings();
    static const char kTag[] = " (Unloaded message!)";  // 14h bytes
    const std::uint32_t old_length = record.text_00.length();
    record.text_00.resize_0041dd40(strings, old_length + 0x14, true);
    std::memcpy(record.text_00.data() + old_length, kTag, 0x14);
    record.sound_id_08 = -1;
    // +20h..+2Ch: four intrusive references, released and zeroed in order.
    for (std::size_t i = 0; i != 4 && i < record.resources_20.size(); ++i) {
        if (void* resource = record.resources_20[i]) {
            calls.release_reference(resource);
        }
        record.resources_20[i] = nullptr;
    }
}

void write_loaded_message_maps_00703020(MessageManagerCalls& calls, PropertyWriter& writer) {
    std::int32_t index = 0;
    for (const std::string& key : calls.loaded_map_keys()) {
        // Node text +10h; a null buffer writes 00E19AC3 ("").
        writer.write_0c(variant_index(index), variant_name(key.c_str()));
        ++index;
    }
}

VoiceTimedKey& assign_timed_key_00703230(VoiceTimedKey& to, const VoiceTimedKey& from,
                                         NativeStringStorage& strings) {
    if (&to != &from) {
        to.text_00.copy_from_00be0a30_fragment(strings, from.text_00);
    }
    if (&to.callback_08 != &from.callback_08) {
        to.callback_08.copy_from_00be0a30_fragment(strings, from.callback_08);
    }
    to.start_10 = from.start_10;
    to.end_14 = from.end_14;
    to.flag_18 = from.flag_18;
    return to;
}

void construct_timed_key_007032f0(VoiceTimedKey* slot, const VoiceTimedKey& source, TimedKeyOps& ops) {
    if (slot != nullptr) {
        ops.copy_construct_00703170(slot, source);
    }
}

VoiceTimedKey* copy_timed_keys_007034a0(const VoiceTimedKey* first, const VoiceTimedKey* last,
                                        VoiceTimedKey* dest, TimedKeyOps& ops) {
    const std::ptrdiff_t count = last - first;
    for (VoiceTimedKey* d = dest; first != last; ++first, ++d) {
        ops.assign_00703230(*d, *first);
    }
    return dest + count;
}

VoiceTimedKey* copy_backward_timed_keys_00703620(const VoiceTimedKey* first, const VoiceTimedKey* last,
                                                 VoiceTimedKey* dest_end, TimedKeyOps& ops) {
    const std::ptrdiff_t count = last - first;
    VoiceTimedKey* d = dest_end;
    while (first != last) {
        --last;
        --d;
        ops.assign_00703230(*d, *last);
    }
    return dest_end - count;
}

VoiceTimedKey* uninitialized_copy_timed_keys_00703590(const VoiceTimedKey* first, const VoiceTimedKey* last,
                                                      VoiceTimedKey* dest, TimedKeyOps& ops) {
    VoiceTimedKey* const start = dest;
    try {
        for (; first != last; ++first, ++dest) {
            if (dest != nullptr) {
                ops.copy_construct_00703170(dest, *first);
            }
        }
    } catch (...) {
        // 007035EA catch-all: destroy [start, dest) through 007029B0, then rethrow (00BF6885).
        for (VoiceTimedKey* p = start; p != dest; ++p) {
            ops.destroy_007029b0(*p);
        }
        throw;
    }
    return dest;
}

void uninitialized_fill_timed_keys_00703860(VoiceTimedKey* first, std::uint32_t count,
                                            const VoiceTimedKey& value, TimedKeyOps& ops) {
    VoiceTimedKey* const start = first;
    try {
        for (; count != 0; --count, ++first) {
            if (first != nullptr) {
                ops.copy_construct_00703170(first, value);
            }
        }
    } catch (...) {
        for (VoiceTimedKey* p = start; p != first; ++p) {
            ops.destroy_007029b0(*p);
        }
        throw;
    }
}

VoiceTimedKey* uninitialized_fill_timed_keys_00703aa0(VoiceTimedKey* first, std::uint32_t count,
                                                      const VoiceTimedKey& value, TimedKeyOps& ops) {
    uninitialized_fill_timed_keys_00703860(first, count, value, ops);
    return first + count;
}

void destroy_timed_keys_00703a70(VoiceTimedKey* first, VoiceTimedKey* last, TimedKeyOps& ops) {
    for (; first != last; ++first) {
        ops.destroy_007029b0(*first);
    }
}

bool buy_timed_keys_00703920(NativeVectorHeader& vector, std::uint32_t n, TimedKeyStorage& storage) {
    vector.first = nullptr;
    vector.last = nullptr;
    vector.end = nullptr;
    if (n == 0) {
        return false;
    }
    if (n > 0x09249249u) {
        storage.throw_length_error_00703710();
    }
    VoiceTimedKey* block = storage.allocate_00701ca0(n);
    vector.first = reinterpret_cast<std::uint8_t*>(block);
    vector.last = reinterpret_cast<std::uint8_t*>(block);
    vector.end = reinterpret_cast<std::uint8_t*>(block + n);
    return true;
}

void insert_fill_timed_keys_00703ce0(NativeVectorHeader& vector, VoiceTimedKey* position,
                                     std::uint32_t count, const VoiceTimedKey& fill, TimedKeyOps& ops,
                                     TimedKeyStorage& storage, TimedKeyInsertHelpers& helpers) {
    constexpr std::uint32_t kMax = 0x09249249u;
    auto* first = reinterpret_cast<VoiceTimedKey*>(vector.first);
    auto* last = reinterpret_cast<VoiceTimedKey*>(vector.last);
    auto* end = reinterpret_cast<VoiceTimedKey*>(vector.end);
    // 00703CE0: the fill is copied into a local before any test (it may alias an element).
    alignas(VoiceTimedKey) unsigned char local_storage[sizeof(VoiceTimedKey)];
    auto* local = reinterpret_cast<VoiceTimedKey*>(local_storage);
    ops.copy_construct_00703170(local, fill);
    struct LocalGuard {
        VoiceTimedKey* key;
        TimedKeyOps& ops;
        ~LocalGuard() { ops.destroy_007029b0(*key); }
    } guard{local, ops};

    std::uint32_t capacity = first != nullptr ? static_cast<std::uint32_t>(end - first) : 0u;
    if (count == 0) {
        return;
    }
    std::uint32_t size = first != nullptr ? static_cast<std::uint32_t>(last - first) : 0u;
    if (kMax - size < count) {
        storage.throw_length_error_00703710();
    }
    size = first != nullptr ? static_cast<std::uint32_t>(last - first) : 0u;
    if (capacity < size + count) {
        // Grow by half, or to exactly size + count when that is not enough (005B5D70 = size()).
        capacity = (kMax - (capacity >> 1) < capacity) ? 0u : capacity + (capacity >> 1);
        if (capacity < size + count) {
            capacity = size + count;
        }
        VoiceTimedKey* fresh = storage.allocate_00701ca0(capacity);
        VoiceTimedKey* tail = uninitialized_copy_timed_keys_00703590(first, position, fresh, ops);
        tail = uninitialized_fill_timed_keys_00703aa0(tail, count, *local, ops);
        uninitialized_copy_timed_keys_00703590(position, last, tail, ops);
        if (first != nullptr) {
            destroy_timed_keys_00703a70(first, last, ops);
            storage.free_00bf65ac(first);
        }
        vector.end = reinterpret_cast<std::uint8_t*>(fresh + capacity);
        vector.last = reinterpret_cast<std::uint8_t*>(fresh + size + count);
        vector.first = reinterpret_cast<std::uint8_t*>(fresh);
        return;
    }
    if (static_cast<std::uint32_t>(last - position) < count) {
        // The new elements reach past the old end.
        helpers.uninitialized_move_00703be0(position, last, position + count);
        uninitialized_fill_timed_keys_00703aa0(last, count - static_cast<std::uint32_t>(last - position),
                                               *local, ops);
        vector.last = reinterpret_cast<std::uint8_t*>(last + count);
        helpers.fill_007037b0(position, last, *local);
    } else {
        VoiceTimedKey* const split = last - count;
        vector.last = reinterpret_cast<std::uint8_t*>(helpers.uninitialized_move_00703be0(split, last, last));
        helpers.copy_backward_007039a0(position, split, last);
        helpers.fill_007037b0(position, position + count, *local);
    }
}

void destroy_record_pair_00704120(MessageRecordPair& pair, MessageRecordOps& ops, NativeStringStorage& strings) {
    ops.destroy_record_00704060(pair.record);
    pair.key.release_to(strings);
}

MessageRecordPair& copy_construct_record_pair_007043e0(MessageRecordPair& to, const MessageRecordPair& from,
                                                       MessageRecordOps& ops, NativeStringStorage& strings) {
    return copy_construct_record_pair_007044a0(to, from.key, from.record, ops, strings);
}

MessageRecordPair& copy_construct_record_pair_007044a0(MessageRecordPair& to, const NativeString& key,
                                                       const VoiceClipRecord& record, MessageRecordOps& ops,
                                                       NativeStringStorage& strings) {
    // The key header is zeroed by the constructor; the copy is self-guarded.
    if (&to.key != &key) {
        to.key.copy_from_00be0a30_fragment(strings, key);
    }
    ops.copy_construct_record_007042b0(to.record, record);
    return to;
}

MessageRecordNode* buy_record_node_007052f0(MessageRecordNode* left, MessageRecordNode* parent,
                                            MessageRecordNode* right, const MessageRecordPair& value,
                                            std::uint8_t color, RecordNodeAllocator& allocator,
                                            MessageRecordOps& ops, NativeStringStorage& strings) {
    void* storage = allocator.new_node_00bf681b();
    if (storage == nullptr) {
        return nullptr;
    }
    auto* node = static_cast<MessageRecordNode*>(storage);
    node->left = left;
    node->parent = parent;
    node->right = right;
    try {
        new (&node->value) MessageRecordPair();
        copy_construct_record_pair_007043e0(node->value, value, ops, strings);
    } catch (...) {
        // 007052F0's catch-all frees the node and rethrows.
        allocator.delete_node_00bf65ac(storage);
        throw;
    }
    node->color = color;
    node->is_nil = 0;
    return node;
}

void destroy_record_subtree_00705980(MessageRecordNode* node, RecordNodeAllocator& allocator,
                                     MessageRecordOps& ops, NativeStringStorage& strings) {
    while (node->is_nil == 0) {
        destroy_record_subtree_00705980(node->right, allocator, ops, strings);
        MessageRecordNode* const next = node->left;
        destroy_record_pair_00704120(node->value, ops, strings);
        node->value.~MessageRecordPair();
        allocator.delete_node_00bf65ac(node);
        node = next;
    }
}

RecordInsertResult insert_unique_record_007055a0(NativeTreeHeader& tree, const MessageRecordPair& value,
                                                 RecordTreeInternals& internals) {
    auto* head = reinterpret_cast<MessageRecordNode*>(tree.head);
    MessageRecordNode* where = head;
    MessageRecordNode* trial = head->parent;  // root
    bool add_left = true;
    while (trial->is_nil == 0) {
        where = trial;
        add_left = compare_ci_null_aware(value.key, trial->value.key) < 0;
        trial = add_left ? trial->left : trial->right;
    }
    MessageRecordNode* probe = where;
    if (add_left) {
        if (where == head->left) {  // begin()
            return {internals.insert_at_007053a0(true, where, value), true};
        }
        probe = internals.decrement_00701bc0(probe);
    }
    if (internals.less_ci_00443d00(probe->value.key, value.key)) {
        return {internals.insert_at_007053a0(add_left, where, value), true};
    }
    return {probe, false};
}

MessageRecordNode* insert_record_with_hint_00705a20(NativeTreeHeader& tree, const void* hint_owner,
                                                    MessageRecordNode* hint, const MessageRecordPair& value,
                                                    RecordTreeInternals& internals) {
    auto* head = reinterpret_cast<MessageRecordNode*>(tree.head);
    if (tree.size == 0) {
        return internals.insert_at_007053a0(true, head, value);
    }
    if (hint_owner == nullptr || hint_owner != &tree) {
        internals.invalid_parameter_00bf6713();
    }
    if (hint == head->left) {  // begin()
        if (internals.less_ci_00443d00(value.key, hint->value.key)) {
            return internals.insert_at_007053a0(true, hint, value);
        }
    } else if (hint == head) {  // end()
        MessageRecordNode* const rightmost = head->right;
        if (internals.less_ci_00443d00(rightmost->value.key, value.key)) {
            return internals.insert_at_007053a0(false, rightmost, value);
        }
    } else if (internals.less_ci_00443d00(value.key, hint->value.key)) {
        MessageRecordNode* const previous = internals.decrement_00701bc0(hint);
        if (internals.less_ci_00443d00(previous->value.key, value.key)) {
            if (previous->right->is_nil != 0) {
                return internals.insert_at_007053a0(false, previous, value);
            }
            return internals.insert_at_007053a0(true, hint, value);
        }
    } else if (internals.less_ci_00443d00(hint->value.key, value.key)) {
        MessageRecordNode* const next = internals.increment_00701fc0(hint);
        if (next == head || internals.less_ci_00443d00(value.key, next->value.key)) {
            if (hint->right->is_nil != 0) {
                return internals.insert_at_007053a0(false, hint, value);
            }
            return internals.insert_at_007053a0(true, next, value);
        }
    }
    return insert_unique_record_007055a0(tree, value, internals).node;
}

void write_message_ref_00703380(const VoiceClip& reference, PropertyWriter& writer, MessageManagerCalls& calls) {
    const NativeString& key = calls.record_key_007030d0(reference.record_04);
    const char* text = key.data() != nullptr ? key.data() : "";  // 00E19AC3
    writer.write_0c(variant_name("msg"), variant_name(text));
    writer.write_0c(variant_name("whoSaysThat"), variant_index(static_cast<std::int32_t>(reference.word_08)));
}

bool has_prefix_insensitive_00703400(const std::string& text, const std::string& prefix) {
    // 00469840 clamps the count to the remaining length.
    const std::string head = text.substr(0, prefix.size());
    return head.size() == prefix.size() && _stricmp(head.c_str(), prefix.c_str()) == 0;
}

VoiceClip& read_message_ref_00706060(VoiceClip& out, PropertyVisitor& reader, MessageManagerCalls& calls,
                                     std::int32_t speaker_scratch) {
    NativeStringStorage& strings = calls.strings();
    NativeString name;
    std::int32_t speaker = speaker_scratch;
    reader.read_10(variant_name("msg"), variant_field(0, &name));
    reader.read_10(variant_name("whoSaysThat"), variant_field(1, &speaker));
    const VoiceClipRecord* record = calls.resolve_record_00705e00(name);
    out.record_04 = record;
    out.native_vtable_00 = 0x00cf0dd0;
    out.word_08 = static_cast<std::uint32_t>(speaker);
    name.release_to(strings);
    return out;
}

void read_message_record_00704590(VoiceClipRecord& record, PropertyVisitor& reader,
                                  const std::array<std::uint8_t, 4>& flags, MessageManagerCalls& calls) {
    NativeStringStorage& strings = calls.strings();
    // 007045BB: "text" with the "" default (00CE3A0C).
    reader.read_or_default_0c(variant_name("text"), variant_field(0, &record.text_00), variant_name(""));

    // 00704602: the "subtitles" array, 1-based, one 1Ch key per entry.
    if (reader.has_key_14(variant_name("subtitles"))) {
        reader.enter_04(variant_name("subtitles"));
        for (std::int32_t i = 1; reader.has_key_14(variant_index(i)); ++i) {
            calls.resize_timed_keys_00704190(record.timed_keys_34, static_cast<std::uint32_t>(i));
            // 007046A2..007046D7: checked back(); an empty vector reaches 00BF6713.
            if (record.timed_keys_34.empty()) {
                calls.invalid_parameter_00bf6713();
                break;
            }
            VoiceTimedKey& key = record.timed_keys_34.back();
            reader.enter_04(variant_index(i));
            reader.read_10(variant_index(1), variant_field(0, &key.text_00));
            reader.read_10(variant_index(2), variant_field(2, &key.start_10));
            reader.read_10(variant_index(3), variant_field(2, &key.end_14));
            reader.read_or_default_0c(variant_index(4), variant_field(0, &key.callback_08), variant_name(""));
            GuiLuaVariant no_flag;
            no_flag.tag = 3;
            no_flag.value.integer = 0;  // byte 0 at +4
            reader.read_or_default_0c(variant_index(5), variant_field(3, &key.flag_18), no_flag);
            reader.leave_08();
        }
        reader.leave_08();
    }

    // 0070483F: "voice" into +0Ch with the "" default.
    reader.read_or_default_0c(variant_name("voice"), variant_field(0, &record.auxiliary_0c), variant_name(""));
    // 00704883..007048E1: the voice is compared with "" through the inline null-aware __stricmp.
    std::string voice = text_of(record.auxiliary_0c);
    if (voice.empty()) {
        record.sound_id_08 = -1;  // 007052C2
        return;
    }

    // 0070490C: a "tutorials" voice gets "/" + language inserted after the token:
    // voice[0, idx+9) + "/" + language + voice[idx+9, end).
    if (record.auxiliary_0c.data() != nullptr) {
        const char* data = record.auxiliary_0c.data();
        if (const char* hit = std::strstr(data, "tutorials")) {
            const std::size_t idx = static_cast<std::size_t>(hit - data);
            const std::size_t cut = idx + 9;
            const std::string language = calls.language_name_008d57a0();
            voice = voice.substr(0, cut) + "/" + language + voice.substr(cut);
            assign_native(record.auxiliary_0c, strings, voice);
        }
    }

    // 00704B39: "actor" with the "" default; then sound id 0 and the fallback flag +14h = 1.
    NativeString actor_native;
    reader.read_or_default_0c(variant_name("actor"), variant_field(0, &actor_native), variant_name(""));
    const std::string actor = text_of(actor_native);
    record.sound_id_08 = 0;
    record.alternate_14 = 1;
    if (record.resources_20.size() < 4) {
        record.resources_20.resize(4, nullptr);
    }

    for (std::size_t slot = 0; slot != 4; ++slot) {
        // 00704BA3: release the slot's previous reference.
        if (void* old = record.resources_20[slot]) {
            calls.release_reference(old);
        }
        record.resources_20[slot] = nullptr;
        const char* templ = calls.voice_directory_template_00e08fcc(slot);
        if (flags[slot] == 0 || templ == nullptr) {
            continue;
        }
        // 00704BF1: the template with every "LANGUAGE" replaced by the language (004CAD40).
        std::string directory = templ;
        const std::string language = calls.language_name_008d57a0();
        for (std::size_t at = directory.find("LANGUAGE"); at != std::string::npos;
             at = directory.find("LANGUAGE", at + language.size())) {
            directory.replace(at, 8, language);
        }
        // 00704CBA: directory + actor, a '/' when the last byte is not one, then voice + ".fsb".
        std::string path = directory + actor;
        if (path.empty() || path.back() != '/') {
            path += "/";
        }
        path += voice + ".fsb";
        auto take = [&](const std::string& file) {
            void* bank = calls.load_sound_bank_00a83fd0(file);
            void* const previous = record.resources_20[slot];
            if (previous != bank) {
                record.resources_20[slot] = bank;
                if (bank != nullptr) {
                    calls.retain_reference(bank);
                }
                if (previous != nullptr) {
                    calls.release_reference(previous);
                }
            }
            if (bank != nullptr) {
                calls.release_reference(bank);  // the temporary's reference
            }
            record.alternate_14 = 0;
        };
        if (calls.file_store_exists_vslot8(path)) {
            take(path);
            continue;
        }
        // 00704ECE: without the actor directory.
        const std::string plain = directory + voice + ".fsb";
        if (calls.file_store_exists_vslot8(plain)) {
            take(plain);
        }
    }

    // 007050C0: nothing found and no alternate name yet: probe the streamed-dialog fallback.
    if (record.alternate_14 != 0 &&
        (record.alternate_name_18.data() == nullptr || std::strcmp(record.alternate_name_18.data(), "") == 0)) {
        const std::string fallback = std::string("sound/messages/streamed_dialogs/") + voice + ".fsb";
        if (calls.file_store_exists_vslotc(fallback)) {
            assign_native(record.alternate_name_18, strings, fallback);
        }
    }
    actor_native.release_to(strings);
}

void load_actors_and_messages_00706160(MessageManagerCalls& calls, bool load, const std::string& path,
                                       const std::array<std::uint8_t, 4>& flags, std::int32_t side) {
    NativeStringStorage& strings = calls.strings();
    PropertyVisitor* reader = calls.open_script_reader(path);
    if (reader == nullptr) {
        return;
    }
    // 00706205: "Actors": every value is read into a temporary string and released.
    if (reader->has_key_14(variant_name("Actors"))) {
        reader->enter_04(variant_name("Actors"));
        for (const GuiLuaVariant& key : calls.enumerate_keys_00bd5f50(*reader)) {
            NativeString scratch;
            reader->read_10(key, variant_field(0, &scratch));
            scratch.release_to(strings);
        }
        reader->leave_08();
    }
    // 00706310: "Messages" is entered without a presence test; [side] when side != -1.
    reader->enter_04(variant_name("Messages"));
    if (side != -1) {
        reader->enter_04(variant_index(side));
    }
    const std::vector<GuiLuaVariant> keys = calls.enumerate_keys_00bd5f50(*reader);
    const std::int32_t count = static_cast<std::int32_t>(keys.size());
    for (std::int32_t i = 0; i < count; ++i) {
        // 007063B0: x87 fild/fdiv by the SSE count, spilled to float.
        calls.report_loading_progress_0057c1a0(static_cast<float>(i) / static_cast<float>(count));
        const char* key_text = keys[static_cast<std::size_t>(i)].value.text;
        NativeString name;
        assign_native(name, strings, key_text != nullptr ? key_text : "");
        if (load) {
            reader->enter_04(variant_name(name.data() != nullptr ? name.data() : ""));
            VoiceClipRecord& record = calls.lookup_record_00705c50(name);
            read_message_record_00704590(record, *reader, flags, calls);
            reader->leave_08();
        } else {
            // 00706479: lower_bound, then the inline null-aware compare; an exact match is marked.
            const NativeString* found = calls.lower_bound_key_00702300(name);
            if (found != nullptr && compare_ci_null_aware(name, *found) >= 0) {
                mark_record_unloaded_00702bd0(calls.lookup_record_00705c50(name), calls);
            }
        }
        name.release_to(strings);
    }
    if (side != -1) {
        reader->leave_08();
    }
    reader->leave_08();
    calls.close_script_reader(reader);
}

std::array<std::uint8_t, 4> load_messages_007065e0(MessageManagerLayout& manager, const MessageLoadOptions& options,
                                                    std::int32_t side, MessageManagerCalls& calls) {
    std::array<std::uint8_t, 4> flags{};
    const std::int32_t kind = options.local_side_kind;
    const std::size_t slot = kind == 0 ? 1u : (kind != 1 ? 3u : 2u);
    flags[slot] = 1;
    const NativeIntArray& first_ids = kind != 0 ? options.ids_1074 : options.ids_1068;
    const NativeIntArray& second_ids = kind != 0 ? options.ids_1068 : options.ids_1074;
    init_message_filter_007021c0(manager.filter_20, manager.flags_59, first_ids, options.filters_enabled);
    init_message_filter_007021c0(manager.filter_33, manager.flags_5c, second_ids, options.filters_enabled);
    init_message_filter_007021c0(manager.filter_46, manager.flags_5f, options.ids_1080, options.filters_enabled);
    load_actors_and_messages_00706160(calls, true, "Scripts/datatables/Messages.lua", flags, side);
    return flags;
}

void load_message_map_00706760(MessageManagerCalls& calls, const std::string& name, std::int32_t side) {
    const std::string key = name + "#" + std::to_string(side);
    if (calls.loaded_map_contains(key)) {
        return;
    }
    const std::array<std::uint8_t, 4> load_flags{1, 0, 0, 1};    // 00CFD2A4
    const std::array<std::uint8_t, 4> unload_flags{0, 0, 0, 0};  // 00CFD2A0
    if (calls.profile_hints_mode_plus8() == 2) {
        // 007068A5: sweep every other language: select it, mount "<language>_<key>", load the map,
        // unload it again, unmount. The list is consumed from the back.
        const std::string current = calls.language_name_008d57a0();
        std::vector<std::string> languages = calls.collect_language_names();
        while (!languages.empty()) {
            const std::string language = languages.back();
            languages.pop_back();
            calls.select_language_by_name(language);
            const bool same = !language.empty() && !current.empty()
                                  ? _stricmp(language.c_str(), current.c_str()) == 0
                                  : language.empty() == current.empty();
            if (!same) {
                void* block = calls.construct_file_block_00be0a30(language + "_" + key);
                load_actors_and_messages_00706160(calls, true, message_map_path_00702a80(name), load_flags, side);
                load_actors_and_messages_00706160(calls, false, message_map_path_00702a80(name), unload_flags, side);
                if (block != nullptr) {
                    calls.delete_file_block(block);
                }
            }
        }
        calls.select_language_by_name(current);
    }
    // 00706B2D: the voice bank block: "<language>_<key>" normally, "englishauthentic_<key>" when the
    // profile owns DL_Content_0000062.
    const bool authentic = calls.profile_owns_content("DL_Content_0000062");
    const std::string block_name =
        (authentic ? std::string("englishauthentic") : calls.language_name_008d57a0()) + "_" + key;
    void* block = calls.construct_file_block_00be0a30(block_name);
    calls.loaded_map_insert(key);
    load_actors_and_messages_00706160(calls, true, message_map_path_00702a80(name), load_flags, side);
    if (block != nullptr) {
        calls.delete_file_block(block);  // vtable +4h(1)
    }
}

void unload_message_map_00706fc0(MessageManagerCalls& calls, const std::string& name, std::int32_t side) {
    const std::string key = name + "#" + std::to_string(side);
    if (!calls.loaded_map_contains(key)) {
        return;
    }
    calls.loaded_map_erase_004cf8a0(key);
    load_actors_and_messages_00706160(calls, false, message_map_path_00702a80(name), {0, 0, 0, 0}, side);
}

void load_message_map_list_007072c0(MessageManagerCalls& calls, PropertyVisitor& reader) {
    NativeStringStorage& strings = calls.strings();
    for (std::int32_t index = 0; reader.has_key_14(variant_index(index)); ++index) {
        NativeString entry_native;
        reader.read_10(variant_index(index), variant_field(0, &entry_native));
        const std::string entry = text_of(entry_native);
        entry_native.release_to(strings);
        // 004BCB80: last position of any byte of "#"; npos + 1 wraps to 0 when absent.
        const std::size_t hash = entry.find_last_of('#');
        const std::string side_text = entry.substr(hash + 1);
        const std::string map_name = entry.substr(0, hash);
        load_message_map_00706760(calls, map_name, calls.atol_00bf8417(side_text.c_str()));
    }
}

MessageManagerLayout& construct_message_manager_00707480(MessageManagerLayout& manager, MessageManagerCalls& calls) {
    manager.vtable_00 = 0x00cfd3a4;
    construct_record_tree_007026b0(manager.records_04, calls);
    manager.names_first_14 = nullptr;
    manager.names_last_18 = nullptr;
    manager.names_end_1c = nullptr;
    NativeTreeLinks* head = calls.buy_string_set_head_004c26b0();
    manager.loaded_maps_64.head = head;
    reinterpret_cast<std::uint8_t*>(head)[0x15] = 1;
    head->parent = head;
    head->left = head;
    head->right = head;
    manager.loaded_maps_64.size = 0;
    return manager;
}

void destruct_message_manager_007071f0(MessageManagerLayout& manager, MessageManagerCalls& calls) {
    manager.vtable_00 = 0x00cfd3a4;
    calls.erase_string_set_004d1a50(manager.loaded_maps_64);
    calls.free_00bf65ac(manager.loaded_maps_64.head);
    manager.loaded_maps_64.head = nullptr;
    manager.loaded_maps_64.size = 0;
    if (manager.names_first_14 != nullptr) {
        calls.destroy_string_range_00432050(manager.names_first_14, manager.names_last_18);
        calls.free_00bf65ac(manager.names_first_14);
    }
    manager.names_first_14 = nullptr;
    manager.names_last_18 = nullptr;
    manager.names_end_1c = nullptr;
    calls.erase_record_map_00705d30(manager.records_04);
    calls.free_00bf65ac(manager.records_04.head);
    manager.records_04.head = nullptr;
    manager.records_04.size = 0;
}

// ===================================================================================================
// Segment 35: model pools.
// ===================================================================================================

namespace {

std::uint8_t* block_at(const ModelPoolLayout& pool, std::uint32_t index) {
    return reinterpret_cast<std::uint8_t*>(static_cast<std::uintptr_t>(pool.blocks_28.data[index]));
}

std::uint16_t& free_count_of(std::uint8_t* block, const PoolGeometry& g) {
    return *reinterpret_cast<std::uint16_t*>(block + g.free_count_offset);
}

std::uint16_t* free_stack_of(std::uint8_t* block, const PoolGeometry& g) {
    return reinterpret_cast<std::uint16_t*>(block + g.free_stack_offset);
}

std::uint32_t& slot_index_of(std::uint8_t* slot, const PoolGeometry& g) {
    return *reinterpret_cast<std::uint32_t*>(slot + g.slot_index_offset);
}

// The copy loop every reallocation shares: new buffer, copy count dwords, free the old buffer.
void reallocate_dword_vector(NativeDwordVector& v, std::uint32_t capacity, PoolMemory& memory) {
    auto* fresh = static_cast<std::uint32_t*>(memory.operator_new_00bf681b(capacity * 4u));
    for (std::uint32_t i = 0; i < v.count; ++i) {
        if (fresh != nullptr) {
            fresh[i] = v.data[i];
        }
    }
    if (v.data != nullptr) {
        memory.free_00bf9dc8(v.data);
    }
    v.data = fresh;
}

}  // namespace

void reserve_exact_dword_vector_007076e0(NativeDwordVector& v, std::uint32_t n, PoolMemory& memory) {
    v.capacity = n;
    reallocate_dword_vector(v, n, memory);
}

void grow_dword_vector_00707730(NativeDwordVector& v, PoolMemory& memory) {
    v.capacity = v.capacity * 2u + 2u;
    reallocate_dword_vector(v, v.capacity, memory);
}

void push_back_dword_vector_00707a10(NativeDwordVector& v, std::uint32_t value, PoolMemory& memory) {
    if (v.count == v.capacity) {
        grow_dword_vector_00707730(v, memory);
    }
    if (v.data != nullptr) {  // the placement-new null test on &data[count]
        v.data[v.count] = value;
    }
    ++v.count;
}

void reserve_dword_vector_00707a90(NativeDwordVector& v, std::uint32_t n, PoolMemory& memory) {
    if (v.capacity < n) {
        v.capacity = n;
        reallocate_dword_vector(v, n, memory);
    }
}

void free_owned_block_007076b0(void** owner, PoolMemory& memory) {
    if (*owner != nullptr) {
        memory.free_00bf9dc8(*owner);
    }
}

void init_pool_block_007075d0(std::uint8_t* block, std::uint32_t pool_index, const PoolGeometry& g) {
    free_count_of(block, g) = g.slots_per_block;
    std::uint16_t* stack = free_stack_of(block, g);
    for (std::uint16_t i = 0; i < g.slots_per_block; ++i) {
        stack[i] = static_cast<std::uint16_t>(g.slots_per_block - 1 - i);
        slot_index_of(block + i * g.slot_bytes, g) = pool_index;
    }
}

ModelPoolLayout& construct_model_pool_007086c0(ModelPoolLayout& pool, ModelPoolLayout*& allocator_list_head,
                                               const PoolGeometry& g, PoolMemory& memory) {
    // Mit::allocator_list_elem: prev = 0, next = old head, old head's prev = this, head = this.
    pool.list_prev_04 = nullptr;
    pool.list_next_08 = allocator_list_head;
    if (allocator_list_head != nullptr) {
        allocator_list_head->list_prev_04 = &pool;
    }
    allocator_list_head = &pool;
    pool.vtable_00 = g.vtable;
    memory.initialize(pool.section_0c);
    pool.lock_depth_24 = 0;
    pool.blocks_28 = NativeDwordVector{nullptr, 0, 0};
    pool.first_free_34 = 0xFFFFFFFFu;
    reserve_dword_vector_00707a90(pool.blocks_28, 0x20, memory);
    return pool;
}

void destruct_model_pool_007087f0(ModelPoolLayout& pool, ModelPoolLayout*& allocator_list_head,
                                  const PoolGeometry& g, PoolMemory& memory) {
    pool.vtable_00 = g.vtable;
    for (std::uint32_t i = 0; i < pool.blocks_28.count; ++i) {
        memory.free_00bf9dc8(block_at(pool, i));
    }
    if (pool.blocks_28.data != nullptr) {
        memory.free_00bf9dc8(pool.blocks_28.data);
    }
    while (pool.lock_depth_24 > 0) {
        --pool.lock_depth_24;
        memory.leave(pool.section_0c);
    }
    memory.remove(pool.section_0c);
    // Back to the allocator_list_elem vtable, then unlink.
    if (pool.list_prev_04 == nullptr) {
        allocator_list_head = pool.list_next_08;
    } else {
        pool.list_prev_04->list_next_08 = pool.list_next_08;
    }
    if (pool.list_next_08 != nullptr) {
        pool.list_next_08->list_prev_04 = pool.list_prev_04;
    }
}

void release_empty_pool_blocks_00708880(ModelPoolLayout& pool, const PoolGeometry& g, PoolMemory& memory) {
    std::uint32_t i = 0;
    if (pool.blocks_28.count != 0) {
        do {
            std::uint8_t* block = block_at(pool, i);
            if (free_count_of(block, g) == g.slots_per_block) {
                memory.free_00bf9dc8(block);
                pool.blocks_28.data[i] = pool.blocks_28.data[pool.blocks_28.count - 1];
                --pool.blocks_28.count;
                if (i < pool.blocks_28.count) {
                    std::uint8_t* moved = block_at(pool, i);
                    for (std::uint16_t s = 0; s < g.slots_per_block; ++s) {
                        slot_index_of(moved + s * g.slot_bytes, g) = i;
                    }
                }
                --i;  // unsigned wrap at 0 is undone by the increment
            }
            ++i;
        } while (i < pool.blocks_28.count);
    }
    pool.first_free_34 = 0xFFFFFFFFu;
    for (std::uint32_t b = 0; b < pool.blocks_28.count; ++b) {
        if (free_count_of(block_at(pool, b), g) != 0) {
            pool.first_free_34 = b;
            return;
        }
    }
}

std::uint8_t* allocate_pool_slot_00708920(ModelPoolLayout& pool, const PoolGeometry& g, PoolMemory& memory) {
    memory.enter(pool.section_0c);
    ++pool.lock_depth_24;
    if (pool.first_free_34 == 0xFFFFFFFFu) {
        pool.first_free_34 = pool.blocks_28.count;
        auto* block = static_cast<std::uint8_t*>(memory.operator_new_00bf681b(g.block_bytes));
        if (block != nullptr) {
            init_pool_block_007075d0(block, pool.first_free_34, g);
        }
        push_back_dword_vector_00707a10(pool.blocks_28,
                                        static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(block)), memory);
    }
    std::uint8_t* block = block_at(pool, pool.first_free_34);
    std::uint16_t& free_count = free_count_of(block, g);
    --free_count;
    std::uint8_t* slot = block + static_cast<std::uint32_t>(free_stack_of(block, g)[free_count]) * g.slot_bytes;
    if (free_count == 0) {
        std::uint32_t next = pool.first_free_34 + 1;
        pool.first_free_34 = 0xFFFFFFFFu;
        for (; next < pool.blocks_28.count; ++next) {
            if (free_count_of(block_at(pool, next), g) != 0) {
                pool.first_free_34 = next;
                break;
            }
        }
    }
    --pool.lock_depth_24;
    memory.leave(pool.section_0c);
    return slot;
}

void free_pool_slot_007079a0(ModelPoolLayout& pool, std::uint8_t* slot, const PoolGeometry& g, PoolMemory& memory) {
    memory.enter(pool.section_0c);
    ++pool.lock_depth_24;
    const std::uint32_t index = slot_index_of(slot, g);
    std::uint8_t* block = block_at(pool, index);
    // The image divides the signed byte offset by the slot size (multiply-high form).
    const auto offset = static_cast<std::int32_t>(slot - block);
    std::uint16_t& free_count = free_count_of(block, g);
    free_stack_of(block, g)[free_count] = static_cast<std::uint16_t>(offset / static_cast<std::int32_t>(g.slot_bytes));
    ++free_count;
    if (index < pool.first_free_34) {
        pool.first_free_34 = index;
    }
    --pool.lock_depth_24;
    memory.leave(pool.section_0c);
}

void free_static_pool_slot_00707af0(ModelPoolLayout& static_pool, std::uint8_t* slot, const PoolGeometry& g,
                                    PoolMemory& memory) {
    free_pool_slot_007079a0(static_pool, slot, g, memory);
}

void** release_reference_cell_0070b1b0(void** cell, ReferenceReleaser& releaser) {
    if (*cell != nullptr) {
        releaser.release_reference(*cell);
        *cell = nullptr;
    }
    return cell;
}

void release_native_string_00708a90(NativeString& s, NativeStringStorage& strings) {
    // The image returns the block (length + 1) and leaves the header as it was; release_to also
    // clears the header, which no caller of these unwind helpers reads afterwards.
    s.release_to(strings);
}

FishSchoolParams& init_fish_school_params_00708b00(FishSchoolParams& p, NativeStringStorage& strings) {
    p.count_00 = 0x9C4;
    p.radius_x_04 = 25.0f;
    p.radius_y_08 = 25.0f;
    p.radius_z_0c = 60.0f;
    p.box_half_x_10 = 50.0f;
    p.box_half_z_18 = 50.0f;
    p.box_half_y_14 = 10.0f;
    p.word_1c = 0.0f;
    p.word_24 = 0.0f;
    p.word_20 = -20.0f;
    p.word_28 = 2.0f;
    p.word_2c = 2.0f;
    p.word_40 = 2.0f;
    p.speed_max_48 = 2.0f;
    p.word_38 = 4.0f;
    p.respawn_min_4c = 0.3f;
    p.word_30 = 1.0f;
    p.word_34 = 1.0f;
    p.word_3c = 7.0f;
    p.speed_min_44 = 1.0f;
    p.respawn_max_50 = 0.75f;
    p.texture_54.assign_0041e870(strings, "");
    return p;
}

FishSchoolParams& assign_fish_school_params_00708c60(FishSchoolParams& to, const FishSchoolParams& from,
                                                     NativeStringStorage& strings) {
    std::memcpy(&to, &from, offsetof(FishSchoolParams, texture_54));
    if (&to.texture_54 != &from.texture_54) {
        to.texture_54.copy_from_00be0a30_fragment(strings, from.texture_54);
    }
    return to;
}

FishSchoolDescLayout& construct_fish_school_desc_00708be0(FishSchoolDescLayout& desc, NativeStringStorage& strings) {
    desc.vtable_00 = 0x00cfd414;
    init_fish_school_params_00708b00(desc.params_04, strings);
    desc.fish_64 = nullptr;
    desc.model_78 = nullptr;
    desc.positions_60 = nullptr;
    return desc;
}

void destruct_fish_school_desc_00708ab0(FishSchoolDescLayout& desc, PoolMemory& memory, NativeStringStorage& strings) {
    desc.vtable_00 = 0x00cfd414;
    if (desc.fish_64 != nullptr) {
        memory.free_00bf9dc8(desc.fish_64);
        memory.free_00bf9dc8(desc.positions_60);
    }
    desc.params_04.texture_54.release_to(strings);
}

void destruct_fish_school_model_00707570(std::uint8_t* model, GeneratedModelCalls& calls) {
    *reinterpret_cast<std::uint32_t*>(model) = 0x00cfd3a8;
    if (void* owned = *reinterpret_cast<void**>(model + 0x184)) {
        calls.delete_owned_object(owned);
    }
    calls.destroy_model_base(model);
}

std::uint8_t* scalar_delete_fish_school_model_00708a60(std::uint8_t* model, std::uint32_t flags,
                                                       GeneratedModelCalls& calls, ModelPoolLayout& static_pool,
                                                       PoolMemory& memory) {
    destruct_fish_school_model_00707570(model, calls);
    if ((flags & 1u) != 0) {
        free_pool_slot_007079a0(static_pool, model, kFishSchoolPool, memory);
    }
    return model;
}

void destruct_flag_model_0070b2c0(std::uint8_t* model, NativeStringStorage& strings, GeneratedModelCalls& calls) {
    *reinterpret_cast<std::uint32_t*>(model) = 0x00cfd488;
    reinterpret_cast<NativeString*>(model + 0x18C)->release_to(strings);
    calls.destroy_model_base(model);
}

// ===================================================================================================
// Segment 66: hardware detection, registry, language.
// ===================================================================================================

const char* gpu_model_name_0098c820(const HardwareInfoFields& info, const GpuTableRow* table) {
    if (info.gpu_index_28 != -1 && info.gpu_index_28 >= 0) {
        return table[info.gpu_index_28].name;
    }
    return table[0].name;
}

std::uint32_t gpu_device_id_0098c870(const HardwareInfoFields& info, const GpuTableRow* table) {
    // AND with (index < 0) - 1: a negative index becomes 0.
    const std::int32_t index = info.gpu_index_28 < 0 ? 0 : info.gpu_index_28;
    return table[index].device_id;
}

const char* gpu_device_name_0098c890(std::uint32_t device_id, const GpuTableRow* table) {
    std::size_t i = 0;
    if (table[0].name != nullptr) {
        while (table[i].device_id != device_id) {
            ++i;
            if (table[i].name == nullptr) {
                return table[0].name;
            }
        }
    }
    return table[i].name;
}

std::uint32_t measure_cpu_clock_0098c940() {
    LARGE_INTEGER frequency;
    QueryPerformanceFrequency(&frequency);
    const unsigned __int64 start = __rdtsc();
    LARGE_INTEGER target;
    QueryPerformanceCounter(&target);
    target.QuadPart += frequency.QuadPart;
    LARGE_INTEGER now;
    do {
        QueryPerformanceCounter(&now);
    } while (static_cast<std::uint64_t>(now.QuadPart) < static_cast<std::uint64_t>(target.QuadPart));
    const unsigned __int64 stop = __rdtsc();
    return static_cast<std::uint32_t>((stop - start) >> 20);
}

bool direct_sound_enum_first_0098d360(const char* description, std::uint32_t& device_counter,
                                      char* name_buffer, DirectSoundProbe& probe) {
    if (device_counter == 0) {
        if (!probe.create_default_device()) {
            return false;
        }
        std::strcpy(name_buffer, description);
    }
    ++device_counter;
    return true;
}

std::int32_t language_row_for_lcid_0098d400(std::uint32_t lcid) noexcept {
    switch (lcid) {
    case 0x40C: return 1;
    case 0x407: return 2;
    case 0x410: return 3;
    case 0x415: return 4;
    case 0x419: return 5;
    default: return 0;
    }
}

namespace {
constexpr char kBsmHwdKey[] = "SOFTWARE\\Eidos\\BSM_HWD";
}  // namespace

bool write_hardware_value_0098d470(const char* value_name, std::uint32_t type, const void* data,
                                   std::uint32_t size) {
    HKEY key = nullptr;
    if (RegCreateKeyExA(HKEY_LOCAL_MACHINE, kBsmHwdKey, 0, nullptr, 0, 0x20006, nullptr, &key, nullptr) != 0) {
        return false;
    }
    const bool ok = RegSetValueExA(key, value_name, 0, type, static_cast<const BYTE*>(data), size) == 0;
    RegCloseKey(key);
    return ok;
}

bool read_hardware_value_0098d4e0(const char* value_name, std::uint32_t expected_type, void* buffer,
                                  std::uint32_t* size) {
    HKEY key = nullptr;
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, kBsmHwdKey, 0, 0x20019, &key) != 0) {
        return false;
    }
    DWORD type = 0;
    DWORD bytes = *size;
    const LSTATUS status = RegQueryValueExA(key, value_name, nullptr, &type, static_cast<BYTE*>(buffer), &bytes);
    *size = bytes;
    const bool ok = status == 0 && type == expected_type;
    RegCloseKey(key);
    return ok;
}

bool write_hardware_dword_0098d570(const char* value_name, std::uint32_t value) {
    return write_hardware_value_0098d470(value_name, REG_DWORD, &value, 4);
}

bool write_hardware_string_0098d590(const char* value_name, const char* text) {
    return write_hardware_value_0098d470(value_name, REG_SZ, text, static_cast<std::uint32_t>(std::strlen(text) + 1));
}

void read_hardware_qword_0098d5c0(const char* value_name, std::uint32_t& out, std::uint32_t scratch_low) {
    std::uint32_t buffer[2] = {scratch_low, 0};
    std::uint32_t size = 8;
    read_hardware_value_0098d4e0(value_name, 0x0B, buffer, &size);  // REG_QWORD; result ignored
    out = buffer[0];
}

void read_hardware_string_0098d5f0(const char* value_name, char* buffer, std::uint32_t* size) {
    read_hardware_value_0098d4e0(value_name, REG_SZ, buffer, size);
}

bool write_default_options_file_0098f430(const char* path, OptionsTextBuilder& builder) {
    std::FILE* file = std::fopen(path, "wb");
    if (file == nullptr) {
        return false;
    }
    const std::size_t length = std::strlen(builder.build_default_options_text_0098e910());
    std::fwrite(builder.build_default_options_text_0098e910(), length, 1, file);
    std::fclose(file);
    return true;
}

bool query_processor_info_00992180(HardwareInfoFields& info) {
    HKEY key = nullptr;
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0", 0, 0x20019, &key) != 0) {
        return false;
    }
    std::uint32_t mhz[2] = {0, 0};
    DWORD type = 0;
    DWORD size = 8;
    if (RegQueryValueExA(key, "~MHz", nullptr, &type, reinterpret_cast<BYTE*>(mhz), &size) != 0) {
        RegCloseKey(key);
        return false;
    }
    size = 0;
    RegQueryValueExA(key, "ProcessorNameString", nullptr, &type, nullptr, &size);
    if (size == 0) {
        return false;  // the key stays open (00992226)
    }
    std::vector<char> name(size, '\0');
    const LSTATUS status =
        RegQueryValueExA(key, "ProcessorNameString", nullptr, &type, reinterpret_cast<BYTE*>(name.data()), &size);
    std::sprintf(info.cpu_name_34, "%s", name.data());
    RegCloseKey(key);
    if (status != 0) {
        return false;
    }
    info.cpu_mhz_30 = mhz[0];
    if (std::strstr(name.data(), "AMD Athlon(tm) 64") != nullptr) {
        // 009922E0: FILD (unsigned fix-up with 2^32), FMUL 1.5 (double 00CE3D78), FISTP with RC=chop.
        const double scaled = static_cast<double>(info.cpu_mhz_30) * 1.5;
        info.cpu_mhz_30 = static_cast<std::uint32_t>(static_cast<std::uint64_t>(scaled));
    }
    return true;
}

void collect_hardware_info_00992640(HardwareInfoFields& info, HardwareCollectCalls& calls) {
    calls.describe_os_version_0098c9c0();
    info.memory_mb_2c = calls.total_physical_memory() >> 20;
    if (!calls.query_processor_info_00992180()) {
        info.cpu_mhz_30 = calls.measure_cpu_clock_0098c940();
    }
    calls.query_display_driver_00991ef0();
    std::uint32_t counter = 0;
    char first = 0;
    calls.reset_sound_probe(counter, first);  // 00F8A0E0 = 0, byte 00F8A0E8 = 0
    calls.enumerate_sound_devices();
}

void construct_hardware_info_009927d0(HardwareInfoVectors& vectors, void*& owner_c38, void* owner,
                                      HardwareInfoFields& info, HardwareCollectCalls& calls) {
    vectors.words[1] = 0;
    vectors.words[2] = 0;
    vectors.words[3] = 0;
    vectors.words[5] = 0;
    vectors.words[6] = 0;
    vectors.words[7] = 0;
    owner_c38 = owner;
    collect_hardware_info_00992640(info, calls);
}

std::uint32_t read_language_from_options_00992830(const char* path) {
    std::uint32_t lcid = 0x409;
    std::FILE* file = std::fopen(path, "rb");
    if (file == nullptr) {
        return lcid;
    }
    // operator>> on std::string (00992480): skip whitespace, read to the next whitespace.
    auto next_token = [file](std::string& token) {
        token.clear();
        int c;
        while ((c = std::fgetc(file)) != EOF && std::isspace(static_cast<unsigned char>(c))) {
        }
        while (c != EOF && !std::isspace(static_cast<unsigned char>(c))) {
            token.push_back(static_cast<char>(c));
            c = std::fgetc(file);
        }
        return !token.empty();
    };
    std::string token;
    while (next_token(token)) {
        // 004B3FC0 over min(size, 8) bytes, then size == 8.
        if (token.size() == 8 && std::memcmp(token.data(), "Language", 8) == 0) {
            std::string value;
            next_token(value);
            switch (std::tolower(static_cast<unsigned char>(value.empty() ? '\0' : value[0]))) {
            case 'd': lcid = 0x407; break;
            case 'e': lcid = 0x409; break;
            case 'f': lcid = 0x40C; break;
            case 'i': lcid = 0x410; break;
            case 'p': lcid = 0x415; break;
            case 'r': lcid = 0x419; break;
            default: break;
            }
            break;
        }
    }
    std::fclose(file);
    return lcid;
}

// ===================================================================================================
// Segment 66: online system base and stats records.
// ===================================================================================================

bool create_online_system_00992a50(void*& published_f8a300, void*& published_f8a2fc, OnlineSystemFactory& factory) {
    if (published_f8a300 != nullptr) {
        return false;
    }
    published_f8a2fc = factory.new_and_construct_xlive_system();
    published_f8a300 = published_f8a2fc;
    return true;
}

bool destroy_online_system_00992ac0(void*& published_f8a300, void*& published_f8a2fc, OnlineSystemFactory& factory) {
    if (published_f8a300 == nullptr) {
        return false;
    }
    factory.delete_system_vslot1a4(published_f8a300);
    published_f8a300 = nullptr;
    published_f8a2fc = nullptr;
    return true;
}

XLiveStatRecord& assign_stat_record_00992f60(XLiveStatRecord& to, const XLiveStatRecord& from, NativeStringStorage& strings) {
    to.key_00 = from.key_00;
    to.key_04 = from.key_04;
    if (&to.name_08 != &from.name_08) {
        to.name_08.copy_from_00be0a30_fragment(strings, from.name_08);
    }
    to.word_10 = from.word_10;
    std::memcpy(to.values_18, from.values_18, sizeof(to.values_18));
    return to;
}

XLiveStatRecord& copy_construct_stat_record_009934d0(XLiveStatRecord& to, const XLiveStatRecord& from,
                                                     NativeStringStorage& strings) {
    // The name header is zeroed before the guarded copy (the caller passes fresh storage).
    return assign_stat_record_00992f60(to, from, strings);
}

XLiveStatRecord& construct_stat_field_00993140(XLiveStatRecord& record, std::uint32_t key0, std::uint32_t key1,
                                               NativeString name_by_value, std::uint32_t word10,
                                               NativeStringStorage& strings) {
    record.key_00 = key0;
    record.key_04 = key1;
    record.name_08.copy_from_00be0a30_fragment(strings, name_by_value);
    record.word_10 = word10;
    record.values_18[0] = 0;                        // +18h
    for (int i = 2; i <= 5; ++i) record.values_18[i] = 0;  // +20h..+2Ch
    reinterpret_cast<std::uint8_t*>(&record.values_18[6])[0] = 0;  // byte +30h
    for (int i = 7; i <= 12; ++i) record.values_18[i] = 0;  // +34h..+48h
    name_by_value.release_to(strings);
    return record;
}

void release_stat_record_name_00992f40(XLiveStatRecord& record, NativeStringStorage& strings) {
    record.name_08.release_to(strings);
}

void construct_xlive_stats_block_00994080(XLiveStatsBlockLayout& b) {
    b.vtable_00 = 0x00d1d5b0;
    // std::string at +0Ch: capacity 0Fh, size 0, first byte 0.
    *reinterpret_cast<std::uint32_t*>(b.name_0c + 0x14) = 0x0F;
    *reinterpret_cast<std::uint32_t*>(b.name_0c + 0x10) = 0;
    b.name_0c[0] = 0;
    b.word_34 = 0;
    b.byte_38 = 0;
    b.byte_40 = 0;
    b.byte_41 = 0;
    b.byte_42 = 1;
    b.byte_43 = 0;
    b.sessions_first_54 = nullptr;
    b.sessions_last_58 = nullptr;
    b.sessions_end_5c = nullptr;
    b.stats_first_68 = nullptr;
    b.stats_last_6c = nullptr;
    b.stats_end_70 = nullptr;
    b.byte_4c = 0;
    b.byte_4d = 0;
    b.network_mode_60 = 2;
}

void set_network_mode_00994120(XLiveStatsBlockLayout& block, std::uint32_t mode) noexcept {
    block.network_mode_60 = mode;
}

void xlive_base_nop_009940f0() noexcept {}

void set_player_info_00994130(std::uint8_t* system, void* info_by_value, OnlinePlayerInfoOps& ops) {
    ops.assign_006883b0(system + 8, info_by_value);
    ops.destroy_by_value_string(info_by_value);
}

void destruct_xlive_stats_block_00994180(XLiveStatsBlockLayout& b, StatsBlockTeardown& t) {
    b.vtable_00 = 0x00d1d5b0;
    if (b.stats_first_68 != nullptr) {
        t.destroy_stat_records_00993990(b.stats_first_68, b.stats_last_6c);
        t.free_00bf65ac(b.stats_first_68);
    }
    b.stats_first_68 = nullptr;
    b.stats_last_6c = nullptr;
    b.stats_end_70 = nullptr;
    if (b.sessions_first_54 != nullptr) {
        t.destroy_sessions(b.sessions_first_54, b.sessions_last_58);
        t.free_00bf65ac(b.sessions_first_54);
    }
    b.sessions_first_54 = nullptr;
    b.sessions_last_58 = nullptr;
    b.sessions_end_5c = nullptr;
    t.reset_std_string(b.name_0c);
}

// ===================================================================================================
// Segment 66: downloadable content.
// ===================================================================================================

void destruct_content_record_009946d0(ContentPackageRecord& record, NativeStringStorage& strings) {
    record.path_08.release_to(strings);
    record.name_00.release_to(strings);
}

ContentPackageRecord& copy_construct_content_record_00994850(ContentPackageRecord& to, const ContentPackageRecord& from,
                                                             NativeStringStorage& strings) {
    if (&to != &from) {
        to.name_00.copy_from_00be0a30_fragment(strings, from.name_00);
        to.path_08.copy_from_00be0a30_fragment(strings, from.path_08);
    }
    std::memcpy(to.flags_10, from.flags_10, 4);
    to.index_14 = from.index_14;
    return to;
}

std::string content_path_compose_00994770(const std::string& base) {
    return base + "\\";
}

void reserve_content_array_00994920(ContentPackageArray& a, std::int32_t n, ContentArrayMemory& memory,
                                    NativeStringStorage& strings) {
    if (n < 1) {
        n = 1;
    }
    if (a.capacity >= n) {
        return;
    }
    ContentPackageRecord* fresh = memory.allocate_records_00bf681b(n);
    for (std::int32_t i = 0; i < a.count; ++i) {
        if (fresh + i != nullptr) {
            new (fresh + i) ContentPackageRecord();
            copy_construct_content_record_00994850(fresh[i], a.data[i], strings);
        }
    }
    for (std::int32_t i = 0; i < a.count; ++i) {
        destruct_content_record_009946d0(a.data[i], strings);
    }
    memory.free_00bf65ac(a.data);
    a.data = fresh;
    a.capacity = n;
}

void push_back_content_array_00994a00(ContentPackageArray& a, const ContentPackageRecord& record,
                                      ContentArrayMemory& memory, NativeStringStorage& strings) {
    if (a.count == a.capacity) {
        std::int32_t n = a.capacity * 2;
        if (n < 2) {
            n = 1;
        }
        reserve_content_array_00994920(a, n, memory, strings);
    }
    if (a.data + a.count != nullptr) {
        new (a.data + a.count) ContentPackageRecord();
        copy_construct_content_record_00994850(a.data[a.count], record, strings);
    }
    ++a.count;
}

void resize_content_array_00994a80(ContentPackageArray& a, std::int32_t n, ContentArrayMemory& memory,
                                   NativeStringStorage& strings) {
    if (a.capacity < n) {
        reserve_content_array_00994920(a, n, memory, strings);
    }
    for (std::int32_t i = a.count; i < n; ++i) {
        ContentPackageRecord* r = a.data + i;
        if (r != nullptr) {
            new (r) ContentPackageRecord();
            std::memset(r->flags_10, 0, 4);
            r->index_14 = -1;
        }
    }
    while (n < a.count) {
        --a.count;
        destruct_content_record_009946d0(a.data[a.count], strings);
    }
    a.count = n;
}

void tidy_content_array_00994da0(ContentPackageArray& a, ContentArrayMemory& memory, NativeStringStorage& strings) {
    resize_content_array_00994a80(a, 0, memory, strings);
    memory.free_00bf65ac(a.data);
}

ContentPackageArray& assign_content_array_00994dc0(ContentPackageArray& a, const ContentPackageArray& source,
                                                   ContentArrayMemory& memory, NativeStringStorage& strings) {
    resize_content_array_00994a80(a, 0, memory, strings);
    reserve_content_array_00994920(a, source.count, memory, strings);
    for (std::int32_t i = 0; i < source.count; ++i) {
        push_back_content_array_00994a00(a, source.data[i], memory, strings);
    }
    return a;
}

void construct_download_manager_base_00994540(std::uint32_t& vtable_00, void* self, void*& published_f8a304,
                                              SingletonLifetimeCalls& lifetime) {
    vtable_00 = 0x00d1d758;
    lifetime.lock_manager();
    published_f8a304 = self;
    lifetime.register_singleton(published_f8a304);
    lifetime.unlock_manager();
}

void destruct_download_manager_base_009945e0(std::uint32_t& vtable_00, void*& published_f8a304,
                                             SingletonLifetimeCalls& lifetime) {
    vtable_00 = 0x00d1d758;
    lifetime.lock_manager();
    lifetime.unregister_singleton(published_f8a304);
    published_f8a304 = nullptr;
    lifetime.unlock_manager();
    vtable_00 = 0x00ce3818;  // the singleton base
}

void construct_content_list_00994f10(ContentPackageListLayout& list, void*& published_f8a304,
                                     SingletonLifetimeCalls& lifetime) {
    construct_download_manager_base_00994540(list.vtable_00, &list, published_f8a304, lifetime);
    list.vtable_00 = 0x00d1d75c;
    list.active_04 = ContentPackageArray{nullptr, 0, 0};
    list.pending_10 = ContentPackageArray{nullptr, 0, 0};
}

void destruct_content_list_00994e10(ContentPackageListLayout& list, void*& published_f8a304,
                                    SingletonLifetimeCalls& lifetime, ContentArrayMemory& memory,
                                    NativeStringStorage& strings) {
    list.vtable_00 = 0x00d1d75c;
    resize_content_array_00994a80(list.pending_10, 0, memory, strings);
    memory.free_00bf65ac(list.pending_10.data);
    resize_content_array_00994a80(list.active_04, 0, memory, strings);
    memory.free_00bf65ac(list.active_04.data);
    destruct_download_manager_base_009945e0(list.vtable_00, published_f8a304, lifetime);
}

void reset_content_enumeration_00994450(DownloadEnumerationState& state, XContentApi& api) {
    state.word_840 = 0;
    state.count_838 = 0;
    state.cursor_83c = 0;
    void* handle = nullptr;
    const std::int32_t created = api.create_enumerator(0x40, handle);
    if (created < 0) {
        return;
    }
    state.count_838 = 0;
    const std::int32_t result = api.enumerate(handle, state.buffer_38, 0x800, state.count_838);
    if (result == static_cast<std::int32_t>(0x80070012u)) {
        state.count_838 = 0;
    }
    if (state.count_838 <= state.cursor_83c) {
        state.cursor_83c = 0;
    }
    api.close_handle(handle);
}

// ===================================================================================================
// Segment 11: scene presentation entities and helpers.
// ===================================================================================================

namespace {

template <typename T>
T& field(std::uint8_t* base, std::size_t offset) {
    return *reinterpret_cast<T*>(base + offset);
}

void stamp_entity(std::uint8_t* entity, const FixedEntitySpec& spec) {
    field<std::uint32_t>(entity, 0x000) = spec.vtable_00;
    field<std::uint32_t>(entity, 0x010) = spec.vtable_10;
    field<std::uint32_t>(entity, 0x024) = spec.vtable_24;
    field<std::uint32_t>(entity, 0x170) = spec.vtable_170;
    field<std::uint32_t>(entity, 0x0C4) = spec.class_id;
}

}  // namespace

void store_entity_name(std::uint8_t* entity, const char* name, NativeStringStorage& strings) {
    auto& s = *reinterpret_cast<NativeString*>(entity + 0x154);
    const std::uint32_t length = name != nullptr ? static_cast<std::uint32_t>(std::strlen(name)) : 0u;
    s.resize_0041dd40(strings, length, false);
    if (s.data() != nullptr) {
        std::memcpy(s.data(), name, s.length());
    }
}

std::uint8_t* create_named_scene_entity_004e99b0(const FixedEntitySpec& spec, const char* name, void* parent,
                                                 void* frame, SceneEntityCalls& calls) {
    std::uint8_t* entity = calls.new_zeroed(spec.size);
    if (entity == nullptr) {
        return nullptr;  // the image calls vtable +98h through the null pointer here
    }
    const std::uint16_t id = calls.network_id_from_name_006af3d0(name);
    calls.construct_game_entity_00928630(entity, id);
    stamp_entity(entity, spec);
    calls.place_vslot98(entity, parent, frame);
    store_entity_name(entity, name, calls.strings());
    return entity;
}

std::uint8_t* create_camera_path_004ea760(const char* name, void* parent, const void* frame,
                                          SceneEntityCalls& calls) {
    std::uint8_t* entity = calls.new_zeroed(0x210);
    if (entity == nullptr) {
        return nullptr;
    }
    entity = calls.construct_camera_path_004e5850(entity, calls.network_id_from_name_006af3d0(name));
    std::uint32_t local_frame[16];
    std::memcpy(local_frame, frame, sizeof(local_frame));  // REP MOVSD 10h dwords at 004EA7E0
    calls.place_vslot98(entity, parent, local_frame);
    store_entity_name(entity, name, calls.strings());
    return entity;
}

std::uint8_t* construct_simple_effect_004e7ef0(std::uint8_t* entity, std::uint32_t id, SceneEntityCalls& calls) {
    calls.construct_game_entity_00928630(entity, id);
    field<std::uint32_t>(entity, 0x000) = 0x00ce8bd0;
    field<std::uint32_t>(entity, 0x010) = 0x00ce8bb4;
    field<std::uint32_t>(entity, 0x024) = 0x00ce8bac;
    field<std::uint32_t>(entity, 0x170) = 0x00ce8ba8;
    field<std::uint32_t>(entity, 0x1E4) = 0;
    field<std::uint32_t>(entity, 0x1E8) = 0;
    field<std::uint32_t>(entity, 0x0C4) = 0x5B;
    return entity;
}

std::uint8_t* construct_periodic_effect_004e7f90(std::uint8_t* entity, std::uint32_t id, SceneEntityCalls& calls) {
    calls.construct_game_entity_00928630(entity, id);
    calls.construct_tick_registration_00875890(entity + 0x1E4, entity);
    field<std::uint32_t>(entity, 0x000) = 0x00ce8d68;
    field<std::uint32_t>(entity, 0x010) = 0x00ce8d4c;
    field<std::uint32_t>(entity, 0x024) = 0x00ce8d44;
    field<std::uint32_t>(entity, 0x170) = 0x00ce8d40;
    field<std::uint32_t>(entity, 0x1E4) = 0x00ce8d28;
    field<std::uint32_t>(entity, 0x220) = 0;
    field<std::uint32_t>(entity, 0x224) = 0;
    field<std::uint32_t>(entity, 0x0C4) = 0x5C;
    return entity;
}

void destruct_periodic_effect_004e8060(std::uint8_t* entity, SceneEntityCalls& calls) {
    for (std::size_t offset : {std::size_t{0x224}, std::size_t{0x220}}) {
        if (void* object = field<void*>(entity, offset)) {
            calls.release_reference(object);
            field<void*>(entity, offset) = nullptr;
        }
    }
    calls.destroy_tick_element_00875490(entity != nullptr ? entity + 0x1E4 : nullptr);
    calls.destroy_game_entity_009287b0(entity);
}

namespace {

std::uint8_t* create_effect(std::uint32_t size, std::uint8_t* (*construct)(std::uint8_t*, std::uint32_t, SceneEntityCalls&),
                            const char* name, void* parent, void* frame, void* properties, SceneEntityCalls& calls) {
    std::uint8_t* entity = calls.new_zeroed(size);
    if (entity == nullptr) {
        return nullptr;  // the image continues with a null entity
    }
    entity = construct(entity, 0, calls);
    if (!calls.frame_jobs_deferring()) {
        calls.place_vslot98(entity, parent, frame);
    } else {
        calls.defer_hierarchy_004f03c0(properties, parent, frame);
    }
    store_entity_name(entity, name, calls.strings());
    return entity;
}

}  // namespace

std::uint8_t* create_simple_effect_004f0db0(const char* name, void* parent, void* frame, void* properties,
                                            SceneEntityCalls& calls) {
    return create_effect(0x1EC, construct_simple_effect_004e7ef0, name, parent, frame, properties, calls);
}

std::uint8_t* create_periodic_effect_004f0eb0(const char* name, void* parent, void* frame, void* properties,
                                              SceneEntityCalls& calls) {
    return create_effect(0x228, construct_periodic_effect_004e7f90, name, parent, frame, properties, calls);
}

void register_camera_path_004e7e90(std::uint8_t* entity, SceneEntityCalls& calls) {
    calls.register_base_00928560(entity);
    calls.camera_path_list_push_back_00484540(field<std::uint8_t*>(entity, 0x30), entity);
}

void unregister_camera_path_004e7eb0(std::uint8_t* entity, SceneEntityCalls& calls) {
    calls.unregister_base_00928570(entity);
    std::uint8_t* world = field<std::uint8_t*>(entity, 0x30);
    for (auto* node = field<std::uint8_t*>(world, 0x394); node != nullptr; node = field<std::uint8_t*>(node, 4)) {
        if (field<std::uint8_t*>(node, 8) == entity) {
            calls.camera_path_list_erase(world, node);
            return;
        }
    }
}

void destruct_simple_array_004e7dd0(std::uint8_t* array, FreeCalls& calls) {
    field<std::uint32_t>(array, 0) = 0x00ce8ba4;
    if (void* block = field<void*>(array, 0xC)) {
        calls.free_00bf6989(block);
        field<void*>(array, 0xC) = nullptr;
    }
    field<std::uint32_t>(array, 8) = 0;
    field<std::uint32_t>(array, 4) = 0;
}

void free_owned_buffer_004e7e20(void** cell, FreeCalls& calls) {
    calls.free_00bf6989(*cell);
    *cell = nullptr;
}

const char* landing_point_class_name_004e95f0() noexcept { return "LandingPoint"; }
const char* spawn_point_class_name_004f16e0() noexcept { return "SpawnPoint"; }
const char* land_convoy_class_name_004f2500() noexcept { return "LandConvoy"; }

bool unit_is_local_player_objective_004f2590(const void* unit, LocalObjectiveSet& local_player_set) {
    return local_player_set.contains_unit_008ddf90(unit);
}

std::int32_t sensor_category_unclassified_004f1740() noexcept { return 6; }

void stub_ret8_004f17f0(std::uint32_t, std::uint32_t) noexcept {}

// ---------------------------------------------------------------------------------------------------
// Front-end screens.

void update_front_end_screen_004f71f0(FrontEndScreenObject& screen, float seconds) {
    screen.update_20(seconds);
}

void base_screen_enter_004f75a0() noexcept {}
void base_screen_exit_004f75b0() noexcept {}
void base_screen_update_004f75c0(float) noexcept {}
void base_screen_collect_children_004f75d0(void*) noexcept {}
void movie_player_vslot18_noop_004f8940() noexcept {}

bool pointer_vector_is_empty_004f7230(const NativeVectorHeader& vector) noexcept {
    if (vector.first == nullptr) {
        return true;
    }
    return ((vector.last - vector.first) >> 2) == 0;
}

void restore_level_set_004f8390(std::int32_t level, const void* saved_set, std::uint8_t& recompute_byte_e18cdc,
                                ScreenSetCalls& calls) {
    recompute_byte_e18cdc = 1;
    calls.assign_level_vector_004f81d0(level, saved_set);
    calls.recompute_requests_004f7620();
}

bool movie_player_is_playing_named_004f8b60(const void* player_name_fc, const std::string& name,
                                            MoviePlayerCalls& calls) {
    return calls.equals_insensitive_00435c40(player_name_fc, name) && calls.movie_is_playing();
}

void reset_tactical_library_order_004f8fa0(std::uint8_t* screen) noexcept {
    std::memset(screen + 0x138, 0, 0x1D8 - 0x138);
}

SignalledFlag& construct_signalled_flag_004f90a0(SignalledFlag& flag, void* (*create_event)()) {
    flag.word_00 = 0;
    flag.word_04 = 0;
    flag.event_08 = create_event();
    return flag;
}

void read_navigation_actions_004f90d0(std::uint8_t& out0, std::uint8_t& out1, std::uint8_t& out2,
                                      bool (*action_edge_or_repeat)(std::int32_t)) {
    out0 = action_edge_or_repeat(0x46) ? 1 : 0;
    out1 = action_edge_or_repeat(0x47) ? 1 : 0;
    out2 = action_edge_or_repeat(0x4A) ? 1 : 0;
}

void tactical_library_on_focus_004f91a0(GuiWidget* const* rows_a_170, GuiWidget* const* rows_b_210,
                                        GuiWidget* arrow_50c, GuiWidget* arrow_510, GuiWidget& widget,
                                        bool focused) {
    if (widget.kind_5c() == 3) {
        widget.set_alpha_4c(focused ? 0.5f : 1.0f);  // 00CE3800 / 00D7A24C
        return;
    }
    if (&widget == arrow_50c || &widget == arrow_510) {
        if (widget.state_94() == 3) {
            return;
        }
    } else {
        for (int i = 0; i < 0x28; ++i) {
            if (&widget == rows_a_170[i] || &widget == rows_b_210[i]) {
                if (widget.kind_5c() != 6) {
                    return;
                }
                if (widget.state_94() > 2) {
                    return;
                }
                break;
            }
        }
    }
    if (widget.kind_5c() != 6) {
        if (widget.kind_5c() == 0x12) {
            widget.set_highlight_84(focused);
        }
        return;
    }
    widget.set_focus_88(focused, 0, 1.0f);
}

void destruct_tactical_library_singleton_004f93a0(std::uint32_t& vtable_00, void*& published_e18d4c) noexcept {
    published_e18d4c = nullptr;
    vtable_00 = 0x00ce3818;
}

void read_lua_integer_field_004f9980(const char* key, std::int32_t& out, LuaFieldReader& table) {
    out = table.integer_field(key);
}

int format_string128_004f9a80(char* dst, const char* format, ...) {
    va_list args;
    va_start(args, format);
    const int written = vsprintf_s(dst, 0x80, format, args);
    va_end(args);
    return written;
}

std::int32_t to_upper_ascii_004f9ab0(const char* c) noexcept {
    const auto value = static_cast<signed char>(*c);
    if (value > '`' && value < '{') {
        return value - 0x20;
    }
    return value;
}

}  // namespace bsp::engine_seg11_35_66_cc13
