#define _CRT_SECURE_NO_WARNINGS  // The image's own strcpy / sprintf / fopen calls are reproduced.
// Segments 11, 35 and 66 leaves (cc13-sys3). See include/bsp/engine_seg11_35_66_cc13.hpp and
// docs/CC13_SYS_3.md for the evidence, the original ABIs and what is not claimed.

#include "bsp/engine_seg11_35_66_cc13.hpp"
#include "bsp/cc13_x87_model.hpp"
#include "bsp/oct18_tail_c.hpp"

#include <windows.h>
#include <intrin.h>

#include <cctype>
#include <cmath>
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
                                                    bool load, MessageManagerCalls& calls) {
    std::array<std::uint8_t, 4> flags{};
    const std::int32_t kind = options.local_side_kind;
    const std::size_t slot = kind == 0 ? 1u : (kind != 1 ? 3u : 2u);
    flags[slot] = 1;
    const NativeIntArray& first_ids = kind != 0 ? options.ids_1074 : options.ids_1068;
    const NativeIntArray& second_ids = kind != 0 ? options.ids_1068 : options.ids_1074;
    init_message_filter_007021c0(manager.filter_20, manager.flags_59, first_ids, options.filters_enabled);
    init_message_filter_007021c0(manager.filter_33, manager.flags_5c, second_ids, options.filters_enabled);
    init_message_filter_007021c0(manager.filter_46, manager.flags_5f, options.ids_1080, options.filters_enabled);
    // 007066C4..007066DB: the argument is 00706160's load flag; its side is the -1 at 007066C8.
    load_actors_and_messages_00706160(calls, load, "Scripts/datatables/Messages.lua", flags, -1);
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

void respawn_fish_007077b0(FishSchoolDescLayout& school, PresentationRandom& random, VectorNormalizer& normalizer) {
    const FishSchoolParams& p = school.params_04;
    const std::int32_t count = p.count_00;
    // 007077B7..007077DD: U(+50h, +54h) times (count - 1) by FIMUL, then __ftol (truncation).
    const std::int32_t n = static_cast<std::int32_t>(random.uniform_00bd2f10(p.respawn_min_4c, p.respawn_max_50) *
                                                     static_cast<float>(count - 1));
    const std::int32_t start = static_cast<std::int32_t>(random.uniform_00bd2f10(1.0f, static_cast<float>(count - n)));
    auto* positions = static_cast<float*>(school.positions_60);
    float saved[3];
    std::memcpy(saved, positions + start * 3, sizeof(saved));
    std::memmove(positions + start * 3, positions + (start + 1) * 3, static_cast<std::size_t>(n * 3 - 3) * 4);
    std::memcpy(positions + (start + n - 1) * 3, saved, sizeof(saved));
    auto* state = reinterpret_cast<float*>(school.state_68);
    state[0] = random.uniform_00bd2f10(p.speed_min_44, p.speed_max_48);  // +68h from +48h/+4Ch
    const float* fish0 = static_cast<const float*>(school.fish_64);
    const float half[3] = {p.box_half_x_10, p.box_half_y_14, p.box_half_z_18};  // +14h/+18h/+1Ch
    bool inside = true;
    for (int axis = 0; axis < 3 && inside; ++axis) {
        if (-half[axis] > fish0[axis] || fish0[axis] > half[axis]) {
            inside = false;
        }
    }
    float* direction = state + 1;  // +6Ch
    if (inside) {
        direction[0] = random.uniform_00bd2f10(-1.0f, 1.0f);  // 00D7A260
        direction[1] = 0.0f;
        direction[2] = random.uniform_00bd2f10(-1.0f, 1.0f);
    } else {
        direction[0] = -0.0f - fish0[0];
        direction[1] = -0.0f - fish0[1];
        direction[2] = -0.0f - fish0[2];
    }
    normalizer.normalize_0042b260(direction);
}

void init_fish_school_00709310(FishSchoolDescLayout& school, const FishSchoolParams& params, PresentationRandom& random,
                               FishSchoolInitCalls& calls, NativeStringStorage& strings) {
    assign_fish_school_params_00708c60(school.params_04, params, strings);
    const FishSchoolParams& p = school.params_04;
    const auto count = static_cast<std::uint32_t>(p.count_00);
    school.fish_64 = calls.new_saturating_00bf55be(count, 0x28);
    school.positions_60 = calls.new_saturating_00bf55be(count, 0x0C);
    auto* state = reinterpret_cast<float*>(school.state_68);
    state[1] = 0.0f;  // +6Ch
    state[2] = 0.0f;  // +70h
    state[3] = 1.0f;  // +74h
    auto* fish = static_cast<float*>(school.fish_64);
    auto* positions = static_cast<float*>(school.positions_60);
    constexpr double kPi = 3.1415927410125732;  // double 00CE3D28
    for (std::int32_t i = 0; i < p.count_00; ++i) {
        const float fi = static_cast<float>(i);
        auto angle = [&] { return static_cast<float>(static_cast<double>(fi / static_cast<float>(p.count_00)) * kPi); };
        const float sx = static_cast<float>(std::sin(angle()));
        const float x = (p.radius_x_04 * sx) * random.uniform_00bd2f10(-0.5f, 0.5f);  // 00CE69D0 / 00CE3800
        const float sy = static_cast<float>(std::sin(angle()));
        const float y = (p.radius_y_08 * sy) * random.uniform_00bd2f10(-0.5f, 0.5f);
        const float cz = static_cast<float>(std::cos(angle()));
        const float z = static_cast<float>(static_cast<float>((cz - 1.0) * p.radius_z_0c) * 0.5);
        float* entry = fish + i * 10;  // 28h bytes
        entry[0] = x;
        entry[1] = y;
        entry[2] = z;
        positions[i * 3 + 0] = entry[0];
        positions[i * 3 + 1] = entry[1];
        positions[i * 3 + 2] = entry[2];
        entry[3] = 0.0f;
        entry[4] = 0.0f;
        entry[5] = 1.0f;
        entry[6] = entry[3];
        entry[7] = entry[4];
        entry[8] = entry[5];
        entry[9] = p.word_38;
        fish[9] = static_cast<float>((p.word_3c + p.word_38) * 0.5);
    }
    fish[9] = static_cast<float>((p.word_3c + p.word_38) * 0.5);  // 00709538, also for count 0
    school.model_78 = calls.allocate_fish_model();
    calls.build_geometry_00708d20();
    calls.model_vslot30(school.model_78, reinterpret_cast<std::uint8_t*>(&school) + 0x20);
}

FishSchoolDescLayout* create_fish_school_00709620(std::int32_t kind, const float position[3], PresentationRandom& random,
                                                  FishSchoolCreateCalls& calls, NativeStringStorage& strings) {
    FishSchoolParams params{};
    init_fish_school_params_00708b00(params, strings);
    params.word_1c = position[0];
    params.word_20 = position[1];
    params.word_24 = position[2];
    static const char* const kTextures[5] = {"Fish_01.tga", "Fish_02.tga", "Fish_03.tga", "Fish_04.tga", "Fish_05.tga"};
    if (kind >= 1 && kind <= 5) {
        assign_native(params.texture_54, strings, kTextures[kind - 1]);  // 0041E350
    }
    FishSchoolDescLayout* school = calls.new_school_00bf681b();
    if (school != nullptr) {
        new (school) FishSchoolDescLayout{};
        construct_fish_school_desc_00708be0(*school, strings);
        init_fish_school_00709310(*school, params, random, calls, strings);
    }
    params.texture_54.release_to(strings);
    return school;
}

namespace {

// cc13-fixture4: 00709EF0 runs on the main thread (00709EF0 <- 0070AA70 flag wave update), so
// precision control is 24-bit: each x87 operation rounds once to 24 bits with the extended
// exponent, each FSTP float is a cast. The groupings below are the image's stack order.
using cc13x87::x87_add24;
using cc13x87::x87_sub24;
using cc13x87::x87_mul24;
using cc13x87::x87_div24;
using cc13x87::x87_sqrt24;
float fstp(double v) { return static_cast<float>(v); }

// One structural spring of 00709EF0 between points a and b (0070A032..0070A1EE).
void flag_spring(FlagClothLayout& cloth, const float* positions, const float* velocities, std::int32_t a,
                 std::int32_t b, float rest) {
    const float* pa = positions + a * 3;
    const float* pb = positions + b * 3;
    const float dx = fstp(x87_sub24(pb[0], pa[0]));
    const float dy = fstp(x87_sub24(pb[1], pa[1]));
    const float dz = fstp(x87_sub24(pb[2], pa[2]));
    const float sq = fstp(x87_add24(x87_add24(x87_mul24(dx, dx), x87_mul24(dy, dy)), x87_mul24(dz, dz)));
    // FCOMI sq, [00CE3820] = 1e-10 / JBE: not greater (NaN included) gives 0; the division by
    // 0 that follows is the image's. 00BF7030 sqrt, stored to float.
    const float len = !(static_cast<double>(sq) > 1e-10) ? 0.0f : fstp(x87_sqrt24(sq));
    const float inv = fstp(x87_div24(1.0, len));
    const float ux = fstp(x87_mul24(dx, inv));
    const float uy = fstp(x87_mul24(dy, inv));
    const float uz = fstp(x87_mul24(inv, dz));
    const float fs = fstp(x87_mul24(x87_sub24(len, rest), cloth.stiffness_4c));
    const float sx = fstp(x87_mul24(fs, ux)), sy = fstp(x87_mul24(uy, fs)), sz = fstp(x87_mul24(fs, uz));
    const float* va = velocities + a * 3;
    const float* vb = velocities + b * 3;
    const float dvx = fstp(x87_sub24(vb[0], va[0]));
    const float dvy = fstp(x87_sub24(vb[1], va[1]));
    const float dvz = fstp(x87_sub24(vb[2], va[2]));
    const float dot = fstp(x87_add24(x87_add24(x87_mul24(dvx, ux), x87_mul24(dvy, uy)), x87_mul24(dvz, uz)));
    const float fd = fstp(x87_mul24(dot, cloth.damping_50));
    const float fx = fstp(x87_add24(fstp(x87_mul24(ux, fd)), sx));
    const float fy = fstp(x87_add24(fstp(x87_mul24(uy, fd)), sy));
    const float fz = fstp(x87_add24(fstp(x87_mul24(uz, fd)), sz));
    float* fa = cloth.forces_18 + a * 3;
    float* fb = cloth.forces_18 + b * 3;
    fa[0] = fstp(x87_add24(fx, fa[0]));
    fa[1] = fstp(x87_add24(fy, fa[1]));
    fa[2] = fstp(x87_add24(fz, fa[2]));
    fb[0] = fstp(x87_sub24(fb[0], fx));
    fb[1] = fstp(x87_sub24(fb[1], fy));
    fb[2] = fstp(x87_sub24(fb[2], fz));
}

}  // namespace

void apply_flag_wind_00709ef0(FlagClothLayout& cloth, const float* positions, const float* velocities) {
    for (std::int32_t i = 0; i < cloth.count_04; ++i) {
        const float* n = cloth.normals_1c + i * 3;
        float* f = cloth.forces_18 + i * 3;
        const float* w = cloth.wind_20;
        // 00709F23..00709F60: (ny wy + wx nx) + nz wz and (ny ny + nx nx) + nz nz, each stored
        // to float, then the quotient stored to float.
        const float num = fstp(x87_add24(x87_add24(x87_mul24(n[1], w[1]), x87_mul24(w[0], n[0])),
                                         x87_mul24(n[2], w[2])));
        const float den = fstp(x87_add24(x87_add24(x87_mul24(n[1], n[1]), x87_mul24(n[0], n[0])),
                                         x87_mul24(n[2], n[2])));
        const float s = fstp(x87_div24(num, den));
        f[0] = fstp(x87_mul24(n[0], s));
        f[1] = fstp(x87_mul24(n[1], s));
        f[2] = fstp(x87_mul24(s, n[2]));
        f[1] = fstp(x87_add24(cloth.gravity_58, f[1]));
        const float* v = velocities + i * 3;
        const float drag = cloth.velocity_drag_54;
        const float px = fstp(x87_mul24(v[0], drag)), py = fstp(x87_mul24(v[1], drag)),
                    pz = fstp(x87_mul24(drag, v[2]));
        f[0] = fstp(x87_add24(f[0], px));
        f[1] = fstp(x87_add24(py, f[1]));
        f[2] = fstp(x87_add24(f[2], pz));
    }
    for (std::int32_t r = 0; r < cloth.rows_40; ++r) {
        for (std::int32_t c = 0; c < cloth.columns_3c - 1; ++c) {
            const std::int32_t a = cloth.columns_3c * r + c;
            flag_spring(cloth, positions, velocities, a, a + 1, cloth.rest_horizontal_5c);
        }
    }
    for (std::int32_t r = 0; r < cloth.rows_40 - 1; ++r) {
        for (std::int32_t c = 0; c < cloth.columns_3c; ++c) {
            flag_spring(cloth, positions, velocities, cloth.columns_3c * r + c, cloth.columns_3c * (r + 1) + c,
                        cloth.rest_vertical_60);
        }
    }
}
void integrate_flag_cloth_0070a450(const FlagClothLayout& cloth, float* out_a, float* out_b, const float* in_a,
                                   const float* in_b, const float* vel, float dt) {
    const float* force = cloth.forces_18;
    for (std::int32_t i = 0; i < cloth.count_04; ++i) {
        out_a[0] = in_a[0] + vel[0] * dt;
        out_a[1] = in_a[1] + vel[1] * dt;
        out_a[2] = in_a[2] + vel[2] * dt;
        const float w = (static_cast<float>(i % cloth.columns_3c) / static_cast<float>(cloth.columns_3c - 1)) * dt;
        out_b[0] = in_b[0] + w * force[0];
        out_b[1] = in_b[1] + force[1] * w;
        out_b[2] = in_b[2] + w * force[2];
        out_a += 3; out_b += 3; in_a += 3; in_b += 3; vel += 3; force += 3;
    }
}

void relax_flag_grid_0070a5a0(FlagClothLayout& cloth, VectorNormalizer& normalizer) {
    for (std::int32_t r = 0; r < cloth.rows_40; ++r) {
        for (std::int32_t c = 1; c < cloth.columns_3c; ++c) {
            float* p = cloth.positions_08 + (cloth.columns_3c * r + c) * 3;
            float d[3] = {p[0] - p[-3], p[1] - p[-2], p[2] - p[-1]};
            normalizer.normalize_0042b260(d);
            const float rest = cloth.rest_horizontal_5c;
            d[0] *= rest;
            d[1] *= rest;
            d[2] *= rest;
            p[0] = d[0] + p[-3];
            p[1] = d[1] + p[-2];
            p[2] = d[2] + p[-1];
        }
    }
}

void update_flag_wave_0070aa70(FlagClothLayout& cloth, float dt, float game_time_648, float& published_e19b10,
                               PresentationRandom& random, FlagClothCalls& calls) {
    cloth.time_74 = game_time_648;
    published_e19b10 = game_time_648;
    if (dt > 0.5f) {
        dt = 0.5f;
    }
    const std::int32_t steps = 1 - static_cast<std::int32_t>(dt / -0.02500000037252903);
    const float sub = static_cast<float>(dt / steps);
    const float c = static_cast<float>(std::cos(cloth.angle_2c));
    const float s = static_cast<float>(std::sin(cloth.angle_2c));
    cloth.wind_20[0] = random.uniform_00bd2f10(-0.01f, 0.01f);  // 00D7A300 / 00D7A238
    cloth.wind_20[1] = s;
    cloth.wind_20[2] = -0.0f - c;
    calls.normalize_0042b260(cloth.wind_20);
    for (float& w : cloth.wind_20) {
        w = static_cast<float>(w * 100.0);  // double 00D7A220
    }
    for (std::int32_t i = steps; i > 0; --i) {
        const float half = static_cast<float>(sub * 0.5);
        calls.calculate_normals_00709cf0(cloth);
        apply_flag_wind_00709ef0(cloth, cloth.positions_08, cloth.velocities_10);
        integrate_flag_cloth_0070a450(cloth, cloth.positions_tmp_0c, cloth.velocities_tmp_14, cloth.positions_08,
                                      cloth.velocities_10, cloth.velocities_10, half);
        calls.calculate_normals_00709cf0(cloth);
        apply_flag_wind_00709ef0(cloth, cloth.positions_tmp_0c, cloth.velocities_tmp_14);
        integrate_flag_cloth_0070a450(cloth, cloth.positions_08, cloth.velocities_10, cloth.positions_08,
                                      cloth.velocities_10, cloth.velocities_tmp_14, sub);
    }
}

void write_flag_vertices_0070a710(FlagClothLayout& cloth, VertexStreamCalls& calls) {
    calls.render_lock();
    const VertexStreamView v = calls.lock_vslot10(cloth.vertex_stream_78, cloth.count_04 * 2);
    calls.render_unlock();
    auto at = [&](std::int32_t vertex, std::int32_t offset) {
        return reinterpret_cast<float*>(v.data_08 + v.stride_0c * vertex + offset);
    };
    auto white = [&](std::int32_t vertex) {
        if (v.colour_34 < 0) {
            *at(vertex, v.channel_38) = 1.0f;
            *at(vertex, v.channel_3c) = 1.0f;
            *at(vertex, v.channel_40) = 1.0f;
            *at(vertex, v.channel_44) = 1.0f;
        } else {
            *reinterpret_cast<std::uint32_t*>(at(vertex, v.colour_34)) = 0xFFFFFFFFu;
        }
    };
    for (std::int32_t i = 0; i < cloth.count_04; ++i) {
        const float* p = cloth.positions_08 + i * 3;
        const float* n = cloth.normals_1c + i * 3;
        const std::int32_t back = cloth.count_04 + i;
        std::memcpy(at(i, v.position_10), p, 12);
        float* nf = at(i, v.normal_1c);
        nf[0] = static_cast<float>(-n[0] * 0.5);
        nf[1] = static_cast<float>(-n[1] * 0.5);
        nf[2] = static_cast<float>(-n[2] * 0.5);
        std::memcpy(at(back, v.position_10), p, 12);
        float* nb = at(back, v.normal_1c);
        nb[0] = static_cast<float>(n[0] * 0.5);
        nb[1] = static_cast<float>(n[1] * 0.5);
        nb[2] = static_cast<float>(n[2] * 0.5);
        const std::int32_t columns = cloth.columns_3c;
        const float u = (static_cast<float>(i % columns) / static_cast<float>(columns - 1) - 0.0f) *
                            ((cloth.uv_u1_6c - cloth.uv_u0_64) / 1.0f) + cloth.uv_u0_64;
        const float w = (static_cast<float>(i / columns) / static_cast<float>(cloth.rows_40 - 1) - 0.0f) *
                            ((cloth.uv_v1_70 - cloth.uv_v0_68) / 1.0f) + cloth.uv_v0_68;
        float* uvf = at(i, v.uv_28);
        uvf[0] = u;
        uvf[1] = w;
        white(i);
        float* uvb = at(back, v.uv_28);
        uvb[0] = u;
        uvb[1] = w;
        white(back);
    }
    calls.render_lock();
    calls.unlock_vslot14(cloth.vertex_stream_78);
    calls.render_unlock();
}

void build_flag_material_0070b9d0(FlagClothLayout& cloth, std::uint32_t index_count_80, void* index_stream_7c,
                                  void* mesh, const NativeString& texture, FlagMaterialCalls& calls) {
    calls.set_vertex_stream_00b73bb0(mesh, 0, cloth.vertex_stream_78);
    calls.set_index_stream_00b73b70(mesh, index_stream_7c);
    void* effect = calls.load_effect_vslot48("flag.mshd");
    void* material = calls.new_material_00b18780_00b18900(effect);
    calls.release_reference(effect);
    if (texture.length() != 0) {
        void* image = calls.load_texture_vslot64(texture);
        calls.set_material_texture_00b189f0(material, 0, image);
        calls.release_reference(image);
    }
    std::uint32_t* section = calls.new_draw_section_00b85ee0_00b857f0();
    calls.set_section_material_00b864c0(section, material);
    calls.release_reference(material);
    if (section != nullptr) {
        section[2] = 4;                                                  // +8h
        section[3] = 0;                                                  // +0Ch
        section[5] = 0;                                                  // +14h
        section[4] = static_cast<std::uint32_t>(cloth.count_04 * 2);   // +10h
        section[6] = index_count_80;                                     // +18h
    }
    calls.clear_section_owners_00b73c10(mesh);
    calls.rebuild_vertex_layout_00b865a0(section, mesh);
    calls.append_section_00b73c60(mesh, section);
    calls.release_reference(section);
}

void build_fish_geometry_00708d20(std::uint8_t* model, FishSchoolDescLayout& school, void* root_19ec,
                                  FishGeometryCalls& calls) {
    *reinterpret_cast<FishSchoolDescLayout**>(model + 0x184) = &school;
    void* mesh = calls.new_mesh_00b73b60_00b73d70();
    calls.set_model_geometry_00b75170(model, mesh, -1.0f, -1.0f);  // 00D7A260 twice
    const FishSchoolParams& p = school.params_04;
    const float sq = (p.box_half_y_14 * p.box_half_y_14 + p.box_half_x_10 * p.box_half_x_10) +
                     p.box_half_z_18 * p.box_half_z_18;
    const float sphere[4] = {0.0f, 0.0f, 0.0f, static_cast<float>(std::sqrt(static_cast<double>(sq)))};
    calls.set_mesh_bounds_00b72650(mesh, sphere);
    void* declaration = calls.vertex_declaration_vslot38("simplecolor.mvfm");
    const std::int32_t count = p.count_00;
    void* vertices = calls.create_vertex_stream_vslot5c(count * 4, 1, declaration);
    calls.set_vertex_stream_00b73bb0(mesh, 0, vertices);
    void* indices = calls.create_index_stream_vslot60(count * 6, 1, 0x65);
    calls.set_index_stream_00b73b70(mesh, indices);
    std::uint16_t* index = calls.lock_indices_vslot0c(indices);
    for (std::int32_t i = 0; i < count; ++i) {
        const auto b = static_cast<std::uint16_t>(i * 4);
        *index++ = b;
        *index++ = static_cast<std::uint16_t>(b + 1);
        *index++ = static_cast<std::uint16_t>(b + 2);
        *index++ = b;
        *index++ = static_cast<std::uint16_t>(b + 2);
        *index++ = static_cast<std::uint16_t>(b + 3);
    }
    calls.unlock_indices_vslot10(indices);
    const VertexStreamView v = calls.lock_vertices_vslot10(vertices);
    static const float kUv[4][2] = {{0.0f, 1.0f}, {1.0f, 1.0f}, {1.0f, 0.0f}, {0.0f, 0.0f}};
    for (std::int32_t vertex = 0; vertex < count * 4; ++vertex) {
        auto* uv = reinterpret_cast<float*>(v.data_08 + v.stride_0c * vertex + v.uv_28);
        uv[0] = kUv[vertex & 3][0];
        uv[1] = kUv[vertex & 3][1];
        std::uint8_t* row = v.data_08 + v.stride_0c * vertex;
        if (v.colour_34 < 0) {
            for (std::int32_t channel : {v.channel_38, v.channel_3c, v.channel_40, v.channel_44}) {
                *reinterpret_cast<float*>(row + channel) = 1.0f;
            }
        } else {
            *reinterpret_cast<std::uint32_t*>(row + v.colour_34) = 0xFFFFFFFFu;
        }
    }
    calls.unlock_vslot14(vertices);
    calls.release_reference(indices);
    calls.release_reference(vertices);
    calls.release_reference(declaration);
    void* effect = calls.load_effect_vslot48("fish.mshd");
    void* material = calls.new_material_00b18780_00b18900(effect);
    calls.release_reference(effect);
    if (p.texture_54.length() != 0) {
        void* image = calls.load_texture_vslot64(p.texture_54);
        calls.set_material_texture_00b189f0(material, 0, image);
        calls.release_reference(image);
    }
    std::uint32_t* section = calls.new_draw_section_00b85ee0_00b857f0();
    calls.set_section_material_00b864c0(section, material);
    calls.release_reference(material);
    if (section != nullptr) {
        section[4] = static_cast<std::uint32_t>(count * 4);  // +10h
        section[2] = 4;                                      // +8h
        section[3] = 0;                                      // +0Ch
        section[5] = 0;                                      // +14h
        section[6] = static_cast<std::uint32_t>(count * 2);  // +18h
    }
    calls.set_section_bounds_00b855b0(section, sphere);
    calls.rebuild_vertex_layout_00b865a0(section, mesh);
    calls.append_section_00b73c60(mesh, section);
    calls.release_reference(section);
    calls.release_reference(mesh);
    calls.propagate_root_registration_00b6d890(model, root_19ec);
}

void render_flag_model_0070ac00(std::uint8_t* model, void* context, float lod, float alpha, std::uint32_t mask,
                                FlagRenderCalls& calls) {
    if (std::uint8_t* parent = *reinterpret_cast<std::uint8_t**>(model + 0x30)) {
        const std::uint32_t inherited = *reinterpret_cast<std::uint32_t*>(parent + 0x48);
        *reinterpret_cast<std::uint32_t*>(model + 0x48) = inherited;
        for (auto* child = *reinterpret_cast<std::uint8_t**>(model + 0x34); child != nullptr;
             child = *reinterpret_cast<std::uint8_t**>(child + 0x3C)) {
            calls.set_hierarchy_mask_007099c0(child, inherited);
        }
    }
    if (lod > 0.5f) {
        return;
    }
    float clamped = lod;
    if (0.4000000059604645 > static_cast<double>(lod)) {
        clamped = 0.4f;  // 00CE7804
    } else if (lod > 0.5f) {
        clamped = 0.5f;
    }
    const float fade = static_cast<float>((0.5 - clamped) / 0.10000000149011612);
    calls.render_generated_model_00b748e0(model, context, lod, fade * alpha, mask);
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
    // 0098D57C MOV EDX,0Bh (REG_QWORD) with 0098D571 PUSH 4: four bytes typed REG_QWORD, so
    // 0098D5C0's REG_QWORD read of the value succeeds. EAX is 0098D470's answer (0098D586 POP ECX).
    return write_hardware_value_0098d470(value_name, REG_QWORD, &value, 4);
}

bool write_hardware_string_0098d590(const char* value_name, const char* text) {
    return write_hardware_value_0098d470(value_name, REG_SZ, text, static_cast<std::uint32_t>(std::strlen(text) + 1));
}

bool read_hardware_qword_0098d5c0(const char* value_name, std::uint32_t& out, std::uint32_t scratch_low) {
    std::uint32_t buffer[2] = {scratch_low, 0};
    std::uint32_t size = 8;
    // 0098D5DD CALL 0098D4E0 (REG_QWORD). Nothing after it writes EAX (0098D5E2 MOV ECX,
    // 0098D5E6 store, POP ESI, ADD ESP), so 0098D4E0's AL is this function's answer.
    const bool ok = read_hardware_value_0098d4e0(value_name, 0x0B, buffer, &size);
    out = buffer[0];  // 0098D5E6: stored whether or not the read succeeded
    return ok;
}

bool read_hardware_string_0098d5f0(const char* value_name, char* buffer, std::uint32_t* size) {
    // 0098D5FB CALL 0098D4E0, then 0098D600 RET 4: EAX is 0098D4E0's answer.
    return read_hardware_value_0098d4e0(value_name, REG_SZ, buffer, size);
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

void query_display_driver_00991ef0(const D3DAdapterSurvey& survey, const GpuTableRow* table, DisplayDriverFields fields,
                                   DisplayModeList& lists) {
    std::sprintf(fields.driver_b34, "%d.%d.%d.%d", survey.driver_version_high >> 16, survey.driver_version_high & 0xFFFF,
                 survey.driver_version_low >> 16, survey.driver_version_low & 0xFFFF);
    std::sprintf(fields.description_a34, "%s", survey.description.c_str());
    *fields.gpu_index_28 = -1;
    if (table[0].name != nullptr) {
        for (std::int32_t i = 0; table[i].name != nullptr; ++i) {
            if (table[i].device_id == survey.device_id) {
                *fields.gpu_index_28 = i;  // no break: the last match wins
            }
        }
    }
    for (const auto& mode : survey.modes_x8r8g8b8) {
        if (mode.first >= 0x280 && mode.second >= 0x1E0 && !lists.contains_0098e0e0(mode.first, mode.second)) {
            lists.append_00991c20(mode.first, mode.second);
        }
    }
    for (std::int32_t type = 2; type < 0x10; ++type) {
        if (static_cast<std::size_t>(type) < survey.multisample_ok.size() && survey.multisample_ok[type]) {
            lists.append_multisample_type(type);
        }
    }
    *fields.shader_model_24 = ((survey.pixel_shader_version & 0xFFFF) >= 0x200 ? 1 : 0) + 1;
    static const std::uint32_t kForcedModel1[16] = {0x301, 0x302, 0x311, 0x312, 0x313, 0x314, 0x320, 0x321,
                                                    0x322, 0x323, 0x326, 0x327, 0x341, 0x342, 0x343, 0x344};
    for (std::uint32_t id : kForcedModel1) {
        if (survey.device_id == id) {
            *fields.shader_model_24 = 1;
        }
    }
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

void fill_stats_from_game_00993220(std::vector<XLiveStatRecord>& records, std::uint8_t& ready_74,
                                   StatsGameView& game) {
    const std::int32_t mode = game.effective_game_mode();
    for (XLiveStatRecord& record : records) {
        for (std::size_t slot = 0; slot < 8; ++slot) {
            const StatsPlayerSlot player = game.player_slot(slot);
            if (!player.present || player.flag_09 || record.key_00 != player.key_80 || record.key_04 != player.key_84) {
                continue;
            }
            // Null-aware case-insensitive name compare (00993318..0099337C).
            int cmp;
            if (record.name_08.length() == 0) {
                cmp = player.name_58.empty() ? 0 : -1;
            } else if (player.name_58.empty()) {
                cmp = 1;
            } else {
                cmp = _stricmp(record.name_08.data(), player.name_58.c_str());
            }
            if (cmp != 0) {
                continue;
            }
            std::uint32_t badges = 0;
            for (const std::string& key : game.side_badge_keys(slot)) {
                if (game.badge_earned_0090c000(key)) {
                    ++badges;
                }
            }
            record.values_18[12] = game.side_word_174(slot);  // +48h
            record.values_18[11] = game.side_word_118(slot);  // +44h
            record.values_18[5] = static_cast<std::uint32_t>(mode);  // +2Ch
            record.values_18[10] = badges;                    // +40h
        }
    }
    ready_74 = 1;
}

void reset_stats_values_00994310(std::vector<XLiveStatRecord>& records, std::uint8_t& ready_74, StatsGameView& game,
                                 NativeStringStorage& strings) {
    for (XLiveStatRecord& record : records) {
        record.name_08.release_to(strings);
    }
    records.clear();
    for (std::size_t slot = 0; slot < 8; ++slot) {
        const StatsPlayerSlot player = game.player_slot(slot);
        if (!player.present || player.flag_09) {
            continue;
        }
        NativeString name;
        assign_native(name, strings, player.name_58);
        XLiveStatRecord record{};
        construct_stat_field_00993140(record, player.key_80, player.key_84, std::move(name), player.word_28, strings);
        records.push_back(std::move(record));
    }
    ready_74 = 0;
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

// 00994EB0 / 00994EC0 / 00994ED0: ContentPackageList slots 7/8/9 are a
// single RET each. 00994EE0 (slot 10) is RET 4. All preserve EAX and have no
// observable body side effects; method purposes are provisional.
void content_list_slot7_noop_00994eb0(ContentPackageListLayout&) noexcept {}
void content_list_slot8_noop_00994ec0(ContentPackageListLayout&) noexcept {}
void content_list_slot9_noop_00994ed0(ContentPackageListLayout&) noexcept {}
void content_list_slot10_noop_00994ee0(ContentPackageListLayout&, std::uint32_t) noexcept {}

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

namespace {

// The per-row display-name read shared by 00994B10 and 00994F40: a size query that must answer
// 8007007Ah with at most 100h characters, then the read, then the low-byte narrowing into the
// record's name. False leaves the record untouched.
bool read_content_name(const std::uint8_t* row, ContentPackageRecord& record, DownloadManagerCalls& calls) {
    std::uint32_t size = 0;
    if (calls.content_display_name(row, nullptr, size) != static_cast<std::int32_t>(0x8007007Au) || size > 0x100) {
        return false;
    }
    wchar_t buffer[0x101] = {};
    calls.content_display_name(row, buffer, size);
    assign_native(record.name_00, calls.strings(), calls.narrow_low_bytes_00436630(buffer));
    return true;
}

}  // namespace

bool enumerate_new_content_00994b10(DownloadManagerFields& manager, DownloadManagerCalls& calls,
                                    ContentArrayMemory& memory) {
    static_cast<void>(memory);
    if (!calls.has_selected_user_00a3e510()) {
        return false;
    }
    bool appended = false;
    reset_content_enumeration_00994450(*manager.enumeration, calls);
    const ContentPackageArray& active = manager.list->active_04;
    for (std::uint32_t i = 0; i < manager.enumeration->count_838; ++i) {
        ContentPackageRecord record{};
        record.index_14 = -1;
        if (read_content_name(manager.enumeration->buffer_38 + i * 0x20, record, calls)) {
            std::int32_t found = -1;
            for (std::int32_t j = 0; j < active.count && found == -1; ++j) {
                if (calls.matches_at_offset_0043e9a0(active.data[j].name_00, record.name_00)) {
                    found = j;
                }
            }
            if (found == -1) {
                calls.append_new_content_name_004cdc20(record.name_00);
                appended = true;
            }
        }
        destruct_content_record_009946d0(record, calls.strings());
    }
    return appended;
}

void refresh_content_licenses_00994f40(DownloadManagerFields& manager, DownloadManagerCalls& calls,
                                       ContentArrayMemory& memory) {
    if (!calls.has_selected_user_00a3e510()) {
        return;
    }
    NativeStringStorage& strings = calls.strings();
    ContentPackageArray& active = manager.list->active_04;
    ContentPackageArray& pending = manager.list->pending_10;
    assign_content_array_00994dc0(pending, active, memory, strings);
    resize_content_array_00994a80(active, 0, memory, strings);
    for (std::uint32_t i = 0; i < manager.enumeration->count_838; ++i) {
        ContentPackageRecord record{};
        record.index_14 = -1;
        if (read_content_name(manager.enumeration->buffer_38 + i * 0x20, record, calls)) {
            std::int32_t found = -1;
            for (std::int32_t j = 0; j < pending.count && found == -1; ++j) {
                if (calls.matches_at_offset_0043e9a0(pending.data[j].name_00, record.name_00)) {
                    found = j;
                }
            }
            push_back_content_array_00994a00(active, found != -1 ? pending.data[found] : record, memory, strings);
        }
        destruct_content_record_009946d0(record, strings);
    }
    resize_content_array_00994a80(pending, 0, memory, strings);
}

void collect_mounted_content_paths_009954e0(DownloadManagerFields& manager, void* list, DownloadManagerCalls& calls) {
    const ContentPackageArray& active = manager.list->active_04;
    for (std::int32_t i = 0; i < active.count; ++i) {
        const NativeString& directory = manager.directories_20->data[i];
        if (directory.length() != 0 && active.data[i].flags_10[1] != 0) {
            calls.list_append(list, active.data[i].path_08);
        }
    }
}

void destruct_download_manager_00995150(std::uint32_t& vtable_00, DownloadManagerFields& manager,
                                        DownloadManagerCalls& calls) {
    vtable_00 = 0x00d1d7d8;
    for (std::int32_t i = 0; i < manager.directories_20->count; ++i) {
        const std::string directory = text_of(manager.directories_20->data[i]);
        const std::string package = text_of(manager.packages_2c->data[i]);
        if (directory.empty()) {
            continue;
        }
        calls.log_004254b0("Unmount active content (dir: %s)(mpkg: %s)", directory.c_str(), package.c_str());
        calls.unmount_00be0750(content_path_compose_00994770(directory), ".");
        if (!package.empty()) {
            calls.unmount_00be0750(package, ".");
        }
    }
    const ContentPackageArray& active = manager.list->active_04;
    for (std::int32_t j = 0; j < active.count; ++j) {
        if (active.data[j].flags_10[1] != 0) {
            const char* path = active.data[j].path_08.data();
            calls.log_004254b0("Remove active content package (%s)", path != nullptr ? path : "", nullptr);
            calls.remove_content_package_00bdd020(active.data[j].path_08);
        }
    }
    calls.destroy_string_array(*manager.packages_2c);
    calls.destroy_string_array(*manager.directories_20);
    calls.destruct_content_list_00994e10();
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

std::uint32_t scene_class_map_insert_004ee1a0(SceneClassMapLayout& map, const NativeString& key, void* value,
                                              SceneClassMapCalls& calls) {
    std::uint32_t bucket = 0;
    if (SceneClassMapNode* hit = calls.find_node_00468fb0(key, bucket)) {
        if (hit->value != nullptr) {
            calls.free_00bf65ac(hit->value);
            hit->value = nullptr;
        }
        hit->value = value;
        return map.count_04;
    }
    void* storage = calls.allocate_node_004e7c00();
    if (storage == nullptr) {
        return map.count_04;  // the image writes through the null node
    }
    auto* node = new (storage) SceneClassMapNode{};
    if (&node->key != &key) {
        node->key.copy_from_00be0a30_fragment(calls.strings(), key);
    }
    node->value = value;
    node->next = map.buckets_08[bucket];
    map.buckets_08[bucket] = node;
    ++map.count_04;
    return map.count_04;
}

void scene_class_map_register_004ee250(SceneClassMapLayout& map, const char* name, std::int32_t class_id,
                                       std::uint32_t create, std::uint32_t register_pass, SceneClassMapCalls& calls) {
    auto* descriptor = static_cast<SceneClassDescriptor*>(calls.new_descriptor_00bf55be());
    if (descriptor != nullptr) {
        descriptor->class_id = class_id;
        descriptor->create = create;
        descriptor->register_pass = register_pass;
    }
    NativeString key;
    key.assign_0041e870(calls.strings(), name);
    scene_class_map_insert_004ee1a0(map, key, descriptor, calls);
    key.release_to(calls.strings());
}

const char* landing_point_class_name_004e95f0() noexcept { return "LandingPoint"; }
const char* spawn_point_class_name_004f16e0() noexcept { return "SpawnPoint"; }
const char* land_convoy_class_name_004f2500() noexcept { return "LandConvoy"; }

bool unit_is_local_player_objective_004f2590(const void* unit, LocalObjectiveSet& local_player_set) {
    return local_player_set.contains_unit_008ddf90(unit);
}

void segment_lerp_004f3030(const float segment[4], float out[2], float t) noexcept {
    const float x1t = segment[2] * t;     // 004F3044
    const float y1t = segment[3] * t;     // 004F304C
    const float s = 1.0f - t;             // FLD1 / FSUBRP
    const float x0s = segment[0] * s;     // 004F3064
    const float y0s = s * segment[1];     // 004F306B
    out[0] = x0s + x1t;
    out[1] = y1t + y0s;
}

bool is_left_turn_004f40c0(float x1, float y1, float x2, float y2, float x3, float y3) noexcept {
    const float dx21 = x2 - x1;
    const float dy21 = y2 - y1;
    const float dx31 = x3 - x1;
    const float dy31 = y3 - y1;
    return 0.0f < dy31 * dx21 - dy21 * dx31;
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

namespace {

std::string format_build(const char* format, const std::string& csd, std::uint32_t build)
{
    char buffer[0x800];   // [ESP+104h]
    std::snprintf(buffer, sizeof buffer, format, csd.c_str(), static_cast<unsigned>(build & 0xFFFFu));
    return buffer;
}

}  // namespace

bool describe_os_version_0098c9c0(std::string& description_234, bool& nt5_or_later_c34, OsVersionCalls& calls)
{
    nt5_or_later_c34 = false;
    description_234.clear();
    OsVersionInfoEx v{};
    const bool have_ex = calls.get_version_ex(0x9C, v);       // EBP keeps this result
    if (!have_ex && !calls.get_version_ex(0x94, v)) return false;
    if (v.major_04 >= 5) nt5_or_later_c34 = true;
    std::string& s = description_234;                          // every append is an inline strcat
    const char csd1 = v.csd_version_14.size() > 1 ? v.csd_version_14[1] : '\0';

    if (v.platform_10 == 0) {
        s += "Microsoft Win32s";
        return true;
    }
    if (v.platform_10 == 1) {
        if (v.major_04 != 4) return true;
        if (v.minor_08 == 0) {
            s += "Microsoft Windows 95 ";
            if (csd1 == 'C' || csd1 == 'B') s += "OSR2 ";
        } else if (v.minor_08 == 10) {
            s += "Microsoft Windows 98 ";
            if (csd1 == 'A') s += "SE ";
        } else if (v.minor_08 == 0x5A) {
            s += "Microsoft Windows Millennium Edition";
        }
        return true;
    }
    if (v.platform_10 != 2) return true;

    // 0098CA5A: the name by version; major > 6 and major == 6 are separate tests.
    if (v.major_04 > 6) s += "Higher than Microsoft Windows Vista ";
    if (v.major_04 == 6) {
        s += "Microsoft Windows Vista ";
    } else if (v.major_04 == 5) {
        if (v.minor_08 == 2) s += "Microsoft Windows Server 2003, ";
        else if (v.minor_08 == 1) s += "Microsoft Windows XP ";
        else if (v.minor_08 == 0) s += "Microsoft Windows 2000 ";
    } else if (v.major_04 <= 4) {
        s += "Microsoft Windows NT ";
    }

    const auto build_suffix = [&]() {                          // 0098CD1C
        s += format_build("%s (Build %d)", v.csd_version_14, v.build_0c);
        return true;
    };
    const auto service_pack_6 = [&]() {                        // 0098D0FC
        if (_stricmp(v.csd_version_14.c_str(), "Service Pack 6") != 0) return build_suffix();
        if (calls.reg_open_hklm("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Hotfix\\Q246009")) {
            char buffer[0x800];
            std::snprintf(buffer, sizeof buffer, "Service Pack 6a (Build %d)",
                          static_cast<unsigned>(v.build_0c & 0xFFFFu));
            s += buffer;
        } else {
            s += format_build("%s (Build %d)", v.csd_version_14, v.build_0c);
        }
        calls.reg_close();                                     // also after a failed open
        return true;
    };
    const auto tail = [&]() {                                  // 0098D0F3: CMP major, 4
        return v.major_04 == 4 ? service_pack_6() : build_suffix();
    };

    if (have_ex) {
        const std::uint8_t suite = static_cast<std::uint8_t>(v.suite_mask_98);
        if (v.product_type_9a == 1) {                          // VER_NT_WORKSTATION
            if (v.major_04 == 4) {
                s += "Workstation 4.0 ";
                return service_pack_6();
            }
            s += (v.suite_mask_98 & 0x200) != 0 ? "Home Edition " : "Professional ";
            return tail();
        }
        if (v.product_type_9a != 3 && v.product_type_9a != 2) return tail();
        if (v.major_04 == 5 && v.minor_08 == 2) {
            if ((suite & 0x80) != 0) s += "Datacenter Edition ";        // TEST AL, AL; JNS
            else if ((suite & 0x02) != 0) s += "Enterprise Edition ";
            else if (v.suite_mask_98 == 0x400) s += "Web Edition ";      // CMP AX, 400h
            else s += "Standard Edition ";
            return build_suffix();
        }
        if (v.major_04 == 5 && v.minor_08 == 0) {
            if ((suite & 0x80) != 0) s += "Datacenter Server ";
            else if ((suite & 0x02) != 0) s += "Advanced Server ";
            else s += "Server ";
            return build_suffix();
        }
        // 0098CF10: every other server version, Vista included.
        if ((v.suite_mask_98 & 0x02) != 0) {
            s += "Server 4.0, Enterprise Edition ";
        } else {
            s += "Server 4.0 ";
        }
        return tail();
    }

    // 0098CF69: no OSVERSIONINFOEX; the product type from the registry.
    if (!calls.reg_open_hklm("SYSTEM\\CurrentControlSet\\Control\\ProductOptions")) return false;
    std::string type;
    std::uint32_t length = 0x50;
    if (!calls.reg_query_string("ProductType", type, length)) return false;
    if (length > 0x50) return false;                           // the key stays open
    calls.reg_close();
    if (_stricmp("WINNT", type.c_str()) == 0) s += "Workstation ";
    if (_stricmp("LANMANNT", type.c_str()) == 0) s += "Server ";
    if (_stricmp("SERVERNT", type.c_str()) == 0) s += "Advanced Server ";
    char buffer[0x800];
    std::snprintf(buffer, sizeof buffer, "%d.%d ", static_cast<int>(v.major_04), static_cast<int>(v.minor_08));
    s += buffer;
    return tail();
}

void download_manager_scan_content_00995690(std::vector<ContentScanRecord>& records,
                                            std::vector<std::string>& folders_20,
                                            std::vector<std::string>& names_2c, bool& busy_1c,
                                            ContentScanCalls& calls)
{
    folders_20.clear();
    names_2c.clear();
    // Function-scope strings: a skipped record reuses the previous record's folder and name.
    std::string folder;   // [ESP+24h]
    std::string name;     // [ESP+1Ch]
    for (std::size_t i = 0; i < records.size(); ++i) {
        ContentScanRecord& rec = records[i];
        rec.damaged_12 = calls.verify_installed_package(i) != 0;
        if (rec.listed_10 && !rec.mounted_11) {
            const std::string wide_path = calls.content_path_low_bytes(i);
            folder = wide_path + "\\content";
            name.clear();
            void* directory = calls.vfs_mount_00be1890(calls.compose_path_00994770(folder), ".", -10, 1, -1);
            std::vector<std::string> listing = calls.directory_list_14(directory, "", "", 1);
            bool found = false;
            while (!listing.empty() && !found) {
                const std::string& entry = listing.back();
                calls.log_004254b0(std::to_string(static_cast<std::int32_t>(i)) + "    " + entry);  // "%d    %s"
                // strstr(entry, "") == entry holds for any non-empty buffer; then ".mpkg".
                if (!entry.empty()) {
                    const std::size_t at = entry.find(".mpkg");
                    const std::int32_t index = at == std::string::npos ? -1 : static_cast<std::int32_t>(at);
                    // size - 5: a four-character name without ".mpkg" also matches (-1 == 4 - 5).
                    if (index == static_cast<std::int32_t>(entry.size()) - 5) {
                        rec.package_08 = entry.substr(entry.size() - 12, 7);  // 00469840(size - 0Ch, 7)
                        name = entry;
                        calls.vfs_mount_00be1890(name, ".", -10, 0, -1);
                        found = true;
                        break;
                    }
                }
                listing.pop_back();                            // the entry's buffer back to the pool
            }
            if (!rec.package_08.empty()) {
                calls.vfs_package_found_00bdef80(rec.package_08);
            } else {
                calls.log_004254b0("Damaged content " + rec.title_00);  // null buffer -> 00F8A308
                rec.damaged_12 = true;
                calls.vfs_unmount_00be0750(calls.compose_path_00994770(calls.compose_path_00994770(wide_path)), ".");
                calls.vfs_package_missing_00bdd020(rec.package_08);
            }
        }
        rec.mounted_11 = rec.listed_10 && !rec.damaged_12;
        if (rec.mounted_11) {
            folders_20.push_back(folder);                       // 004CDC20
            names_2c.push_back(name);
        } else {
            folders_20.emplace_back();
            names_2c.emplace_back();
        }
    }
    busy_1c = false;
}

void update_fish_school_00707b00(FishSchoolDescLayout& school, float dt, FishUpdateCalls& calls,
                                 PresentationRandom& random, VectorNormalizer& normalizer)
{
    if (dt == 0.0f) return;                                    // UCOMISS dt, 0; JNP: equal only
    void* model = school.model_78;
    void* camera = calls.camera_node_19fc();
    if (camera == nullptr) {
        calls.set_visibility_factor_00b6da70(model, 0.0f, 0);
        return;
    }
    calls.node_refresh_world_if_stale(camera);
    if (calls.node_translation_120(camera)[1] > 0.0f) {        // COMISS [camera+124h], 0; JA
        calls.set_visibility_factor_00b6da70(model, 0.0f, 0);
        return;
    }
    calls.node_refresh_world_if_stale(model);
    auto* fish = static_cast<float*>(school.fish_64);          // 28h records: pos, +0Ch, dir +18h, speed +24h
    const std::array<float, 3> p = calls.transform_point_00414d10(model, fish);
    calls.node_refresh_world_if_stale(camera);
    const std::array<float, 3> c = calls.node_translation_120(camera);
    const float dx = p[0] - c[0];
    const float dy = p[1] - c[1];
    const float dz = p[2] - c[2];
    const float dist2 = (dx * dx + dy * dy) + dz * dz;          // y and x squared first, then z
    if (dist2 > 90000.0f) {                                    // FCOMIP; JA (00CFD404)
        calls.set_visibility_factor_00b6da70(model, 0.0f, 0);
        return;
    }
    calls.set_visibility_factor_00b6da70(model, 1.0f, 0);
    auto* state = reinterpret_cast<float*>(school.state_68);   // +68h timer, +6Ch target direction
    state[0] = state[0] - dt;
    if (!(0.0f < state[0])) respawn_fish_007077b0(school, random, normalizer);  // FCOMIP 0, timer; JB skips
    const FishSchoolParams& prm = school.params_04;
    const float k = calls.min_float_00415510(1.0f, prm.word_40 * dt);   // ECX = &1.0, EDX = &(+44h * dt)

    float* dir0 = fish + 6;
    const float r[3] = {state[1] - dir0[0], state[2] - dir0[1], state[3] - dir0[2]};  // 00427C90
    for (int a = 0; a < 3; ++a) dir0[a] = k * r[a] + dir0[a];
    normalizer.normalize_0042b260(dir0);
    const float v0 = fish[9] * dt;
    for (int a = 0; a < 3; ++a) fish[a] = fish[a] + v0 * dir0[a];       // 0042AEB0
    const float up[3] = {0.0f, 1.0f, 0.0f};
    std::array<float, 3> basis_a = calls.cross_004f9b30(up, dir0);
    std::array<float, 3> basis_b = calls.cross_004f9b30(dir0, basis_a.data());
    normalizer.normalize_0042b260(basis_a.data());
    normalizer.normalize_0042b260(basis_b.data());

    const auto* offsets = static_cast<const float*>(school.positions_60);
    for (std::int32_t i = 1; i < prm.count_00; ++i) {
        float* f = fish + i * 10;
        const float* o = offsets + i * 3;
        float target[3];
        for (int a = 0; a < 3; ++a) {
            const float t2 = o[2] * dir0[a];
            const float t0 = o[0] * basis_a[a];
            const float t1 = o[1] * basis_b[a];
            target[a] = ((fish[a] + t2) + t0) + t1;
        }
        float u[3] = {target[0] - f[0], target[1] - f[1], target[2] - f[2]};
        const float length = calls.length_00419440(u) + 0.005f;      // 00CEC8F8, float-exact
        for (float& x : u) x = x / length;                            // DE FA, D8 F1, D8 7C
        float* dir = f + 6;
        for (int a = 0; a < 3; ++a) dir[a] = dir[a] + k * (u[a] - dir[a]);  // DE EA: u - dir
        normalizer.normalize_0042b260(dir);
        float speed_target = prm.word_38 + (length / 5.0f) * (prm.word_3c - prm.word_38);
        if (speed_target > prm.word_3c) speed_target = prm.word_3c;   // FCOMIP; JBE keeps it
        f[9] = f[9] + (speed_target - f[9]) * k;
        const float step = f[9] * dt;
        for (int a = 0; a < 3; ++a) f[a] = f[a] + step * dir[a];
    }
}

}  // namespace bsp::engine_seg11_35_66_cc13

// Lane oct18-tC: this already-registered TU is separately leased. Keeping the
// leaves here avoids changing the concurrently leased CMake source registry.
// Each naked getter retains the original one-load/LEA + RET sequence, including
// the unspilled ST0 result of FLD. No fabricated layout or dependency stubs.
namespace bsp::oct18_tail_c {
__declspec(naked) const void* __fastcall accepted_invite_info_00a3e460(const void*) {
    __asm { lea eax, [ecx + 32h] }
    __asm { ret }
}
__declspec(naked) void __fastcall enumerate_input_noop_00a90ed0(void*) {
    __asm { ret }
}
__declspec(naked) std::uint8_t __fastcall gui_layer_visible_00aa38e0(const void*) {
    __asm { mov al, byte ptr [ecx + 0f5h] }
    __asm { ret }
}
void set_pointer_activation_reader_00aa6be0(std::uint32_t& image_f8bc84,
                                          std::uint32_t reader) {
    image_f8bc84 = reader;
}
void set_selection_callback_00aab5e0(std::uint32_t& image_f8bcfc,
                                    std::uint32_t callback) {
    image_f8bcfc = callback;
}
__declspec(naked) std::uint32_t __fastcall terrain_field3c_00ad56d0(const void*) {
    __asm { mov eax, dword ptr [ecx + 3ch] }
    __asm { ret }
}
__declspec(naked) float __fastcall foliage_scalar0c_00ad5720(const void*) {
    __asm { fld dword ptr [ecx + 0ch] }
    __asm { ret }
}
__declspec(naked) std::uint32_t __fastcall foliage_bush_texture_00ad57d0(const void*) {
    __asm { mov eax, dword ptr [ecx + 9ch] }
    __asm { ret }
}
__declspec(naked) std::uint32_t __fastcall foliage_bush_normal_texture_00ad57e0(const void*) {
    __asm { mov eax, dword ptr [ecx + 0a0h] }
    __asm { ret }
}
__declspec(naked) float __fastcall atmosphere_time_of_day_00bbdd30(const void*) {
    __asm { fld dword ptr [ecx + 20h] }
    __asm { ret }
}
__declspec(naked) float __fastcall atmosphere_sky_blend_00bbdd40(const void*) {
    __asm { fld dword ptr [ecx + 28h] }
    __asm { ret }
}
__declspec(naked) const void* __fastcall atmosphere_wind_direction_00bbdd60(const void*) {
    __asm { lea eax, [ecx + 2ch] }
    __asm { ret }
}
__declspec(naked) float __fastcall atmosphere_wind_speed_00bbdd70(const void*) {
    __asm { fld dword ptr [ecx + 38h] }
    __asm { ret }
}
__declspec(naked) std::uint32_t __fastcall atmosphere_skybox_00bbddb0(const void*) {
    __asm { mov eax, dword ptr [ecx + 18h] }
    __asm { ret }
}
} // namespace bsp::oct18_tail_c

// Lane tC batch 2: actual borrowed storage and mandatory dependency contracts.
namespace bsp::oct18_tail_c {
namespace {
using CWord = std::uint32_t;
template<class T> T c_read(const void* p, std::size_t offset = 0) {
    T value;
    std::memcpy(&value, static_cast<const unsigned char*>(p) + offset, sizeof value);
    return value;
}
template<class T> void c_write(void* p, std::size_t offset, T value) {
    std::memcpy(static_cast<unsigned char*>(p) + offset, &value, sizeof value);
}
void* c_at(void* p, CWord offset) {
    return reinterpret_cast<void*>(reinterpret_cast<CWord>(p) + offset);
}
template<class R, class... Args> R c_virtual(void* p, CWord slot, Args... args) {
    const auto table = c_read<const unsigned char*>(p);
    const auto target = c_read<R (__thiscall*)(void*, Args...)>(table, slot);
    return target(p, args...);
}
void* c_first_entity(void* group, CheckedIterator checked) {
    void* const head = c_read<void*>(group, 0x5640);
    void* const node = c_read<void*>(head);
    if (node == head) checked();
    return c_read<void*>(node, 8);
}
const float* c_leader_point(void* group, const AiBindings& b) {
    if (c_read<CWord>(group, 0x5644) == 0) return b.zero_vector_f87574;
    void* const entity = c_first_entity(group, b.checked_iterator_bf6713);
    if (c_read<std::uint8_t>(entity, 0xc8) == 0) b.refresh_pose_00414db0(entity);
    return static_cast<const float*>(c_at(entity, 0xfc));
}
void c_fld_store(float* destination, const float* source) {
    __asm { mov eax, source }
    __asm { mov edx, destination }
    __asm { fld dword ptr [eax] }
    __asm { fstp dword ptr [edx] }
}
void c_release_ref(void* owner) {
    if (owner && InterlockedDecrement(static_cast<volatile LONG*>(c_at(owner, 4))) == 0)
        c_virtual<void>(owner, 0);
}
}
__declspec(naked) std::uint32_t __fastcall active_user_slot_00a3eac0(const void*) {
    __asm { mov eax, dword ptr [ecx + 11ch] }
    __asm { ret }
}
__declspec(naked) void __fastcall node_local_position_00b6dab0(void*, void*, const void*) {
    __asm {
        mov eax, dword ptr [esp + 4]
        fld dword ptr [eax]
        lea edx, [ecx + 0b0h]
        fstp dword ptr [ecx + 0e0h]
        mov dword ptr [esp + 4], edx
        fld dword ptr [eax + 4]
        fstp dword ptr [ecx + 0e4h]
        fld dword ptr [eax + 8]
        mov eax, dword ptr [ecx]
        mov eax, dword ptr [eax + 38h]
        fstp dword ptr [ecx + 0e8h]
        jmp eax
    }
}
bool ai_group_contains_entity_00a2c720(void* group, void* entity, CheckedIterator checked) {
    void* node = c_read<void*>(c_read<void*>(group, 0x5640));
    for (;;) {
        void* const head = c_read<void*>(group, 0x5640);
        if (c_at(group, 0x563c) == nullptr) checked();
        if (node == head) return false;
        if (c_at(group, 0x563c) == nullptr) checked();
        if (node == c_read<void*>(group, 0x5640)) checked();
        if (c_read<void*>(node, 8) == entity) return true;
        if (node == c_read<void*>(group, 0x5640)) checked();
        node = c_read<void*>(node);
    }
}
void ai_enable_torpedoes_00a11af0(void* command, const AiBindings& b) {
    void* const group = c_read<void*>(command, 4);
    void* node = c_read<void*>(c_read<void*>(group, 0x5640));
    for (;;) {
        void* const current = c_read<void*>(command, 4);
        void* const end = c_read<void*>(current, 0x5640);
        if (!c_at(group, 0x563c) || c_at(group, 0x563c) != c_at(current, 0x563c))
            b.checked_iterator_bf6713();
        if (node == end) return;
        if (!c_at(group, 0x563c)) b.checked_iterator_bf6713();
        if (node == c_read<void*>(group, 0x5640)) b.checked_iterator_bf6713();
        void* const entity = c_read<void*>(node, 8);
        if (c_virtual<std::uint8_t>(entity, 0x5c, 6) != 0)
            b.torpedo_enable_0071e0d0(c_read<void*>(entity, 0x738), 1);
        if (node == c_read<void*>(group, 0x5640)) b.checked_iterator_bf6713();
        node = c_read<void*>(node);
    }
}
void ai_close_attack_tick_00a15490(void* command, const AiBindings& b) {
    void* const target = c_read<void*>(command, 0x1c);
    const float* const point = c_leader_point(target, b);
    float scale;
    c_fld_store(&scale, b.close_attack_scale_ce380c);
    b.target_pass_00a13b60(command, scale, point, target, 1);
    b.landing_00a11b80(command, c_read<void*>(command, 0x1c));
    ai_enable_torpedoes_00a11af0(command, b);
}
void ai_defend_position_tick_00a15500(void* command, const AiBindings& b) {
    b.followers_00a10dc0(command);
    b.formation_00a11690(command);
    const float* const point = c_leader_point(c_read<void*>(command, 4), b);
    b.target_pass_00a13b60(command, 1.0f, point, nullptr, 0);
    b.landing_00a11b80(command, nullptr);
}
bool ai_merge_strength_00a10d50(void* command, void* other, const AiBindings& b) {
    void* const own = c_first_entity(c_read<void*>(command, 4), b.checked_iterator_bf6713);
    void* const candidate = c_first_entity(other, b.checked_iterator_bf6713);
    const auto weight = b.class_weight_009ffd70;
    double own_weight;
    unsigned result;
    __asm {
        mov ecx, own
        call weight
        fstp qword ptr own_weight
        mov ecx, candidate
        call weight
        fld qword ptr own_weight
        fxch
        fcomip st(0), st(1)
        fstp st(0)
        setae al
        movzx eax, al
        mov result, eax
    }
    return result != 0;
}
bool ai_merge_distance_00a10c60(void* command, void* other, const AiBindings& b) {
    const float* const candidate = c_leader_point(other, b);
    const float* const own = c_leader_point(c_read<void*>(command, 4), b);
    float delta_x, delta_z;
    __asm {
        mov eax, own
        mov edx, candidate
        fld dword ptr [eax]
        fsub dword ptr [edx]
        fstp dword ptr delta_x
        fld dword ptr [eax + 8]
        fsub dword ptr [edx + 8]
        fstp dword ptr delta_z
    }
    const float* const tuning_radius = static_cast<const float*>(c_at(b.active_tuning_00a371a0(), 0x208));
    float radius, distance2, radius2;
    unsigned result;
    __asm {
        mov eax, tuning_radius
        fld dword ptr [eax]
        fstp dword ptr radius
        fld dword ptr delta_z
        fld dword ptr delta_x
        fld dword ptr radius
        fld st(1)
        fmulp st(2), st(0)
        fld st(2)
        fmulp st(3), st(0)
        fxch st(1)
        faddp st(2), st(0)
        fxch st(1)
        fstp dword ptr distance2
        fld dword ptr distance2
        fld st(1)
        fmulp st(2), st(0)
        fxch st(1)
        fstp dword ptr radius2
        fld dword ptr radius2
        fcomip st(0), st(1)
        fstp st(0)
        seta al
        movzx eax, al
        mov result, eax
    }
    return result != 0;
}
void ai_build_entity_records_00a07e40(void* group, void* out, CWord record_class, const AiBindings& b) {
    alignas(4) unsigned char scratch[0x20]; // actual 32-byte caller temporary
    void* node = c_read<void*>(c_read<void*>(group, 0x5640));
    while (node != c_read<void*>(group, 0x5640)) {
        if (node == c_read<void*>(group, 0x5640)) b.checked_iterator_bf6713();
        void* const record = b.entity_record_00a04560(scratch, c_read<void*>(node, 8), record_class);
        b.append_record_00a078f0(out, record);
        if (node == c_read<void*>(group, 0x5640)) b.checked_iterator_bf6713();
        node = c_read<void*>(node);
    }
}
void sound_dsp_resize_00a7a980(void* array, std::int32_t requested,
    void (__thiscall* reserve_00a7a8b0)(void*, std::int32_t)) {
    if (c_read<std::int32_t>(array, 8) < requested) reserve_00a7a8b0(array, requested);
    for (std::int32_t i = c_read<std::int32_t>(array, 4); i < requested; ++i) {
        void* const slot = c_at(c_read<void*>(array), static_cast<CWord>(i) * 4);
        if (slot) c_write<CWord>(slot, 0, 0);
    }
    while (requested < c_read<std::int32_t>(array, 4))
        c_write<CWord>(array, 4, c_read<CWord>(array, 4) - 1);
    c_write<std::int32_t>(array, 4, requested);
}
void gui_line_describe_00ac2090(void* line, void* visitor, const GuiLineBindings& b) {
    b.describe_base_00aaaed0(line, visitor);
    auto count = [&]() -> CWord {
        const CWord difference = c_read<CWord>(line, 0xf4) - c_read<CWord>(line, 0xf0);
        return static_cast<CWord>(static_cast<std::int32_t>(difference) >> 3);
    };
    if (!c_read<void*>(line, 0xf0) || count() == 0) return;
    c_virtual<void>(visitor, 4, GuiTaggedWord{0, reinterpret_cast<CWord>(b.points_key_cfd91c)});
    CWord i = 0;
    while (c_read<void*>(line, 0xf0) && i < count()) {
        if (!c_read<void*>(line, 0xf0) || count() <= i) b.checked_iterator_bf6713();
        const CWord point = c_read<CWord>(line, 0xf0) + i * 8;
        ++i;
        c_virtual<void>(visitor, 0xc, GuiTaggedWord{1, i}, GuiTaggedWord{6, point});
    }
    c_virtual<void>(visitor, 8);
}
void gui_line_reset_00ac2950(void* line, const GuiLineBindings& b) {
    void* const captured_end = c_read<void*>(line, 0xf4);
    if (c_read<CWord>(line, 0xf0) > reinterpret_cast<CWord>(captured_end)) b.checked_iterator_bf6713();
    void* const begin = c_read<void*>(line, 0xf0);
    if (c_read<CWord>(line, 0xf4) < reinterpret_cast<CWord>(begin)) b.checked_iterator_bf6713();
    if (begin != captured_end)
        c_write<void*>(line, 0xf4, b.copy_pod8_0043b920(captured_end, c_read<void*>(line, 0xf4), begin));
    c_write<CWord>(line, 0xfc, 0);
    b.ensure_capacity_00ac2690(line, 4);
    void* const geometry = b.geometry_00b74640(c_read<void*>(line, 0x4c), 0);
    void* const element = b.element_00b732c0(geometry, 0);
    c_write<CWord>(element, 0x10, 0);
    c_write<CWord>(element, 0x18, 0);
}
void* group_world_sphere_00b8f100(void* group, const GroupSphereBindings& b) {
    if (c_read<std::uint8_t>(group, 0x175) != 0) {
        if ((c_read<std::uint8_t>(group, 0x138) & 0x30) == 0) {
            b.aggregate_world_sphere_00b8ebe0(group);
            c_write<CWord>(group, 0x138, c_read<CWord>(group, 0x138) | 0x30);
        }
        return c_at(group, 0x13c);
    }
    if ((c_read<std::uint8_t>(group, 0x138) & 0x30) == 0) {
        if ((c_read<std::uint8_t>(group, 0x5c) & 2) == 0) b.refresh_world_00b6db70(group);
        float scratch[4];
        const auto* source = static_cast<const float*>(b.transform_sphere_007c1180(
            c_at(group, 8), scratch, c_at(group, 0xf0)));
        for (CWord i = 0; i != 4; ++i)
            c_fld_store(static_cast<float*>(c_at(group, 0x13c + i * 4)), source + i);
        c_write<CWord>(group, 0x138, c_read<CWord>(group, 0x138) | 0x30);
    }
    return c_at(group, 0x13c);
}
void release_whole_file_00bd91f0(void* buffer, void (__cdecl* free_bf6989)(void*)) {
    free_bf6989(buffer);
}
void cache_file_from_vfs_00be7ab0(void* store, void* vfs, const void* name,
    CWord flags, const FileStoreBindings& b) {
    void* const stream = c_virtual<void*>(vfs, 4, name, flags);
    void* const memory = b.memory_view_00bef750(stream);
    b.add_file_00be7760(store, name, memory);
    c_release_ref(memory);
    c_release_ref(stream);
}
} // namespace bsp::oct18_tail_c

#include "bsp/native_lua_objects.hpp"
extern "C" {
#include <lua.h>
}

namespace bsp::oct18_tail_c {
void section_loaded_00abe630(void* self, RawThis base_00aa7170) { base_00aa7170(self); }
void widget_describe_alias_00ac6fe0(void* self, void* visitor,
    void (__thiscall* base_00aaaed0)(void*, void*)) { base_00aaaed0(self, visitor); }
void icon_geometry_alias_00ad2df0(void* self, RawThis base_00ab2540) { base_00ab2540(self); }
void render_release_alias_00b0fc00(void* self, RawThis base_00b0f6e0) { base_00b0f6e0(self); }
void* section_construct_00abf420(void* self, const SectionBindings& b) {
    b.base_construct_00aa9390(self, 0x11);
    const CWord a = *b.scalar_d7a24c;
    const CWord d = *b.scalar_ce3800;
    c_write<CWord>(self, 0, 0x00d5c8e8);
    c_write<CWord>(self, 0xec, 0);
    c_write<CWord>(self, 0xf0, 0);
    c_write<CWord>(self, 0x110, 0);
    c_write<CWord>(self, 0x114, 0);
    for (CWord offset = 0xf4; offset <= 0x100; offset += 4) c_write<CWord>(self, offset, 0);
    c_write<CWord>(self, 0x104, a);
    c_write<CWord>(self, 0x108, d);
    c_write<std::uint8_t>(self, 0x10c, 1);
    c_write<CWord>(self, 0x118, 0);
    c_write<CWord>(self, 0x11c, 0);
    c_write<CWord>(self, 0x120, a);
    c_write<CWord>(self, 0x124, a);
    return self;
}
void section_constructed_00abeda0(void* self, const SectionBindings& b) {
    if (c_read<std::uint8_t>(self, 0x74) == 0) {
        void* const model = c_read<void*>(self, 0x4c);
        void* const allocation = b.mesh_allocate_00b73b60(0xbc);
        void* const mesh = allocation ? b.mesh_construct_00b73d70(allocation) : nullptr;
        float scale;
        c_fld_store(&scale, b.mesh_scalar_d7a260);
        b.set_geometry_00b75170(model, 0, mesh, scale, scale);
        // Original dereferences mesh+4 even when allocation returned null.
        if (InterlockedDecrement(static_cast<volatile LONG*>(c_at(mesh, 4))) == 0)
            c_virtual<void>(mesh, 0);
    }
    const CWord zero[3] = {};
    b.position_00aa7dc0(self, zero);
}
void section_destroy_00abf4f0(void* self, const SectionBindings& b) {
    c_write<CWord>(self, 0, 0x00d5c8e8);
    void* const texture = c_read<void*>(self, 0x114);
    if (texture) {
        c_release_ref(texture);
        c_write<CWord>(self, 0x114, 0);
    }
    void* const block = c_read<void*>(self, 0xf0);
    if (block) b.release_string_00419cc0_00bd1510(block, c_read<CWord>(self, 0xec) + 1, 1);
    b.base_destroy_00aa9730(self);
}
__declspec(naked) float __fastcall normalize_angle_00abe960(void*, const double* const*, float) {
    __asm {
        push esi
        push edi
        mov esi, edx
        mov edi, ecx
        fld dword ptr [esp + 0ch]
        mov eax, dword ptr [esi]
        fld qword ptr [eax]
        call edi
        fstp dword ptr [esp + 0ch]
        fld dword ptr [esp + 0ch]
        fstp dword ptr [esp + 0ch]
        fld dword ptr [esp + 0ch]
        mov eax, dword ptr [esi + 4]
        fld qword ptr [eax]
        fcomip st(0), st(1)
        jc c_angle_upper
        mov eax, dword ptr [esi]
        fadd qword ptr [eax]
        fstp dword ptr [esp + 0ch]
        fld dword ptr [esp + 0ch]
        jmp c_angle_done
    c_angle_upper:
        mov eax, dword ptr [esi + 8]
        fld qword ptr [eax]
        fxch st(1)
        fcomi st(0), st(1)
        fstp st(1)
        jbe c_angle_done
        mov eax, dword ptr [esi]
        fsub qword ptr [eax]
        fstp dword ptr [esp + 0ch]
        fld dword ptr [esp + 0ch]
    c_angle_done:
        pop edi
        pop esi
        ret 4
    }
}
void* font_registry_construct_00ac3690(void* self, const FontRegistryBindings& b) {
    c_write<CWord>(self, 0, 0x00d5ca84);
    c_write<void*>(self, 8, b.sentinel_00ac3400(c_at(self, 4)));
    c_write<CWord>(self, 0xc, 0);
    return self;
}
void font_registry_destroy_00ac36f0(void* self, CWord& image_f8bf44, const FontRegistryBindings& b) {
    c_write<CWord>(self, 0, 0x00d5ca84);
    void* node = c_read<void*>(c_read<void*>(self, 8));
    while (node != c_read<void*>(self, 8)) {
        if (node == c_read<void*>(self, 8)) b.checked_iterator_bf6713();
        void* const font = c_read<void*>(node, 8);
        if (font) { b.font_destroy_00ad53a0(font); b.free_bf65ac(font); }
        if (node == c_read<void*>(self, 8)) b.checked_iterator_bf6713();
        node = c_read<void*>(node);
    }
    void* head = c_read<void*>(self, 8);
    node = c_read<void*>(head);
    c_write<void*>(head, 0, head);
    head = c_read<void*>(self, 8);
    c_write<void*>(head, 4, head);
    c_write<CWord>(self, 0xc, 0);
    while (node != c_read<void*>(self, 8)) {
        void* const next = c_read<void*>(node);
        b.free_bf65ac(node);
        node = next;
    }
    b.free_bf65ac(c_read<void*>(self, 8));
    c_write<CWord>(self, 8, 0);
    image_f8bf44 = 0;
    c_write<CWord>(self, 0, 0x00ce3818);
}
void sound_cache_resize_00a845a0(void* array, std::int32_t requested, const SoundCacheBindings& b) {
    if (requested > c_read<std::int32_t>(array, 8)) b.reserve_00a84450(array, requested);
    for (std::int32_t i = c_read<std::int32_t>(array, 4); i < requested; ++i) {
        void* const record = c_at(c_read<void*>(array), static_cast<CWord>(i) * 0x2c);
        if (record) {
            c_write<CWord>(record, 0, 0);
            c_write<CWord>(record, 4, 0);
            c_write<void*>(record, 0xc, b.sentinel_004c3020(c_at(record, 8)));
            c_write<CWord>(record, 0x10, 0);
            for (CWord offset = 0x24; offset >= 0x14; offset -= 4) c_write<CWord>(record, offset, 0);
        }
    }
    while (requested < c_read<std::int32_t>(array, 4)) {
        const CWord count = c_read<CWord>(array, 4) - 1;
        c_write<CWord>(array, 4, count);
        b.destroy_record_00a842e0(c_at(c_read<void*>(array), count * 0x2c));
    }
    c_write<std::int32_t>(array, 4, requested);
}
void gui_list_visible_00a9cce0(void* self, CWord visible, const GuiListVisibilityBindings& b) {
    b.base_visible_00aa8530(self, visible);
    if (c_read<CWord>(self, 0x118) != 0xffffffffU) {
        void* const manager = b.manager_004c12b0();
        void* const other = b.select_widget_00aa0f50(manager, c_read<CWord>(self, 0x118));
        c_virtual<void>(other, 0x34, visible);
    }
}
void* lua_vector4_or_00b67d40(void* reference, void* output, const void* fallback) {
    auto& table = *static_cast<NativeLuaObjectStorage*>(reference);
    bool use_fallback = true;
    NativeLuaObjectStorage fifth;
    if (native_lua_is_table_00b661b0(table)) {
        NativeLuaObjectStorage* const field = native_lua_get_by_index_00b67720(table, &fifth, 5);
        use_fallback = !native_lua_is_nil_00b65fb0(*field);
        if (fifth.kind_04 != 0)
            release_native_lua_tracked_object_00b66de0(fifth.owner_00, fifth, fifth.index_08, 1);
    }
    if (use_fallback) {
        c_write<CWord>(output, 0, c_read<CWord>(fallback));
        c_write<CWord>(output, 4, c_read<CWord>(fallback, 4));
        const CWord third = c_read<CWord>(fallback, 8);
        const CWord fourth = c_read<CWord>(fallback, 0xc);
        c_write<CWord>(output, 8, third);
        c_write<CWord>(output, 0xc, fourth);
        return output;
    }
    float values[4];
    NativeLuaObjectStorage field;
    for (int i = 0; i != 4; ++i) {
        NativeLuaObjectStorage* const current = native_lua_get_by_index_00b67720(table, &field, i + 1);
        values[i] = static_cast<float>(lua_tonumber(current->owner_00->state_04, current->index_08));
        if (field.kind_04 != 0)
            release_native_lua_tracked_object_00b66de0(field.owner_00, field, field.index_08, 1);
    }
    for (CWord i = 0; i != 4; ++i) c_write<CWord>(output, i * 4, c_read<CWord>(values, i * 4));
    return output;
}
void* renderer_query_create_00b27c20(void* renderer, const volatile std::uint8_t& enabled,
    QueryGuardScratch& scratch, const QueryBindings& b) {
    if (enabled) { scratch.owner = renderer; scratch.taken = b.enter_00b33ad0(renderer); }
    void* const storage = b.allocate_00bf681b(0x14);
    void* const query = storage ? b.construct_00b5fe90(storage) : nullptr;
    void* const array = c_at(renderer, 0x19a0);
    if (c_read<CWord>(array, 4) == c_read<CWord>(array, 8)) {
        std::int32_t capacity = static_cast<std::int32_t>(c_read<CWord>(array, 8) * 2);
        if (capacity <= 1) capacity = 1;
        b.reserve_00b22530(array, capacity);
    }
    void* const slot = c_at(c_read<void*>(array), c_read<CWord>(array, 4) * 4);
    if (slot) c_write<void*>(slot, 0, query);
    c_write<CWord>(array, 4, c_read<CWord>(array, 4) + 1);
    if (enabled) b.leave_00b33b00(scratch.owner, scratch.taken);
    return query;
}
namespace {
class CWarningGuard {
public:
    explicit CWarningGuard(void* section) : section_(section) {
        if (section_) {
            EnterCriticalSection(static_cast<LPCRITICAL_SECTION>(section_));
            c_write<CWord>(section_, 0x18, c_read<CWord>(section_, 0x18) + 1);
        }
    }
    ~CWarningGuard() {
        if (section_) {
            c_write<CWord>(section_, 0x18, c_read<CWord>(section_, 0x18) - 1);
            LeaveCriticalSection(static_cast<LPCRITICAL_SECTION>(section_));
        }
    }
private:
    void* section_;
};
}
void warning_remove_listener_00980d30(void* manager, const void* channel, CWord id,
    const WarningRemoveBindings& b) {
    CWarningGuard guard(c_read<void*>(manager, 0x24));
    void* const tree = b.channel_00980150(c_at(manager, 0xf8), channel);
    WarningIteratorPair iterator;
    b.find_0096d6f0(tree, &iterator, id);
    if (!iterator.owner) b.checked_iterator_bf6713();
    if (iterator.node == c_read<void*>(iterator.owner, 4)) b.checked_iterator_bf6713();
    void* const subscription = c_read<void*>(iterator.node, 0x14);
    if (subscription) {
        c_virtual<void*>(subscription, 0, 1);
        c_write<CWord>(iterator.node, 0x14, 0);
    }
    b.erase_009713d0(tree, &iterator, iterator.owner, iterator.node);
}
void* vfs_load_whole_file_00bdc8b0(void* manager, const void* name, CWord* size_output,
    const WholeFileBindings& b) {
    NativeString copied{};
    if (&copied != name) {
        b.string_resize_0041dd40(&copied, c_read<CWord>(name), 1);
        if (c_read<CWord>(name) != 0)
            std::memcpy(c_read<void*>(&copied, 4), c_read<const void*>(name, 4), c_read<CWord>(&copied));
    }
    struct NameGuard {
        NativeString& value;
        const WholeFileBindings& bindings;
        ~NameGuard() {
            void* const data = c_read<void*>(&value, 4);
            if (data) bindings.release_string_00419cc0_00bd1510(data, c_read<CWord>(&value) + 1, 1);
        }
    } cleanup{copied, b};
    void* const stream = c_virtual<void*>(manager, 4, &copied, CWord{2});
    CWord high_size = 0;
    const CWord low_size = c_virtual<CWord>(stream, 0x2c, &high_size);
    void* const bytes = b.allocate_00bf55be(low_size);
    c_virtual<CWord>(stream, 0x24, bytes, low_size, static_cast<CWord*>(nullptr));
    // Native has no stream/buffer unwind owner: only copied name is guarded.
    if (InterlockedDecrement(static_cast<volatile LONG*>(c_at(stream, 4))) == 0)
        c_virtual<void>(stream, 0);
    *size_output = low_size;
    return bytes;
}
} // namespace bsp::oct18_tail_c

#include "bsp/native_mpkg_directory.hpp"
namespace bsp::oct18_tail_c {
void archive_write_field_00bd8cd0(void* writer, GuiTaggedWord key, GuiTaggedWord value,
    const ArchiveFieldBindings& b) {
    b.indent_00bd6990(c_read<CWord>(writer, 4));
    b.key_00bd7b30(c_read<CWord>(writer, 4) == 0, key);
    b.value_00bd7f50(value);
    b.terminator_00bd6ac0(c_read<CWord>(writer, 4) == 0);
}
std::uint32_t mpkg_entry_offset_00bb8a90(void* state, void* entry, const MpkgBindings& b) {
    if (c_read<std::uint8_t>(entry, 0xc)) return c_read<CWord>(entry, 0x10);
    c_virtual<void>(c_read<void*>(state, 0xc), 0x1c, c_read<CWord>(entry, 8), CWord{0}, CWord{0});
    void* const source = c_read<void*>(state, 0xc); // captured before allocation
    struct Reader { void* base; CWord reported; void* cursor; } reader;
    static_assert(sizeof(Reader) == 0xc);
    reader.base = b.allocate_header_00bf55be(0x1e);
    c_virtual<void>(source, 0x24, reader.base, CWord{0x1e}, &reader.reported);
    reader.cursor = reader.base;
    alignas(4) unsigned char header[0x34]; // actual header; local-only fields stay unwritten
    read_native_mpkg_header_00bb8850(&reader, header, 1);
    const CWord offset = c_read<std::uint16_t>(header, 0x20)
        + c_read<std::uint16_t>(header, 0x22) + CWord{0x1e} + c_read<CWord>(entry, 8);
    c_write<CWord>(entry, 0x10, offset);
    c_write<std::uint8_t>(entry, 0xc, 1);
    b.free_header_00bf65ac(reader.base);
    return offset;
}
void* mpkg_open_entry_00bb8be0(void* state, void* entry, CWord flags,
    void* volatile& vfs, const MpkgBindings& b) {
    (void)flags; // original stacked flags are unused
    const CWord offset = mpkg_entry_offset_00bb8a90(state, entry, b);
    if (c_read<std::uint16_t>(entry, 0x14) != 0) {
        c_virtual<void>(c_read<void*>(state, 0xc), 0x1c, offset, CWord{0}, CWord{0});
        void* const allocation = b.allocate_owner_00bf681b(0x34);
        void* inflate = nullptr;
        if (allocation) {
            const CWord parameters[3] = {c_read<CWord>(entry, 0x10),
                c_read<CWord>(entry, 0x18), c_read<CWord>(entry, 0x1c)};
            inflate = b.inflate_00bbc1d0(allocation, c_read<void*>(state, 0xc), parameters, 0x4000, 0x10000);
        }
        void* const memory = b.memory_00bef750(inflate);
        if (InterlockedDecrement(static_cast<volatile LONG*>(c_at(inflate, 4))) == 0)
            c_virtual<void>(inflate, 0);
        return memory;
    }
    if (c_read<CWord>(entry, 0x1c) <= 0x40000) {
        c_virtual<void>(c_read<void*>(state, 0xc), 0x1c, offset, CWord{0}, CWord{0});
        return b.copy_range_00bef840(c_read<void*>(state, 0xc), offset, 0, c_read<CWord>(entry, 0x1c), 0);
    }
    void* const reopened = c_virtual<void*>(vfs, 4, c_at(state, 4), CWord{2});
    void* const allocation = b.allocate_owner_00bf681b(0x28);
    if (!allocation) return nullptr; // original does not retire reopened source here
    return b.adopted_00bf1130(allocation, reopened, offset, 0, c_read<CWord>(entry, 0x1c), 0);
}
void* mpkg_open_named_00bb8d60(void* state, const void* name, CWord flags,
    void* volatile& vfs, const MpkgBindings& b) {
    if (flags & 1) return nullptr;
    CWord byte_offset = 0;
    for (std::int32_t i = 0; i < c_read<std::int32_t>(state, 0x2c); ++i, byte_offset += 0x24) {
        void* const entry = c_at(c_read<void*>(state, 0x28), byte_offset);
        const CWord stored_length = c_read<CWord>(entry);
        if (c_read<CWord>(name) != stored_length) continue;
        bool match = c_read<CWord>(name) == 0;
        if (!match) match = b.stricmp_00bf7fbf(c_read<const char*>(name, 4), c_read<const char*>(entry, 4)) == 0;
        if (match) return mpkg_open_entry_00bb8be0(state, entry, flags, vfs, b);
    }
    return nullptr;
}
void voice_packet_00a44000(void* self, const void* packet, void* volatile& game, const VoiceBindings& b) {
    const CWord low = c_read<CWord>(packet), high = c_read<CWord>(packet, 4);
    if (c_read<std::uint8_t>(self, 0x84) && !b.accept_00a43f40(self, low, high)) return;
    bool allowed = false;
    void* const current_game = game;
    if (current_game) {
        if (c_read<CWord>(c_read<void*>(self, 0x2210), 0x5d4) == 0xd) allowed = true;
        else allowed = b.state_00687860(current_game, 0x14, 0) != 0;
    }
    if (!c_read<CWord>(self, 0x1d28) || !allowed) return;
    c_write<CWord>(self, 0x1f34, 0);
    const CWord user[2] = {c_read<CWord>(packet), c_read<CWord>(packet, 4)};
    auto count = [&]() -> CWord {
        return static_cast<CWord>(static_cast<std::int32_t>(c_read<CWord>(packet, 0x10)
            - c_read<CWord>(packet, 0xc)) / 10);
    };
    CWord offset = 0;
    for (CWord i = 0; c_read<void*>(packet, 0xc) && i < count(); ++i, offset += 10) {
        if (!c_read<void*>(packet, 0xc) || count() <= i) b.checked_iterator_bf6713();
        b.incoming_00a49040(c_at(self, 0x1c24), c_at(c_read<void*>(packet, 0xc), offset), 10, user);
    }
}
void voice_refresh_flags_00a44150(void* self, std::uint8_t mode, const VoiceBindings& b) {
    bool has_remote = false;
    void* const first_owner = c_virtual<void*>(self, 0x80);
    void* node = c_read<void*>(c_read<void*>(first_owner, 4));
    void* const last_owner = c_virtual<void*>(self, 0x80);
    void* const end = c_read<void*>(last_owner, 4);
    for (;;) {
        if (first_owner != last_owner) b.checked_iterator_bf6713();
        if (node == end) break;
        if (node == c_read<void*>(first_owner, 4)) b.checked_iterator_bf6713();
        if (c_read<std::uint8_t>(node, 0x42) == 0) {
            if (node == c_read<void*>(first_owner, 4)) b.checked_iterator_bf6713();
            if (c_read<CWord>(node, 0x34) == 0) has_remote = true;
        }
        if (node == c_read<void*>(first_owner, 4)) b.checked_iterator_bf6713();
        node = c_read<void*>(node);
    }
    if (mode == 0 || (mode == 1 && !has_remote)) {
        void* const owner = c_virtual<void*>(self, 0x80);
        void* const head = c_read<void*>(owner, 4);
        node = c_read<void*>(head);
        if (node == head) b.checked_iterator_bf6713();
        b.mute_00a49180(c_at(self, 0x1c24), c_read<std::uint8_t>(node, 0x38), mode);
        if (mode == 1) {
            __asm {
                mov ecx, self
                mov edx, dword ptr [ecx]
                fld dword ptr [ecx + 1d40h]
                mov eax, dword ptr [edx + 0f0h]
                push ecx
                fstp dword ptr [esp]
                call eax
            }
        }
    }
}
void text_register_clip_00ab7a40(void* self, const TextClipBindings& b) {
    for (CWord owner_offset : {CWord{0x4c}, CWord{0x188}}) {
        void* const model = c_read<void*>(self, owner_offset);
        if (model && b.has_geometry_00b74650(model)) {
            void* const geometry = b.geometry_00b74640(model, 0);
            if (b.count_00b72b40(geometry)) {
                void* const reloaded = b.geometry_00b74640(model, 0);
                void* const element = b.element_00b732c0(reloaded, 0);
                b.clip_00aa9f10(self, c_read<void*>(element, 0x20));
            }
        }
    }
    void* node = c_read<void*>(c_read<void*>(self, 0x68));
    while (node != c_read<void*>(self, 0x68)) {
        if (node == c_read<void*>(self, 0x68)) b.checked_iterator_bf6713();
        c_virtual<void>(c_read<void*>(node, 8), 0x70);
        if (node == c_read<void*>(self, 0x68)) b.checked_iterator_bf6713();
        node = c_read<void*>(node);
    }
}
void layer_read_properties_00ac4c50(void* self, void* visitor, CWord& priority,
    const LayerReadBindings& b) {
    c_virtual<void>(visitor, 0xc, GuiTaggedWord{0, reinterpret_cast<CWord>(b.priority_d5cb00)},
        GuiTaggedWord{1, reinterpret_cast<CWord>(&priority)}, GuiTaggedWord{1, 0});
    const CWord value = priority;
    c_write<CWord>(self, 0xfc, value);
    c_write<CWord>(self, 0x118, value);
    c_virtual<void>(visitor, 0xc, GuiTaggedWord{0, reinterpret_cast<CWord>(b.render_order_d5caf4)},
        GuiTaggedWord{2, reinterpret_cast<CWord>(c_at(self, 0x11c))}, GuiTaggedWord{2, 0});
    c_virtual<void>(self, 0x78);
    void* const root = c_read<void*>(self, 0xec);
    b.register_root_00b6d890(c_read<void*>(self, 0x4c), root);
    const CWord zero[3] = {};
    b.position_00aa7dc0(self, zero);
    b.read_base_00aaa710(self, visitor);
}
void gui_manager_destroy_00aa6340(void* self, CWord& singleton, const GuiDestroyBindings& b) {
    c_write<CWord>(self, 0, 0x00d5bfcc);
    CWord cursor = c_read<CWord>(self, 0x18);
    if (cursor > c_read<CWord>(self, 0x1c)) b.checked_iterator_bf6713();
    for (;;) {
        const CWord end = c_read<CWord>(self, 0x1c);
        if (c_read<CWord>(self, 0x18) > end) b.checked_iterator_bf6713();
        if (cursor == end) break;
        if (cursor >= c_read<CWord>(self, 0x1c)) b.checked_iterator_bf6713();
        void* const slot = reinterpret_cast<void*>(cursor);
        c_virtual<void>(c_read<void*>(slot), 0x20);
        if (cursor >= c_read<CWord>(self, 0x1c)) b.checked_iterator_bf6713();
        void* const page = c_read<void*>(slot);
        if (page) c_virtual<void*>(page, 4, 1);
        if (cursor >= c_read<CWord>(self, 0x1c)) b.checked_iterator_bf6713();
        cursor += 4;
    }
    for (CWord offset : {CWord{0x28}, CWord{0x2c}}) {
        if (offset == 0x2c) {
            b.clear_strings_00aa51d0(c_at(self, 0x34));
            b.free_00bf65ac(c_read<void*>(self, 0x38));
            c_write<CWord>(self, 0x38, 0);
        }
        void* const owner = c_read<void*>(self, offset);
        if (owner) { c_release_ref(owner); c_write<CWord>(self, offset, 0); }
    }
    void* const pages = c_read<void*>(self, 0x18);
    if (pages) b.free_00bf65ac(pages);
    c_write<CWord>(self, 0x18, 0);
    c_write<CWord>(self, 0x1c, 0);
    c_write<CWord>(self, 0x20, 0);
    void* const tree = c_at(self, 8);
    void* const head = c_read<void*>(self, 0xc);
    WarningIteratorPair result;
    b.erase_tree_00aa49e0(tree, &result, tree, c_read<void*>(head), tree, head);
    b.free_00bf65ac(c_read<void*>(self, 0xc));
    c_write<CWord>(self, 0xc, 0);
    c_write<CWord>(self, 0x10, 0);
    singleton = 0;
    c_write<CWord>(self, 0, 0x00ce3818);
}
std::intptr_t platform_handle_message_00bed3b0(void* active, std::uintptr_t handle,
    CWord message, CWord wp, std::intptr_t lp, const PlatformBindings& b) {
    HWND const window = reinterpret_cast<HWND>(handle);
    void* const extra = message == WM_CREATE ? nullptr : reinterpret_cast<void*>(GetWindowLongA(window, 0));
    auto standard = [&]() -> std::intptr_t { return DefWindowProcA(window, message, wp, lp); };
    auto z_order = [&](void* owner, HWND after) {
        SetWindowPos(c_read<HWND>(owner, 0x30), after, 0, 0,
            c_read<int>(owner, 0x24), c_read<int>(owner, 0x28), 0x20);
    };
    if (message == WM_CREATE) {
        void* const owner = c_read<void*>(reinterpret_cast<const void*>(lp));
        SetWindowLongA(window, 0, reinterpret_cast<LONG>(owner));
        c_write<std::uint8_t>(owner, 0x40, 1);
        return 0;
    }
    if (message == WM_SIZE) {
        if (wp != 4 && wp != 1) {
            if (c_read<std::uint8_t>(extra, 0xc)) z_order(extra, HWND_TOPMOST);
            c_write<std::uint8_t>(extra, 0x40, 1);
            c_write<std::uint8_t>(extra, 0x41, 1);
            SetFocus(c_read<HWND>(active, 0x30));
            SetForegroundWindow(c_read<HWND>(active, 0x30));
        } else {
            if (c_read<std::uint8_t>(extra, 0xc)) z_order(extra, HWND_NOTOPMOST);
            c_write<std::uint8_t>(extra, 0x40, 0);
            c_write<std::uint8_t>(extra, 0x41, 0);
        }
        return standard();
    }
    if (message == WM_ACTIVATE) {
        if (static_cast<std::uint16_t>(wp) == 0) {
            b.xinput_enable_00c2f172(0);
            const bool reorder = c_read<std::uint8_t>(extra, 0xc) != static_cast<std::uint8_t>(wp);
            c_write<std::uint8_t>(extra, 0x41, static_cast<std::uint8_t>(wp));
            if (reorder) z_order(extra, HWND_NOTOPMOST);
            b.sound_pause_00a7a480(b.sound_f8bbd8);
            b.media_pause_00a4c2d0(b.media_get_004c1710(), 1);
            b.gui_pause_00aa33a0(b.gui_get_004c12b0(), 1);
        } else {
            b.xinput_enable_00c2f172(1);
            if (c_read<std::uint8_t>(extra, 0xc)) z_order(extra, HWND_TOPMOST);
            c_write<std::uint8_t>(extra, 0x40, 1);
            c_write<std::uint8_t>(extra, 0x41, 1);
            b.gui_pause_00aa33a0(b.gui_get_004c12b0(), 0);
            b.sound_resume_00a7a4a0(b.sound_f8bbd8);
            b.media_pause_00a4c2d0(b.media_get_004c1710(), 0);
            b.reload_renderer_00b24fb0(b.renderer_f8d394);
            SetForegroundWindow(c_read<HWND>(active, 0x30));
            SetFocus(c_read<HWND>(active, 0x30));
            if (c_read<std::uint8_t>(active, 0xc)) {
                void* const service = b.render_service_f8d39c;
                if (service) b.refresh_render_service_00b0d1e0(service);
            }
        }
        return standard();
    }
    if (message == WM_PAINT) { standard(); return 0; }
    if (message == WM_CLOSE) { c_write<std::uint8_t>(active, 0x180, 1); return 0; }
    if (message == WM_SETTINGCHANGE && (wp == 0x20 || wp == 0x21))
        c_write<std::uint8_t>(active, 0x2c, 1);
    if (message == WM_GETMINMAXINFO) {
        c_write<CWord>(reinterpret_cast<void*>(lp), 0x18, 100);
        c_write<CWord>(reinterpret_cast<void*>(lp), 0x1c, 100);
    }
    if (message == WM_SYSCOMMAND && wp == 0xf140) return 1;
    if (message == WM_ENTERSIZEMOVE) c_write<std::uint8_t>(extra, 0x42, 0);
    if (message == WM_EXITSIZEMOVE) c_write<std::uint8_t>(extra, 0x42, 1);
    if ((message == WM_CHAR || message == WM_KEYDOWN) && c_read<std::uint8_t>(active, 0x170)) {
        bool queue = message == WM_CHAR;
        if (message == WM_CHAR && wp == 0x16) c_write<std::uint8_t>(active, 0x44, 1);
        if (message == WM_KEYDOWN) {
            switch (wp) {
            case 0x26: case 0x28: case 0x25: case 0x27: case 0x24: case 9:
            case 0x23: case 0x2e: case 0x2d: case 0x14: case 0x21: case 0x22:
                queue = true; break;
            default: break;
            }
        }
        if (queue) {
            const CWord payload = (reinterpret_cast<CWord>(active) & 0xffff0000U)
                | (message == WM_KEYDOWN ? 0x100U : 0) | (wp & 0xff);
            b.append_text_00bed370(c_at(active, 0x174), &payload);
        }
    }
    return standard();
}
} // namespace bsp::oct18_tail_c

namespace bsp::oct18_tail_c {
namespace {
static_assert(sizeof(TailNativeString) == 8 && sizeof(TailCheckedVector) == 16);
TailNativeString c_string_copy(const TailNativeString* source, const TailStringBindings& b) {
    TailNativeString result{};
    b.resize_0041dd40(&result, source->length, 1);
    if (source->length) std::memcpy(result.data, source->data, result.length);
    return result;
}
TailNativeString c_string_literal(const char* source, const TailStringBindings& b) {
    TailNativeString result{};
    b.resize_0041dd40(&result, static_cast<CWord>(std::strlen(source)), 1);
    if (result.data) std::memcpy(result.data, source, result.length + 1);
    return result;
}
void c_string_release(const TailNativeString& value, const TailStringBindings& b) {
    if (value.data) b.release_00419cc0_00bd1510(value.data, value.length + 1, 1);
}
CWord c_vector_count(const TailCheckedVector& v, unsigned shift) {
    return v.begin ? static_cast<CWord>(static_cast<std::int32_t>(
        reinterpret_cast<CWord>(v.end) - reinterpret_cast<CWord>(v.begin)) >> shift) : 0;
}
void c_warning_insert(TailCheckedVector& v, void* box, const WarningDispatchBindings& b) {
    if (!v.begin || c_vector_count(v, 2) >= static_cast<CWord>(
        static_cast<std::int32_t>(reinterpret_cast<CWord>(v.capacity) -
            reinterpret_cast<CWord>(v.begin)) >> 2)) {
        if (reinterpret_cast<CWord>(v.end) < reinterpret_cast<CWord>(v.begin))
            b.checked_iterator_bf6713();
        WarningIteratorPair result;
        b.insert_box_009736e0(&v, &result, {&v, v.end}, &box);
    } else {
        c_write<void*>(v.end, 0, box);
        v.end = c_at(v.end, 4);
    }
}
void c_warning_destroy_boxes(TailCheckedVector& boxes) {
    for (CWord i = 0; i < c_vector_count(boxes, 2); ++i) {
        void* const slot = c_at(boxes.begin, 4 * i);
        void* const box = c_read<void*>(slot);
        if (box) {
            c_virtual<void>(box, 0, CWord{1});
            c_write<void*>(slot, 0, nullptr);
        }
    }
}
void c_warning_cleanup(TailCheckedVector& boxes, TailCheckedVector& names,
    const WarningDispatchBindings& b) {
    if (names.begin) {
        void* const end = names.end;
        for (void* p = names.begin; p != end; p = c_at(p, 8))
            c_string_release(c_read<TailNativeString>(p), b.strings);
        b.free_00bf65ac(names.begin);
    }
    names.begin = names.end = names.capacity = nullptr;
    if (boxes.begin) b.free_00bf65ac(boxes.begin);
}
void* c_warning_integer(CWord value, const WarningDispatchBindings& b) {
    void* const result = b.allocate_00bf681b(8);
    if (result) {
        c_write<void*>(result, 0, b.integer_box_vtable_00d1af2c);
        c_write<CWord>(result, 4, value);
    }
    return result;
}
void* c_warning_string(const TailNativeString* value, bool copy_constructor,
    const WarningDispatchBindings& b) {
    void* const result = b.allocate_00bf681b(12);
    if (!result) return nullptr;
    TailNativeString argument{};
    if (copy_constructor) b.strings.copy_00426060(&argument, value);
    else argument = c_string_copy(value, b.strings);
    return b.string_box_0096dc00(result, argument); // callee consumes argument
}
void* c_warning_arguments(const WarningDispatchBindings& b) {
    void* const result = b.allocate_00bf681b(12);
    if (result) {
        c_write<CWord>(result, 0, 0); c_write<CWord>(result, 4, 0);
        c_write<CWord>(result, 8, 0);
    }
    return result;
}
void c_warning_push_variant(void* arguments, void* variant, const WarningDispatchBindings& b) {
    b.variant_push_006edf00(arguments, variant);
    b.variant_destroy_00884af0(variant);
}
void c_warning_calls(TailCheckedVector& names, void* arguments,
    void* volatile& game, const WarningDispatchBindings& b) {
    for (CWord i = 0; i < c_vector_count(names, 3); ++i)
        b.lua_call_00887e50(c_read<void*>(game, 0x1a08), 0,
            c_at(names.begin, i * 8), arguments, 0, -1);
    if (arguments) {
        b.variant_resize_006b88e0(arguments, 0);
        b.delete_array_00bf6989(c_read<void*>(arguments));
        b.free_00bf65ac(arguments);
    }
}
}

// 00985F30: ECX manager, three DWORD stack arguments, RET0C. The source owns
// the real temporary headers and boxes; all allocation/pool/Lua calls are bound.
void warning_dispatch_chat_00985f30(void* manager, const TailNativeString* player_name,
    std::int32_t message_type, const TailNativeString* message,
    void* volatile& image_e188a8, const WarningDispatchBindings& b) {
    CWarningGuard guard(c_read<void*>(manager, 0x24));
    std::int32_t player_index = -1;
    for (CWord i = 0; i < 8; ++i) {
        void* const player = c_read<void*>(image_e188a8, 0x18cc + 4 * i);
        if (!c_read<std::uint8_t>(player, 8) || c_read<std::uint8_t>(player, 9)) continue;
        const char* const name = static_cast<const char*>(c_at(player, 0x58));
        bool match;
        if (!player_name->data) match = !name || std::strlen(name) == 0;
        else match = (name ? b.stricmp_00bf7fbf(player_name->data, name) :
            static_cast<std::int32_t>(player_name->length)) == 0;
        if (match) { player_index = static_cast<std::int32_t>(i); break; }
    }
    c_write<std::uint8_t>(manager, 0x104, 0);
    c_write<std::uint8_t>(manager, 0x105, 0);
    const auto channel_name = c_string_literal("chat", b.strings);
    void* const channel = b.channel_00980150(c_at(manager, 0xf8), &channel_name);
    c_string_release(channel_name, b.strings);
    if (!c_read<CWord>(channel, 8)) return;
    TailCheckedVector boxes{}, names{};
    c_warning_insert(boxes, c_warning_integer(static_cast<CWord>(player_index), b), b);
    c_warning_insert(boxes, c_warning_string(message, false, b), b);
    c_warning_insert(boxes, c_warning_integer(static_cast<CWord>(message_type), b), b);
    b.evaluate_0097b8c0(channel, &boxes, &names);
    c_warning_destroy_boxes(boxes);
    if (c_vector_count(names, 3)) {
        void* const arguments = c_warning_arguments(b);
        std::uint32_t variant[5];
        b.variant_int_00884a00(variant, player_index);
        c_warning_push_variant(arguments, variant, b);
        b.variant_string_00884fc0(variant, message);
        c_warning_push_variant(arguments, variant, b);
        b.variant_int_00884a00(variant, message_type);
        c_warning_push_variant(arguments, variant, b);
        c_warning_calls(names, arguments, image_e188a8, b);
        c_write<std::uint8_t>(manager, 0x105, 1);
    }
    c_warning_cleanup(boxes, names, b);
}

// 00986B00: ECX manager, entity/reason/oldHP/newHP stack, RET10. Strict ordered
// x87 comparisons reject equality and unordered values. No +104/+105 stores.
void warning_dispatch_hp_00986b00(void* manager, void* entity,
    const TailNativeString* reason, float old_hp, float new_hp,
    void* volatile& image_e188a8, const WarningDispatchBindings& b) {
    CWarningGuard guard(c_read<void*>(manager, 0x24));
    if (b.ancestor_00923810(entity, 1)) {
        void* const ancestor = b.ancestor_00923810(entity, 1);
        if (!c_virtual<std::uint8_t>(ancestor, 0x5c)) return;
    }
    TailNativeString channel_name;
    b.strings.assign_0041e870(&channel_name, "hpEvent");
    void* const channel = b.channel_00980150(c_at(manager, 0xf8), &channel_name);
    c_string_release(channel_name, b.strings);
    if (!c_read<CWord>(channel, 8)) return;
    TailCheckedVector boxes{}, names{};
    void* box = b.allocate_00bf681b(8);
    if (box) {
        c_write<void*>(box, 0, b.entity_box_vtable_00d1af24);
        c_write<std::uint16_t>(box, 4, entity ? c_read<std::uint16_t>(entity, 0x174) : 0);
    }
    b.push_box_00975ea0(&boxes, &box);
    box = c_warning_string(reason, true, b);
    b.push_box_00975ea0(&boxes, &box);
    WarningIteratorPair iterator{channel, c_read<void*>(c_read<void*>(channel, 4))};
    for (;;) {
        void* const node = iterator.node;
        void* const owner = iterator.owner;
        void* const sentinel = c_read<void*>(channel, 4);
        if (!owner || owner != channel) b.checked_iterator_bf6713();
        if (node == sentinel) break;
        if (!owner) b.checked_iterator_bf6713();
        if (node == c_read<void*>(owner, 4)) b.checked_iterator_bf6713();
        if (c_virtual<std::uint8_t>(c_read<void*>(node, 0x14), 0xc, &boxes)) {
            if (node == c_read<void*>(owner, 4)) b.checked_iterator_bf6713();
            void* const subscription = c_read<void*>(node, 0x14);
            bool crossing = false;
            if (reason->data && !b.stricmp_00bf7fbf(reason->data, "damage")) {
                const float threshold = c_read<float>(subscription, 0x2c);
                crossing = old_hp > threshold && threshold > new_hp;
            }
            if (reason->data && !b.stricmp_00bf7fbf(reason->data, "repair")) {
                const float threshold = c_read<float>(subscription, 0x2c);
                crossing = (old_hp < threshold && threshold < new_hp) || crossing;
            }
            if (crossing) {
                if (node == c_read<void*>(owner, 4)) b.checked_iterator_bf6713();
                b.append_name_00450540(&names, c_at(c_read<void*>(node, 0x14), 4));
            }
        }
        b.iterator_next_00966960(&iterator);
    }
    c_warning_destroy_boxes(boxes);
    if (c_vector_count(names, 3)) {
        void* const arguments = c_warning_arguments(b);
        std::uint32_t variant[5];
        b.variant_entity_00884a90(variant, entity);
        c_warning_push_variant(arguments, variant, b);
        b.variant_string_00884fc0(variant, reason);
        c_warning_push_variant(arguments, variant, b);
        c_warning_calls(names, arguments, image_e188a8, b);
    }
    c_warning_cleanup(boxes, names, b);
}

// 0098E910: ECX owner, EAX owner+C3C, RET. Preserve the original append order,
// native antialias header reads, eight fixed settings, and mutable image byte.
char* hardware_default_options_0098e910(void* owner,
    const volatile std::uint32_t& image_f8a0e0, const HardwareOptionsBindings& b) {
    char* const output = static_cast<char*>(c_at(owner, 0xc3c));
    *output = 0;
    CWord width = 640, height = 480;
    const auto level = b.detail_0098df90(owner);
    if (level == 1) {
        if (b.supports_0098e0e0(owner, 800, 600)) { width = 800; height = 600; }
    } else if (level == 2 || level == 3) {
        if (level == 3 && b.supports_0098e0e0(owner, 1280, 1024)) {
            width = 1280; height = 1024;
        } else if (b.supports_0098e0e0(owner, 1024, 768)) { width = 1024; height = 768; }
    }
    char temporary[2048];
    auto append = [&](const char* value) { std::strcat(output, value); };
    append("fullscreen 1\n");
    std::sprintf(temporary, "Resolution %i %i\n", width, height); append(temporary);
    append("Vsync 1\n");
    std::sprintf(temporary, "ShaderModel %i\n", c_read<std::int32_t>(owner, 0x24)); append(temporary);
    CWord antialias = 0;
    void* const first = c_read<void*>(owner, 0x14);
    const CWord count = first ? static_cast<CWord>(static_cast<std::int32_t>(
        c_read<CWord>(owner, 0x18) - reinterpret_cast<CWord>(first)) >> 2) : 0;
    if (level == 3 && first && count) {
        if (!c_read<void*>(owner, 0x14) || static_cast<CWord>(static_cast<std::int32_t>(
            c_read<CWord>(owner, 0x18) - c_read<CWord>(owner, 0x14)) >> 2) <= count - 1)
            b.checked_iterator_bf6713();
        antialias = c_read<CWord>(c_read<void*>(owner, 0x14), (count - 1) * 4);
    }
    std::sprintf(temporary, "Antialias %i\n", antialias); append(temporary);
    append("Clouds 1\n");
    append(level == 3 ? "Foliage 1\n" : "Foliage 0\n");
    append(level < 2 ? "Shadow 0\n" : "Shadow 1\n");
    append(level < 2 ? "Reflection 0\n" : "Reflection 1\n");
    append(level < 2 ? "OceanDetail 0\n" : "OceanDetail 1\n");
    append(level < 2 ? "TextureDetail 0\n" : "TextureDetail 2\n");
    append(level < 2 ? "ObjectDetail 0\n" : "ObjectDetail 2\n");
    append("EAX 0\n"); append("SoundSW 0\n"); append("Log 0\n");
    append("CheatsOnAtStartup 0\n"); append("UseFpsLimit 0\n");
    append("FPSLimit 30\n"); append("HiResCapture 2\n"); append("Firewall 1\n");
    append(image_f8a0e0 ? "SoundEnabled 1\n" : "SoundEnabled 0\n");
    append("Language ");
    const char* language = "english";
    switch (c_read<CWord>(owner, 0xc38)) {
    case 0x407: language = "deutsch"; break;
    case 0x40c: language = "francais"; break;
    case 0x410: language = "italiano"; break;
    case 0x415: language = "polski"; break;
    case 0x419: language = "russian"; break;
    }
    append(language); append("\n");
    return output;
}

void icon_visibility_changed_00ab6120(void* icon, std::uint8_t visible,
    void* volatile& image_f8d394, const IconVisibilityBindings& b) {
    CWord index = 0, offset = 0;
    const auto valid = [&]() {
        const CWord begin = c_read<CWord>(icon, 0xf4);
        return begin && index < static_cast<CWord>(static_cast<std::int32_t>(
            c_read<CWord>(icon, 0xf8) - begin) >> 6);
    };
    const auto check = [&]() { if (!valid()) b.checked_iterator_bf6713(); };
    const auto state = [&]() { return c_at(c_read<void*>(icon, 0xf4), offset); };
    while (valid()) {
        check();
        if (c_read<std::uint8_t>(state(), 0x3c) && visible) {
            void* const renderer = image_f8d394;
            check();
            void* const texture = c_virtual<void*>(renderer, 0x68, c_at(state(), 4), CWord{0});
            check();
            auto argument = c_string_copy(static_cast<TailNativeString*>(c_at(state(), 4)), b.strings);
            if (texture) {
                b.apply_00ab3840(icon, argument, texture); // consumes name
                c_release_ref(texture);
            } else {
                void* const delayed = b.delayed_singleton_00ab5700();
                b.request_00ab5f00(delayed, argument, icon); // consumes name
            }
        } else {
            check();
            if (c_read<std::uint8_t>(state(), 0x3d) && !visible) {
                check();
                b.placeholder_00ab3b30(state());
                if (index == static_cast<CWord>(static_cast<std::int32_t>(c_read<std::int16_t>(icon, 0xec))))
                    c_virtual<void>(icon, 0x80, index);
            }
        }
        ++index; offset += 0x40;
    }
}
} // namespace bsp::oct18_tail_c

namespace bsp::oct18_tail_c {
namespace {
float c_network_seconds(const void* timestamp) {
    float result;
    __asm {
        mov eax, timestamp
        fild qword ptr [eax]
        fild qword ptr [eax + 8]
        fdivp st(1), st(0)
        fstp result
    }
    return result;
}
float c_network_rate(void* channel, const NetworkWorkerImage& image) {
    const CWord count = c_read<CWord>(channel, 0x48c);
    if (!count) return image.no_samples_d7a2f0;
    if (count < 5) return c_read<float>(image.current_owner_f8abdc,
        0x10 + c_read<CWord>(channel, 0x494) * 0x14);
    const void* const numerator = c_at(channel, 0x490);
    const volatile float* const bias = &image.unsigned_bias_ce3978;
    float result;
    __asm {
        mov eax, numerator
        fld dword ptr [eax]
        fild count
        test count, 80000000h
        jz rate_nonnegative
        mov eax, bias
        fadd dword ptr [eax]
    rate_nonnegative:
        fdivp st(1), st(0)
        fstp result
    }
    return result;
}
float c_network_interval(CWord round, float rate, const NetworkWorkerImage& image) {
    const volatile double* const cap = &image.latency_cap_cf1748;
    const volatile float* const clamped = &image.capped_latency_cf2e4c;
    const volatile float* const unsigned_bias = &image.unsigned_bias_ce3978;
    const volatile double* const weight = &image.round_weight_cf0ac0;
    const volatile double* const bias = &image.round_bias_d7a270;
    float result, extra;
    __asm {
        mov eax, cap
        fld qword ptr [eax]
        fld rate
        fcomip st(0), st(1)
        fstp st(0)
        jbe rate_not_clamped
        mov eax, clamped
        fld dword ptr [eax]
        fstp rate
    rate_not_clamped:
        fild round
        test round, 80000000h
        jz round_nonnegative
        mov eax, unsigned_bias
        fadd dword ptr [eax]
    round_nonnegative:
        mov eax, weight
        fmul qword ptr [eax]
        mov eax, bias
        fadd qword ptr [eax]
        fstp extra
        fld extra
        fadd rate
        fstp result
    }
    return result;
}
bool c_network_ack_due(float now, float last, const volatile double& delay) {
    const volatile double* const p_delay = &delay;
    std::uint8_t result;
    __asm {
        fld now
        fld last
        mov eax, p_delay
        fadd qword ptr [eax]
        fxch st(1)
        fcomip st(0), st(1)
        fstp st(0)
        seta result
    }
    return result != 0;
}
bool c_network_error_due(float now, float last, const NetworkWorkerImage& image) {
    const volatile double* const delay = &image.error_delay_ce6628;
    std::uint8_t result;
    __asm {
        fld now
        fsub last
        mov eax, delay
        fld qword ptr [eax]
        fxch st(1)
        fcomip st(0), st(1)
        fstp st(0)
        seta result
    }
    return result != 0;
}
void c_network_receive_record(void* owner, void* record, CWord socket_index,
    NetworkWorkerScratch& scratch, CWord size) {
    c_write<void*>(owner, 0x4003ec, c_read<void*>(record));
    c_write<CWord>(record, 0, 0);
    c_write<CWord>(record, 4, c_read<CWord>(owner, 0x160 + socket_index * 4));
    for (CWord i = 0; i < 4; ++i) c_write<CWord>(record, 8 + i * 4, scratch.receive_address[i]);
    c_write<CWord>(record, 0x18, size);
}
}

// 00A3C5E0: complete ordinary loop. ECX actual4003F0h owner, EAX0/RET only
// after quit is observed under +14C. Native EH/concurrency/SDK ABI unvalidated.
std::uint32_t network_send_worker_00a3c5e0(void* owner, NetworkWorkerScratch& scratch,
    const NetworkWorkerImage& image, const NetworkWorkerBindings& b) {
    for (;;) {
        {
            CWarningGuard guard(c_read<void*>(owner, 0x14c));
            if (image.quit_f8abe0) { image.send_active_f8abe1 = 0; return 0; }
            image.send_active_f8abe1 = 1;
        }
        c_virtual<void*>(image.clock_1090ab0, 0x20, scratch.discarded_timestamp);
        b.sleep_25(25);
        {
            CWarningGuard guard(c_read<void*>(owner, 0x148));
            for (CWord i = 0; i < 16; ++i) {
                void* const channel = c_at(owner, 0x19c + i * 0x49c);
                if (c_read<std::uint8_t>(channel)) continue;
                const float now = c_network_seconds(c_virtual<void*>(image.clock_1090ab0,
                    0x20, scratch.maintenance_timestamp));
                const auto closing = c_read<std::uint8_t>(channel, 0x18);
                const float deadline = c_read<float>(channel, 0x1c);
                // FCOMIP/JC: unordered follows the not-yet-due branch.
                if (closing && now >= deadline) b.remove_00a3bc60(owner, c_at(channel, 4));
                else if (static_cast<std::int32_t>(c_read<CWord>(channel, 0x70) -
                    c_read<CWord>(channel, 0x60)) > 0xa8 && !closing)
                    b.disconnect_00a3adf0(channel, 1.0f);
            }
        }
        CWarningGuard guard(c_read<void*>(owner, 0x148));
        if (c_read<CWord>(owner, 0x4b60)) {
            for (CWord i = 0; i < c_read<CWord>(owner, 0x4b60); ++i) {
                void* const record = c_at(owner, 0x5b08 +
                    CWord{c_read<std::uint16_t>(owner, 0x4b68 + 2 * i)} * 0x82c);
                if (c_read<CWord>(record, 0xc))
                    b.sendto_00a4d55a(c_read<CWord>(record, 4), c_at(record, 0x28),
                        c_read<CWord>(record, 0x20), 0, c_at(record, 8), 16);
                c_write<void*>(record, 0, c_read<void*>(owner, 0x2046e8));
                c_write<void*>(owner, 0x2046e8, record);
            }
            c_write<CWord>(owner, 0x4b60, 0);
        }
        const float now = c_network_seconds(c_virtual<void*>(image.clock_1090ab0,
            0x20, scratch.send_timestamp));
        CWord round = 1;
        bool progress = false;
        for (CWord i = 0; static_cast<std::int32_t>(i) < 16; ++i) {
            void* const channel = c_at(owner, 0x19c + i * 0x49c);
            if (!c_read<std::uint8_t>(channel)) {
                const float interval = c_network_interval(round, c_network_rate(channel, image), image);
                std::uint8_t removed, stale;
                void* const record = b.select_00a3a8f0(channel, &removed, &stale, now, interval);
                if (record) {
                    void* const header = c_at(record, 0x28);
                    b.ack_00a3aab0(channel, header);
                    const CWord size = c_read<CWord>(record, 0x20);
                    if (!stale) {
                        const auto result = b.sendto_00a4d55a(c_read<CWord>(record, 4),
                            header, size, 0, c_at(record, 8), 16);
                        void* const statistics = image.statistics_f871b4; // before find callback
                        const auto index = b.find_00a3bb30(owner, c_at(record, 8));
                        b.sent_00787b00(statistics, index, header, size);
                        if (result != -1) {
                            c_write<float>(channel, 0x24, now);
                            progress = true;
                        } else {
                            b.last_error_00a4d4e8();
                            b.htons_00a4d542(c_read<std::uint16_t>(record, 0xa));
                            if (!c_read<std::uint8_t>(channel, 0x18)) {
                                const float last = c_read<float>(channel, 0x24);
                                if (last > image.zero_d7a218 && c_network_error_due(now, last, image))
                                    b.disconnect_00a3adf0(channel, 0.0f);
                            }
                        }
                    } else progress = true;
                    if (removed || stale) {
                        c_write<void*>(record, 0, c_read<void*>(owner, 0x2046e8));
                        c_write<void*>(owner, 0x2046e8, record);
                    }
                } else {
                    const float last = c_read<float>(channel, 0x24);
                    if (!c_read<std::uint8_t>(channel) && !c_read<std::uint8_t>(channel, 0x18)) {
                        const CWord socket = c_read<CWord>(channel, 0x14);
                        if (socket && c_network_ack_due(now, last, image.ack_delay_d7a280) &&
                            !b.enqueue_00a3c1a0(owner, socket, c_at(channel, 4), 8, nullptr, 0, 0))
                            c_write<CWord>(channel, 0x14, 0);
                    }
                }
            }
            if (i == 15 && progress) { ++round; i = 0xffffffffu; progress = false; }
        }
    }
}

// 00A3CAF0: full socket-drain loop, RET/EAX0 on quit; no short-packet clamp,
// synthetic packet allocation, list sorting, worker timeout or rollback.
std::uint32_t network_receive_worker_00a3caf0(void* owner, NetworkWorkerScratch& scratch,
    const NetworkWorkerImage& image, const NetworkWorkerBindings& b) {
    for (;;) {
        {
            CWarningGuard guard(c_read<void*>(owner, 0x14c));
            if (image.quit_f8abe0) { image.receive_active_f8abe2 = 0; return 0; }
            image.receive_active_f8abe2 = 1;
        }
        c_virtual<void*>(image.clock_1090ab0, 0x20, scratch.discarded_timestamp);
        b.sleep_25(25);
        CWarningGuard guard(c_read<void*>(owner, 0x148));
        bool progress = false;
        for (CWord i = 0; i < c_read<CWord>(owner, 0x15c); ++i) {
            CWord address_size = 16;
            const auto size = b.recvfrom_00a4d54e(c_read<CWord>(owner, 0x160 + i * 4),
                scratch.receive_packet, 0x800, 0, scratch.receive_address, &address_size);
            if (size > 0) {
                progress = true;
                if (scratch.receive_packet[0] == 3 && scratch.receive_packet[1] == 0) {
                    if (!c_read<std::uint8_t>(owner, 0x4b5c)) {
                        void* const record = c_read<void*>(owner, 0x4003ec);
                        if (record) {
                            c_network_receive_record(owner, record, i, scratch, static_cast<CWord>(size));
                            std::memcpy(c_at(record, 0x20), scratch.receive_packet, size);
                            const CWord index = static_cast<CWord>(static_cast<std::int32_t>(
                                reinterpret_cast<CWord>(record) - reinterpret_cast<CWord>(owner) - 0x2046ec) / 0x820);
                            c_write<std::uint16_t>(owner, 0x5338 + c_read<CWord>(owner, 0x4b64) * 2,
                                static_cast<std::uint16_t>(index));
                            c_write<CWord>(owner, 0x4b64, c_read<CWord>(owner, 0x4b64) + 1);
                        }
                    }
                } else {
                    auto index = b.find_00a3bb30(owner, scratch.receive_address);
                    if (index == -1 && c_read<std::uint8_t>(owner, 0x150) &&
                        !(scratch.receive_packet[2] & 0xf)) {
                        b.activate_00a3baa0(owner, scratch.receive_address);
                        index = b.find_00a3bb30(owner, scratch.receive_address);
                    }
                    if (index != -1) {
                        void* const channel = c_at(owner, 0x19c + CWord{static_cast<CWord>(index)} * 0x49c);
                        std::int32_t stream = -1;
                        CWord sequence = 0;
                        void* const released = b.classify_00a3b5f0(channel, scratch.receive_packet, &stream, &sequence);
                        if (released) {
                            void* tail = released;
                            while (c_read<void*>(tail)) tail = c_read<void*>(tail);
                            c_write<void*>(tail, 0, c_read<void*>(owner, 0x2046e8));
                            c_write<void*>(owner, 0x2046e8, released);
                        }
                        if (stream >= 0) {
                            void* const record = c_read<void*>(owner, 0x4003ec);
                            if (record) {
                                c_network_receive_record(owner, record, i, scratch, static_cast<CWord>(size));
                                c_write<CWord>(record, 0x1c, sequence);
                                std::memcpy(c_at(record, 0x20), scratch.receive_packet, size);
                                b.append_00a3ab30(channel, record, static_cast<CWord>(stream));
                            }
                        }
                    }
                    const auto type = scratch.receive_packet[2] & 0xf;
                    if (type == 5 || type == 6)
                        b.received_00787aa0(image.statistics_f871b4, index, scratch.receive_packet,
                            static_cast<CWord>(size));
                }
            }
            if (i == c_read<CWord>(owner, 0x15c) - 1 && progress) {
                i = 0xffffffffu; progress = false;
            }
        }
    }
}
} // namespace bsp::oct18_tail_c

namespace bsp::oct18_tail_c {
namespace {
void c_cstring_release(char* data, const TailStringBindings& b) {
    if (data) b.release_00419cc0_00bd1510(data, static_cast<CWord>(std::strlen(data)) + 1, 1);
}
void c_atlas_append(void* header, void* value,
    void (__thiscall* reserve)(void*, std::int32_t)) {
    if (c_read<CWord>(header, 4) == c_read<CWord>(header, 8)) {
        auto size = static_cast<std::int32_t>(c_read<CWord>(header, 8) * 2);
        if (size <= 1) size = 1;
        reserve(header, size);
    }
    void* const slot = c_at(c_read<void*>(header), c_read<CWord>(header, 4) * 4);
    if (slot) c_write<void*>(slot, 0, value);
    c_write<CWord>(header, 4, c_read<CWord>(header, 4) + 1);
}
void c_string_assign_header(void* destination, const TailNativeString* source,
    const TailStringBindings& b) {
    if (destination == source) return;
    b.resize_0041dd40(destination, source->length, 1);
    if (source->length) std::memcpy(c_read<void*>(destination, 4), source->data,
        c_read<CWord>(destination));
}
std::uint16_t c_font_scaled_metric(std::int32_t value, const void* scale) {
    std::int32_t result;
    __asm {
        cvtsi2ss xmm0, value
        mov eax, scale
        mulss xmm0, dword ptr [eax]
        cvttss2si eax, xmm0
        mov result, eax
    }
    return static_cast<std::uint16_t>(result);
}
}

// 00AEEAF0: ECX manager, stack actual TextBuffer, RET4/AL. EBP is a local
// buffer capture, not an additional input. The decompiler's unaff ESI/EBP are
// artifacts of changing stack offsets. EOF after the header still succeeds.
std::uint8_t atlas_parse_00aeeaf0(void* manager, void* buffer,
    void* volatile& image_f8d394, const AtlasParseBindings& b) {
    b.rewind_00af55f0(buffer);
    char* line = nullptr;
    if (!b.line_00af5740(buffer, &line)) {
        c_cstring_release(line, b.text.strings); return 0;
    }
    char* token;
    char** result = b.token_00aee3c0(&line, &token, 0);
    const bool bad_header = b.stricmp_00bf7fbf(*result, "TextureAtlas") != 0;
    c_cstring_release(token, b.text.strings);
    if (bad_header) { c_cstring_release(line, b.text.strings); return 0; }
    result = b.token_00aee3c0(&line, &token, 1);
    TailNativeString texture_name;
    b.text.strings.assign_0041e870(&texture_name, *result);
    c_cstring_release(token, b.text.strings);
    TailNativeString directory, canonicalized;
    b.text.strings.copy_00426060(&directory, c_at(buffer, 0xc));
    // Native computes this temporary and immediately destroys it. Its result is
    // NOT assigned back to directory; preserve that observable path distinction.
    b.canonicalize_00bee390(&canonicalized, &directory);
    c_string_release(canonicalized, b.text.strings);
    const CWord length = b.reverse_find_004bcb80(&directory, "/", 0x7fffffffu);
    const CWord old_length = directory.length;
    b.text.strings.resize_0041dd40(&directory, length, 1);
    if (old_length < directory.length)
        std::memset(directory.data + old_length, 0x20, directory.length - old_length);
    TailNativeString slash;
    b.text.strings.assign_0041e870(&slash, "/");
    const CWord slash_length = slash.length;
    char* const slash_data = slash.data;
    if (slash_length) {
        const CWord before = directory.length;
        b.text.strings.resize_0041dd40(&directory, before + slash_length, 1);
        std::memcpy(directory.data + before, slash_data, slash_length);
    }
    if (slash_data) b.text.strings.release_00419cc0_00bd1510(slash_data, slash_length + 1, 1);
    void* const renderer = image_f8d394;
    TailNativeString full_name;
    void* const name = b.text.concat_004261a0(&directory, &full_name, &texture_name);
    void* const texture = c_virtual<void*>(renderer, 0x64, name, CWord{0});
    c_string_release(full_name, b.text.strings);
    c_atlas_append(c_at(manager, 0x10), texture, b.reserve_textures_00735ff0);
    while (b.line_00af5740(buffer, &line)) {
        if (!b.stricmp_00bf7fbf(line, "{")) break;
    }
    if (b.line_00af5740(buffer, &line)) {
        do {
            if (!b.stricmp_00bf7fbf(line, "}")) break;
            if (b.stricmp_00bf7fbf(line, "")) {
                result = b.token_00aee3c0(&line, &token, 0);
                const bool is_item = !b.stricmp_00bf7fbf(*result, "TextureItem");
                c_cstring_release(token, b.text.strings);
                if (is_item) {
                    void* const allocation = b.allocate_00bf681b(0x30);
                    void* item = nullptr;
                    if (allocation) {
                        result = b.token_00aee3c0(&line, &token, 1);
                        TailNativeString item_name;
                        b.text.strings.assign_0041e870(&item_name, *result);
                        item = b.item_00aee4d0(allocation, &item_name, texture, c_at(buffer, 0xc));
                        c_string_release(item_name, b.text.strings);
                        c_cstring_release(token, b.text.strings);
                    }
                    b.item_params_00aee620(item, buffer); // status intentionally ignored
                    c_atlas_append(c_at(manager, 4), item, b.reserve_items_00735f90);
                }
            }
        } while (b.line_00af5740(buffer, &line));
    }
    c_string_release(directory, b.text.strings);
    c_string_release(texture_name, b.text.strings);
    c_cstring_release(line, b.text.strings);
    return 1;
}

// 00AD4C30: ECX actual font; stack root, DAT, GFX, alpha, language; RET14.
// All scalar reads, cache publications and native map calls retain their actual
// owners. STL lookup/insertion and pool/renderer/VFS services remain bindings.
void font_load_00ad4c30(void* font, const TailNativeString* root,
    const TailNativeString* dat, const TailNativeString* gfx, const TailNativeString* alpha,
    const TailNativeString* language, void* volatile& image_f8d394,
    void* volatile& image_109ceec, FontLoadScratch& scratch, const FontLoadBindings& b) {
    TailNativeString gfx_suffix = c_string_copy(gfx, b.text.strings);
    void* const gfx_renderer = image_f8d394;
    TailNativeString localized_root, full_gfx;
    void* intermediate = b.text.concat_004261a0(root, &localized_root, language);
    void* name = b.text.concat_004261a0(intermediate, &full_gfx, &gfx_suffix);
    void* const gfx_texture = c_virtual<void*>(gfx_renderer, 0x64, name, CWord{0});
    c_string_release(full_gfx, b.text.strings);
    c_string_release(localized_root, b.text.strings);
    c_string_assign_header(c_at(font, 0x38), root, b.text.strings);
    c_string_assign_header(c_at(font, 0x40), language, b.text.strings);
    c_string_assign_header(c_at(font, 0x20), dat, b.text.strings);
    c_string_assign_header(c_at(font, 0x30), gfx, b.text.strings);
    c_string_assign_header(c_at(font, 0x28), alpha, b.text.strings);
    void* alpha_texture;
    if (alpha->length) {
        void* const renderer = image_f8d394;
        TailNativeString full_alpha;
        name = b.text.concat_004261a0(root, &full_alpha, alpha);
        alpha_texture = c_virtual<void*>(renderer, 0x64, name, CWord{0});
        c_string_release(full_alpha, b.text.strings);
    } else {
        TailNativeString white;
        b.text.strings.assign_0041e870(&white, "white.tga");
        alpha_texture = c_virtual<void*>(image_f8d394, 0x64, &white, CWord{0});
        c_string_release(white, b.text.strings);
    }
    void* const files = image_109ceec;
    TailNativeString dat_root, full_dat;
    intermediate = b.text.concat_004261a0(root, &dat_root, language);
    name = b.text.concat_004261a0(intermediate, &full_dat, dat);
    void* const stream = c_virtual<void*>(files, 4, name, CWord{2});
    c_string_release(full_dat, b.text.strings);
    c_string_release(dat_root, b.text.strings);
    CWord remaining = c_virtual<CWord>(stream, 0x38, CWord{0});
    const auto height = static_cast<std::int16_t>(c_virtual<std::uint16_t>(stream, 0x40, CWord{0}));
    c_write<std::uint16_t>(font, 0x14, c_font_scaled_metric(height, c_at(font, 0x18)));
    while (remaining) {
        c_write<std::uint16_t>(&scratch.key, 0, c_virtual<std::uint16_t>(stream, 0x40, CWord{0}));
        void* const glyph = b.allocate_00bf681b(0x20);
        for (CWord i = 0; i < 4; ++i) {
            const float value = c_virtual<float>(stream, 0x44, CWord{0});
            c_fld_store(static_cast<float*>(c_at(glyph, i * 4)), &value);
        }
        c_write<std::uint16_t>(glyph, 0x10, c_virtual<std::uint16_t>(stream, 0x3c, CWord{0}));
        auto metric = c_virtual<std::uint16_t>(stream, 0x3c, CWord{0});
        c_write<std::uint16_t>(glyph, 0x12, metric);
        c_write<std::uint16_t>(glyph, 0x12, c_font_scaled_metric(metric, c_at(font, 0x18)));
        metric = c_virtual<std::uint16_t>(stream, 0x40, CWord{0});
        c_write<std::uint16_t>(glyph, 0x14, metric);
        const auto scaled_width = c_font_scaled_metric(metric, c_at(font, 0x18));
        c_write<void*>(glyph, 0x18, gfx_texture);
        c_write<void*>(glyph, 0x1c, alpha_texture);
        c_write<std::uint16_t>(glyph, 0x14, scaled_width);
        void* const sentinel = c_read<void*>(font, 4);
        WarningIteratorPair iterator;
        WarningIteratorPair* const found = b.lookup_00ad4540(font, &iterator, &scratch.key);
        if (!found->owner || found->owner != font) b.checked_iterator_bf6713();
        if (found->node == sentinel) {
            c_write<std::uint16_t>(scratch.pair, 0, c_read<std::uint16_t>(&scratch.key));
            c_write<void*>(scratch.pair, 4, glyph);
            b.insert_00ad4aa0(font, scratch.insertion_result, scratch.pair);
        } else b.free_00bf65ac(glyph);
        --remaining;
    }
    c_release_ref(stream);
    for (CWord offset = 0x4c; offset <= 0x68; offset += 4) c_write<CWord>(font, offset, 0);
    c_write<std::uint16_t>(font, 0x5e,
        static_cast<std::uint16_t>(static_cast<std::int32_t>(c_read<std::int16_t>(font, 0x14)) / 4));
    c_write<void*>(font, 0x68, alpha_texture);
    c_write<void*>(font, 0x64, gfx_texture);
    scratch.key = 0x91;
    WarningIteratorPair fallback;
    b.lookup_00ad4540(font, &fallback, &scratch.key);
    if (!fallback.owner) b.checked_iterator_bf6713();
    if (fallback.node == c_read<void*>(fallback.owner, 4)) b.checked_iterator_bf6713();
    const void* const glyph = c_read<void*>(fallback.node, 0x10);
    for (CWord i = 0; i < 8; ++i) c_write<CWord>(font, 0x6c + i * 4, c_read<CWord>(glyph, i * 4));
    for (CWord offset = 0x8c; offset <= 0xa0; offset += 4) c_write<CWord>(font, offset, 0);
    c_write<std::uint16_t>(font, 0x9e, 0);
    c_write<void*>(font, 0xa4, gfx_texture);
    c_write<void*>(font, 0xa8, alpha_texture);
    c_string_release(gfx_suffix, b.text.strings);
}
} // namespace bsp::oct18_tail_c

namespace bsp::oct18_tail_c {
namespace {
void c_glyph_coordinates(void* context, const void* glyph, const float* pen,
    std::int32_t height, void* first_position, const GlyphQuadImage& image,
    float* coordinates, float& white) {
    const auto bearing = static_cast<std::int32_t>(c_read<std::int16_t>(glyph, 0x10));
    const auto width = static_cast<std::int32_t>(c_read<std::uint16_t>(glyph, 0x14));
    const float y = pen[1];
    const volatile double* const horizontal = &image.width_cec380;
    const volatile double* const vertical = &image.height_cef1b8;
    const volatile float* const vertical_scale = &image.vertical_scale_e12fd4;
    const volatile float* const white_source = &image.white_d7a24c;
    float* const white_destination = &white;
    float raw_left, raw_right, raw_bottom;
    __asm {
        fild bearing
        mov eax, pen
        fadd dword ptr [eax]
        fst raw_left
        mov eax, context
        fld dword ptr [eax + 1d8h]
        fild width
        fmulp st(1), st(0)
        faddp st(1), st(0)
        fstp raw_right
        fld y
        fild height
        faddp st(1), st(0)
        fld1
        fadd st(1), st(0)
        mov eax, white_source
        movss xmm0, dword ptr [eax]
        mov eax, white_destination
        movss dword ptr [eax], xmm0
        mov eax, first_position
        mov dword ptr [eax + 8], 0
        fsubp st(1), st(0)
        fstp raw_bottom
        fld raw_left
        mov eax, horizontal
        fld qword ptr [eax]
        fdiv st(1), st(0)
        fxch st(1)
        mov edx, coordinates
        fstp dword ptr [edx]
        fld y
        mov eax, vertical
        fld qword ptr [eax]
        fdiv st(1), st(0)
        mov eax, vertical_scale
        fld dword ptr [eax]
        fld st(0)
        fmulp st(3), st(0)
        fxch st(2)
        fstp dword ptr [edx + 4]
        fld raw_right
        fdivrp st(3), st(0)
        fxch st(2)
        fstp dword ptr [edx + 8]
        fld raw_bottom
        fdivrp st(2), st(0)
        fmulp st(1), st(0)
        fstp dword ptr [edx + 12]
    }
}
float c_glyph_sum(float a, float b) {
    float result;
    __asm {
        fld a
        fadd b
        fstp result
    }
    return result;
}
float c_glyph_difference(float value, const volatile double& amount) {
    const volatile double* const p_amount = &amount;
    float result;
    __asm {
        fld value
        mov eax, p_amount
        fsub qword ptr [eax]
        fstp result
    }
    return result;
}
float c_glyph_input_x(const float* position, float reference_x, const GlyphQuadImage& image) {
    const volatile double* const bias = &image.input_x_bias_d06880;
    const volatile double* const width = &image.width_cec380;
    float result;
    __asm {
        mov eax, position
        fld dword ptr [eax]
        mov eax, bias
        fsub qword ptr [eax]
        mov eax, width
        fdiv qword ptr [eax]
        fadd reference_x
        fstp result
    }
    return result; // AB9F79 spills before subtracting D7A358.
}
}

// 00AB98F0: ECX actual Text, ten stack slots/RET28. Includes every ordinary
// branch after the established prefix. Native FH3, faults and allocation-failure
// transport are not reproduced; callbacks are the identified original callees.
void glyph_quad_00ab98f0(void* context, const void* glyph, const float* position,
    void* writer, std::uint16_t* indices, const void* unused_5, const void* unused_6,
    CWord quad_index, CWord height_word, CWord first_vertex, CWord code_unit_word,
    const GlyphQuadImage& image, const GlyphQuadBindings& b) {
    (void)unused_5; (void)unused_6; // neither original stack slot is ever read
    const auto base = [&](CWord vertex) {
        return c_at(c_read<void*>(writer, 8), c_read<CWord>(writer, 0xc) * vertex);
    };
    void* const first_position = c_at(base(first_vertex), c_read<CWord>(writer, 0x10));
    float rectangle[4], white;
    c_glyph_coordinates(context, glyph, position, static_cast<std::uint16_t>(height_word),
        first_position, image, rectangle, white);
    const CWord x_index[4] = {0, 2, 2, 0}, y_index[4] = {1, 1, 3, 3};
    const CWord u_offset[4] = {4, 12, 12, 4}, v_offset[4] = {0, 0, 8, 8};
    for (CWord i = 0; i < 4; ++i) {
        const CWord vertex = first_vertex + i;
        void* const output = i ? c_at(base(vertex), c_read<CWord>(writer, 0x10)) : first_position;
        if (!i) {
            c_fld_store(static_cast<float*>(output), &rectangle[x_index[i]]);
            c_fld_store(static_cast<float*>(c_at(output, 4)), &rectangle[y_index[i]]);
        } else {
            c_write<float>(output, 0, rectangle[x_index[i]]);
            c_write<float>(output, 4, rectangle[y_index[i]]);
            c_write<CWord>(output, 8, 0);
        }
        void* const uv = c_at(base(vertex), c_read<CWord>(writer, 0x28));
        const CWord u = c_read<CWord>(glyph, u_offset[i]);
        const CWord v = c_read<CWord>(glyph, v_offset[i]);
        c_write<CWord>(uv, 0, u); c_write<CWord>(uv, 4, v);
        const auto color = c_read<std::int32_t>(writer, 0x34);
        void* const color_base = base(vertex);
        if (color < 0) {
            for (CWord c = 0; c < 4; ++c)
                c_write<float>(color_base, c_read<CWord>(writer, 0x38 + 4 * c), white);
        } else c_write<CWord>(color_base, static_cast<CWord>(color), 0xffffffffu);
    }
    const CWord index = quad_index * 4;
    indices[1] = static_cast<std::uint16_t>(index + 1);
    indices[0] = static_cast<std::uint16_t>(index);
    indices[3] = static_cast<std::uint16_t>(index);
    indices[2] = static_cast<std::uint16_t>(index + 2);
    indices[4] = static_cast<std::uint16_t>(index + 2);
    indices[5] = static_cast<std::uint16_t>(index + 3);
    if (!c_read<void*>(context, 0x1b0) || !c_read<void*>(context, 0x1ac) ||
        c_read<CWord>(context, 0xdc) || !c_read<CWord>(context, 0x1a4)) return;
    const volatile std::uint16_t* codes = c_read<const std::uint16_t*>(context, 0x1a8);
    if (!codes) codes = image.empty_utf16_f8be50;
    if (!*codes) return;
    const auto code = static_cast<std::uint16_t>(code_unit_word);
    while (*codes != code) { ++codes; if (!*codes) return; }
    for (CWord i = 0; i < 4; ++i) {
        void* const uv = c_at(base(first_vertex + i), c_read<CWord>(writer, 0x28));
        c_write<CWord>(uv, 0, 0); c_write<CWord>(uv, 4, 0);
    }
    void* const allocation = b.allocate_00ab78a0(image.text_pool_f8bdf0);
    void* child = allocation ? b.construct_00ab9650(allocation) : nullptr;
    b.push_child_005dad40(c_at(context, 0x194), &child);
    void* const reference_node = c_read<void*>(c_read<void*>(context, 0x1b0), 0x4c);
    void* const node = c_virtual<void*>(reference_node, 0x10, CWord{0x26}, CWord{0});
    b.set_node_00aa6720(child, node);
    b.sections_00ab8530(child);
    c_virtual<void>(child, 0x34, CWord{1});
    b.listeners_00aa6bc0(child, c_read<void*>(context, 0x1ac), 0);
    c_write<std::uint8_t>(child, 0x78, 1);
    b.font_00ab8c30(child, c_at(c_read<void*>(context, 0x1b0), 0x1c8));
    float shadow;
    const float shadow_source = image.shadow_d5c5c0;
    c_fld_store(&shadow, &shadow_source);
    const float shadow_color[4] = {0, 0, 0, image.white_d7a24c};
    b.shadow_00ab6c30(child, 1, 0, shadow, shadow_color);
    TailNativeString shader;
    void* const shader_argument = b.strings.assign_0041e870(&shader, "GuiFontBilinear.mshd");
    b.shader_00ab8e70(child, shader_argument);
    c_string_release(shader, b.strings);
    TailNativeString wide;
    void* const text = b.code_unit_00ab81c0(&wide, code_unit_word);
    b.content_00aba8d0(child, text);
    c_virtual<void>(child, 0x50, c_at(child, 0x50));
    if (wide.data) b.strings.release_00419cc0_00bd1510(wide.data, wide.length * 2 + 2, 1);
    b.attach_00aaa5a0(context, child, 0);
    const float center = image.pivot_ce3800;
    const float pivot[2] = {center, center};
    c_virtual<void>(child, 0x30, pivot);
    float reference_position[3];
    b.position_00aa6750(c_read<void*>(context, 0x1b0), reference_position);
    float target[3];
    if (c_read<std::uint8_t>(context, 0x1b4)) {
        float x_offset, y_offset;
        const float x = c_read<float>(context, 0x1b8), y = c_read<float>(context, 0x1bc);
        c_fld_store(&x_offset, &x); c_fld_store(&y_offset, &y);
        target[0] = c_glyph_sum(x_offset, reference_position[0]);
        target[1] = c_glyph_sum(y_offset, reference_position[1]);
    } else {
        float x = reference_position[0];
        if (c_read<CWord>(c_read<void*>(context, 0x1b0), 0xec) != 1)
            x = c_glyph_input_x(position, x, image);
        target[0] = c_glyph_difference(x, image.x_subtract_d7a358);
        target[1] = c_glyph_difference(reference_position[1], image.y_subtract_d5c7a8);
    }
    target[2] = c_glyph_difference(reference_position[2], image.z_subtract_d7a220);
    b.set_position_00aa8240(child, target);
}
} // namespace bsp::oct18_tail_c
