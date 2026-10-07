#include "bsp/native_land_state_registry.hpp"
#include "bsp/singleton_lifetime.hpp"

namespace bsp {
namespace {
using Word = std::uint32_t;
using Pair = NativeBotStatePair;

Word address(const void* pointer) noexcept {
    return static_cast<Word>(reinterpret_cast<std::uintptr_t>(pointer));
}

// Native SUB/SAR3 and LEA arithmetic; no host vector or saturation policy.
Word distance(const Pair* last, const Pair* first) noexcept {
    return static_cast<Word>(static_cast<std::int32_t>(
        address(last) - address(first)) >> 3);
}
Pair* offset(const Pair* pointer, Word count) noexcept {
    return reinterpret_cast<Pair*>(static_cast<std::uintptr_t>(
        address(pointer) + count * 8u));
}
void* member(void* identity, Word bytes) noexcept {
    return reinterpret_cast<void*>(static_cast<std::uintptr_t>(
        address(identity) + bytes));
}
} // namespace

// 410400: ECX count, EAX allocation, RET; overflow exception construction is
// outside the admitted <=1FFFFFFF normal domain. Count0 still calls new(0).
NativeBotStatePair* allocate_native_bot_state_pairs_00410400(Word count) {
    const Word bytes = count * 8u;
    return static_cast<Pair*>(singleton_lifetime_allocate({
        SingletonAllocationKind::object, bytes, bytes}));
}

// 410C00: ECX first/EDX last, stacked dest/metadata/unused words, RET10;
// EAX dest end. A null destination skips that element's two writes only.
Pair* copy_native_bot_state_pairs_00410c00(const Pair* first,
    const Pair* last, Pair* destination) {
    while (first != last) {
        if (destination != nullptr) {
            destination->name_00 = first->name_00;
            destination->state_04 = first->state_04;
        }
        first = offset(first, 1);
        destination = offset(destination, 1);
    }
    return destination;
}

// 410FA0: three stacked ranges/pair, cdecl, no meaningful return.
void assign_native_bot_state_pairs_00410fa0(Pair* first, Pair* last,
    const Pair& pair) {
    while (first != last) {
        first->name_00 = pair.name_00;
        first->state_04 = pair.state_04;
        first = offset(first, 1);
    }
}

// 410FE0: ECX dest/EDX count, stacked pair/metadata/unused words, RET10.
// The zero-count branch does not even read the stacked pair pointer.
void fill_native_bot_state_pairs_00410fe0(Pair* destination,
    Word count, const Pair& pair) {
    while (count != 0) {
        if (destination != nullptr) {
            destination->name_00 = pair.name_00;
            destination->state_04 = pair.state_04;
        }
        --count;
        destination = offset(destination, 1);
    }
}

// 4112E0: cdecl first/last/dest end, EAX first destination. Native order
// reads/stores low word before reading/storing high word, walking backward.
Pair* shift_native_bot_state_pairs_backward_004112e0(Pair* first,
    Pair* last, Pair* destination_end) {
    const Word initial_last = address(last);
    const Word initial_destination_end = address(destination_end);
    while (last != first) {
        last = offset(last, 0xffffffffu);
        destination_end = offset(destination_end, 0xffffffffu);
        destination_end->name_00 = last->name_00;
        destination_end->state_04 = last->state_04;
    }
    return reinterpret_cast<Pair*>(static_cast<std::uintptr_t>(
        initial_destination_end - distance(
            reinterpret_cast<Pair*>(static_cast<std::uintptr_t>(initial_last)),
            first) * 8u));
}

// Primary-reviewed 4114D0: ECX metadata (unread), stack dest/count/pair,
// EAX dest+8*count, RETC. The local low-byte-zero flag is unread by 410FE0.
Pair* fill_native_bot_state_pairs_004114d0(NativeBotStateVectorStorage&,
    Pair* destination, Word count, const Pair& pair) {
    fill_native_bot_state_pairs_00410fe0(destination, count, pair);
    return offset(destination, count);
}

// Primary-reviewed 411670: ECX metadata (unread), stack first/last/dest,
// RETC. 410C00 supplies the returned EAX destination end.
Pair* copy_native_bot_state_pairs_00411670(NativeBotStateVectorStorage&,
    const Pair* first, const Pair* last, Pair* destination) {
    return copy_native_bot_state_pairs_00410c00(first, last, destination);
}

// 411960: ECX vector, stack iterator-owner/position/count/pair, RET10.
// Iterator owner is unread here. Both in-place branches and allocation branch
// are preserved. Native max-length diagnostics and private EH remain external.
void insert_native_bot_state_pairs_00411960(NativeBotStateVectorStorage& v,
    Pair* position, Word count, const Pair& pair) {
    void* const captured_state = pair.state_04; //1197E
    const char* const captured_name = pair.name_00; //11985
    Pair* const initial_begin = v.begin_04; //11987
    Pair captured;
    captured.name_00 = captured_name;
    captured.state_04 = captured_state;
    Word capacity = initial_begin == nullptr ? 0u
        : distance(v.capacity_end_0c, initial_begin);
    if (count == 0) return;
    // <=1FFFFFFF-size/count is a caller precondition;119CA throw not adopted.
    const Word length_size = initial_begin == nullptr ? 0u
        : distance(v.end_08, initial_begin); //119B7 length-check observation
    (void)length_size;
    Word size = initial_begin == nullptr ? 0u
        : distance(v.end_08, initial_begin);
    if (capacity < size + count) {
        const Word half = capacity >> 1;
        capacity = 0x1fffffffu - half < capacity ? 0u : capacity + half;
        size = initial_begin == nullptr ? 0u
            : distance(v.end_08, initial_begin);
        if (capacity < size + count) {
            size = initial_begin == nullptr ? 0u
                : distance(v.end_08, initial_begin);
            capacity = size + count;
        }
        Pair* const replacement = allocate_native_bot_state_pairs_00410400(capacity);
        Pair* cursor = copy_native_bot_state_pairs_00410c00(
            v.begin_04, position, replacement);
        cursor = fill_native_bot_state_pairs_004114d0(v, cursor, count, captured);
        copy_native_bot_state_pairs_00410c00(position, v.end_08, cursor);
        Pair* const old_begin = v.begin_04;
        const Word new_size = count + (old_begin == nullptr ? 0u
            : distance(v.end_08, old_begin));
        if (old_begin != nullptr) singleton_lifetime_free(old_begin);
        v.capacity_end_0c = offset(replacement, capacity); //11AAB
        v.end_08 = offset(replacement, new_size);          //11AAE
        v.begin_04 = replacement;                         //11AB1
        return;
    }
    Pair* const old_end = v.end_08; //11ADC
    if (distance(old_end, position) < count) {
        copy_native_bot_state_pairs_00411670(v, position, old_end,
            offset(position, count));
        Pair* const current_end = v.end_08; //11B03
        fill_native_bot_state_pairs_004114d0(v, current_end,
            count - distance(current_end, position), captured);
        v.end_08 = offset(v.end_08, count); //11B29 read/modify/write
        Pair* const published_end = v.end_08; //11B2C fresh
        assign_native_bot_state_pairs_00410fa0(position,
            offset(published_end, 0u - count), captured);
        return;
    }
    Pair* const tail_begin = offset(old_end, 0u - count);
    Pair* const extended_end = copy_native_bot_state_pairs_00411670(
        v, tail_begin, old_end, old_end);
    v.end_08 = extended_end; //11B73 BEFORE backward shift
    shift_native_bot_state_pairs_backward_004112e0(position, tail_begin, old_end);
    assign_native_bot_state_pairs_00410fa0(position, offset(position, count), captured);
}

// 411BB0: ECX vector, stack requested count, RET4. Existing capacity suffices
// ->no allocation/header writes. Growth captures end after real allocation,
// reloads begin for copy/free and publishes cap/end/begin after returning free.
void reserve_native_bot_state_pairs_00411bb0(NativeBotStateVectorStorage& v,
    Word requested_capacity) {
    Pair* const initial_begin = v.begin_04;
    const Word old_capacity = initial_begin == nullptr ? 0u
        : distance(v.capacity_end_0c, initial_begin);
    if (old_capacity >= requested_capacity) return;
    Pair* const replacement = allocate_native_bot_state_pairs_00410400(requested_capacity);
    Pair* const captured_end = v.end_08; //11C01
    Pair* const checked_begin = v.begin_04; //11C04 native diagnostic observation
    (void)checked_begin;
    Pair* const first = v.begin_04;     //11C1A
    Pair* const checked_end = v.end_08; //11C1D native diagnostic observation
    (void)checked_end;
    copy_native_bot_state_pairs_00410c00(first, captured_end, replacement);
    Pair* const old_begin = v.begin_04; //11C42
    const Word size = old_begin == nullptr ? 0u : distance(v.end_08, old_begin);
    if (old_begin != nullptr) singleton_lifetime_free(old_begin);
    v.capacity_end_0c = offset(replacement, requested_capacity); //11C6B
    v.end_08 = offset(replacement, size);                       //11C6E
    v.begin_04 = replacement;                                  //11C71
}

// 411CA0: ECX vector, stack output/iterator-owner/position/pair, RET10.
// Native valid-iterator checks are admitted preconditions, not substitute
// diagnostics. Capture insertion index, then use fresh begin after insertion.
NativeBotStateIterator* insert_native_bot_state_pair_00411ca0(NativeBotStateVectorStorage& v,
    NativeBotStateIterator& output, const NativeBotStateIterator& position,
    const Pair& pair) {
    NativeBotStateVectorStorage* const original_owner = position.owner_00;
    (void)original_owner;
    Pair* const initial_begin = v.begin_04;
    const Word index = initial_begin == nullptr
        || distance(v.end_08, initial_begin) == 0 ? 0u
        : distance(position.position_04, initial_begin);
    insert_native_bot_state_pairs_00411960(v, position.position_04, 1, pair);
    Pair* const fresh_begin = v.begin_04;
    Pair* const checked_end = v.end_08; //11CF8 native valid-iterator checks
    (void)checked_end;
    Pair* const checked_position_end = v.end_08; //11D09
    (void)checked_position_end;
    Pair* const checked_position_begin = v.begin_04; //11D0E
    (void)checked_position_begin;
    output.position_04 = offset(fresh_begin, index); //11D1C before11D20
    output.owner_00 = &v;
    return &output;
}

// 411D90: ECX vector/stack pair, RET4. Spare branch captures current end and
// publishes that captured end+8 after fill; full branch uses real insertion.
void append_native_bot_state_pair_00411d90(NativeBotStateVectorStorage& v,
    const Pair& pair) {
    Pair* const begin = v.begin_04;
    const Word size = begin == nullptr ? 0u : distance(v.end_08, begin);
    if (begin != nullptr && size < distance(v.capacity_end_0c, begin)) {
        Pair* const captured_end = v.end_08;
        fill_native_bot_state_pairs_00410fe0(captured_end, 1, pair);
        v.end_08 = offset(captured_end, 1);
        return;
    }
    NativeBotStateIterator position;
    position.owner_00 = &v;
    position.position_04 = v.end_08;
    NativeBotStateIterator output;
    insert_native_bot_state_pair_00411ca0(v, output, position, pair);
}

// 411E20: ECX registry/EAX same, RET. No proxy initialization, no old-buffer
// destruction. Numeric CE37DC is retained as an explicitly uncallable word.
NativeBotStateRegistryStorage* construct_native_bot_state_registry_00411e20(
    NativeBotStateRegistryStorage& registry) {
    registry.profile_word_00 = 0x00ce37dcu;
    registry.vector_04.begin_04 = nullptr;
    registry.vector_04.end_08 = nullptr;
    registry.vector_04.capacity_end_0c = nullptr;
    reserve_native_bot_state_pairs_00411bb0(registry.vector_04, 0x10);
    return &registry;
}

// 411E70: ECX registry, stack name/state, EAX same registry, RET8.
NativeBotStateRegistryStorage* add_native_bot_state_00411e70(
    NativeBotStateRegistryStorage& registry, const char* name, void* state) {
    Pair pair;
    pair.state_04 = state; //11E82 before11E8A first-word store
    pair.name_00 = name;
    append_native_bot_state_pair_00411d90(registry.vector_04, pair);
    return &registry;
}

// 9AF9A0: ECX actual approach, RET. Captures registry+B8 once; all states
// denote members of this SAME approach. In particular line1A0/standby1C0.
void register_native_land_states_009af9a0(void* approach,
    const NativeLandStateNames& names) {
    auto& registry = *static_cast<NativeBotStateRegistryStorage*>(member(approach, 0xb8));
    add_native_bot_state_00411e70(registry, names.moveto_00d1fe1c, member(approach, 0xcc));
    add_native_bot_state_00411e70(registry, names.follow_00d1fe0c, member(approach, 0x108));
    add_native_bot_state_00411e70(registry, names.line_00d1fe00, member(approach, 0x1a0));
    add_native_bot_state_00411e70(registry, names.standby_00d1fdf0, member(approach, 0x1c0));
    add_native_bot_state_00411e70(registry, names.begin_00d1fde4, member(approach, 0x1e0));
    add_native_bot_state_00411e70(registry, names.final_00d1fdd8, member(approach, 0x200));
    add_native_bot_state_00411e70(registry, names.park_00d1fdcc, member(approach, 0x228));
    add_native_bot_state_00411e70(registry, names.abort_00d1fdc0, member(approach, 0x254));
}

} // namespace bsp
