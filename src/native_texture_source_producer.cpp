#include "bsp/native_texture_source_producer.hpp"
#include "bsp/native_string_pool_owner.hpp"
#include "bsp/native_texture_source_child_append.hpp"
#include <cstring>
#include <exception>
#include <stdexcept>
#include <type_traits>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native texture-source producer requires MSVC Win32.
#endif

namespace bsp {
static_assert(sizeof(NativeString) == 8 && alignof(NativeString) == 4);
static_assert(std::is_standard_layout_v<NativeString>);
namespace {
std::uint32_t current_length(NativeString& value) noexcept {
    return *reinterpret_cast<volatile std::uint32_t*>(&value);
}
char* current_data(NativeString& value) noexcept {
    return *reinterpret_cast<char* volatile*>(reinterpret_cast<unsigned char*>(&value) + 4);
}
void zero_child_name(NativeString& value) noexcept {
    *reinterpret_cast<volatile std::uint32_t*>(&value) = 0;
    *reinterpret_cast<char* volatile*>(reinterpret_cast<unsigned char*>(&value) + 4) = nullptr;
}
void return_captured(char* captured, std::uint32_t size, NativeStringRawPoolContext& raw) {
    auto* const pool = native_string_pool_get_or_create_00419cc0(
        raw.actual_published_01090aa8, raw.actual_manager_publication_01090aa0);
    return_native_string_pool_00bd1510(pool, captured, size,
        raw.actual_small_returns_disabled_01090aa4);
}
std::uint32_t scan_native_length(const char* text) {
    auto cursor = static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(text));
    const auto after_first = cursor + 1u;
    std::uint8_t byte;
    do {
        byte = *reinterpret_cast<const volatile std::uint8_t*>(static_cast<std::uintptr_t>(cursor));
        cursor += 1u;
    } while (byte != 0);
    return cursor - after_first;
}
} // namespace

NativeTextureSourceProducerAcquired::NativeTextureSourceProducerAcquired(
    NativeString& scratch) noexcept : scratch(scratch) {}

NativeTextureSourceProducerAcquired::~NativeTextureSourceProducerAcquired() {
    if (phase != Phase::fresh && phase != Phase::complete) std::terminate();
}

