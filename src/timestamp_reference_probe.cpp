#include "bsp/d3d9_startup.hpp"
#include "bsp/frame_clock.hpp"
#include "timestamp_reference.hpp"
#include <cstdio>
#include <cstring>
#include <limits>

bool probe_timestamp_reference() {
    static_assert(sizeof(bsp::ClockTimestamp) == 16);
    constexpr auto size = sizeof(timestamp_reference_bytes);
    auto* code = static_cast<unsigned char*>(VirtualAlloc(nullptr, size,
        MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE));
    if (!code) return false;
    std::memcpy(code, timestamp_reference_bytes, size);
    for (const auto& patch : timestamp_reference_patches) {
        const unsigned relative = patch.target - (patch.offset + 4);
        std::memcpy(code + patch.offset, &relative, sizeof(relative));
    }
    DWORD old{};
    if (!VirtualProtect(code, size, PAGE_EXECUTE_READ, &old)
        || !FlushInstructionCache(GetCurrentProcess(), code, size)) {
        VirtualFree(code, 0, MEM_RELEASE); return false;
    }
    using Timestamp = bsp::ClockTimestamp;
    // Native ECX left, stack destination then right, RET8, EAX destination.
    using Subtract = Timestamp* (__thiscall *)(const Timestamp*, Timestamp*, const Timestamp*);
    Subtract subtract{};
    auto* entry = code + timestamp_reference_entry;
    static_assert(sizeof(subtract) == sizeof(entry));
    std::memcpy(&subtract, &entry, sizeof(subtract));
    constexpr auto minimum = (std::numeric_limits<std::int64_t>::min)();
    constexpr auto maximum = (std::numeric_limits<std::int64_t>::max)();
    struct Pair { Timestamp left, right; };
    // One compact arithmetic-risk batch: equal-frequency wrap, signed
    // truncation, low64 product wrap, and the helper's MIN/-1 behavior.
    const Pair pairs[] = {{{maximum, 10}, {-1, 10}}, {{9, 10}, {-7, 3}},
        {{9, 4}, {0x4000000000000001LL, 3}}, {{17, 1}, {minimum, -1}}};
    bool matched = true;
    unsigned comparisons = 0;
    for (const auto& pair : pairs) {
        for (unsigned alias = 0; alias < 3; ++alias) {
            auto native_left = pair.left, native_right = pair.right;
            auto rebuilt_left = pair.left, rebuilt_right = pair.right;
            Timestamp native{}, rebuilt{};
            auto* native_out = alias == 1 ? &native_left : alias == 2 ? &native_right : &native;
            auto* rebuilt_out = alias == 1 ? &rebuilt_left : alias == 2 ? &rebuilt_right : &rebuilt;
            auto* returned_native = subtract(&native_left, native_out, &native_right);
            auto* returned_rebuilt = &bsp::subtract_timestamp_00530890(*rebuilt_out, rebuilt_left, rebuilt_right);
            matched = matched && returned_native == native_out && returned_rebuilt == rebuilt_out
                && std::memcmp(native_out, rebuilt_out, sizeof(Timestamp)) == 0;
            ++comparisons;
        }
    }
    Timestamp native_same{minimum, -7}, rebuilt_same = native_same;
    auto* returned_native = subtract(&native_same, &native_same, &native_same);
    auto* returned_rebuilt = &bsp::subtract_timestamp_00530890(rebuilt_same, rebuilt_same, rebuilt_same);
    matched = matched && returned_native == &native_same && returned_rebuilt == &rebuilt_same
        && std::memcmp(&native_same, &rebuilt_same, sizeof(Timestamp)) == 0;
    ++comparisons;
    std::printf("Timestamp native comparison: arithmetic_and_exact_aliases=%d comparisons=%u\n", matched, comparisons);
    VirtualFree(code, 0, MEM_RELEASE);
    return matched;
}
