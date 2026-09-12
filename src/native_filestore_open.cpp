#include "bsp/native_filestore_open.hpp"
#include "bsp/native_adopted_substream.hpp"

#include "bsp/native_memory_stream.hpp"
#include "bsp/native_physical_stream_conversion.hpp"
#include "bsp/native_physical_stream_open.hpp"
#include "bsp/native_vfs_date_leaf_providers.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <stdexcept>

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native FileStore open requires MSVC Win32.
#endif

namespace bsp {
namespace {
void* at(void* base, std::uint32_t offset) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(base) + offset);
}
const void* at(const void* base, std::uint32_t offset) noexcept {
    return reinterpret_cast<const void*>(reinterpret_cast<std::uintptr_t>(base) + offset);
}
std::uint32_t word(const void* base, std::uint32_t offset = 0) noexcept {
    return *static_cast<const volatile std::uint32_t*>(at(base, offset));
}
void* pointer(const void* base, std::uint32_t offset = 0) noexcept {
    return reinterpret_cast<void*>(word(base, offset));
}
void put(void* base, std::uint32_t offset, std::uint32_t value) noexcept {
    *static_cast<volatile std::uint32_t*>(at(base, offset)) = value;
}
void require_memory_slot(const void* owner, std::uint32_t offset,
    std::uint32_t identity, NativeRetainedMemoryOwnerContext& context) noexcept {
    const auto profile = word(owner);
    __assume(profile == 0x00d642c0);
    const auto slot = context.actual_stream_profile_00d642c0[offset / 4];
    __assume(slot == identity);
}
using TypeEntry = bool (__fastcall*)(void*, void*, std::uint32_t);
using SeekEntry = void (__fastcall*)(void*, void*, std::uint32_t,
    std::uint32_t, std::uint32_t);
using LengthEntry = std::int64_t (__fastcall*)(void*, void*);
using ReadEntry = void (__fastcall*)(void*, void*, void*, std::uint32_t,
    std::uint32_t*);

void require_physical_slot(std::uint32_t profile, std::uint32_t offset,
    std::uint32_t identity, const NativeStoredStreamConversionContext& context) {
    if (!context.physical || !context.actual_physical_type_ids_0109dc30)
        throw std::invalid_argument("Native physical stream conversion requires physical services and type IDs");
    if (word(reinterpret_cast<const void*>(profile), offset) != identity)
        throw std::invalid_argument("Unimplemented native physical stream conversion method");
}
void require_substream_slot(std::uint32_t profile, std::uint32_t offset,
    std::uint32_t identity, const NativeStoredStreamConversionContext& context) {
    if (!context.adopted_substreams || !context.actual_file_type_ids_0109db58)
        throw std::invalid_argument("Native adopted substream conversion requires dispatch and file IDs");
    if (word(reinterpret_cast<const void*>(profile), offset) != identity)
        throw std::invalid_argument("Unimplemented native adopted substream conversion method");
}

