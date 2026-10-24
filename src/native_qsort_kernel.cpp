#include "bsp/native_qsort_kernel.hpp"

namespace bsp {
namespace {

static_assert(sizeof(void*) == 4 && sizeof(std::uint32_t) == 4);

// New private Source interface for whole normal 00BFC290 geometry. Volatile
// byte accesses retain the measured exchange order; they add no synchronization.
void swap_bytes_00bfc290(unsigned char* first, unsigned char* second,
    std::uint32_t width) noexcept {
    if (first == second || width == 0) return;
    volatile unsigned char* left = first;
    volatile unsigned char* right = second;
    auto remaining = width;
    do {
        const unsigned char second_byte = *right;
        const unsigned char first_byte = *left;
        *left = second_byte;
        --remaining;
        *right = first_byte;
        ++left;
        ++right;
    } while (remaining != 0);
}

// New private Source interface for whole normal 00BFC2C0 geometry. Endpoints
// are offsets in one admitted extent, not synthetic callback element storage.
void shortsort_bytes_00bfc2c0(unsigned char* base, std::uint32_t low,
    std::uint32_t high, std::uint32_t width, NativeQsortCompare compare) {
    if (high <= low) return;
    const auto first_scan = low + width;
    do {
        auto maximum = low;
        auto scan = first_scan;
        while (scan <= high) {
            if (compare(base + scan, base + maximum) > 0) maximum = scan;
            scan += width;
        }
        if (maximum != high && width != 0) {
            volatile unsigned char* high_byte_address = base + high;
            volatile unsigned char* maximum_byte_address = base + maximum;
            auto remaining = width;
            do {
                const unsigned char high_byte = *high_byte_address;
                const unsigned char maximum_byte = *maximum_byte_address;
                *maximum_byte_address = high_byte;
                --remaining;
                *high_byte_address = maximum_byte;
                ++high_byte_address;
                ++maximum_byte_address;
            } while (remaining != 0);
        }
        high -= width;
    } while (high > low);
}

} // namespace

void sort_valid_native_bytes_00bfc350(void* actual_base, std::uint32_t count,
    std::uint32_t width, NativeQsortCompare compare) {
    // The excluded Original validity predicates are mandatory caller
    // preconditions, including on this no-element/no-callback path.
    if (count < 2) return;

    auto* const base = static_cast<unsigned char*>(actual_base);
    std::uint32_t low = 0;
    std::uint32_t high = (count - 1u) * width;
    std::uint32_t low_stack[30];
    std::uint32_t high_stack[30];
    std::int32_t pending = 0;

    for (;;) {
        const auto subrange_count = (high - low) / width + 1u;
        if (subrange_count <= 8) {
            shortsort_bytes_00bfc2c0(base, low, high, width, compare);
        } else {
            auto pivot = low + (subrange_count / 2u) * width;
            if (compare(base + low, base + pivot) > 0)
                swap_bytes_00bfc290(base + low, base + pivot, width);
            if (compare(base + low, base + high) > 0)
                swap_bytes_00bfc290(base + low, base + high, width);
            if (compare(base + pivot, base + high) > 0)
                swap_bytes_00bfc290(base + pivot, base + high, width);

            auto low_scan = low;
            auto high_scan = high;
            for (;;) {
                if (pivot > low_scan) {
                    do {
                        low_scan += width;
                        if (low_scan >= pivot) break;
                    } while (compare(base + low_scan, base + pivot) <= 0);
                }
                if (pivot <= low_scan) {
                    do {
                        low_scan += width;
                        if (low_scan > high) break;
                    } while (compare(base + low_scan, base + pivot) <= 0);
                }
                do {
                    high_scan -= width;
                    if (high_scan <= pivot) break;
                } while (compare(base + high_scan, base + pivot) > 0);

                if (low_scan > high_scan) break;
                if (low_scan != high_scan) {
                    volatile unsigned char* low_byte_address = base + low_scan;
                    volatile unsigned char* high_byte_address = base + high_scan;
                    auto remaining = width;
                    do {
                        const unsigned char low_byte = *low_byte_address;
                        const unsigned char high_byte = *high_byte_address;
                        *low_byte_address = high_byte;
                        --remaining;
                        *high_byte_address = low_byte;
                        ++low_byte_address;
                        ++high_byte_address;
                    } while (remaining != 0);
                }
                if (pivot == high_scan) pivot = low_scan;
            }

            high_scan += width;
            if (pivot < high_scan) {
                do {
                    high_scan -= width;
                    if (high_scan <= pivot) break;
                } while (compare(base + high_scan, base + pivot) == 0);
            }
            if (pivot >= high_scan) {
                do {
                    high_scan -= width;
                    if (high_scan <= low) break;
                } while (compare(base + high_scan, base + pivot) == 0);
            }

            // Empty right has high-low_scan == -width. Compute differences
            // without unsigned underflow or an out-of-allocation pointer.
            const auto left_distance = static_cast<std::int64_t>(high_scan) - low;
            const auto right_distance = static_cast<std::int64_t>(high) - low_scan;
            if (left_distance < right_distance) {
                if (low_scan < high) {
                    low_stack[pending] = low_scan;
                    high_stack[pending] = high;
                    ++pending;
                }
                if (low < high_scan) {
                    high = high_scan;
                    continue;
                }
            } else {
                if (low < high_scan) {
                    low_stack[pending] = low;
                    high_stack[pending] = high_scan;
                    ++pending;
                }
                if (low_scan < high) {
                    low = low_scan;
                    continue;
                }
            }
        }

        --pending;
        if (pending < 0) return;
        low = low_stack[pending];
        high = high_stack[pending];
    }
}

// 00BFC350 _qsort, cdecl(base, num, width, comp), plain RET. The validation arms
// read from the listing (cc13-P4): 00BFC35E TEST EBX,EBX (base) / JNZ; 00BFC36A
// TEST ESI,ESI (num) / JZ -> both pass; else 00BFC36E _errno, MOV [EAX],16h,
// 00BFC383 _invalid_parameter(0,0,0,0,0), RET (00BFC393). 00BFC39C TEST EBP,EBP
// (width) / JBE and 00BFC3A0 CMP comp,0 / JNZ; else the same errno/_invalid_parameter
// pair and RET (00BFC3D0). From 00BFC3D1 (CMP ESI,2 / JC) the kernel above.
// _invalid_parameter does not return in this image (its handler is the encoded
// null, so it takes __invoke_watson); the return after it is the listing's.
void qsort_00bfc350(NativeQsortCrtHost& crt, void* base, std::uint32_t count,
    std::uint32_t width, NativeQsortCompare compare) {
    if (base == nullptr && count != 0) {
        *crt.errno_slot_00bffb8b() = 0x16;
        crt.invalid_parameter_00bf66ef();
    } else if (width == 0 || compare == nullptr) {
        *crt.errno_slot_00bffb8b() = 0x16;
        crt.invalid_parameter_00bf66ef();
    } else {
        sort_valid_native_bytes_00bfc350(base, count, width, compare);
    }
}

} // namespace bsp
