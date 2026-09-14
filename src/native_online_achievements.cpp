#include "bsp/native_online_achievements.hpp"

#include <Windows.h>

#include <cstring>
#include <stdexcept>
#include <string>

namespace bsp {
namespace {
static_assert(sizeof(void*) == 4 && sizeof(std::size_t) == 4);
static_assert(sizeof(NativeOnlineAchievementsSdk::Write) == 4);
constexpr std::uint32_t pending = 0x3e5;

std::uint32_t address(const void* p) {
    return static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(p));
}
void* pointer(std::uint32_t a) {
    return reinterpret_cast<void*>(static_cast<std::uintptr_t>(a));
}
std::uint32_t load(std::uint32_t a) {
    std::uint32_t value;
    std::memcpy(&value, pointer(a), 4);
    return value;
}
void store(std::uint32_t a, std::uint32_t value) {
    std::memcpy(pointer(a), &value, 4);
}
std::int32_t signed_word(std::uint32_t value) {
    std::int32_t result;
    std::memcpy(&result, &value, 4);
    return result;
}
std::uint32_t sar2(std::uint32_t value) {
    return (value >> 2) | ((value & 0x80000000u) ? 0xc0000000u : 0u);
}
std::uint32_t queue_count(std::uint32_t manager) {
    const auto begin = load(manager + 0x364);
    return begin ? sar2(load(manager + 0x368) - begin) : 0;
}
void check_index(std::uint32_t manager, std::uint32_t index,
    const NativeOnlineAchievementsCrt& crt) {
    const auto begin = load(manager + 0x364);
    if (!begin || index >= sar2(load(manager + 0x368) - begin))
        crt.invalid_parameter_noinfo(); // can return; subsequent fields are reloaded
}
template<class Function>
Function ordinal(HMODULE module, std::uint16_t number) {
    const FARPROC entry = GetProcAddress(module, MAKEINTRESOURCEA(number));
    if (!entry) throw std::runtime_error("Missing XLive ordinal " + std::to_string(number));
    static_assert(sizeof(entry) == sizeof(Function));
    Function function;
    std::memcpy(&function, &entry, sizeof(function));
    return function;
}
} // namespace

NativeOnlineAchievementsSdk resolve_native_online_achievements_sdk(void* loaded_module) {
    if (!loaded_module) throw std::invalid_argument("XLive module is not loaded");
    const auto module = static_cast<HMODULE>(loaded_module);
    return {ordinal<NativeOnlineAchievementsSdk::Write>(module, 5278),
        ordinal<NativeOnlineStorageSdk::Result>(module, 1083),
        ordinal<NativeOnlineStorageSdk::Error>(module, 1082)};
}

void pump_native_online_achievements_00a3fa70(NativeOnlineManagerStorage& manager,
    std::uint32_t force_word, const NativeOnlineAchievementsSdk& sdk,
    const NativeOnlineStorageMemory& memory, const NativeOnlineAchievementsCrt& crt) {
    const auto m = address(&manager);
    if (queue_count(m) == 0) return;
    if ((load(m + 0x3a8) != pending && load(m + 0x3a4) == 0 &&
            load(m + 0x384) != pending) || (force_word & 0xffu) != 0) {
        for (std::uint32_t offset = 0x384; offset <= 0x394; offset += 4)
            store(m + offset, 0); // leaves +398/+39C untouched
        if (void* old = pointer(load(m + 0x3a0))) {
            memory.release(old);
            store(m + 0x3a0, 0);
        }
        const auto count = queue_count(m);
        store(m + 0x3a4, count);
        // MUL r32 / SETO / NEG / OR saturates any unsigned overflow to FFFFFFFF.
        const auto product = static_cast<std::uint64_t>(count) * 8u;
        const auto bytes = product > 0xffffffffull ? 0xffffffffu :
            static_cast<std::uint32_t>(product);
        void* allocated = memory.allocate(bytes);
        store(m + 0x3a0, address(allocated));
        std::uint32_t index = 0;
        if (signed_word(load(m + 0x3a4)) > 0) {
            do {
                check_index(m, index, crt);
                const auto id = load(load(m + 0x364) + index * 4u);
                store(load(m + 0x3a0) + index * 8u + 4u, id);
                const auto batch = load(m + 0x3a0);
                store(batch + index * 8u, load(m + 0x11c));
                // 4254B0 is RET. Preserve its potentially faulting argument loads
                // and second checked-vector access without inventing a logger.
                const volatile auto signin = load(m + load(m + 0x11c) * 4u + 0x8c);
                (void)signin;
                check_index(m, index, crt);
                const volatile auto diagnostic_id = load(load(m + 0x364) + index * 4u);
                (void)diagnostic_id;
                ++index;
            } while (signed_word(index) < signed_word(load(m + 0x3a4)));
        }
        const auto batch = load(m + 0x3a0);
        const auto submitted = sdk.write(load(m + 0x3a4), pointer(batch), pointer(m + 0x384));
        store(m + 0x3a8, submitted);
        return;
    }
    if (load(m + 0x384) == pending) {
        const auto error = sdk.overlapped_error(pointer(m + 0x384));
        store(m + 0x3a8, error);
        return; // 4254B0 diagnostic is a proven RET
    }
    if (sdk.overlapped_result(pointer(m + 0x384), nullptr, 1) != 0) return;
    std::uint32_t pair_index = 0;
    if (signed_word(load(m + 0x3a4)) > 0) {
        do {
            auto current = load(m + 0x364);
            if (current > load(m + 0x368)) crt.invalid_parameter_noinfo();
            for (;;) {
                const auto end = load(m + 0x368);
                if (load(m + 0x364) > end) crt.invalid_parameter_noinfo();
                // A3FC61 cannot execute: CMP ESI,ESI / JZ A3FC66.
                if (current == end) break;
                if (current >= load(m + 0x368)) crt.invalid_parameter_noinfo();
                const auto batch = load(m + 0x3a0);
                if (load(current) == load(batch + pair_index * 8u + 4u)) {
                    const auto next = current + 4u;
                    const auto tail_count = sar2(load(m + 0x368) - next);
                    if (signed_word(tail_count) > 0) {
                        const auto bytes = tail_count * 4u;
                        (void)crt.memmove_s(pointer(current), bytes, pointer(next), bytes);
                    }
                    store(m + 0x368, load(m + 0x368) - 4u);
                    break; // exactly the first current queue match for this pair
                }
                if (current >= load(m + 0x368)) crt.invalid_parameter_noinfo();
                current += 4u;
            }
            ++pair_index;
        } while (signed_word(pair_index) < signed_word(load(m + 0x3a4)));
    }
    store(m + 0x3a4, 0);
    if (void* old = pointer(load(m + 0x3a0))) {
        memory.release(old);
        store(m + 0x3a0, 0);
    }
    store(m + 0x3a8, 0);
}
} // namespace bsp
