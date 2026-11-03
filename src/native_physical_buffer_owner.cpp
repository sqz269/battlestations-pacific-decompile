#include "bsp/native_physical_buffer_owner.hpp"

#include <cstring>
#include <type_traits>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native physical-buffer owners require MSVC Win32.
#endif

namespace bsp {
namespace {
static_assert(sizeof(void*) == 4);
static_assert(sizeof(NativeString) == 8);
static_assert(std::is_trivially_destructible_v<NativeString>);

void* at(const void* base, std::uint32_t offset) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(base) + offset);
}
volatile std::uint32_t& word(const void* base, std::uint32_t offset = 0) noexcept {
    return *static_cast<volatile std::uint32_t*>(at(base, offset));
}
volatile std::uint16_t& half(const void* base, std::uint32_t offset) noexcept {
    return *static_cast<volatile std::uint16_t*>(at(base, offset));
}
void* pointer(const void* base, std::uint32_t offset = 0) noexcept {
    return reinterpret_cast<void*>(word(base, offset));
}
std::int32_t signed_bits(std::uint32_t bits) noexcept {
    std::int32_t value;
    std::memcpy(&value, &bits, sizeof(value));
    return value;
}
void base_profile(void* owner) noexcept { word(owner) = 0x00ceb130u; }
void* construct(void* owner, std::uint32_t profile) noexcept {
    base_profile(owner);
    word(owner, 4) = 1;
    for (std::uint32_t offset = 8; offset != 0x24; offset += 4)
        word(owner, offset) = 0;
    word(owner, 0x28) = 0;
    word(owner, 0x24) = 0;
    word(owner) = profile;
    return owner;
}
void support(NativePhysicalBufferOwnerContext& context) {
    (void)resource_support_singleton_00b3e730(context.actual_support_0108fedc,
        context.actual_lifetime_01090aa0);
}
using ComReference = ULONG (STDMETHODCALLTYPE*)(void*);
void com_reference(void* com, std::uint32_t slot) {
    const void* const table = pointer(com);
    const auto call = reinterpret_cast<ComReference>(word(table, slot));
    (void)call(com);
}
struct DiagnosticRecord {
    void* borrowed_com;
    NativeString name;
};
static_assert(sizeof(DiagnosticRecord) == 12);
static_assert(offsetof(DiagnosticRecord, name) == 4);
constexpr char buffer_label[] = "Vertex or Index Buffer"; // D61EA0,22+NUL.

void attach(void* owner, void* com, std::uint32_t flags,
    std::uint32_t capacity, NativePhysicalBufferOwnerContext& context) {
    word(owner, 0x14) = flags;
    word(owner, 0x18) = capacity;
    void* const old = pointer(owner, 0x28);
    if (old != com) {
        word(owner, 0x28) = reinterpret_cast<std::uintptr_t>(com);
        if (com) com_reference(com, 4);
        if (old) com_reference(old, 8);
    }

    NativeString temporary;
    auto& storage = context.actual_string_storage;
    resize_native_string_header_0041dd40(&temporary, storage, 0x16, true);
    auto* const captured_data = static_cast<char*>(pointer(&temporary, 4));
    const auto captured_length = word(&temporary);
    if (captured_data) std::memcpy(captured_data, buffer_label, captured_length + 1u);

    // Native state0 begins only after the first construction/copy. Capture
    // current COM into the diagnostic record before initializing its string.
    DiagnosticRecord record{pointer(owner, 0x28), {}};
    try {
        resize_native_string_header_0041dd40(&record.name, storage, captured_length, true);
        auto* const record_data = static_cast<char*>(pointer(&record.name, 4));
        if (captured_length != 0) {
            const auto copied = word(&record.name);
            if (copied != 0) std::memcpy(record_data, captured_data, copied);
        }
        try { // State1 only after the complete second string copy.
            support(context);
        } catch (...) {
            destroy_native_buffer_diagnostic_record_00b3f4c0(&record, storage);
            throw;
        }
        // State0: normal record cleanup uses captured data, current length.
        if (record_data) storage.release(record_data, word(&record.name) + 1u);
    } catch (...) {
        // State0 EH reads the CURRENT first header, unlike normal cleanup.
        destroy_native_string_header_0041dd20(&temporary, storage);
        throw;
    }
    // State-1 before final normal release: both pointer and length captured.
    if (captured_data) storage.release(captured_data, captured_length + 1u);
}
void destroy_base(void* owner) {
    try {
        void* const array = at(owner, 8);
        resize_native_physical_buffer_pointer_array_00b49700(array, 0);
        singleton_lifetime_free(pointer(array));
    } catch (...) {
        base_profile(owner);
        throw;
    }
    base_profile(owner);
}
void unwind_destroy_base(void* owner) noexcept {
    // Original FH3 invokes this base cleanup as an unwind action. A second
    // C++ exception terminates; normal base destruction remains throwable.
    destroy_base(owner);
}
void destroy(void* owner, std::uint32_t profile, NativePhysicalBufferOwnerContext& context) {
    word(owner) = profile;
    try {
        support(context);
        if (void* const com = pointer(owner, 0x28)) {
            com_reference(com, 8);
            word(owner, 0x28) = 0;
        }
    } catch (...) {
        unwind_destroy_base(owner);
        throw;
    }
    destroy_base(owner);
}
} // namespace

