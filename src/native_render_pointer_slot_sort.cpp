#include "bsp/native_render_pointer_slot_sort.hpp"

#include <cstring>

namespace bsp {
namespace {
using Word = std::uint32_t;
using Signed = std::int32_t;
using Compare = NativeRenderPointerSlotComparator;
static_assert(sizeof(void*) == 4, "Native pointer cells are DWORDs.");

// These are actual native addresses, not indices in a copied C++ container.
// Unsigned arithmetic retains x86 wrapping; signed views retain SAR/CMP rules.
Word address(const void* value) noexcept {
    return static_cast<Word>(reinterpret_cast<std::uintptr_t>(value));
}
Signed signed_word(Word value) noexcept {
    Signed result;
    std::memcpy(&result, &value, sizeof(result));
    return result;
}
Signed distance(Word first, Word last) noexcept {
    const Word bytes = last - first;
    return signed_word((bytes >> 2) | ((bytes & 0x80000000u) ? 0xc0000000u : 0u));
}
Word at(Word first, Signed index) noexcept {
    return first + static_cast<Word>(index) * 4u;
}
Signed add(Signed left, Signed right) noexcept {
    return signed_word(static_cast<Word>(left) + static_cast<Word>(right));
}
Word read(Word location) noexcept {
    return *reinterpret_cast<const volatile Word*>(location);
}
void write(Word location, Word value) noexcept {
    *reinterpret_cast<volatile Word*>(location) = value;
}
bool compare_values(Word left, Word right, Compare compare) {
    return static_cast<std::uint8_t>(compare(reinterpret_cast<const void*>(left),
        reinterpret_cast<const void*>(right))) != 0;
}
bool compare_slots(Word left, Word right, Compare compare) {
    // Every slot-pair comparator call in this family loads EDX before ECX.
    const Word right_value = read(right);
    const Word left_value = read(left);
    return compare_values(left_value, right_value, compare);
}
void exchange_read_right_first(Word left, Word right) noexcept {
    const Word right_value = read(right);
    const Word left_value = read(left);
    write(left, right_value);
    write(right, left_value);
}
void exchange_read_left_first(Word left, Word right) noexcept {
    const Word left_value = read(left);
    const Word right_value = read(right);
    write(left, right_value);
    write(right, left_value);
}

// ECX first, EDX middle, stack last plus two unread words; RET0Ch.
void rotate_00b1c210(Word first, Word middle, Word last) noexcept {
    const Signed length = distance(first, last);
    const Signed left = distance(first, middle);
    Signed common = length;
    for (Signed divisor = left; divisor != 0;) {
        const Signed remainder = common % divisor;
        common = divisor;
        divisor = remainder;
    }
    if (common >= length || common <= 0) return;
    Word start = at(first, common);
    do {
        const Word value = read(start);
        Word next = at(start, left);
        Word hole = start;
        if (next == last) next = first;
        while (next != start) {
            write(hole, read(next));
            const Signed remaining = distance(next, last);
            hole = next;
            next = left < remaining ? at(next, left)
                : at(first, add(left, -remaining));
        }
        --common;
        start -= 4u;
        write(hole, value);
    } while (common > 0);
}

// ECX first, EDX last, stack comparator; RET4. Sentinel search reloads the
// entry at the insertion slot on every comparison, then rotates actual cells.
void insertion_00b1d420(Word first, Word last, Compare compare) {
    if (first == last) return;
    for (Word current = first + 4u; current != last; current += 4u) {
        const Word next = current + 4u;
        if (compare_slots(current, first, compare)) {
            if (first != current && current != next)
                rotate_00b1c210(first, current, next);
        } else if (compare_slots(current, next - 8u, compare)) {
            Word destination = next - 8u;
            while (compare_slots(current, destination - 4u, compare))
                destination -= 4u;
            if (destination != current && current != next)
                rotate_00b1c210(destination, current, next);
        }
    }
}

// ECX first, EDX middle, stack final slot/comparator; RET8. The third
// comparison is unconditional, even when the second comparison did not swap.
void median_three_00b1c8c0(Word first, Word middle, Word final, Compare compare) {
    if (compare_slots(middle, first, compare))
        exchange_read_right_first(middle, first);
    if (compare_slots(final, middle, compare))
        exchange_read_right_first(final, middle);
    if (compare_slots(middle, first, compare))
        exchange_read_right_first(middle, first);
}
void choose_pivot_00b1cf80(Word first, Word middle, Word final, Compare compare) {
    const Signed gap = distance(first, final);
    if (gap > 40) {
        const Signed step = add(gap, 1) / 8;
        const Word bytes = static_cast<Word>(step) * 4u;
        median_three_00b1c8c0(first, first + bytes, first + bytes * 2u, compare);
        median_three_00b1c8c0(middle - bytes, middle, middle + bytes, compare);
        median_three_00b1c8c0(final - bytes * 2u, final - bytes, final, compare);
        median_three_00b1c8c0(first + bytes, middle, final - bytes, compare);
    } else {
        median_three_00b1c8c0(first, middle, final, compare);
    }
}

struct EqualBand { Word first; Word last; };

// Native ECX output pair, EDX first, stack last/comparator; RET8/EAX pair.
// Bounds use unsigned addresses; swaps reload after the comparator returns.
EqualBand partition_00b1d280(Word first, Word last, Compare compare) {
    Word equal_first = at(first, distance(first, last) / 2);
    choose_pivot_00b1cf80(first, equal_first, last - 4u, compare);
    Word equal_last = equal_first + 4u;
    while (first < equal_first
        && !compare_slots(equal_first - 4u, equal_first, compare)
        && !compare_slots(equal_first, equal_first - 4u, compare)) equal_first -= 4u;
    while (equal_last < last
        && !compare_slots(equal_last, equal_first, compare)
        && !compare_slots(equal_first, equal_last, compare)) equal_last += 4u;
    Word scan_right = equal_last;
    Word scan_left = equal_first;
    for (;;) {
        for (; scan_right < last; scan_right += 4u) {
            if (compare_slots(equal_first, scan_right, compare)) continue;
            if (compare_slots(scan_right, equal_first, compare)) break;
            exchange_read_right_first(equal_last, scan_right);
            equal_last += 4u;
        }
        for (; first < scan_left; scan_left -= 4u) {
            if (compare_slots(scan_left - 4u, equal_first, compare)) continue;
            if (compare_slots(equal_first, scan_left - 4u, compare)) break;
            equal_first -= 4u;
            exchange_read_right_first(equal_first, scan_left - 4u);
        }
        if (scan_left == first) {
            if (scan_right == last) return {equal_first, equal_last};
            if (equal_last != scan_right)
                exchange_read_right_first(equal_first, equal_last);
            exchange_read_left_first(equal_first, scan_right);
            equal_last += 4u;
            equal_first += 4u;
            scan_right += 4u;
        } else {
            scan_left -= 4u;
            if (scan_right == last) {
                equal_first -= 4u;
                if (scan_left != equal_first)
                    exchange_read_right_first(scan_left, equal_first);
                equal_last -= 4u;
                exchange_read_right_first(equal_first, equal_last);
            } else {
                exchange_read_left_first(scan_right, scan_left);
                scan_right += 4u;
            }
        }
    }
}

// ECX first, EDX signed hole, stack signed top/saved entry/comparator; RET0Ch.
void push_heap_00b1c1b0(Word first, Signed hole, Signed top,
    Word value, Compare compare) {
    Signed parent = add(hole, -1) / 2;
    while (top < hole) {
        if (!compare_values(read(at(first, parent)), value, compare)) break;
        write(at(first, hole), read(at(first, parent)));
        hole = parent;
        parent = add(parent, -1) / 2;
    }
    write(at(first, hole), value);
}
// ECX first, EDX signed hole, stack signed length/saved entry/comparator;
// RET0Ch. Equivalent children select RIGHT; equivalent parents stop ascent.
void adjust_heap_00b1c910(Word first, Signed hole, Signed length,
    Word value, Compare compare) {
    const Signed top = hole;
    Signed child = add(add(hole, hole), 2);
    while (child < length) {
        if (compare_slots(at(first, child), at(first, add(child, -1)), compare))
            child = add(child, -1);
        write(at(first, hole), read(at(first, child)));
        hole = child;
        child = add(add(child, child), 2);
    }
    if (child == length) {
        write(at(first, hole), read(at(first, add(length, -1))));
        hole = add(length, -1);
    }
    push_heap_00b1c1b0(first, hole, top, value, compare);
}
// ECX first, EDX last, stack comparator plus two unread words; RET0Ch.
void make_heap_00b1d020(Word first, Word last, Compare compare) {
    const Signed length = distance(first, last);
    Signed parent = length / 2;
    while (parent > 0) {
        const Word value = read(at(first, add(parent, -1)));
        --parent;
        adjust_heap_00b1c910(first, parent, length, value, compare);
    }
}
// ECX first, EDX last, stack comparator; RET4. Keep the unshifted byte span:
// the original derives each final slot from it before SAR computes length.
void sort_heap_00b1d6a0(Word first, Word last, Compare compare) {
    Word bytes = last - first;
    while (distance(0u, bytes) > 1) {
        const Word final = first + bytes - 4u;
        const Word value = read(final);
        const Word root = read(first);
        write(final, root);
        adjust_heap_00b1c910(first, 0, distance(0u, bytes - 4u), value, compare);
        bytes -= 4u;
    }
}

void sort_range_00b1dce0(Word first, Word last, Signed ideal, Compare compare) {
    Signed count = distance(first, last);
    while (count > 32) {
        if (ideal <= 0) {
            if (signed_word((last - first) & 0xfffffffcu) > 4)
                make_heap_00b1d020(first, last, compare);
            sort_heap_00b1d6a0(first, last, compare);
            return;
        }
        const EqualBand equal = partition_00b1d280(first, last, compare);
        ideal /= 2;
        ideal = add(ideal, ideal / 2);
        const Signed left_bytes = signed_word((equal.first - first) & 0xfffffffcu);
        const Signed right_bytes = signed_word((last - equal.last) & 0xfffffffcu);
        // Smaller side recurses; equal side lengths recurse RIGHT.
        if (left_bytes < right_bytes) {
            sort_range_00b1dce0(first, equal.first, ideal, compare);
            first = equal.last;
        } else {
            sort_range_00b1dce0(equal.last, last, ideal, compare);
            last = equal.first;
        }
        count = distance(first, last);
    }
    if (count > 1) insertion_00b1d420(first, last, compare);
}
} // namespace

std::uint32_t __fastcall native_render_entry_unsigned_key_less_00b51b00(
    const void* actual_left_entry, const void* actual_right_entry) noexcept {
    const Word left = address(actual_left_entry);
    const Word right = address(actual_right_entry);
    const Word left_high = read(left + 0x24u);
    const Word right_high = read(right + 0x24u);
    if (left_high > right_high) return 0u;
    if (left_high < right_high) return 1u;
    const Word left_low = read(left + 0x20u);
    const Word right_low = read(right + 0x20u);
    return left_low < right_low ? 1u : 0u;
}

void sort_native_render_pointer_slots_00b1dce0(void* actual_first_slot,
    void* actual_one_past_last_slot, std::int32_t ideal,
    NativeRenderPointerSlotComparator comparator) {
    sort_range_00b1dce0(address(actual_first_slot), address(actual_one_past_last_slot),
        ideal, comparator);
}
} // namespace bsp
