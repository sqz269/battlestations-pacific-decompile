#include "bsp/instance_sort.hpp"
#include "bsp/instance_upload.hpp"
#include "bsp/instance_geometry.hpp"
#include "bsp/render_sort.hpp"
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <utility>

namespace bsp {
namespace {
using Entries = std::vector<InstanceRenderEntry*>;
using Index = std::size_t;

bool less(const InstanceRenderEntry* left, const InstanceRenderEntry* right) {
    return render_entry_material_value_less_00b51ab0(
        {left->sort_key, left->section->material_order, left->depth},
        {right->sort_key, right->section->material_order, right->depth});
}

//00b1c210: rotate [first,middle),[middle,last) using gcd cycles. The native
// cycle starts at first+gcd, then decrements; insertion uses middle=last-1.
void rotate_00b1c210(Entries& entries, Index first, Index middle, Index last) {
    const auto length = last - first;
    const auto left = middle - first;
    auto common = length;
    for (auto divisor = left; divisor;) {
        const auto remainder = common % divisor;
        common = divisor;
        divisor = remainder;
    }
    if (!common || common >= length) return;
    for (auto cycle = common; cycle; --cycle) {
        const auto start = first + cycle;
        auto* value = entries[start];
        auto hole = start;
        auto next = start + left;
        if (next == last) next = first;
        while (next != start) {
            entries[hole] = entries[next];
            const auto remaining = last - next;
            hole = next;
            next = left < remaining ? next + left : first + left - remaining;
        }
        entries[hole] = value;
    }
}

void insertion_00b1d420(Entries& entries, Index first, Index last) {
    if (first == last) return;
    for (auto at = first + 1; at != last; ++at) {
        if (less(entries[at], entries[first])) {
            rotate_00b1c210(entries, first, at, at + 1);
        } else if (less(entries[at], entries[at - 1])) {
            auto destination = at - 1;
            // The first comparison establishes a non-less sentinel at first.
            while (less(entries[at], entries[destination - 1])) --destination;
            rotate_00b1c210(entries, destination, at, at + 1);
        }
    }
}

//00b1c8c0 makes all three comparisons, including the last when the second
// comparison did not swap. Only strict comparisons trigger swaps.
void median_three_00b1c8c0(Entries& entries, Index first, Index middle, Index last) {
    if (less(entries[middle], entries[first])) std::swap(entries[middle], entries[first]);
    if (less(entries[last], entries[middle])) std::swap(entries[last], entries[middle]);
    if (less(entries[middle], entries[first])) std::swap(entries[middle], entries[first]);
}

void choose_pivot_00b1cf80(Entries& entries, Index first, Index middle, Index last) {
    // last is the final element, not the half-open bound. CMP40 therefore
    // selects the nine-sample path at42 entries, not at41 entries.
    if (last - first > 40) {
        const auto step = (last - first + 1) / 8;
        median_three_00b1c8c0(entries, first, first + step, first + 2 * step);
        median_three_00b1c8c0(entries, middle - step, middle, middle + step);
        median_three_00b1c8c0(entries, last - 2 * step, last - step, last);
        median_three_00b1c8c0(entries, first + step, middle, last - step);
    } else {
        median_three_00b1c8c0(entries, first, middle, last);
    }
}

std::pair<Index, Index> partition_00b1d280(Entries& entries, Index first, Index last) {
    auto equal_first = first + (last - first) / 2;
    choose_pivot_00b1cf80(entries, first, equal_first, last - 1);
    auto equal_last = equal_first + 1;
    while (first < equal_first
        && !less(entries[equal_first - 1], entries[equal_first])
        && !less(entries[equal_first], entries[equal_first - 1])) --equal_first;
    while (equal_last < last
        && !less(entries[equal_last], entries[equal_first])
        && !less(entries[equal_first], entries[equal_last])) ++equal_last;
    auto scan_right = equal_last;
    auto scan_left = equal_first;
    for (;;) {
        for (; scan_right < last; ++scan_right) {
            if (less(entries[equal_first], entries[scan_right])) continue;
            if (less(entries[scan_right], entries[equal_first])) break;
            std::swap(entries[equal_last], entries[scan_right]);
            ++equal_last;
        }
        for (; first < scan_left; --scan_left) {
            if (less(entries[scan_left - 1], entries[equal_first])) continue;
            if (less(entries[equal_first], entries[scan_left - 1])) break;
            --equal_first;
            std::swap(entries[equal_first], entries[scan_left - 1]);
        }
        if (scan_left == first) {
            if (scan_right == last) return {equal_first, equal_last};
            //00b1d381..00b1d3aa: slide the equal band right, preserving the
            // exact two-swap order when the right scan has a smaller value.
            if (equal_last != scan_right)
                std::swap(entries[equal_first], entries[equal_last]);
            std::swap(entries[equal_first], entries[scan_right]);
            ++equal_first;
            ++equal_last;
            ++scan_right;
        } else {
            --scan_left;
            if (scan_right == last) {
                //00b1d3ba..00b1d3d3: symmetric equal-band slide left.
                --equal_first;
                if (scan_left != equal_first)
                    std::swap(entries[scan_left], entries[equal_first]);
                --equal_last;
                std::swap(entries[equal_first], entries[equal_last]);
            } else {
                std::swap(entries[scan_right], entries[scan_left]);
                ++scan_right;
            }
        }
    }
}

void push_heap_00b1c1b0(Entries& entries, Index first, Index hole, Index top,
    InstanceRenderEntry* value) {
    while (hole > top) {
        const auto parent = (hole - 1) / 2;
        if (!less(entries[first + parent], value)) break;
        entries[first + hole] = entries[first + parent];
        hole = parent;
    }
    entries[first + hole] = value;
}

void adjust_heap_00b1c910(Entries& entries, Index first, Index hole, Index length,
    InstanceRenderEntry* value) {
    const auto top = hole;
    auto child = 2 * hole + 2;
    while (child < length) {
        // Native compares RIGHT < LEFT. Equivalent children select RIGHT.
        if (less(entries[first + child], entries[first + child - 1])) --child;
        entries[first + hole] = entries[first + child];
        hole = child;
        child = 2 * child + 2;
    }
    if (child == length) {
        entries[first + hole] = entries[first + length - 1];
        hole = length - 1;
    }
    push_heap_00b1c1b0(entries, first, hole, top, value);
}

void make_heap_00b1d020(Entries& entries, Index first, Index last) {
    const auto length = last - first;
    for (auto parent = length / 2; parent;) {
        --parent;
        auto* value = entries[first + parent];
        adjust_heap_00b1c910(entries, first, parent, length, value);
    }
}

void sort_heap_00b1d6a0(Entries& entries, Index first, Index last) {
    for (auto length = last - first; length > 1; --length) {
        auto* value = entries[first + length - 1];
        entries[first + length - 1] = entries[first];
        adjust_heap_00b1c910(entries, first, 0, length - 1, value);
    }
}

void sort_range_00b1dce0(Entries& entries, Index first, Index last, Index ideal) {
    while (last - first > 32) {
        if (!ideal) {
            make_heap_00b1d020(entries, first, last);
            sort_heap_00b1d6a0(entries, first, last);
            return;
        }
        const auto equal = partition_00b1d280(entries, first, last);
        ideal = ideal / 2 + (ideal / 2) / 2;
        // The smaller side recurses. Equal side sizes recurse RIGHT.
        if (equal.first - first < last - equal.second) {
            sort_range_00b1dce0(entries, first, equal.first, ideal);
            first = equal.second;
        } else {
            sort_range_00b1dce0(entries, equal.second, last, ideal);
            last = equal.first;
        }
    }
    if (last - first > 1) insertion_00b1d420(entries, first, last);
}
}

bool sort_instance_entries_00b1dce0(std::vector<InstanceRenderEntry*>& entries,
    std::string& error) {
    error.clear();
    constexpr auto maximum_count =
        static_cast<std::size_t>(std::numeric_limits<std::int32_t>::max()) / 4;
    if (entries.size() > maximum_count) {
        error = "Instance sort pointer distance exceeds native signed32-bit arithmetic";
        return false;
    }
    for (const auto* entry : entries) {
        if (!entry || !entry->section || !std::isfinite(entry->depth)) {
            error = "Instance sort requires valid entry/section pointers and finite depths";
            return false;
        }
    }
    sort_range_00b1dce0(entries, 0, entries.size(), entries.size());
    return true;
}
}
