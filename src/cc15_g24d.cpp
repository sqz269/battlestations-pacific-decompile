// cc15_g24d: the spawn manager request-list bodies and the quote-aware string split
// (packet cc15-g24d, docs/CC15_G24D.md). Reconstructed and build-tested only; see the header.

#include "bsp/cc15_g24d.hpp"

namespace bsp::cc15_g24d {
namespace {

// The walk shared by 00943E40, 00943EC0 and 00946970 (each inlines the same checked
// std::list iteration): every pass tests the iterator's container (the list itself, so the
// CMP ECX,ECX half always passes), re-reads the head into the end iterator, stops at the head,
// tests "not end" before the dereference, runs `body`, and tests "not end" again before the
// increment. `body` returns true to stop on the current node. Returns the node it stopped on,
// or null at the end.
template <class Body>
std::uint8_t* walk_requests_checked(Raw list, Cc15G24dHost& host, Body body) {
    std::uint8_t* node = static_cast<std::uint8_t*>(Raw{static_cast<std::uint8_t*>(list.ptr(kListHead))}.ptr(kNodeNext));
    for (;;) {
        if (list.p == nullptr) host.checked_iterator_report_00bf6713();
        std::uint8_t* end = static_cast<std::uint8_t*>(list.ptr(kListHead));
        if (node == end) return nullptr;
        if (list.p == nullptr) host.checked_iterator_report_00bf6713();
        if (node == list.ptr(kListHead)) host.checked_iterator_report_00bf6713();
        if (body(Raw{node})) return node;
        if (node == list.ptr(kListHead)) host.checked_iterator_report_00bf6713();
        node = static_cast<std::uint8_t*>(Raw{node}.ptr(kNodeNext));
    }
}

bool request_has_id(Raw node, std::uint32_t id) {
    return Raw{static_cast<std::uint8_t*>(node.ptr(kNodeValue))}.at<std::uint32_t>(kRequestId) == id;
}

const char* string_data(const std::uint8_t* text, std::uint32_t capacity) {
    const std::uint8_t* buffer = text + kStringBuffer;
    if (capacity >= kStringInlineLimit) buffer = *reinterpret_cast<std::uint8_t* const*>(buffer);
    return reinterpret_cast<const char*>(buffer);
}

std::uint32_t string_field(const std::uint8_t* text, std::size_t off) {
    return *reinterpret_cast<const std::uint32_t*>(text + off);
}

}  // namespace

// 00943E40. EDI = ECX = the list (manager +0), EBP = id. The walk is the shared one; on a match
// the inlined erase tests "not end" (00943E8D), and on that report re-tests and returns when the
// node is the head (00943E97), then unlinks (prev->next = next; next->prev = prev, both re-read
// from the node), frees the node and decrements +8.
void spawn_manager_remove_request_00943e40(Raw manager, std::uint32_t id, Cc15G24dHost& host) {
    std::uint8_t* node = walk_requests_checked(manager, host, [&](Raw n) { return request_has_id(n, id); });
    if (node == nullptr) return;
    if (node == manager.ptr(kListHead)) {
        host.checked_iterator_report_00bf6713();
        if (node == manager.ptr(kListHead)) return;
    }
    Raw n{node};
    Raw{static_cast<std::uint8_t*>(n.ptr(kNodePrev))}.ptr(kNodeNext) = n.ptr(kNodeNext);
    Raw{static_cast<std::uint8_t*>(n.ptr(kNodeNext))}.ptr(kNodePrev) = n.ptr(kNodePrev);
    host.free_00bf65ac(node);
    manager.at<std::uint32_t>(kListSize) -= 1;
}

// 00943EC0. EBP = [entity +28Ch]; 0 returns at once (00943ED3). On a match the image calls
// 009439B0(ECX = list; out = its 8-byte local at ESP+10h, iterator {list, node}) and discards
// the returned iterator.
void spawn_manager_remove_request_for_entity_00943ec0(Raw manager, Raw entity, Cc15G24dHost& host) {
    const std::uint32_t id = entity.at<std::uint32_t>(kEntityRequestId);
    if (id == 0) return;
    std::uint8_t* node = walk_requests_checked(manager, host, [&](Raw n) { return request_has_id(n, id); });
    if (node == nullptr) return;
    void* out_iterator[2] = {};
    host.request_list_erase_009439b0(manager, out_iterator, manager.p, node);
}

// 00946970. The sum starts as XORPS 0 spilled with MOVSS; each node FADDs the spilled float to
// the ST0 of 00946870 and FSTPs it back as a float (the 24-bit main-thread precision and the
// float spill both round to float), then FLDs it for the return.
float spawn_manager_player_usage_00946970(Raw manager, std::uint32_t player, Cc15G24dHost& host) {
    float sum = 0.0f;
    walk_requests_checked(manager, host, [&](Raw n) {
        sum = host.request_player_usage_00946870(n.ptr(kNodeValue), player) + sum;
        return false;
    });
    return sum;
}

// 0094EDC0. ESI = index from 0, EBP = token start (-1 = none), [ESP+13h] = the quote flag,
// EDI = text. The operator[] bound test (index <= size) runs once before the loop (0094EDE3);
// the loop re-enters at 0094EDF9, which re-reads the capacity, and the quote toggle reads the
// byte at index even when index == size (the terminator). Without a token, an index below size
// whose byte is not in the set, or any byte while quoted, starts one; the strchr byte uses the
// capacity read at the loop head. With a token, index == size closes it at once; otherwise the
// image re-reads size, tests index > size (never true here), re-reads the capacity and closes the
// token only on an unquoted separator. Closing builds the substring [start, index) in a stack
// temporary through 004CDBE0, pushes it through 00849170, clears the start and frees the
// temporary's heap buffer when its capacity is >= 10h. The loop runs while index <= size,
// re-read after every step.
void string_vector_split_0094edc0(std::uint8_t* vector, std::uint8_t* text, const char* separators,
                                  Cc15G24dHost& host) {
    std::uint32_t index = 0;
    std::uint32_t start = 0xffffffffu;
    bool quoted = false;
    if (index > string_field(text, kStringSize)) host.checked_iterator_report_00bf6713();
    do {
        const std::uint32_t capacity = string_field(text, kStringCapacity);
        if (string_data(text, capacity)[index] == '"') quoted = !quoted;
        if (start == 0xffffffffu) {
            if (index < string_field(text, kStringSize)) {
                const std::int32_t c = static_cast<signed char>(string_data(text, capacity)[index]);
                if (host.strchr_00bf86f0(separators, c) == nullptr || quoted) start = index;
            }
            continue;
        }
        const std::uint32_t size = string_field(text, kStringSize);
        if (index != size) {
            if (index > size) host.checked_iterator_report_00bf6713();
            const std::int32_t c =
                static_cast<signed char>(string_data(text, string_field(text, kStringCapacity))[index]);
            if (host.strchr_00bf86f0(separators, c) == nullptr) continue;
            if (quoted) continue;
        }
        alignas(4) std::uint8_t temporary[kStringBytes];
        void* token = host.string_construct_substring_004cdbe0(text, temporary, start, index - start);
        host.string_vector_push_back_00849170(vector, token);
        start = 0xffffffffu;
        if (*reinterpret_cast<std::uint32_t*>(temporary + kStringCapacity) >= kStringInlineLimit) {
            host.free_00bf65ac(*reinterpret_cast<void**>(temporary + kStringBuffer));
        }
    } while (++index <= string_field(text, kStringSize));
}

}  // namespace bsp::cc15_g24d
