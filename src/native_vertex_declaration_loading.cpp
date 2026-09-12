#include "bsp/native_vertex_declaration_loading.hpp"
#include "bsp/native_vertex_declaration_owner.hpp"
#include "bsp/native_string.hpp"
#include "bsp/native_string_compare.hpp"
#include "bsp/native_physical_file_date.hpp"
#include "bsp/native_pooled_string_substring.hpp"
#include "bsp/singleton_lifetime.hpp"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <atomic>
#include <cstring>
#include <exception>
#include <new>
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native vertex declaration loading requires MSVC Win32.
#endif

namespace bsp {
namespace {
static_assert(sizeof(void*) == 4);
static_assert(sizeof(CRITICAL_SECTION) == 24);
std::uint32_t word(const volatile void* base, std::uint32_t byte_offset = 0) noexcept {
    std::uint32_t result;
    __asm {
        mov eax, base
        mov edx, byte_offset
        mov eax, dword ptr [eax + edx]
        mov result, eax
    }
    return result;
}
void put(void* base, std::uint32_t byte_offset, std::uint32_t value) noexcept {
    __asm {
        mov eax, base
        mov edx, byte_offset
        mov ecx, value
        mov dword ptr [eax + edx], ecx
    }
}
std::uint16_t half(const void* base, std::uint32_t byte_offset) noexcept {
    std::uint16_t result;
    __asm {
        mov eax, base
        mov edx, byte_offset
        mov ax, word ptr [eax + edx]
        mov result, ax
    }
    return result;
}
void put_half(void* base, std::uint32_t byte_offset, std::uint16_t value) noexcept {
    __asm {
        mov eax, base
        mov edx, byte_offset
        mov cx, value
        mov word ptr [eax + edx], cx
    }
}
std::int32_t signed_word(std::uint32_t value) noexcept {
    std::int32_t result;
    std::memcpy(&result, &value, sizeof(result));
    return result;
}
void* pointer(std::uint32_t bits) noexcept { return reinterpret_cast<void*>(bits); }
void* at(const void* base, std::uint32_t offset) noexcept {
    return pointer(reinterpret_cast<std::uintptr_t>(base) + offset);
}
char* string_data(const void* header) noexcept {
    return static_cast<char*>(pointer(word(header, 4)));
}
struct Alias { const char* name; const char* encoded; };
// Literal pairings verified at B2DC32..B2E2B8. Sequence below is the first
// chain; the three direct terrain rewrites and final four tests follow it.
constexpr Alias primary_aliases[] = {
    {"simpleindexed.mvfm", "pf43nf43uf42if41.mvfm"},
    {"color.mvfm", "pf43cc.mvfm"},
    {"position.mvfm", "pf43.mvfm"},
    {"oceanheightmapgen.mvfm", "pf44uf42.mvfm"},
    {"oceannormalmapgen.mvfm", "pf44uf42uf42uf42uf42uf42.mvfm"},
    {"oceanheightmap.mvfm", "nf24.mvfm"},
    {"simplecolor.mvfm", "pf43ccuf42.mvfm"},
    {"simplecolor2.mvfm", "pf44ccccuf42uf42.mvfm"},
    {"ship.mvfm", "pf43nf43uf42uf42tf43bf43.mvfm"},
    {"shipvc.mvfm", "pf43nf43uf42uf42tf43bf43cc.mvfm"},
    {"gun.mvfm", "pf43nf43uf42.mvfm"},
    {"gunvc.mvfm", "pf43nf43uf42cc.mvfm"},
    {"gunvcindexed.mvfm", "pf43nf43uf42ccif41.mvfm"},
    {"ocean.mvfm", "pf43nf43uf43.mvfm"},
    {"projocean.mvfm", "pf42.mvfm"},
    {"projoceanfull.mvfm", "pf42nc.mvfm"},
    {"terraindx9ati.mvfm", "pcpc.mvfm"},
    {"shadowmap.mvfm", "pf43nf43uf42.mvfm"},
    {"airplane.mvfm", "pf43nf43uf42tf43bf43.mvfm"},
    {"airplaneindexed.mvfm", "pf43nf43uf42tf43bf43if41.mvfm"},
    {"transformpps.mvfm", "pf43.mvfm"},
    {"particlesprite.mvfm", "pf43ccuf41uf41uf42uf42.mvfm"},
    {"particleaxial.mvfm", "pf43ccuf43uf42uf42.mvfm"},
    {"particlefloating.mvfm", "pf44ccuf24uf42.mvfm"},
    {"particleaxialsprite.mvfm", "pf44ccuf44uf24uf42uf22.mvfm"},
    {"coast.mvfm", "pf43.mvfm"},
    {"beam.mvfm", "pf43uf43cc.mvfm"},
    {"waterparticle.mvfm", "cc.mvfm"},
    {"watertracer.mvfm", "pf43ccuf42.mvfm"},
    {"choppy.mvfm", "pf43nf43cc.mvfm"},
    {"cloud.mvfm", "pf43ccuf42uf42.mvfm"},
    {"impostor.mvfm", "pf43uf42.mvfm"},
    {"skined.mvfm", "pf43nf43uf42wf44if44.mvfm"},
    {"skinedUW.mvfm", "pf43nf43tf43bf43uf42wf44if44.mvfm"},
    {"FoliageNoInst.mvfm", "pf43nf43uf42uf41cf43.mvfm"},
    {"rope.mvfm", "pf43nf43uf42uf41.mvfm"},
    {"FullscreenQuad.mvfm", "pf44uf42uf42uf42uf42.mvfm"},
    {"lightning.mvfm", "pf43uf41uf42.mvfm"},
    {"foliagesprite.mvfm", "pf43uf42ccuf43.mvfm"},
    {"traceline.mvfm", "pf43uf43ccuf44uf41.mvfm"},
    {"airfield.mvfm", "pf43nf43uf42uf42.mvfm"},
    {"watertracerskined.mvfm", "pf44if43wf43uf44uf42.mvfm"},
    {"watertracerskinedstatic.mvfm", "pf44uf42uf42.mvfm"},
    {"waterspray.mvfm", "pf43uf42uf44uf43.mvfm"},
    {"bterrain2.mvfm", "pf43nf43uf42uf42.mvfm"},
    {"bterrain3.mvfm", "pf43nf43uf42uf42uf42.mvfm"},
};
constexpr Alias terrain_aliases[] = {
    {"bterrain4.mvfm", "pf43nf43uf42uf42uf42uf42.mvfm"},
    {"uterrain3.mvfm", "pf43nf43uf42uf42uf42cc.mvfm"},
    {"uterrain4.mvfm", "pf43nf43uf42uf42uf42uf42cc.mvfm"},
};
constexpr Alias final_aliases[] = {
    {"shore.mvfm", "pf43uf42cc.mvfm"},
    {"river.mvfm", "pf43nf43tf43bf43uf42cc.mvfm"},
    {"foliagevc.mvfm", "pf43nf43uf42cc.mvfm"},
    {"decal.mvfm", "pf43nf43uf44uf42.mvfm"},
};
constexpr const char* usages[] = {"p", "w", "i", "n", "u", "t", "b", "c"};
constexpr std::uint32_t usage_values[] = {0, 1, 2, 3, 5, 6, 7, 10};
constexpr const char* types[] = {"f41", "f42", "f43", "f44", "c", "ub4",
    "ss2", "ss4", "ubn4", "ssn2", "ssn4", "usn2", "usn4", "ud3",
    "sdn3", "f22", "f24"};

struct StringCleanup {
    void* header;
    NativeStringStorage& strings;
    bool armed = true;
    ~StringCleanup() noexcept {
        if (armed) destroy_native_string_header_0041dd20(header, strings);
    }
};

void expand_aliases(void* name, NativeStringStorage& strings) {
    if (string_data(name) && _stricmp(string_data(name), "simple.mvfm") == 0) {
        resize_native_string_header_0041dd40(name, strings, 17, false);
        if (auto* data = string_data(name))
            std::memcpy(data, "pf43nf43uf42.mvfm", word(name));
    } else {
        for (const auto& alias : primary_aliases) {
            if (equal_native_string_header_00425850(name, alias.name)) {
                assign_native_string_cstring_0041e350(name, alias.encoded, strings);
                break;
            }
        }
    }
    // Native direct stricmp tests retain ESI between calls and reload only
    // after an actual rewrite. The last rewrite jumps past the final chain.
    auto* captured = string_data(name);
    if (captured) {
        for (unsigned i = 0; i != 3; ++i) {
            const auto& alias = terrain_aliases[i];
            if (_stricmp(captured, alias.name) == 0) {
                const auto length = static_cast<std::uint32_t>(std::strlen(alias.encoded));
                resize_native_string_header_0041dd40(name, strings, length, false);
                captured = string_data(name);
                if (!captured) break;
                std::memcpy(captured, alias.encoded, word(name));
                if (i == 2) return;
            }
        }
    }
    for (const auto& alias : final_aliases) {
        if (equal_native_string_header_00425850(name, alias.name)) {
            assign_native_string_cstring_0041e350(name, alias.encoded, strings);
            break;
        }
    }
}

void initialize_tokens(NativeVertexDeclarationLoadingContext& context,
    void* table, std::uint32_t bit, unsigned count, const char* const* literals,
    const std::uint32_t* values, std::uint32_t shutdown) {
    if ((context.initialized_0108d6d8 & bit) != 0) return;
    context.initialized_0108d6d8 |= bit;
    unsigned completed = 0;
    try {
        for (; completed != count; ++completed) {
            auto* const entry = at(table, completed * 12u);
            construct_native_string_cstring_0041e870(entry, literals[completed], context.strings);
            put(entry, 8, values ? values[completed] : completed);
        }
    } catch (...) {
        // Native state is advanced BEFORE the constructor, so a failing
        // current entry is not destroyed. Only the completed prefix unwinds.
        while (completed != 0)
            destroy_native_string_header_0041dd20(at(table, --completed * 12u), context.strings);
        context.initialized_0108d6d8 &= ~bit;
        throw;
    }
    if (!context.register_atexit)
        throw std::invalid_argument("native vertex-format atexit domain is unbound");
    (void)context.register_atexit(context.atexit_context, shutdown);
}

bool same_token(const void* value, const void* token) noexcept {
    const auto length = word(value);
    const auto token_length = word(token);
    if (length != token_length) return false;
    if (length == 0) return true;
    return _stricmp(string_data(value), string_data(token)) == 0;
}

std::uint32_t find_token(const void* name, std::uint32_t cursor, void* table,
    const volatile std::int32_t& count, NativeStringStorage& strings) {
    std::uint32_t index = 0;
    if (count <= 0) return 0xffffffffu;
    auto* entry = table;
    do {
        std::uint32_t substring[2];
        construct_native_string_substring_00469840(name, substring, cursor, word(entry), strings);
        const bool equal = same_token(substring, entry);
        destroy_native_string_header_0041dd20(substring, strings);
        if (equal) return index;
        ++index;
        entry = at(entry, 12);
    } while (signed_word(index) < count);
    return 0xffffffffu;
}

void* construct_allocated(void* slot, void* pool) {
    if (!slot) return nullptr;
    try {
        return construct_native_vertex_declaration_00b48af0(slot);
    } catch (...) {
        // Native state1A action CBD6AA -> B47C00 -> B47950.
        return_native_vertex_declaration_slot_00b47950(pool, slot);
        throw;
    }
}

void delete_failed_declaration(void* owner, NativeVertexDeclarationLoadingContext& context) {
    if (!owner) return;
    if (word(owner) != 0x00d61d1c || !context.declaration_vtable_00d61d1c ||
        context.declaration_vtable_00d61d1c[1] != 0x00b48ca0)
        throw std::invalid_argument("unimplemented current vertex declaration scalar-delete profile");
    delete_native_vertex_declaration_00b48ca0(owner, 1, context.pool_0108fd38,
        context.type_sizes_00d61cc0);
}

std::atomic<std::int32_t>& actual_references(void* owner) {
    if (!owner) throw std::invalid_argument("native vertex declaration owner is null");
    return *std::launder(reinterpret_cast<std::atomic<std::int32_t>*>(at(owner, 4)));
}
} // namespace

void* initialize_native_vertex_declaration_slab_00b47680(void* slab, std::uint32_t index) noexcept {
    put_half(slab, 0x1ac0, 32);
    for (std::uint32_t i = 0; i != 32; ++i) {
        put_half(slab, 0x1a80u + i * 2u, static_cast<std::uint16_t>(31u - i));
        put(slab, 0xd0u + i * 0xd4u, index);
    }
    return slab;
}

void* allocate_native_vertex_declaration_slot_00b48560(void* pool) {
    auto* const critical_section = reinterpret_cast<CRITICAL_SECTION*>(at(pool, 0x0c));
    EnterCriticalSection(critical_section);
    put(pool, 0x24, word(pool, 0x24) + 1u);
    if (word(pool, 0x34) == 0xffffffffu) {
        put(pool, 0x34, word(pool, 0x2c));
        auto* raw = singleton_lifetime_allocate({SingletonAllocationKind::object, 0x1ac4, 0x1ac4});
        auto* const slab = raw ? initialize_native_vertex_declaration_slab_00b47680(raw, word(pool, 0x34)) : nullptr;
        const auto capacity = word(pool, 0x30);
        if (word(pool, 0x2c) == capacity) {
            const auto next_capacity = capacity * 2u + 2u;
            put(pool, 0x30, next_capacity);
            const auto bytes = next_capacity * 4u;
            auto* const replacement = singleton_lifetime_allocate({
                SingletonAllocationKind::pointer_slots, bytes, bytes});
            auto* destination = replacement;
            std::uint32_t index = 0;
            while (index < word(pool, 0x2c)) {
                if (destination) put(destination, 0, word(pointer(word(pool, 0x28)), index * 4u));
                ++index;
                destination = at(destination, 4);
            }
            if (auto* const old = pointer(word(pool, 0x28))) singleton_lifetime_free(old);
            // B485F6 ADD ESP,4 returns from free; the old decompile omitted it.
            put(pool, 0x28, reinterpret_cast<std::uintptr_t>(replacement));
        }
        auto* const entry = pointer(word(pool, 0x28) + word(pool, 0x2c) * 4u);
        if (entry) put(entry, 0, reinterpret_cast<std::uintptr_t>(slab));
        put(pool, 0x2c, word(pool, 0x2c) + 1u);
    }
    auto* const slab = pointer(word(pointer(word(pool, 0x28)), word(pool, 0x34) * 4u));
    put_half(slab, 0x1ac0, static_cast<std::uint16_t>(half(slab, 0x1ac0) - 1u));
    const auto remaining = half(slab, 0x1ac0);
    auto* const slot = at(slab, static_cast<std::uint32_t>(half(slab,
        0x1a80u + static_cast<std::uint32_t>(remaining) * 2u)) * 0xd4u);
    if (remaining == 0) {
        auto index = word(pool, 0x34) + 1u;
        put(pool, 0x34, 0xffffffffu);
        if (index < word(pool, 0x2c)) {
            auto* entry = pointer(word(pool, 0x28) + index * 4u);
            do {
                if (half(pointer(word(entry)), 0x1ac0) != 0) {
                    put(pool, 0x34, index);
                    break;
                }
                ++index;
                entry = at(entry, 4);
            } while (index < word(pool, 0x2c));
        }
    }
    put(pool, 0x24, word(pool, 0x24) - 1u);
    LeaveCriticalSection(critical_section);
    return slot;
}

void* allocate_native_vertex_declaration_slot_00b488c0(void* pool) {
    return allocate_native_vertex_declaration_slot_00b48560(pool);
}

void append_native_vertex_element_00b47d90(void* header, const void* record) {
    const auto capacity = word(header, 8);
    if (word(header, 4) == capacity) {
        auto next = capacity * 2u;
        if (signed_word(next) <= 1) next = 1;
        reserve_native_vertex_elements_00b47a30(header, signed_word(next));
    }
    auto* const destination = pointer(word(header) + word(header, 4) * 20u);
    if (destination) {
        put(destination, 0, word(record));
        put(destination, 4, word(record, 4));
        put(destination, 8, word(record, 8));
        put(destination, 12, word(record, 12));
        put(destination, 16, word(record, 16));
    }
    put(header, 4, word(header, 4) + 1u);
}

void recompute_native_vertex_declaration_stride_00b47d20(void* owner,
    const volatile std::uint32_t* type_sizes) noexcept {
    const auto count = signed_word(word(owner, 0x10));
    put(owner, 0xcc, 0);
    if (count <= 0) return;
    auto* cursor = pointer(word(owner, 0x0c) + 4u);
    std::uint32_t index = 0;
    do {
        const auto size = word(type_sizes, word(cursor) * 4u);
        put(owner, 0xcc, word(owner, 0xcc) + size);
        ++index;
        cursor = at(cursor, 20);
    } while (signed_word(index) < signed_word(word(owner, 0x10)));
}

void append_native_vertex_declaration_00b48330(void* owner, std::uint32_t type,
    std::uint32_t usage, std::uint32_t offset, const volatile std::uint32_t* type_sizes) {
    if (offset == 0xffffffffu) offset = word(owner, 8);
    const std::uint32_t record[] = {offset, type, 0, usage, 0xffffffffu};
    const auto size = word(type_sizes, type * 4u);
    put(owner, 8, word(owner, 8) + size);
    append_native_vertex_element_00b47d90(at(owner, 0x0c), record);
    append_native_vertex_element_00b47d90(at(owner, 0x18u + usage * 12u), record);
    recompute_native_vertex_declaration_stride_00b47d20(owner, type_sizes);
}

void* decode_native_vertex_declaration_00b2dbd0(const void* source,
    NativeVertexDeclarationLoadingContext& context) {
    std::uint32_t name[2]{};
    copy_native_string_header_00be0a30_fragment(name, context.strings, source);
    // The original state remains -1 until the input copy has fully returned.
    StringCleanup name_cleanup{name, context.strings};
    expand_aliases(name, context.strings);
    initialize_tokens(context, context.usages_0108d678, 1, 8, usages, usage_values, 0x00ce0c30);
    initialize_tokens(context, context.types_0108d5a8, 2, 17, types, nullptr, 0x00ce0c10);
    auto* const slot = allocate_native_vertex_declaration_slot_00b488c0(context.pool_0108fd38);
    auto* const owner = construct_allocated(slot, context.pool_0108fd38);
    // State0 again: parser exceptions retain the allocated declaration.
    std::uint32_t cursor = 0;
    bool invalid = false;
    while (cursor < word(name) && string_data(name)[cursor] != '.') {
        const auto usage_index = find_token(name, cursor, context.usages_0108d678,
            context.usage_count_00e13074, context.strings);
        if (usage_index == 0xffffffffu) { invalid = true; break; }
        auto* const usage = at(context.usages_0108d678, usage_index * 12u);
        const auto usage_length = word(usage);
        const auto usage_value = word(usage, 8);
        cursor += usage_length;
        const auto type_index = find_token(name, cursor, context.types_0108d5a8,
            context.type_count_00e13070, context.strings);
        if (type_index == 0xffffffffu) { invalid = true; break; }
        auto* const type = at(context.types_0108d5a8, type_index * 12u);
        const auto type_value = word(type, 8);
        cursor += word(type);
        append_native_vertex_declaration_00b48330(owner, type_value, usage_value,
            0xffffffffu, context.type_sizes_00d61cc0);
    }
    if (!invalid) {
        std::uint32_t suffix[2]{};
        resize_native_string_header_0041dd40(suffix, context.strings, 5, true);
        auto* const suffix_data = string_data(suffix);
        const auto suffix_length = word(suffix);
        if (suffix_data) std::memcpy(suffix_data, ".mvfm", suffix_length + 1u);
        StringCleanup suffix_cleanup{suffix, context.strings};
        std::uint32_t tail[2];
        construct_native_string_substring_00469840(name, tail, cursor, 0x7fffffffu, context.strings);
        StringCleanup tail_cleanup{tail, context.strings};
        const bool equal = word(tail) == 0 ? suffix_length == 0 :
            (suffix_length != 0 && _stricmp(string_data(tail), suffix_data) == 0);
        // B47900 is exactly MOV EAX,[ECX+10]; RET. No length equality test
        // is added before suffix stricmp, unlike token comparisons above.
        invalid = !equal || word(owner, 0x10) == 0;
        destroy_native_string_header_0041dd20(tail, context.strings);
        tail_cleanup.armed = false;
        // Native normal path uses ESI/EDI captured before suffix substring.
        if (suffix_data) context.strings.release(suffix_data, suffix_length + 1u);
        suffix_cleanup.armed = false;
    }
    if (invalid) {
        delete_failed_declaration(owner, context);
        return nullptr;
    }
    return owner;
}

void destroy_native_vertex_format_types_00ce0c10(NativeVertexDeclarationLoadingContext& context) noexcept {
    for (unsigned i = 17; i != 0;)
        destroy_native_string_header_0041dd20(at(context.types_0108d5a8, --i * 12u), context.strings);
}
void destroy_native_vertex_format_usages_00ce0c30(NativeVertexDeclarationLoadingContext& context) noexcept {
    for (unsigned i = 8; i != 0;)
        destroy_native_string_header_0041dd20(at(context.usages_0108d678, --i * 12u), context.strings);
}

NativeVertexDeclarationReference::NativeVertexDeclarationReference(void* owner, void* pool,
    const volatile std::uint32_t* type_sizes, const volatile std::uint32_t* vtable,
    NativeVertexDeclarationCompanionDisposal disposal)
    : RenderCommandReference(actual_references(owner)), storage_(owner), pool_(pool),
      type_sizes_(type_sizes), vtable_(vtable), disposal_(disposal) {
    if (!pool || !type_sizes || !disposal.retire ||
        reference_count.load(std::memory_order_relaxed) <= 0)
        throw std::invalid_argument("native vertex declaration requires live storage and canonical retirement");
    require_current_profile();
}
NativeVertexDeclarationReference::~NativeVertexDeclarationReference() {
    if (phase_ != Phase::retired) std::terminate();
}
void NativeVertexDeclarationReference::require_current_profile() const noexcept {
    if (word(storage_) != 0x00d61d1c || !vtable_ ||
        vtable_[0] != 0x00bd30e0 || vtable_[1] != 0x00b48ca0) std::terminate();
}
void NativeVertexDeclarationReference::release_zero_references() noexcept {
    if (phase_ != Phase::bound) std::terminate();
    require_current_profile();
    phase_ = Phase::destroying;
    const auto disposal = disposal_;
    delete_native_vertex_declaration_00b48ca0(storage_, 1, pool_, type_sizes_);
    phase_ = Phase::retired;
    disposal.retire(disposal.context, *this);
}
} // namespace bsp
