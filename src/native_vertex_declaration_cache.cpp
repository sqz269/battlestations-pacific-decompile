#include "bsp/native_vertex_declaration_cache.hpp"

#include "bsp/native_alias_count_growth.hpp"
#include "bsp/native_physical_file_date.hpp"
#include "bsp/native_pooled_resource_path.hpp"
#include "bsp/native_render_resource_alias_nodes.hpp"
#include "bsp/native_render_resource_record_construction.hpp"
#include "bsp/native_string_pool_storage.hpp"
#include "bsp/native_vertex_declaration_loading.hpp"
#include "bsp/native_vfs_date_route.hpp"
#include "bsp/resource_load_events.hpp"
#include "bsp/singleton_lifetime.hpp"

#include <Windows.h>
#include <cstring>
#include <new>
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native declaration cache requires MSVC Win32.
#endif

namespace bsp {
namespace {
static_assert(sizeof(NativeRenderResourceRecord) == 0x2c);
static_assert(offsetof(NativeRenderResourceRecord, sentinel_0c) == 0xc);
static_assert(offsetof(NativeRenderResourceRecord, resource_28) == 0x28);
struct Name { std::uint32_t length; char* data; };
static_assert(sizeof(Name) == 8);
template<class T> volatile T& field(void* p, std::size_t offset) {
    return *reinterpret_cast<volatile T*>(static_cast<unsigned char*>(p) + offset);
}
template<class T> const volatile T& field(const void* p, std::size_t offset) {
    return *reinterpret_cast<const volatile T*>(static_cast<const unsigned char*>(p) + offset);
}
void* plus(void* p, std::uint32_t offset) {
    return reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(p) + offset);
}
std::int32_t signed_bits(std::uint32_t v) {
    std::int32_t result;
    std::memcpy(&result, &v, 4);
    return result;
}
void copy_name(void* destination, const void* source, ActualNativeStringPoolStorage& strings) {
    if (destination == source) return;
    resize_native_string_header_0041dd40(destination, strings,
        field<std::uint32_t>(source, 0), true);
    if (field<std::uint32_t>(source, 0) != 0) {
        const auto count = field<std::uint32_t>(destination, 0);
        const auto* input = field<char*>(source, 4);
        auto* output = field<char*>(destination, 4);
        if (count != 0) std::memcpy(output, input, count);
    }
}
struct NameCleanup {
    void* name;
    ActualNativeStringPoolStorage& strings;
    bool armed = true;
    ~NameCleanup() { if (armed) destroy_native_string_header_0041dd20(name, strings); }
};
bool string_equal(const void* a, const void* b, bool require_length) {
    const auto left = field<std::uint32_t>(a, 0);
    const auto right = field<std::uint32_t>(b, 0);
    if (require_length && left != right) return false;
    if (left == 0) return right == 0;
    if (right == 0) return false;
    return _stricmp(field<char*>(a, 4), field<char*>(b, 4)) == 0;
}
void append_alias(void* list, const void* name, NativeVertexDeclarationCacheContext& c) {
    auto* const sentinel = field<NativeRenderResourceAliasNode*>(list, 4);
    auto* const previous = field<NativeRenderResourceAliasNode*>(sentinel, 4);
    auto* const node = allocate_native_render_alias_node_004ce6f0(
        sentinel, previous, name, c.strings);
    grow_native_alias_list_count_004ce780(list, 1);
    // Native count failure leaks the allocated but unlinked node.
    field<NativeRenderResourceAliasNode*>(sentinel, 4) = node;
    field<NativeRenderResourceAliasNode*>(field<NativeRenderResourceAliasNode*>(node, 4), 0) = node;
}
void require_slot(void* registry, NativeVertexDeclarationCacheContext& c,
    unsigned slot, std::uint32_t target) {
    if (field<std::uint32_t>(registry, 0) != 0x00d5f060u ||
        c.registry_vtable_00d5f060[slot] != target)
        throw std::invalid_argument("unimplemented current declaration registry slot");
}
void require_domains(NativeVertexDeclarationCacheContext& c) {
    if (&c.strings != &c.declarations.strings || &c.strings != &c.dates.physical.strings)
        throw std::invalid_argument("declaration cache requires the same actual string domain");
}
} // namespace

NativeRenderResourceRecord& copy_construct_native_declaration_record_00b2fbb0(
    NativeRenderResourceRecord& destination, const NativeRenderResourceRecord& source,
    ActualNativeStringPoolStorage& strings, const SingletonLifetimeCallbacks& callbacks) {
    volatile auto& d = destination;
    const volatile auto& s = source;
    d.name_length_00 = 0;
    d.name_data_04 = nullptr;
    copy_name(&destination, &source, strings);
    try {
        copy_construct_native_render_alias_list_004d48a0(
            plus(&destination, 8), reinterpret_cast<const unsigned char*>(&source) + 8,
            strings, callbacks);
        for (unsigned i = 0; i != 5; ++i) d.payload_14_24[i] = s.payload_14_24[i];
        d.resource_28 = s.resource_28;
    } catch (...) {
        destroy_native_string_header_0041dd20(&destination, strings);
        throw;
    }
    return destination;
}