bool current_type_query(void* owner, std::uint32_t token,
    NativeStoredStreamConversionContext& context) {
    const auto profile = word(owner);
    if (profile == 0x00d642c0)
        return dispatch_native_memory_stream_type_query(owner, token, context);
    if (profile == 0x00d691b0) {
        require_physical_slot(profile, 0x0c, 0x00bf4ff0, context);
        return query_native_physical_stream_type_00bf4ff0(token,
            context.actual_physical_type_ids_0109dc30);
    }
    if (profile == 0x00d68db0) {
        require_substream_slot(profile, 0x0c, 0x00bb8b80, context);
        return query_native_file_stream_type_00bb8b80(token,
            context.actual_file_type_ids_0109db58);
    }
    auto* const target = pointer(reinterpret_cast<void*>(profile), 0x0c);
    return reinterpret_cast<TypeEntry>(target)(owner, target, token);
}
void current_seek_zero(void* owner, NativeStoredStreamConversionContext& context) {
    const auto profile = word(owner);
    if (profile == 0x00d642c0) {
        dispatch_native_memory_stream_seek(owner, 0, 0, 0, context.memory_owners);
        return;
    }
    if (profile == 0x00d691b0) {
        require_physical_slot(profile, 0x1c, 0x00bf4f20, context);
        (void)seek_native_physical_stream_00bf4f20(owner, 0, 0, 0);
        return;
    }
    if (profile == 0x00d68db0) {
        require_substream_slot(profile, 0x1c, 0x00bf10e0, context);
        (void)seek_native_adopted_substream_00bf10e0(owner, 0, 0, 0,
            *context.adopted_substreams);
        return;
    }
    auto* const target = pointer(reinterpret_cast<void*>(profile), 0x1c);
    reinterpret_cast<SeekEntry>(target)(owner, target, 0, 0, 0);
}
std::uint32_t current_length_low(void* owner,
    NativeStoredStreamConversionContext& context) {
    const auto profile = word(owner);
    if (profile == 0x00d642c0)
        return static_cast<std::uint32_t>(dispatch_native_memory_stream_length(
            owner, context.memory_owners));
    if (profile == 0x00d691b0) {
        require_physical_slot(profile, 0x30, 0x00bf4f90, context);
        return static_cast<std::uint32_t>(size_native_physical_stream_00bf4f90(owner));
    }
    if (profile == 0x00d68db0) {
        require_substream_slot(profile, 0x30, 0x00bf10a0, context);
        return static_cast<std::uint32_t>(length_native_adopted_substream_00bf10a0(owner));
    }
    auto* const target = pointer(reinterpret_cast<void*>(profile), 0x30);
    return static_cast<std::uint32_t>(reinterpret_cast<LengthEntry>(target)(owner, target));
}
void current_read_once(void* owner, void* data, std::uint32_t count,
    NativeStoredStreamConversionContext& context) {
    const auto profile = word(owner);
    if (profile == 0x00d642c0) {
        dispatch_native_memory_stream_read(owner, data, count, nullptr, context.memory_owners);
        return;
    }
    if (profile == 0x00d691b0) {
        require_physical_slot(profile, 0x24, 0x00bf5030, context);
        (void)read_native_physical_stream_00bf5030(owner, data, count, nullptr,
            *context.physical);
        return;
    }
    if (profile == 0x00d68db0) {
        require_substream_slot(profile, 0x24, 0x00bf1000, context);
        (void)read_native_adopted_substream_00bf1000(owner, data, count, nullptr,
            *context.adopted_substreams);
        return;
    }
    auto* const target = pointer(reinterpret_cast<void*>(profile), 0x24);
    reinterpret_cast<ReadEntry>(target)(owner, target, data, count, nullptr);
}
struct RawBackingConstructionCleanup {
    void* raw;
    bool armed = true;
    ~RawBackingConstructionCleanup() noexcept {
        if (armed) singleton_lifetime_free(raw);
    }
};
} // namespace

bool query_native_memory_stream_type_00bb8f60(std::uint32_t token,
    const volatile std::uint32_t* ids) noexcept {
    for (unsigned i = 0; i != 3; ++i)
        if (ids[i] == token) return true;
    return false;
}

__declspec(naked) void __fastcall native_memory_stream_seek_00bef540(
    void*, void*, std::uint32_t, std::uint32_t, std::uint32_t) noexcept {
    __asm {
        mov eax, dword ptr [esp + 0ch]
        test eax, eax
        jnz seek_current
        mov eax, dword ptr [ecx + 8]
        mov eax, dword ptr [eax + 8]
        mov edx, dword ptr [esp + 4]
        add edx, eax
        mov dword ptr [ecx + 10h], edx
        ret 0ch
    seek_current:
        cmp eax, 1
        jnz seek_end
        mov eax, dword ptr [ecx + 10h]
        mov edx, dword ptr [esp + 4]
        add edx, eax
        mov dword ptr [ecx + 10h], edx
        ret 0ch
    seek_end:
        mov eax, dword ptr [ecx + 0ch]
        mov edx, dword ptr [esp + 4]
        add edx, eax
        mov dword ptr [ecx + 10h], edx
        ret 0ch
    }
}

