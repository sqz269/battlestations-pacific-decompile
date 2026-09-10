#include "bsp/instance_sort.hpp"
#include "bsp/instance_upload.hpp"
#include "bsp/instance_geometry.hpp"
#include "bsp/render_sort.hpp"
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <limits>
#include <utility>

namespace bsp {
namespace {
using Entries = std::vector<InstanceRenderEntry*>;
using Index = std::size_t;
using Predicate = bool (*)(const InstanceRenderEntry*, const InstanceRenderEntry*);

bool less(const InstanceRenderEntry* left, const InstanceRenderEntry* right) {
    return render_entry_material_value_less_00b51ab0(
        {left->sort_key, left->section->material_order, left->depth},
        {right->sort_key, right->section->material_order, right->depth});
}

bool key_less(const InstanceRenderEntry* left, const InstanceRenderEntry* right) {
    return render_entry_key_less_00b51b00(
        {left->sort_key, 0, 0.0f}, {right->sort_key, 0, 0.0f});
}

bool valid_pointer_distance(const Entries& entries, std::string& error) {
    constexpr auto maximum_count =
        static_cast<std::size_t>(std::numeric_limits<std::int32_t>::max()) / 4;
    if (entries.size() <= maximum_count) return true;
    error = "Instance sort pointer distance exceeds native signed32-bit arithmetic";
    return false;
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

void insertion_00b1d420(Entries& entries, Index first, Index last, Predicate compare) {
    if (first == last) return;
    for (auto at = first + 1; at != last; ++at) {
        if (compare(entries[at], entries[first])) {
            rotate_00b1c210(entries, first, at, at + 1);
        } else if (compare(entries[at], entries[at - 1])) {
            auto destination = at - 1;
            // The first comparison establishes a non-less sentinel at first.
            while (compare(entries[at], entries[destination - 1])) --destination;
            rotate_00b1c210(entries, destination, at, at + 1);
        }
    }
}

//00b1c8c0 makes all three comparisons, including the last when the second
// comparison did not swap. Only strict comparisons trigger swaps.
void median_three_00b1c8c0(Entries& entries, Index first, Index middle, Index last, Predicate compare) {
    if (compare(entries[middle], entries[first])) std::swap(entries[middle], entries[first]);
    if (compare(entries[last], entries[middle])) std::swap(entries[last], entries[middle]);
    if (compare(entries[middle], entries[first])) std::swap(entries[middle], entries[first]);
}

void choose_pivot_00b1cf80(Entries& entries, Index first, Index middle, Index last, Predicate compare) {
    // last is the final element, not the half-open bound. CMP40 therefore
    // selects the nine-sample path at42 entries, not at41 entries.
    if (last - first > 40) {
        const auto step = (last - first + 1) / 8;
        median_three_00b1c8c0(entries, first, first + step, first + 2 * step, compare);
        median_three_00b1c8c0(entries, middle - step, middle, middle + step, compare);
        median_three_00b1c8c0(entries, last - 2 * step, last - step, last, compare);
        median_three_00b1c8c0(entries, first + step, middle, last - step, compare);
    } else {
        median_three_00b1c8c0(entries, first, middle, last, compare);
    }
}

std::pair<Index, Index> partition_00b1d280(Entries& entries, Index first, Index last, Predicate compare) {
    auto equal_first = first + (last - first) / 2;
    choose_pivot_00b1cf80(entries, first, equal_first, last - 1, compare);
    auto equal_last = equal_first + 1;
    while (first < equal_first
        && !compare(entries[equal_first - 1], entries[equal_first])
        && !compare(entries[equal_first], entries[equal_first - 1])) --equal_first;
    while (equal_last < last
        && !compare(entries[equal_last], entries[equal_first])
        && !compare(entries[equal_first], entries[equal_last])) ++equal_last;
    auto scan_right = equal_last;
    auto scan_left = equal_first;
    for (;;) {
        for (; scan_right < last; ++scan_right) {
            if (compare(entries[equal_first], entries[scan_right])) continue;
            if (compare(entries[scan_right], entries[equal_first])) break;
            std::swap(entries[equal_last], entries[scan_right]);
            ++equal_last;
        }
        for (; first < scan_left; --scan_left) {
            if (compare(entries[scan_left - 1], entries[equal_first])) continue;
            if (compare(entries[equal_first], entries[scan_left - 1])) break;
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
    InstanceRenderEntry* value, Predicate compare) {
    while (hole > top) {
        const auto parent = (hole - 1) / 2;
        if (!compare(entries[first + parent], value)) break;
        entries[first + hole] = entries[first + parent];
        hole = parent;
    }
    entries[first + hole] = value;
}

void adjust_heap_00b1c910(Entries& entries, Index first, Index hole, Index length,
    InstanceRenderEntry* value, Predicate compare) {
    const auto top = hole;
    auto child = 2 * hole + 2;
    while (child < length) {
        // Native compares RIGHT < LEFT. Equivalent children select RIGHT.
        if (compare(entries[first + child], entries[first + child - 1])) --child;
        entries[first + hole] = entries[first + child];
        hole = child;
        child = 2 * child + 2;
    }
    if (child == length) {
        entries[first + hole] = entries[first + length - 1];
        hole = length - 1;
    }
    push_heap_00b1c1b0(entries, first, hole, top, value, compare);
}

void make_heap_00b1d020(Entries& entries, Index first, Index last, Predicate compare) {
    const auto length = last - first;
    for (auto parent = length / 2; parent;) {
        --parent;
        auto* value = entries[first + parent];
        adjust_heap_00b1c910(entries, first, parent, length, value, compare);
    }
}

void sort_heap_00b1d6a0(Entries& entries, Index first, Index last, Predicate compare) {
    for (auto length = last - first; length > 1; --length) {
        auto* value = entries[first + length - 1];
        entries[first + length - 1] = entries[first];
        adjust_heap_00b1c910(entries, first, 0, length - 1, value, compare);
    }
}

void sort_range_00b1dce0(Entries& entries, Index first, Index last, Index ideal, Predicate compare = less) {
    while (last - first > 32) {
        if (!ideal) {
            make_heap_00b1d020(entries, first, last, compare);
            sort_heap_00b1d6a0(entries, first, last, compare);
            return;
        }
        const auto equal = partition_00b1d280(entries, first, last, compare);
        ideal = ideal / 2 + (ideal / 2) / 2;
        // The smaller side recurses. Equal side sizes recurse RIGHT.
        if (equal.first - first < last - equal.second) {
            sort_range_00b1dce0(entries, first, equal.first, ideal, compare);
            first = equal.second;
        } else {
            sort_range_00b1dce0(entries, equal.second, last, ideal, compare);
            last = equal.first;
        }
    }
    if (last - first > 1) insertion_00b1d420(entries, first, last, compare);
}
}

bool sort_instance_entries_00b1dce0(std::vector<InstanceRenderEntry*>& entries,
    std::string& error) {
    error.clear();
    if (!valid_pointer_distance(entries, error)) return false;
    for (const auto* entry : entries) {
        if (!entry || !entry->section || !std::isfinite(entry->depth)) {
            error = "Instance sort requires valid entry/section pointers and finite depths";
            return false;
        }
    }
    sort_range_00b1dce0(entries, 0, entries.size(), entries.size(), less);
    return true;
}

bool get_render_batch_sort_configuration_00b1cb30(
    const std::vector<RenderBatchSortConfiguration>& configurations,
    std::uint32_t batch_index, RenderBatchSortConfiguration& output,
    std::string& error) {
    error.clear();
    if (batch_index >= configurations.size()) {
        error = "Render batch has no sort configuration for the original index";
        return false;
    }
    output.enabled = configurations[batch_index].enabled;
    output.value = configurations[batch_index].value;
    return true;
}

std::uint32_t render_batch_depth_word_00bf7456(float depth) noexcept {
    static_assert(sizeof(float) == sizeof(std::uint32_t)
        && std::numeric_limits<float>::is_iec559,
        "Batch depth conversion requires IEEE754 binary32");
    std::uint32_t bits;
    std::memcpy(&bits, &depth, sizeof(bits));
    const auto exponent = (bits >> 23) & 0xffu;
    // Zero/subnormal/abs<1 truncate to zero. At abs>=2^63 every binary32
    // value either converts to integer-indefinite or is exactly -2^63;
    // both yield EAX=0. This includes NaN/infinity (exponent255).
    if (exponent < 127 || exponent >= 190) return 0;
    const auto significand = (bits & 0x7fffffu) | 0x800000u;
    const auto integer_exponent = exponent - 127;
    // Integer binary32 values with exponent>=55 have at least32 trailing
    // zero bits. Avoid oversized shifts, retain unsigned modulo2^32 below.
    const auto magnitude = integer_exponent < 23
        ? significand >> (23 - integer_exponent)
        : integer_exponent < 55 ? significand << (integer_exponent - 23) : 0u;
    return bits & 0x80000000u ? 0u - magnitude : magnitude;
}

std::uint64_t make_render_batch_key_00b51df0(float depth,
    const RenderBatchMaterialKeyFields& fields) noexcept {
    const auto texture = fields.material_count34 > 0 && fields.texture0_present
        ? fields.texture20 & 0xfffu : 0u;
    const auto prefix = ((static_cast<std::uint32_t>(fields.effect_b0) & 0x3fu)
        * 0x1000u + texture) * 0x100u + fields.effect_c0;
    // __allmul's second operand is highDWORD=20h,lowDWORD=0: 2^37.
    // XOR EDX,EDX after00bf7456 discards its signed high word.
    return (static_cast<std::uint64_t>(prefix) << 37)
        + render_batch_depth_word_00bf7456(depth);
}

bool prepare_render_batch_00b51df0(std::vector<InstanceRenderEntry*>& entries,
    std::uint32_t batch_index,
    const std::vector<RenderBatchSortConfiguration>& configurations,
    const RenderBatchMaterialKeySource& source, std::string& error) {
    RenderBatchSortConfiguration configuration;
    if (!get_render_batch_sort_configuration_00b1cb30(
        configurations, batch_index, configuration, error)) return false;
    if (!configuration.enabled) return true;
    if (batch_index != 0) return sort_instance_entries_00b1dce0(entries, error);
    if (!valid_pointer_distance(entries, error)) return false;
    if (!entries.empty() && !source.read) {
        error = "Render batch key preparation requires a material field reader";
        return false;
    }
    for (std::size_t index = 0; index < entries.size(); ++index) {
        auto* entry = entries[index];
        if (!entry || !entry->section) {
            error = "Render batch key preparation requires valid entry/section pointers";
            return false;
        }
        RenderBatchMaterialKeyFields fields;
        if (!source.read(source.context, *entry, fields, error)) {
            if (error.empty()) error = "Render batch material field reader failed";
            return false;
        }
        entry->sort_key = make_render_batch_key_00b51df0(entry->depth, fields);
    }
    sort_range_00b1dce0(entries, 0, entries.size(), entries.size(), key_less);
    return true;
}
}
