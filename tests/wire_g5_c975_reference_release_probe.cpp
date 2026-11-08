#include "bsp/seg3_4_cc13.hpp"
#include <windows.h>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <stdexcept>

namespace {
struct Counted;
using Terminal = void(__fastcall*)(Counted*, void*);
struct Counted { Terminal* table; volatile LONG count; };
static_assert(offsetof(Counted, count) == 4);
struct Trace {
    unsigned decrements{}, terminals{};
    bool terminal_saw_zero{}, terminal_saw_slot{};
};
Trace trace;
void* slot;
Counted replacement;
LONG WINAPI decrement(volatile LONG* counter) {
    ++trace.decrements;
    return ::InterlockedDecrement(counter);
}
void __fastcall terminal(Counted* object, void*) {
    ++trace.terminals;
    trace.terminal_saw_zero = object->count == 0;
    trace.terminal_saw_slot = slot == object;
    slot = &replacement; // 00440A30 must clear the slot AFTER this callback.
}
decltype(&decrement) decrement_import = &decrement;
struct Release final : bsp::seg3_4_cc13::ReferenceSlotHost {
    std::int32_t interlocked_decrement_ce2220(void* counter) override {
        return decrement(static_cast<volatile LONG*>(counter));
    }
    void object_destroy_00(void* object) override {
        auto* counted = static_cast<Counted*>(object);
        counted->table[0](counted, nullptr);
    }
};
struct Result { LONG count; Trace calls; bool cleared; };
Result run(bool original, void(__fastcall* native)(void*), bool empty, LONG count) {
    Terminal table[]{&terminal};
    Counted object{table, count};
    trace = {};
    slot = empty ? nullptr : &object;
    if (original) native(&slot);
    else { Release host; bsp::seg3_4_cc13::ref_slot_release_00440a30(&slot, host); }
    return {object.count, trace, slot == nullptr};
}
void require(bool value) { if (!value) throw std::runtime_error("release mismatch"); }
}

int main() {
    // Original 00440A30..00440A58 from bsp.py ghidra bytes (41 bytes).
    // Only the absolute 00CE2220 IAT operand is rebound to the fixture import.
    unsigned char bytes[]{0x56,0x57,0x8b,0xf9,0x8b,0x37,0x85,0xf6,0x74,0x1c,
        0x8d,0x46,0x04,0x50,0xff,0x15,0x20,0x22,0xce,0x00,0x85,0xc0,0x75,0x08,
        0x8b,0x16,0x8b,0x02,0x8b,0xce,0xff,0xd0,0xc7,0x07,0,0,0,0,0x5f,0x5e,0xc3};
    auto* code = static_cast<unsigned char*>(::VirtualAlloc(nullptr, sizeof(bytes),
        MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE));
    if (!code) return 2;
    const auto import = reinterpret_cast<std::uintptr_t>(&decrement_import);
    std::memcpy(bytes + 16, &import, 4);
    std::memcpy(code, bytes, sizeof(bytes));
    DWORD previous;
    if (!::VirtualProtect(code, sizeof(bytes), PAGE_EXECUTE_READ, &previous)) return 2;
    ::FlushInstructionCache(::GetCurrentProcess(), code, sizeof(bytes));
    const auto native = reinterpret_cast<void(__fastcall*)(void*)>(code);
    try {
        for (unsigned scenario = 0; scenario != 3; ++scenario) {
            const bool empty = scenario == 0;
            const LONG before = scenario == 1 ? 2 : 1;
            const auto a = run(true, native, empty, before);
            const auto b = run(false, native, empty, before);
            require(a.count == b.count && a.cleared && b.cleared);
            require(a.calls.decrements == b.calls.decrements
                && a.calls.terminals == b.calls.terminals
                && a.calls.terminal_saw_zero == b.calls.terminal_saw_zero
                && a.calls.terminal_saw_slot == b.calls.terminal_saw_slot);
            require(b.calls.decrements == (empty ? 0u : 1u));
            require(b.calls.terminals == (scenario == 2 ? 1u : 0u));
            if (scenario == 2) require(b.calls.terminal_saw_zero && b.calls.terminal_saw_slot);
        }
        ::VirtualFree(code, 0, MEM_RELEASE);
        std::puts("PASS: 00440A30 original-byte differential: null, surviving, terminal with slot mutation");
        return 0;
    } catch (const std::exception& error) {
        ::VirtualFree(code, 0, MEM_RELEASE);
        std::fprintf(stderr, "FAIL: %s\n", error.what());
        return 1;
    }
}