void* construct_native_physical_index_buffer_00b4bb60(void* owner) noexcept {
    return construct(owner, 0x00d61e58u);
}
void* construct_native_physical_vertex_buffer_00b4bbb0(void* owner) noexcept {
    return construct(owner, 0x00d61e7cu);
}
void attach_native_physical_index_buffer_00b4c250(void* owner,
    IDirect3DIndexBuffer9* com, std::uint32_t flags, std::uint32_t capacity,
    NativePhysicalBufferOwnerContext& context) { attach(owner, com, flags, capacity, context); }
void attach_native_physical_vertex_buffer_00b4c370(void* owner,
    IDirect3DVertexBuffer9* com, std::uint32_t flags, std::uint32_t capacity,
    NativePhysicalBufferOwnerContext& context) { attach(owner, com, flags, capacity, context); }

void destroy_native_buffer_diagnostic_record_00b3f4c0(void* record,
    NativeStringStorage& storage) noexcept {
    if (auto* const data = static_cast<char*>(pointer(record, 8)))
        storage.release(data, word(record, 4) + 1u);
}
void destroy_native_physical_index_buffer_base_00b4b490(void* owner) { destroy_base(owner); }
void destroy_native_physical_vertex_buffer_base_00b4b530(void* owner) { destroy_base(owner); }
void destroy_native_physical_index_buffer_00b4b900(void* owner,
    NativePhysicalBufferOwnerContext& context) { destroy(owner, 0x00d61e10u, context); }
void destroy_native_physical_vertex_buffer_00b4bab0(void* owner,
    NativePhysicalBufferOwnerContext& context) { destroy(owner, 0x00d61e34u, context); }

void* delete_native_private_index_buffer_00b4bb20(void* owner, std::uint32_t flags,
    NativePhysicalBufferOwnerContext& context) {
    destroy_native_physical_index_buffer_00b4b900(owner, context);
    if ((flags & 1u) != 0) singleton_lifetime_free(owner);
    return owner;
}
void* delete_native_private_vertex_buffer_00b4bb40(void* owner, std::uint32_t flags,
    NativePhysicalBufferOwnerContext& context) {
    destroy_native_physical_vertex_buffer_00b4bab0(owner, context);
    if ((flags & 1u) != 0) singleton_lifetime_free(owner);
    return owner;
}
void* delete_native_pooled_index_buffer_00b4c210(void* owner, std::uint32_t flags,
    NativePhysicalBufferOwnerContext& context) {
    destroy_native_physical_index_buffer_00b4b900(owner, context);
    if ((flags & 1u) != 0) return_native_physical_buffer_slot_00b49500(
        context.actual_index_pool_0108fda8, owner);
    return owner;
}
void* delete_native_pooled_vertex_buffer_00b4c230(void* owner, std::uint32_t flags,
    NativePhysicalBufferOwnerContext& context) {
    destroy_native_physical_vertex_buffer_00b4bab0(owner, context);
    if ((flags & 1u) != 0) return_native_physical_buffer_slot_00b49500(
        context.actual_vertex_pool_0108fde0, owner);
    return owner;
}

void return_native_physical_buffer_slot_00b49500(void* pool, void* slot) {
    auto* const section = static_cast<CRITICAL_SECTION*>(at(pool, 0x0c));
    EnterCriticalSection(section);
    word(section, 0x18) = word(section, 0x18) + 1u;
    const auto slab_index = word(slot, 0x2c);
    void* const slabs = pointer(pool, 0x28);
    void* const slab = pointer(slabs, slab_index * 4u);
    const auto delta = reinterpret_cast<std::uintptr_t>(slot) -
        reinterpret_cast<std::uintptr_t>(slab);
    const auto index = signed_bits(delta) / 0x30;
    const auto old_free_count = half(slab, 0x640);
    half(slab, 0x600u + static_cast<std::uint32_t>(old_free_count) * 2u) =
        static_cast<std::uint16_t>(index);
    half(slab, 0x640) = static_cast<std::uint16_t>(half(slab, 0x640) + 1u);
    if (slab_index < word(pool, 0x34)) word(pool, 0x34) = slab_index;
    word(section, 0x18) = word(section, 0x18) - 1u;
    LeaveCriticalSection(section);
}

void reserve_native_physical_buffer_pointer_array_00b496a0(void* header,
    std::uint32_t requested) {
    if (signed_bits(requested) < 1) requested = 1;
    if (signed_bits(word(header, 8)) >= signed_bits(requested)) return;
    const auto bytes = requested * 4u;
    void* const replacement = singleton_lifetime_allocate(
        {SingletonAllocationKind::object, bytes, bytes});
    std::uint32_t index = 0;
    while (signed_bits(index) < signed_bits(word(header, 4))) {
        void* const destination = at(replacement, index * 4u);
        if (destination) word(destination) = word(pointer(header), index * 4u);
        ++index;
    }
    singleton_lifetime_free(pointer(header));
    word(header) = reinterpret_cast<std::uintptr_t>(replacement);
    word(header, 8) = requested;
}
void resize_native_physical_buffer_pointer_array_00b49700(void* header,
    std::uint32_t requested) {
    if (signed_bits(requested) > signed_bits(word(header, 8)))
        reserve_native_physical_buffer_pointer_array_00b496a0(header, requested);
    auto index = word(header, 4);
    while (signed_bits(index) < signed_bits(requested)) {
        if (void* const destination = at(pointer(header), index * 4u))
            word(destination) = 0;
        ++index;
    }
    while (signed_bits(requested) < signed_bits(word(header, 4)))
        word(header, 4) = word(header, 4) - 1u;
    word(header, 4) = requested;
}
} // namespace bsp
