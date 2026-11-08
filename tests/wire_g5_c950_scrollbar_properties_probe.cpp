// Focused AC85A0 derived-reader fixture. This is not a live Page/game binding.
#include "bsp/native_gui_scrollbar_storage.hpp"
#include "bsp/native_string_pool_storage.hpp"
#include "bsp/native_singleton_destruction.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <array>
#include <cstdio>
#include <cstring>
#include <stdexcept>

namespace {
using Word = std::uint32_t;
void check(bool value, const char* message) {
    if (!value) throw std::runtime_error(message);
}
Word bits(const void* value) { return reinterpret_cast<Word>(value); }
Word& word(void* p, Word offset = 0) {
    return *reinterpret_cast<Word*>(static_cast<std::byte*>(p) + offset);
}
void store_float(void* p, Word offset, float value) { std::memcpy(&word(p, offset), &value, 4); }
float load_float(void* p, Word offset) {
    float value;
    std::memcpy(&value, &word(p, offset), 4);
    return value;
}
struct Fixture final : bsp::NativeGuiWidgetPropertyDispatch, bsp::NativeGuiScrollbarCalls {
    bsp::NativeStringRawPoolContext& strings;
    alignas(4) std::array<std::byte, 0xfc> actual{};
    bsp::NativeGuiScrollbarPropertyScratch scratch{};
    Word reader_tables[3][4]{{0,0,0,0x11111111}, {0,0,0,0x22222222}, {0,0,0,0x33333333}};
    Word reader{};
    Word scrollbar_table[32]{};
    int reads{}, layouts{};
    bool fail_third{};
    char* allocated{};
    const bsp::NativeGuiScrollbarPropertyLiterals literals{
        "Alignment", "Vertical", "MinValue", "MaxValue", ""};
    char* alignment_data() { return reinterpret_cast<char*>(scratch.alignment[1]); }
    explicit Fixture(bsp::NativeStringRawPoolContext& pool, bool fail)
        : strings(pool), fail_third(fail) {
        reader = bits(reader_tables[0]);
        word(actual.data()) = 0x00d5ccf8;
        store_float(actual.data(), 0xec, 5.0f);
        scrollbar_table[31] = 0x00ac8a80;
    }
    void call_virtual0c(Word target, void* visitor, void* key_raw,
        void* field_raw, void* fallback_raw) override {
        auto* key = static_cast<Word*>(key_raw);
        auto* field = static_cast<Word*>(field_raw);
        auto* fallback = static_cast<Word*>(fallback_raw);
        check(visitor == &reader && reads < 3 && target == reader_tables[reads][3],
            "reader profile/target was cached across calls");
        check(scratch.argument_home == bits(key), "argument metadata did not name current key");
        check(key[0] == 0 && scratch.alignment_live, "key tag/string lifetime");
        const char* names[]{"Alignment", "MinValue", "MaxValue"};
        check(std::strcmp(reinterpret_cast<const char*>(key[1]), names[reads]) == 0, "key order");
        const int index = reads++;
        if (index == 0) {
            check(field[0] == 0 && field[1] == bits(&scratch.alignment) && fallback[0] == 0 &&
                std::strcmp(reinterpret_cast<const char*>(fallback[1]), "") == 0, "Alignment pairs");
            bsp::construct_native_string_header_0041e870(&scratch.alignment, strings, "vErTiCaL");
            allocated = alignment_data();
        } else {
            check(field[0] == 2 && fallback[0] == 2 && fallback[1] == 0 &&
                field[1] == bits(actual.data() + (index == 1 ? 0xf0 : 0xf4)), "actual float pairs");
            check(word(actual.data(), 0xf8) == 0, "Alignment was not published before numeric reads");
            if (index == 2 && fail_third) throw std::runtime_error("fixture visitor escape");
            store_float(reinterpret_cast<void*>(field[1]), 0, index == 1 ? 8.0f : 10.0f);
        }
        if (reads < 3) reader = bits(reader_tables[reads]);
    }
    const volatile Word* current_table(void* widget, Word profile) override {
        check(widget == actual.data() && profile == 0x00d5ccf8, "actual Scrollbar layout receiver");
        return scrollbar_table;
    }
    void call_layout_7c(void* widget, Word target) override {
        check(widget == actual.data() && target == 0x00ac8a80 && scratch.alignment_live &&
            std::strcmp(alignment_data(), "vErTiCaL") == 0, "early string cleanup/layout dispatch");
        // The property tail must reload MaxValue after SetMinimum's callbacks.
        if (++layouts == 1) store_float(actual.data(), 0xf4, 6.0f);
    }
    void* find_child_00aaa2b0(void*, const void*) override { throw std::runtime_error("unreached child lookup"); }
    void loaded_base_00aa7170(void*) override { throw std::runtime_error("unreached loaded hook"); }
    void run() {
        bool escaped = false;
        try {
            bsp::read_native_gui_scrollbar_properties_after_base(actual.data(), &reader,
                scratch, literals, *this, strings, *this);
        } catch (const std::runtime_error& error) {
            if (std::strcmp(error.what(), "fixture visitor escape") != 0) throw;
            escaped = true;
        }
        check(escaped == fail_third && reads == 3 && !scratch.alignment_live,
            "visitor escape/cleanup state");
        check(alignment_data() == allocated && scratch.alignment[0] == 8,
            "cleanup cleared the native stale header");
        if (fail_third) check(layouts == 0 && load_float(actual.data(), 0xf0) == 8.0f,
            "escaping reader rolled back state or reached layout");
        else check(layouts == 4 && load_float(actual.data(), 0xec) == 6.0f &&
            load_float(actual.data(), 0xf0) == 6.0f && load_float(actual.data(), 0xf4) == 6.0f,
            "late maximum read or clamp/layout schedule");
        auto* recycled = bsp::allocate_native_string_pool_00bd1120(strings.actual_published_01090aa8, 9);
        check(recycled == allocated, "Alignment allocation was not returned to its actual size-class ring");
        bsp::return_native_string_pool_00bd1510(strings.actual_published_01090aa8,
            recycled, 9, strings.actual_small_returns_disabled_01090aa4);
    }
};
}
int main() {
    try {
        bsp::NativeStringPoolStorage* volatile pool = nullptr;
        volatile Word returns_disabled = 0;
        void* volatile manager = nullptr;
        bsp::NativeStringRawPoolContext strings{pool, returns_disabled, manager};
        Fixture(strings, false).run();
        Fixture(strings, true).run();
        bsp::NativeSingletonDeletionBindings deletion{};
        deletion.actual_string_pool_publication_01090aa8 = &pool;
        deletion.actual_string_returns_disabled_01090aa4 = &returns_disabled;
        bsp::destroy_native_singleton_manager_00bd0400(manager, deletion);
        bsp::singleton_lifetime_free(manager);
        check(pool == nullptr, "fixture pool shutdown");
        std::puts("AC85A0 raw derived reader: current targets, real cells, late clamp and string cleanup PASS");
    } catch (const std::exception& error) {
        std::fprintf(stderr, "AC85A0 reader probe: %s\n", error.what());
        return 1;
    }
}
