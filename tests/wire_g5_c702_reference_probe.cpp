#include "bsp/native_resource_reader_references.hpp"
#include "bsp/native_adopted_substream.hpp"
#include <Windows.h>
#include <cstdint>
#include <cstdio>
#include <stdexcept>

struct Owner { std::uint32_t table; volatile LONG count; };
struct Dispatch final : bsp::NativeAdoptedSubstreamDispatch {
    void** slot;
    Owner* owner;
    Owner* replacement;
    std::uint32_t* table;
    int calls = 0;
    [[noreturn]] static void unexpected() { throw std::logic_error("unexpected stream operation"); }
    std::uint8_t source_is_open(std::uintptr_t, void*) override { unexpected(); }
    std::uint32_t source_seek(std::uintptr_t, void*, std::uint32_t, std::uint32_t, std::uint32_t) override { unexpected(); }
    void source_read(std::uintptr_t, void*, void*, std::uint32_t, std::uint32_t*) override { unexpected(); }
    void source_write(std::uintptr_t, void*, const void*, std::uint32_t, std::uint32_t*) override { unexpected(); }
    void source_zero_reference(std::uintptr_t entry, void* actual, std::uintptr_t captured_table) override {
        if (actual != owner || entry != table[0] || captured_table != reinterpret_cast<std::uintptr_t>(table)
            || owner->count != 0 || *slot != owner)
            throw std::logic_error("terminal dispatch lost live owner/count/slot/profile");
        ++calls;
        *slot = replacement;
    }
};
int main() {
    std::uint32_t table[1]{0x12345678};
    Owner owner{reinterpret_cast<std::uint32_t>(table), 1};
    Owner replacement{reinterpret_cast<std::uint32_t>(table), 9};
    void* slot = &owner;
    Dispatch dispatch;
    dispatch.slot = &slot; dispatch.owner = &owner;
    dispatch.replacement = &replacement; dispatch.table = table;
    void* returned = bsp::release_native_resource_slot_00483850(&slot, dispatch);
    if (returned != &slot || slot != nullptr || owner.count != 0
        || replacement.count != 9 || dispatch.calls != 1) return 1;
    std::puts("resource-slot terminal reentry: PASS");
}