bool dispatch_native_memory_stream_type_query(void* owner, std::uint32_t token,
    NativeStoredStreamConversionContext& context) noexcept {
    require_memory_slot(owner, 0x0c, 0x00bb8f60, context.memory_owners);
    return query_native_memory_stream_type_00bb8f60(token,
        context.actual_memory_type_ids_0109dba0);
}
void dispatch_native_memory_stream_seek(void* owner, std::uint32_t low,
    std::uint32_t high, std::uint32_t origin,
    NativeRetainedMemoryOwnerContext& context) noexcept {
    require_memory_slot(owner, 0x1c, 0x00bef540, context);
    native_memory_stream_seek_00bef540(owner, nullptr, low, high, origin);
}

void* convert_native_stored_stream_00bef750(void* source,
    NativeStoredStreamConversionContext& context) {
    if (!source) return nullptr;
    const auto memory_type = context.actual_memory_type_ids_0109dba0[0];
    if (current_type_query(source, memory_type, context))
        return create_native_memory_stream_from_backing_00bef6d0(
            pointer(source, 8), context.memory_owners);

    current_seek_zero(source, context);
    const auto low = current_length_low(source, context);
    auto* const raw = singleton_lifetime_allocate({
        SingletonAllocationKind::object, 0x10, 0x10});
    void* backing = nullptr;
    {
        // E02228 state0 -> E02220 -> CC7650 frees only the raw construction
        // allocation. Native state becomes -1 before calling read at BEF80B.
        RawBackingConstructionCleanup cleanup{raw};
        if (raw) backing = construct_native_memory_backing_008d43c0(
            raw, static_cast<std::int32_t>(low), context.memory_owners);
        cleanup.armed = false;
    }
    current_read_once(source, pointer(backing, 8), low, context);
    auto* const result = create_native_memory_stream_from_backing_00bef6d0(
        backing, context.memory_owners);
    if (InterlockedDecrement(static_cast<volatile LONG*>(at(backing, 4))) == 0)
        dispatch_native_memory_owner_zero_reference(backing, context.memory_owners);
    return result;
}

void* find_native_file_store_open_name_00be5e90(void* tree, void* output,
    const void* name, const SingletonLifetimeCallbacks& invalid_parameters) {
    auto* node = lower_bound_native_file_store_name_00be54d0(tree, name);
    if (!tree) invalid_parameters.invalid_parameter(invalid_parameters.context);
    if (node == pointer(tree, 4) ||
        less_native_string_headers_00443d00(name, at(node, 0x0c)))
        node = pointer(tree, 4);
    const auto owner_word = reinterpret_cast<std::uint32_t>(tree);
    const auto node_word = reinterpret_cast<std::uint32_t>(node);
    put(output, 0, owner_word);
    put(output, 4, node_word);
    return output;
}

void* open_native_file_store_00be5fa0(void* store, const void* name,
    std::uint32_t flags, const SingletonLifetimeCallbacks& invalid_parameters,
    NativeStoredStreamConversionContext& context) {
    if ((flags & 1) != 0) return nullptr;
    auto* const tree = at(store, 0x14);
    NativeFileStoreNameIterator iterator;
    find_native_file_store_open_name_00be5e90(tree, &iterator, name, invalid_parameters);
    auto* const owner = pointer(&iterator);
    auto* const head = pointer(tree, 4);
    if (!owner || owner != tree)
        invalid_parameters.invalid_parameter(invalid_parameters.context);
    auto* const node = pointer(&iterator, 4);
    if (node == head) return nullptr;
    // The diagnostic target 4254B0 is a single RET. Preserve the current name
    // pointer read; selecting fallback address109DB6C never reads its bytes.
    const auto name_data = word(name, 4);
    (void)name_data;
    if (!owner) invalid_parameters.invalid_parameter(invalid_parameters.context);
    if (node == pointer(owner, 4))
        invalid_parameters.invalid_parameter(invalid_parameters.context);
    return convert_native_stored_stream_00bef750(pointer(node, 0x14), context);
}
} // namespace bsp
