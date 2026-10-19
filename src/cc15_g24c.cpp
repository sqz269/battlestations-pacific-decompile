// cc15-stlA game-code rows (packet cc15-g24c). Evidence and ABI:
// include/bsp/cc15_g24c.hpp and docs/CC15_G24C.md.
#include "bsp/cc15_g24c.hpp"

#include <cstring>

namespace bsp::cc15_g24c {
namespace {

template <typename T>
T load(const std::uint8_t* object, std::uint32_t offset) {
    T value;
    std::memcpy(&value, object + offset, sizeof value);
    return value;
}

template <typename T>
void store(std::uint8_t* object, std::uint32_t offset, T value) {
    std::memcpy(object + offset, &value, sizeof value);
}

std::uint8_t* load_ptr(const std::uint8_t* object, std::uint32_t offset) {
    return load<std::uint8_t*>(object, offset);
}

std::uint8_t* bytes(void* p) { return static_cast<std::uint8_t*>(p); }
const std::uint8_t* bytes(const void* p) { return static_cast<const std::uint8_t*>(p); }

// The node insert the three narrative writers inline: a stack entry {vtable
// 00CFE960, five zero dwords}, 007342B0 builds the node from it, 007341B0
// bumps the size, the two links are stored, and the temporary is destroyed
// (EH state 0 covers the BuyNode/Incsize window; that unwind is not modelled).
// Returns nothing: every caller re-reads the node from the list afterwards.
void insert_default_entry(std::uint8_t* list, std::uint8_t* where, NarrativeListHost& host) {
    std::uint8_t temp[kNarrativeEntrySize] = {};
    store<std::uint32_t>(temp, 0x00, kNarrativeEntryVtable);
    std::uint8_t* const node =
        host.narrative_list_buy_node_007342b0(list, where, load_ptr(where, 0x04), temp);
    host.narrative_list_incsize_007341b0(list, 1);
    store<std::uint8_t*>(where, 0x04, node);              // where->prev = node
    store<std::uint8_t*>(load_ptr(node, 0x04), 0x00, node);  // node->prev->next = node
    host.narrative_entry_destruct_00733dc0(temp);
}

// One NativeString assignment as inlined: skip self-assignment, resize the
// destination to the source length with flag 1, then (when the source length,
// re-read after the resize, is non-zero) copy the destination's post-resize
// length of bytes.
void assign_string(std::uint8_t* dst, const void* src, NarrativeListHost& host) {
    if (dst == src) {
        return;
    }
    host.native_string_resize_0041dd40(dst, load<std::uint32_t>(bytes(src), 0x00), 1);
    if (load<std::uint32_t>(bytes(src), 0x00) != 0) {
        host.crt_memcpy_00bf7680(load_ptr(dst, 0x04), load_ptr(bytes(src), 0x04),
                                 load<std::uint32_t>(dst, 0x00));
    }
}

void assign_entry(std::uint8_t* node, const void* key, const void* text, std::uint32_t value,
                  NarrativeListHost& host) {
    assign_string(node + 0x0C, key, host);
    assign_string(node + 0x14, text, host);
    store<std::uint32_t>(node, 0x1C, value);
}

// The push_back tail shared by 00734870 (from 007348AA) and 00734E50 (from
// 00734E94): insert before the head, then back() through two checked-iterator
// tests, the second re-reading the head word.
void append_entry(std::uint8_t* narrative, const void* key, const void* text, std::uint32_t value,
                  NarrativeListHost& host) {
    std::uint8_t* const list = narrative + 0x04;
    insert_default_entry(list, load_ptr(list, 0x04), host);
    std::uint8_t* const head = load_ptr(list, 0x04);
    std::uint8_t* const back = load_ptr(head, 0x04);
    if (back == head) {
        host.crt_invalid_parameter_noinfo_00bf6713();
    }
    if (back == load_ptr(list, 0x04)) {
        host.crt_invalid_parameter_noinfo_00bf6713();
    }
    assign_entry(back, key, text, value, host);
}

} // namespace

// 00734870-007349AF.
void narrative_list_append_00734870(void* narrative, const void* key, const void* text,
                                    std::uint32_t value, NarrativeListHost& host) {
    append_entry(bytes(narrative), key, text, value, host);
}

// 007349C0-00734B09.
void narrative_list_push_front_unless_same_007349c0(void* narrative, const void* key,
                                                    const void* text, std::uint32_t value,
                                                    NarrativeListHost& host) {
    std::uint8_t* const self = bytes(narrative);
    if (load<std::uint32_t>(self, 0x0C) != 0) {  // 007349DC
        std::uint8_t* head = load_ptr(self, 0x08);
        std::uint8_t* front = load_ptr(head, 0x00);
        if (front == head) {
            host.crt_invalid_parameter_noinfo_00bf6713();
        }
        if (host.native_string_equals_insensitive_00435c40(front + 0x0C, key)) {  // 007349FC
            head = load_ptr(self, 0x08);
            front = load_ptr(head, 0x00);
            if (front == head) {
                host.crt_invalid_parameter_noinfo_00bf6713();
            }
            if (host.native_string_equals_insensitive_00435c40(front + 0x14, text)) {  // 00734A1B
                return;
            }
        }
    }
    std::uint8_t* const list = self + 0x04;
    insert_default_entry(list, load_ptr(load_ptr(list, 0x04), 0x00), host);  // 00734A45
    std::uint8_t* const head = load_ptr(list, 0x04);
    std::uint8_t* const front = load_ptr(head, 0x00);
    if (front == head) {
        host.crt_invalid_parameter_noinfo_00bf6713();
    }
    assign_entry(front, key, text, value, host);
}

// 00734E50-00734F96 (the tail matches 00734870's from 00734ED0).
void narrative_list_replace_all_00734e50(void* narrative, const void* key, const void* text,
                                         std::uint32_t value, NarrativeListHost& host) {
    host.narrative_list_clear_00734ce0(bytes(narrative) + 0x04);  // 00734E71
    append_entry(bytes(narrative), key, text, value, host);
}

// 008DC500-008DC5FC. EDI (the iterator's container) is always the list
// itself, so the 008DC511 test fails only for a list at address 0 (an
// objective at -20h); the walk re-reads the head word wherever the image does.
void objective_erase_position_entries_008dc500(void* objective, const float* pos3,
                                               ObjectivePositionHost& host) {
    std::uint8_t* const list = bytes(objective) + 0x20;
    const bool null_container = reinterpret_cast<std::uintptr_t>(list) == 0;
    std::uint8_t* node = load_ptr(load_ptr(list, 0x04), 0x00);  // 008DC500/008DC50C
    for (;;) {
        std::uint8_t* const end = load_ptr(list, 0x04);  // 008DC513
        if (null_container) {
            host.crt_invalid_parameter_noinfo_00bf6713();
        }
        if (node == end) {
            return;  // 008DC5F5
        }
        if (null_container) {
            host.crt_invalid_parameter_noinfo_00bf6713();
        }
        if (node == load_ptr(list, 0x04)) {
            host.crt_invalid_parameter_noinfo_00bf6713();
        }
        bool match = load<std::int32_t>(load_ptr(node, 0x08), 0x00) == 0;  // 008DC53F
        if (match) {
            if (node == load_ptr(list, 0x04)) {
                host.crt_invalid_parameter_noinfo_00bf6713();
            }
            const std::uint8_t* const record = load_ptr(node, 0x08);
            match = load<float>(record, 0x04) == pos3[0] && load<float>(record, 0x08) == pos3[1] &&
                    load<float>(record, 0x0C) == pos3[2];
        }
        if (!match) {
            // 008DC5E4
            if (node == load_ptr(list, 0x04)) {
                host.crt_invalid_parameter_noinfo_00bf6713();
            }
            node = load_ptr(node, 0x00);
            continue;
        }
        if (node == load_ptr(list, 0x04)) {
            host.crt_invalid_parameter_noinfo_00bf6713();
        }
        if (std::uint8_t* const record = load_ptr(node, 0x08)) {  // 008DC597
            host.crt_free_00bf65ac(record);
            store<std::uint8_t*>(node, 0x08, nullptr);
        }
        if (node == load_ptr(list, 0x04)) {  // 008DC5AE
            host.crt_invalid_parameter_noinfo_00bf6713();
        }
        const bool erasable = node != load_ptr(list, 0x04);  // 008DC5BC
        std::uint8_t* const next = load_ptr(node, 0x00);
        if (erasable) {
            store<std::uint8_t*>(load_ptr(node, 0x04), 0x00, next);
            store<std::uint8_t*>(load_ptr(node, 0x00), 0x04, load_ptr(node, 0x04));
            host.crt_free_00bf65ac(node);
            store<std::uint32_t>(list, 0x08, load<std::uint32_t>(list, 0x08) - 1u);
        }
        node = next;  // 008DC5DD
    }
}

// 007D05E0-007D0699. The local vector's words are read back from the stack on
// every pass (+4h first, +8h last); nothing between passes writes them.
void node_append_children_by_name_007d05e0(cc13_seg47::Raw node, const char* name, void* vector,
                                           NodeAppendChildrenHost& host) {
    std::uint8_t local[0x10] = {};  // [ESP+18h]; 0071BA20 constructs it
    std::uint8_t out_iterator[0x08] = {};  // [ESP+10h], the 00505D50 result
    host.node_collect_children_by_name_prefix_0071ba20(node.p, local, name);  // 007D0602
    std::uint8_t* const v = bytes(vector);
    for (std::uint32_t index = 0;; ++index) {
        std::uint8_t* const first = load_ptr(local, 0x04);  // 007D0611
        if (first == nullptr) {
            return;  // 007D0617: nothing to free
        }
        const std::uint8_t* const last = load_ptr(local, 0x08);
        const auto count = static_cast<std::uint32_t>(static_cast<std::int32_t>(last - first) >> 2);
        if (index >= count) {
            host.crt_free_00bf65ac(first);  // 007D067F
            return;
        }
        std::uint8_t* const element = first + index * 4u;  // EBX
        std::uint8_t* const begin = load_ptr(v, 0x04);
        if (begin != nullptr) {
            const auto size = static_cast<std::uint32_t>(
                static_cast<std::int32_t>(load_ptr(v, 0x08) - begin) >> 2);
            const auto capacity = static_cast<std::uint32_t>(
                static_cast<std::int32_t>(load_ptr(v, 0x0C) - begin) >> 2);
            if (size < capacity) {
                // 007D064C: construct in place at last.
                std::uint8_t* const slot = load_ptr(v, 0x08);
                store<std::uint32_t>(slot, 0x00, load<std::uint32_t>(element, 0x00));
                store<std::uint8_t*>(v, 0x08, slot + 4);
                continue;
            }
        }
        // 007D065E: end() iterator check (begin > last, unsigned), then insert.
        std::uint8_t* const where = load_ptr(v, 0x08);
        if (reinterpret_cast<std::uintptr_t>(begin) > reinterpret_cast<std::uintptr_t>(where)) {
            host.crt_invalid_parameter_noinfo_00bf6713();
        }
        host.dword_vector_insert_one_00505d50(vector, out_iterator, vector, where, element);
    }
}

} // namespace bsp::cc15_g24c
