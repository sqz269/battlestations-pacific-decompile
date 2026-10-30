#include "bsp/native_lua_type_name.hpp"
#include "bsp/native_lua_objects.hpp"
#include "bsp/native_string.hpp"

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <intrin.h>
extern "C" {
#include <lua.h>
}

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Raw Lua type-name construction requires MSVC Win32.
#endif

namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == 4 && sizeof(std::uintptr_t) == 4 &&
    sizeof(Word) == 4 && sizeof(int) == 4 && sizeof(std::int32_t) == 4);
static_assert(offsetof(NativeLuaObjectStorage, owner_00) == 0 &&
    offsetof(NativeLuaObjectStorage, kind_04) == 4 &&
    offsetof(NativeLuaObjectStorage, index_08) == 8 &&
    offsetof(NativeLuaStateStorage, state_04) == 4);

struct ExpectedSpan { std::uintptr_t address; Word extent; };
ExpectedSpan expected(NativeLuaTypeNameRole role) {
    switch (role) {
    case NativeLuaTypeNameRole::none: return {0x00ce43ecu, 5u};
    case NativeLuaTypeNameRole::nil: return {0x00ce4380u, 4u};
    case NativeLuaTypeNameRole::boolean: return {0x00d62c2cu, 5u};
    case NativeLuaTypeNameRole::light_userdata: return {0x00d62c1cu, 14u};
    case NativeLuaTypeNameRole::number: return {0x00d58560u, 7u};
    case NativeLuaTypeNameRole::string: return {0x00cf8334u, 7u};
    case NativeLuaTypeNameRole::table: return {0x00cf8344u, 6u};
    case NativeLuaTypeNameRole::function: return {0x00d04d44u, 9u};
    case NativeLuaTypeNameRole::userdata: return {0x00d58568u, 9u};
    case NativeLuaTypeNameRole::thread: return {0x00d58558u, 7u};
    case NativeLuaTypeNameRole::unknown: return {0x00ceb808u, 8u};
    }
    throw std::invalid_argument("Lua type-name context has an unknown RO role");
}
Word word(const void* base, Word offset) noexcept {
    const auto address = reinterpret_cast<std::uintptr_t>(base) + offset;
    Word result;
    _ReadWriteBarrier();
    std::memcpy(&result, reinterpret_cast<const void*>(address), sizeof result);
    _ReadWriteBarrier();
    return result;
}
} // namespace

NativeLuaTypeNameContext::NativeLuaTypeNameContext(NativeStringRawPoolContext& strings,
    const std::array<NativeLuaTypeNameSpan, 11>& spans)
    : strings_(strings), spans_(spans) {
    for (std::size_t i = 0; i < spans_.size(); ++i) {
        const auto& span = spans_[i];
        const auto role = static_cast<NativeLuaTypeNameRole>(i);
        const auto actual = expected(role);
        if (span.role != role || span.original_address != actual.address ||
            span.extent_including_nul != actual.extent ||
            reinterpret_cast<std::uintptr_t>(span.data) != actual.address)
            throw std::invalid_argument("Lua type-name context requires exact actual RO span metadata");
    }
}

void* construct_native_lua_type_name_00b68460(void* fresh, std::int32_t type,
    const NativeLuaTypeNameContext& context) {
    const Word selector = static_cast<Word>(type) + 1u;
    std::size_t selected = 10; // actual unknown branch
    if (selector <= 9u) {
        switch (selector) {
        case 0u: selected = 0; break;
        case 1u: selected = 1; break;
        case 2u: selected = 2; break;
        case 3u: selected = 3; break;
        case 4u: selected = 4; break;
        case 5u: selected = 5; break;
        case 6u: selected = 6; break;
        case 7u: selected = 7; break;
        case 8u: selected = 8; break;
        case 9u: selected = 9; break;
        }
    }
    construct_native_string_header_0041e870(fresh, context.strings_, context.spans_[selected].data);
    return fresh; // Original SAME output address, no invented ownership receipt
}

void* construct_native_lua_object_type_name_00b68550(const NativeLuaObjectStorage& object,
    void* fresh, const NativeLuaTypeNameContext& context) {
    const Word kind = word(&object, 4u); // exactly ONCE, before all owner/index/state reads
    if (kind == 0u) return construct_native_lua_type_name_00b68460(fresh, -1, context);
    if (kind == 2u) {
        const Word owner_bits = word(&object, 0u);
        const auto* const owner = reinterpret_cast<const NativeLuaStateStorage*>(
            static_cast<std::uintptr_t>(owner_bits));
        const Word state_bits = word(owner, 4u);
        auto* const state = reinterpret_cast<lua_State*>(static_cast<std::uintptr_t>(state_bits));
        const Word index_bits = word(&object, 8u); // AFTER the owner's CURRENT state
        std::int32_t index;
        std::memcpy(&index, &index_bits, sizeof index); // preserve signed DWORD bits
        const int type = lua_type(state, index); // linked real API, exactly ONCE
        return construct_native_lua_type_name_00b68460(fresh, type, context);
    }
    return construct_native_lua_type_name_00b68460(fresh, 5, context);
}
} // namespace bsp
