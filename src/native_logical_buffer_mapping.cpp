#include "bsp/native_logical_buffer_mapping.hpp"
#include "bsp/native_physical_buffer_access.hpp"

namespace bsp {
namespace {
static_assert(sizeof(void*) == 4);
void* at(const void* base, std::uint32_t offset) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(base) + offset);
}
std::uint32_t load(const void* address) noexcept {
    std::uint32_t value;
    __asm {
        mov eax, address
        mov eax, dword ptr [eax]
        mov value, eax
    }
    return value;
}
void store(void* address, std::uint32_t value) noexcept {
    __asm {
        mov eax, address
        mov edx, value
        mov dword ptr [eax], edx
    }
}
void* pointer(const void* base, std::uint32_t offset = 0) noexcept {
    return reinterpret_cast<void*>(load(at(base, offset)));
}
struct Guard {
    explicit Guard(NativeLogicalBufferMappingContext& c) : context(c) {
        if (context.actual_synchronization_0108d6dc.mode_00) {
            saved.renderer_04 = context.actual_renderer_00f8d394;
            saved.entered_00 = enter_native_renderer_optional_guard_00b33ad0(
                saved.renderer_04, context.actual_synchronization_0108d6dc);
        }
    }
    ~Guard() {
        if (!finished)
            destroy_native_renderer_optional_guard_00b21110(saved,
                context.actual_synchronization_0108d6dc);
    }
    void finish(std::uint8_t current_mode) {
        finished = true; // Native clears its EH state before ordinary Leave.
        if (current_mode)
            leave_native_renderer_optional_guard_00b33b00(saved.renderer_04,
                saved.entered_00, context.actual_synchronization_0108d6dc);
    }
    NativeLogicalBufferMappingContext& context;
    NativeRendererOptionalGuardStorage saved; // Native skipped-entry preimage.
    bool finished{false};
};
using Profile = const volatile std::uint32_t*;
Profile profile(const void* physical, NativeLogicalBufferMappingContext& context) noexcept {
    const auto token = load(physical);
    const auto& p = context.actual_physical_profiles;
    if (token == 0x00d61e10u) return p.private_index_00d61e10;
    if (token == 0x00d61e34u) return p.private_vertex_00d61e34;
    if (token == 0x00d61e58u) return p.pooled_index_00d61e58;
    if (token == 0x00d61e7cu) return p.pooled_vertex_00d61e7c;
    __assume(0);
}
void* lock_physical(void* physical, NativeLogicalBufferMappingContext& context,
    std::uint32_t bytes, std::uint32_t offset, void* output, std::uint8_t read_only) {
    const auto target = profile(physical, context)[2];
    if (target == 0x00b4b850u)
        return lock_native_physical_index_buffer_00b4b850(physical,
            context.actual_physical_lock, bytes, offset, 1, output, read_only);
    if (target == 0x00b4ba00u)
        return lock_native_physical_vertex_buffer_00b4ba00(physical,
            context.actual_physical_lock, bytes, offset, 1, output, read_only);
    __assume(0);
}
void unlock_physical(void* physical, NativeLogicalBufferMappingContext& context) {
    const auto target = profile(physical, context)[3];
    if (target == 0x00b4b820u) {
        unlock_native_physical_index_buffer_00b4b820(physical);
        return;
    }
    if (target == 0x00b4b9d0u) {
        unlock_native_physical_vertex_buffer_00b4b9d0(physical);
        return;
    }
    __assume(0);
}
std::uint32_t index_stride(std::uint32_t format) noexcept {
    if (format == 0x65u) return 2;
    if (format == 0x66u) return 4;
    return 0;
}
} // namespace

void* __fastcall lock_native_logical_vertex_stream_00b49980(void* logical,
    NativeLogicalBufferMappingContext& context, std::uint32_t count,
    std::uint32_t offset, std::uint8_t read_only) {
    Guard guard(context);
    void* physical;
    if ((load(at(logical, 0x60)) & 0xf000u) == 0x1000u &&
        (physical = pointer(logical, 0x58)) != nullptr) {
        void* const declaration = pointer(logical, 0x68);
        store(at(logical, 0x64), count);
        const auto stride = load(at(declaration, 0xcc));
        void* const mapped = lock_physical(physical, context, stride * count,
            stride * offset, at(logical, 0x5c), 0);
        store(at(logical, 8), reinterpret_cast<std::uint32_t>(mapped));
    } else if ((physical = pointer(logical, 0x58)) != nullptr) {
        const auto stride = load(at(pointer(logical, 0x68), 0xcc));
        auto bytes = stride * count;
        if (bytes == 0) bytes = load(at(logical, 0x64)) * stride;
        const auto byte_offset = (load(at(logical, 0x70)) + offset) * stride;
        void* const mapped = lock_physical(physical, context, bytes, byte_offset,
            &count, read_only); // Native reuses its incoming count stack cell.
        store(at(logical, 8), reinterpret_cast<std::uint32_t>(mapped));
    }
    // Native tests current mode BEFORE capturing+08, then leaves with the
    // captured renderer. Leave itself reads the current mode/lock again.
    const auto leave_mode = context.actual_synchronization_0108d6dc.mode_00;
    void* const result = pointer(logical, 8);
    guard.finish(leave_mode);
    return result;
}

void* __fastcall lock_native_logical_index_stream_00b49b60(void* logical,
    NativeLogicalBufferMappingContext& context, std::uint32_t count,
    std::uint32_t offset, std::uint8_t read_only) {
    Guard guard(context);
    void* mapped = nullptr;
    if (pointer(logical, 8)) {
        const auto format = load(at(logical, 0x18));
        auto bytes = index_stride(format) * count;
        if (bytes == 0) bytes = load(at(logical, 0x14)) * index_stride(format);
        void* const physical = pointer(logical, 8);
        const auto byte_offset = (load(at(logical, 0x20)) + offset) * index_stride(format);
        mapped = lock_physical(physical, context, bytes, byte_offset,
            at(logical, 0xc), read_only);
    }
    guard.finish(context.actual_synchronization_0108d6dc.mode_00);
    return mapped;
}

void __fastcall unlock_native_logical_vertex_stream_00b49a80(void* logical,
    NativeLogicalBufferMappingContext& context) {
    Guard guard(context);
    if (void* const physical = pointer(logical, 0x58)) unlock_physical(physical, context);
    store(at(logical, 8), 0); // AFTER the physical callback, even if it changed+08.
    guard.finish(context.actual_synchronization_0108d6dc.mode_00);
}
void __fastcall unlock_native_logical_index_stream_00b49c70(void* logical,
    NativeLogicalBufferMappingContext& context) {
    Guard guard(context);
    if (pointer(logical, 8)) unlock_physical(pointer(logical, 8), context);
    guard.finish(context.actual_synchronization_0108d6dc.mode_00);
}
} // namespace bsp