void populate_native_texture_source_00c30570(
    NativeTextureSourcePayload& payload, const NativeString& requested,
    NativeTextureSourceProducerContext& c, NativeTextureSourceProducerAcquired& a) {
    if (a.phase != NativeTextureSourceProducerAcquired::Phase::fresh)
        throw std::logic_error("texture-source producer cannot replay a retained operation");
    a.phase = NativeTextureSourceProducerAcquired::Phase::in_progress_or_unclassified;
    volatile auto& actual = payload;
    a.native_site = 0x00c30597;
    if (actual.initialized_14 != 0) {
        a.phase = NativeTextureSourceProducerAcquired::Phase::complete;
        return;
    }
    actual.initialized_14 = 1;
    a.body_entered = true;
    try {
        a.native_site = 0x00c305b0;
        a.owner = construct_native_lua_state_00b66bd0(a.owner_cell);
        a.unwind_state = 0; a.native_site = 0x00c305c5;
        open_native_lua_state_00b6a020(*a.owner, 1, c.strings, c.lua.bootstrap());
        a.native_site = 0x00c305d3;
        auto* const script = static_cast<NativeString*>(construct_native_string_header_0041e870(
            &a.scratch, c.raw_strings, c.actual_script_00d79b98));
        a.unwind_state = 1; a.native_site = 0x00c305e9;
        run_native_lua_file_00b69d40(*a.owner, *script, 0, c.strings, c.lua.files());
        char* const script_data = current_data(a.scratch);
        a.unwind_state = 0;
        if (script_data) {
            const auto size = current_length(a.scratch) + 1u;
            a.native_site = 0x00c30608;
            return_captured(script_data, size, c.raw_strings);
        }
        a.native_site = 0x00c30623;
        a.globals = native_lua_globals_00b67980(*a.owner, a.globals_cell);
        a.unwind_state = 2; a.native_site = 0x00c3063c;
        a.animated = native_lua_get_by_name_00b67800(*a.globals, a.animated_cell, c.actual_animated_00d79b88);
        a.unwind_state = 4; a.native_site = 0x00c30650; // No normal state3 store.
        destroy_native_lua_object_00b67700(*a.globals);
        a.native_site = 0x00c30662;
        a.selected = native_lua_get_by_string_00b68100(*a.animated, a.selected_cell, requested);
        a.unwind_state = 5; a.native_site = 0x00c30680;
        a.fps = native_lua_get_by_name_00b67800(*a.selected, a.fps_cell, c.actual_fps_00d5d1f8);
        a.unwind_state = 6; a.native_site = 0x00c3068f;
        actual.rate_04 = native_lua_number_00b66270(*a.fps);
        a.unwind_state = 5; a.native_site = 0x00c306a6;
        destroy_native_lua_object_00b67700(*a.fps);
        a.native_site = 0x00c306b9;
        a.table = native_lua_get_by_name_00b67800(*a.selected, a.textures_cell, c.actual_textures_00d0d9b0);
        a.unwind_state = 7; a.native_site = 0x00c306ca;
        a.key = construct_native_lua_object_00b65f50(a.key_cell);
        a.unwind_state = 8; a.native_site = 0x00c306db;
        a.value = construct_native_lua_object_00b65f50(a.value_cell);
        a.unwind_state = 9; a.native_site = 0x00c306f6;
        native_lua_iterate_first_00b67080(*a.table, *a.key, *a.value);
        a.native_site = 0x00c30704;
        while (!native_lua_is_unbound_00b66420(*a.value)) {
            a.native_site = 0x00c30715;
            if (native_lua_is_integer_number_00b66a60(*a.key)) {
                a.native_site = 0x00c30726;
                a.borrowed_value = native_lua_string_00b662b0(*a.value);
                zero_child_name(a.scratch);
                const auto length = scan_native_length(a.borrowed_value);
                a.native_site = 0x00c3074a;
                resize_native_string_header_0041dd40(&a.scratch, c.raw_strings, length, true);
                char* const copied = current_data(a.scratch);
                if (copied) {
                    const auto copied_size = current_length(a.scratch) + 1u;
                    a.native_site = 0x00c30761;
                    std::memmove(copied, a.borrowed_value, copied_size);
                }
                // Source-only persistent diagnostics, before current capture.
                // Caller admits host metadata allocation; no engine credit.
                a.children.push_back(std::make_unique<NativeTextureSourceChildAcquired>());
                auto& child = *a.children.back();
                a.native_site = 0x00c30769;
                child.captured_renderer = const_cast<void*>(c.textures.textures.current_renderer_00f8d394);
                child.captured_profile = *static_cast<const volatile std::uint32_t*>(child.captured_renderer);
                if (child.captured_profile == 0x00d5f0a8u) {
                    child.captured_target = c.actual_renderer_profile_00d5f0a8[0x64 / 4];
                    child.target_captured = true;
                }
                a.unwind_state = 10; a.native_site = 0x00c30782;
                if (!child.target_captured || child.captured_target != 0x00b319b0u)
                    throw std::invalid_argument("unsupported captured texture-source renderer slot64");
                child.call_started = true;
                child.result = load_native_renderer_texture_00b319b0(
                    child.captured_renderer, &a.scratch, 0, c.textures, &child.cache);
                child.call_returned = true;
                char* const returned_name = current_data(a.scratch);
                a.unwind_state = 9;
                if (returned_name) {
                    const auto size = current_length(a.scratch) + 1u;
                    child.name_return_started = true; a.native_site = 0x00c307a1;
                    return_captured(returned_name, size, c.raw_strings);
                    child.name_return_completed = true;
                }
                child.append_started = true; a.native_site = 0x00c307ad;
                append_native_texture_source_child_00c307ad_fragment(payload, child.result);
                child.append_completed = true;
            }
            a.native_site = 0x00c307f4;
            native_lua_iterate_next_00b67190(*a.table, *a.key, *a.value);
            a.native_site = 0x00c30802;
        }
        a.native_site = 0x00c30813; actual.selected_00 = 0;
        a.unwind_state = 8; a.native_site = 0x00c3081e;
        destroy_native_lua_object_00b67700(*a.value);
        a.unwind_state = 7; a.native_site = 0x00c3082f;
        destroy_native_lua_object_00b67700(*a.key);
        a.unwind_state = 5; a.native_site = 0x00c30840;
        destroy_native_lua_object_00b67700(*a.table);
        a.unwind_state = 4; a.native_site = 0x00c30851;
        destroy_native_lua_object_00b67700(*a.selected);
        a.unwind_state = 0; a.native_site = 0x00c30861;
        destroy_native_lua_object_00b67700(*a.animated);
        a.unwind_state = -1; a.native_site = 0x00c30878;
        close_native_lua_state_00b669a0(*a.owner);
        a.phase = NativeTextureSourceProducerAcquired::Phase::complete;
    } catch (...) {
        a.caught_cpp_exception = true;
        throw; // Metadata/rethrow only; no classification, Lua API or cleanup.
    }
}

} // namespace bsp