void destroy_native_declaration_record_00b2f910(NativeRenderResourceRecord& record,
    ActualNativeStringPoolStorage& strings) {
    char* captured;
    try {
        destroy_native_render_alias_list_004d0a10(plus(&record, 8), strings);
        captured = field<char*>(&record, 4);
    } catch (...) {
        destroy_native_string_header_0041dd20(&record, strings);
        throw;
    }
    if (captured != nullptr) strings.release(captured, field<std::uint32_t>(&record, 0) + 1u);
}

void reserve_native_declaration_records_00b2fe20(void* header, std::uint32_t requested,
    NativeVertexDeclarationCacheContext& c) {
    if (signed_bits(requested) < 64) requested = 64;
    if (signed_bits(field<std::uint32_t>(header, 8)) >= signed_bits(requested)) return;
    auto* const replacement = c.allocate_array_00bf55be(requested * 0x2cu);
    std::uint32_t i = 0;
    while (signed_bits(i) < signed_bits(field<std::uint32_t>(header, 4))) {
        auto* p = plus(replacement, i * 0x2cu);
        if (p != nullptr) {
            auto* const destination = ::new (p) NativeRenderResourceRecord;
            const auto* const source = static_cast<NativeRenderResourceRecord*>(
                plus(field<void*>(header, 0), i * 0x2cu));
            copy_construct_native_declaration_record_00b2fbb0(*destination, *source,
                c.strings, c.validation);
        }
        ++i;
    }
    // CBD940's only call is the no-op placement-delete00401130. In particular,
    // a failed copy neither frees replacement nor destroys completed records.
    i = 0;
    while (signed_bits(i) < signed_bits(field<std::uint32_t>(header, 4))) {
        auto* const record = static_cast<NativeRenderResourceRecord*>(
            plus(field<void*>(header, 0), i * 0x2cu));
        destroy_native_declaration_record_00b2f910(*record, c.strings);
        ++i;
    }
    c.free_array_00bf6989(field<void*>(header, 0));
    field<void*>(header, 0) = replacement;
    field<std::uint32_t>(header, 8) = requested;
}

void append_native_declaration_record_00b300c0(void* header,
    const NativeRenderResourceRecord& source, NativeVertexDeclarationCacheContext& c) {
    const auto capacity = field<std::uint32_t>(header, 8);
    if (field<std::uint32_t>(header, 4) == capacity) {
        const auto doubled = capacity + capacity;
        reserve_native_declaration_records_00b2fe20(header,
            signed_bits(doubled) > 64 ? doubled : 64u, c);
    }
    auto* const p = plus(field<void*>(header, 0), field<std::uint32_t>(header, 4) * 0x2cu);
    if (p != nullptr) {
        auto* const destination = ::new (p) NativeRenderResourceRecord;
        try {
            copy_construct_native_declaration_record_00b2fbb0(*destination, source,
                c.strings, c.validation);
        } catch (...) {
            // CBD9D0 reloads the current begin/count before a no-op00401130.
            (void)plus(field<void*>(header, 0), field<std::uint32_t>(header, 4) * 0x2cu);
            throw;
        }
    }
    field<std::uint32_t>(header, 4) = field<std::uint32_t>(header, 4) + 1u;
}

void* copy_native_declaration_resolved_name_00b2c280(void* output,
    const void* name, ActualNativeStringPoolStorage& strings) {
    field<std::uint32_t>(output, 0) = 0;
    field<char*>(output, 4) = nullptr;
    copy_name(output, name, strings);
    return output;
}

void* acquire_native_cached_declaration_00b31d20(void* declaration) noexcept {
    InterlockedIncrement(&field<LONG>(declaration, 4));
    return declaration;
}

void* load_native_cached_vertex_declaration_00b305f0(void* registry, const void* name,
    std::uint32_t loader_word, std::uint8_t acquire_new, std::uint8_t allow_load,
    NativeVertexDeclarationCacheContext& c) {
    require_domains(c);
    pump_resource_load_events_00beccd0(*c.platform_0109cf04);
    Name normalized{0, nullptr};
    copy_name(&normalized, name, c.strings);
    NameCleanup normalized_cleanup{&normalized, c.strings};
    normalize_native_resource_path_header_00bee690(&normalized, c.strings);
    void* cached = nullptr;
    auto* row = field<void*>(registry, 4);
    auto* const end = plus(row, field<std::uint32_t>(registry, 8) * 0x2cu);
    while (row != end) {
        auto* const captured_sentinel = field<NativeRenderResourceAliasNode*>(row, 0xc);
        auto* node = field<NativeRenderResourceAliasNode*>(captured_sentinel, 0);
        while (node != captured_sentinel) {
            if (node == field<NativeRenderResourceAliasNode*>(row, 0xc))
                c.validation.invalid_parameter(c.validation.context);
            if (string_equal(plus(node, 8), &normalized, true)) {
                cached = field<void*>(row, 0x28);
                break;
            }
            if (node == field<NativeRenderResourceAliasNode*>(row, 0xc))
                c.validation.invalid_parameter(c.validation.context);
            node = field<NativeRenderResourceAliasNode*>(node, 0);
        }
        if (cached != nullptr) break;
        row = plus(row, 0x2c);
    }
    Name resolved{0, nullptr};
    NameCleanup resolved_cleanup{&resolved, c.strings};
    if (cached != nullptr) {
        require_slot(registry, c, 3, 0x00b31d20);
        return acquire_native_cached_declaration_00b31d20(cached);
    }
    // B2C280 ignores the forwarded loader word, as does B2DBD0. Preserve it in
    // this interface because it is an original B305F0 stack argument.
    (void)loader_word;
    Name output;
    require_slot(registry, c, 1, 0x00b2c280);
    auto* const returned = copy_native_declaration_resolved_name_00b2c280(
        &output, &normalized, c.strings);
    {
        NameCleanup output_cleanup{&output, c.strings};
        copy_name(&resolved, returned, c.strings);
    }
    normalize_native_resource_path_header_00bee690(&resolved, c.strings);
    if (!string_equal(&resolved, &normalized, false)) {
        row = field<void*>(registry, 4);
        auto* const resolved_end = plus(row, field<std::uint32_t>(registry, 8) * 0x2cu);
        while (row != resolved_end) {
            auto* const sentinel = field<NativeRenderResourceAliasNode*>(row, 0xc);
            auto* const first = field<NativeRenderResourceAliasNode*>(sentinel, 0);
            if (first == sentinel) c.validation.invalid_parameter(c.validation.context);
            // Only the first alias participates in this SECOND search.
            if (string_equal(plus(first, 8), &resolved, true)) {
                append_alias(plus(row, 8), &normalized, c);
                cached = field<void*>(row, 0x28);
                break;
            }
            row = plus(row, 0x2c);
        }
        if (cached != nullptr) {
            require_slot(registry, c, 3, 0x00b31d20);
            return acquire_native_cached_declaration_00b31d20(cached);
        }
    }
    if (allow_load == 0) return nullptr;
    require_slot(registry, c, 2, 0x00b2dbd0);
    void* const loaded = decode_native_vertex_declaration_00b2dbd0(&resolved, c.declarations);
    NativeRenderResourceRecord record; // Native leaves +08 and +28 unset here.
    record.name_length_00 = 0;
    record.name_data_04 = nullptr;
    {
        NameCleanup name_cleanup{&record, c.strings};
        record.sentinel_0c = allocate_native_render_alias_sentinel_004c3020();
        record.alias_count_10 = 0;
        for (unsigned i = 5; i != 0; --i) record.payload_14_24[i - 1] = 0;
        name_cleanup.armed = false;
    }
    bool record_cleanup_armed = true;
    try {
        copy_name(&record, &resolved, c.strings);
        append_alias(plus(&record, 8), &resolved, c);
        std::uint32_t date[5];
        auto* const returned_date = query_native_vfs_file_date_00bdd340(
            c.dates.physical.manager_0109ceec, date, &resolved, c.dates);
        for (unsigned i = 0; i != 5; ++i)
            record.payload_14_24[i] = field<std::uint32_t>(returned_date, i * 4u);
        if (!string_equal(&normalized, &resolved, false)) append_alias(plus(&record, 8), &normalized, c);
        record.resource_28 = loaded;
        append_native_declaration_record_00b300c0(plus(registry, 4), record, c);
        if (loaded != nullptr) field<std::uint32_t>(registry, 0x10) =
            field<std::uint32_t>(registry, 0x10) + field<std::uint32_t>(loaded, 0xcc);
        void* result = loaded;
        if (acquire_new != 0 && loaded != nullptr) {
            require_slot(registry, c, 3, 0x00b31d20);
            result = acquire_native_cached_declaration_00b31d20(loaded);
        }
        record_cleanup_armed = false; // Native state1 before B2F910 call.
        destroy_native_declaration_record_00b2f910(record, c.strings);
        return result;
    } catch (...) {
        if (record_cleanup_armed) destroy_native_declaration_record_00b2f910(record, c.strings);
        throw;
    }
}

void* load_native_renderer_vertex_declaration_00b317e0(void* renderer,
    const void* name, NativeVertexDeclarationCacheContext& c) {
    Name lower{0, nullptr};
    copy_name(&lower, name, c.strings);
    char* const captured_data = lower.data;
    NameCleanup cleanup{&lower, c.strings};
    lowercase_native_string_header_004bcc00(&lower);
    auto* const result = load_native_cached_vertex_declaration_00b305f0(
        plus(renderer, 0x1a60), &lower, 0, 1, 1, c);
    cleanup.armed = false;
    if (captured_data != nullptr) c.strings.release(captured_data, lower.length + 1u);
    return result;
}
} // namespace bsp
