#include "bsp/native_hardware_layout_construct.hpp"
#include "bsp/native_physical_buffer_owner.hpp"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <d3d9.h>

#include <cstddef>
#include <cstring>
#include <exception>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native hardware layout construction requires MSVC Win32.
#endif

namespace bsp {
namespace {
static_assert(sizeof(void*) == 4);
static_assert(sizeof(D3DVERTEXELEMENT9) == 8);

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
std::uint32_t half(const void* base, std::uint32_t byte_offset = 0) noexcept {
    std::uint32_t result;
    __asm {
        mov eax, base
        mov edx, byte_offset
        movzx eax, word ptr [eax + edx]
        mov result, eax
    }
    return result;
}
std::uint32_t byte(const void* base, std::uint32_t byte_offset = 0) noexcept {
    std::uint32_t result;
    __asm {
        mov eax, base
        mov edx, byte_offset
        movzx eax, byte ptr [eax + edx]
        mov result, eax
    }
    return result;
}
void* pointer(std::uint32_t value) noexcept { return reinterpret_cast<void*>(value); }
void* at(const void* base, std::uint32_t byte_offset) noexcept {
    return pointer(reinterpret_cast<std::uintptr_t>(base) + byte_offset);
}
std::int32_t signed_word(std::uint32_t value) noexcept {
    std::int32_t result;
    std::memcpy(&result, &value, sizeof(result));
    return result;
}

// Native frame+30h..E8h: 14 usage counters, 16 packed eight-byte slots, live
// output count immediately after the slots. Unused output bytes stay untouched.
struct Scratch {
    std::uint32_t usage_counts[14];
    std::uint32_t elements[16][2];
    std::uint32_t count;
};
static_assert(offsetof(Scratch, elements) == 0x38);
static_assert(offsetof(Scratch, count) == 0xb8);
static_assert(sizeof(Scratch) == 0xbc);
constexpr char vertex_format[] = "VertexFormat"; // D61BD0, 12 characters + NUL.

struct StringCleanup {
    void* temporary;
    void* diagnostic;
    NativeStringStorage& storage;
    unsigned state = 0;
    bool armed = true;
    ~StringCleanup() noexcept {
        if (!armed) return;
        if (state != 0)
            destroy_native_buffer_diagnostic_record_00b3f4c0(diagnostic, storage);
        destroy_native_string_header_0041dd20(temporary, storage);
    }
};

void diagnostic(void* owner, NativeHardwareLayoutConstructContext& context) {
    std::uint32_t temporary[2];
    std::uint32_t record[3];
    auto& storage = context.actual_string_storage;
    put(temporary, 0, 0);
    put(temporary, 4, 0);
    resize_native_string_header_0041dd40(temporary, storage, 12, true);
    auto* const captured_data = static_cast<char*>(pointer(word(temporary, 4)));
    const auto captured_length = word(temporary);
    if (captured_data)
        std::memcpy(captured_data, vertex_format, captured_length + 1u);

    put(record, 0, word(owner, 0x40));
    StringCleanup cleanup{temporary, record, storage}; // Native state0 here.
    put(record, 4, 0);
    put(record, 8, 0);
    resize_native_string_header_0041dd40(at(record, 4), storage, captured_length, true);
    auto* const record_data = static_cast<char*>(pointer(word(record, 8)));
    const auto record_length = word(record, 4);
    if (captured_length != 0 && record_length != 0)
        std::memcpy(record_data, captured_data, record_length);
    cleanup.state = 1;
    (void)resource_support_singleton_00b3e730(
        context.actual_owner.actual_support_0108fedc,
        context.actual_owner.actual_lifetime_01090aa0);
    cleanup.state = 0;
    // Normal release uses the captured pointers AND lengths. EH uses current
    // actual headers through B3F4C0/41DD20 instead.
    if (record_data) storage.release(record_data, record_length + 1u);
    cleanup.armed = false;
    if (captured_data) storage.release(captured_data, captured_length + 1u);
}

int terminate_cpp_cleanup_exception(unsigned long code) noexcept {
    if (code == 0xe06d7363u) std::terminate();
    return EXCEPTION_CONTINUE_SEARCH;
}
void unwind_base(void* owner, NativeHardwareLayoutOwnerContext& context) noexcept {
    __try {
        destroy_native_hardware_layout_base_00b48960(owner, context);
    } __except (terminate_cpp_cleanup_exception(GetExceptionCode())) {
        __assume(0);
    }
}
struct BaseCleanup {
    void* owner;
    NativeHardwareLayoutOwnerContext& context;
    bool armed = true;
    ~BaseCleanup() noexcept { if (armed) unwind_base(owner, context); }
};
} // namespace

void create_native_hardware_layout_from_streams_00b60790(
    void* owner, const void* streams, NativeHardwareLayoutConstructContext& context) {
    Scratch scratch;
    for (std::uint32_t i = 0; i != 14; ++i) put(scratch.usage_counts, i * 4u, 0);
    const auto initial_count = signed_word(word(streams, 0x10));
    put(&scratch.count, 0, 0);
    std::uint32_t stream_index = 0;
    if (initial_count > 0) {
        do {
            auto* const declaration = pointer(word(streams, stream_index * 4u));
            append_native_hardware_layout_stream_00b48a00(owner, declaration, context.actual_owner);
            std::uint32_t raw_index = 0;
            std::uint32_t raw_offset = 0;
            if (signed_word(word(declaration, 0x10)) > 0) {
                do {
                    auto* const raw = pointer(word(declaration, 0x0c) + raw_offset);
                    const auto offset = half(raw);
                    const auto type = byte(raw, 4);
                    const auto method = byte(raw, 8);
                    const auto usage_byte = byte(raw, 12);
                    const auto usage = word(raw, 12);
                    auto* const counter = at(scratch.usage_counts, usage * 4u);
                    const auto usage_index = byte(counter);
                    put(counter, 0, word(counter) + 1u);
                    const auto first = (stream_index & 0xffffu) | (offset << 16u);
                    put(scratch.elements, word(&scratch.count) * 8u, first);
                    const auto second = type | (method << 8u) | (usage_byte << 16u) | (usage_index << 24u);
                    put(scratch.elements, word(&scratch.count) * 8u + 4u, second);
                    put(&scratch.count, 0, word(&scratch.count) + 1u);
                    ++raw_index;
                    raw_offset += 20u;
                } while (signed_word(raw_index) < signed_word(word(declaration, 0x10)));
            }
            diagnostic(owner, context);
            ++stream_index;
        } while (signed_word(stream_index) < signed_word(word(streams, 0x10)));
    }
    put(scratch.elements, word(&scratch.count) * 8u, 0x000000ff);
    const auto end_index = word(&scratch.count);
    auto* const renderer = context.actual_owner.actual_renderer_00f8d394;
    put(scratch.elements, end_index * 8u + 4u, 0x00000011);
    put(&scratch.count, 0, word(&scratch.count) + 1u);
    auto* const device = pointer(word(renderer, 0x1a10));
    auto* const table = pointer(word(device));
    using CreateDeclaration = HRESULT (STDMETHODCALLTYPE*)(void*, const D3DVERTEXELEMENT9*, IDirect3DVertexDeclaration9**);
    const auto create = reinterpret_cast<CreateDeclaration>(word(table, 0x158));
    (void)create(device, reinterpret_cast<const D3DVERTEXELEMENT9*>(scratch.elements),
        reinterpret_cast<IDirect3DVertexDeclaration9**>(at(owner, 0x40)));
    recompute_native_hardware_layout_stride_00b47d60(owner);
}

void* construct_native_hardware_layout_00b60cb0(
    void* owner, const void* streams, NativeHardwareLayoutConstructContext& context) {
    initialize_native_hardware_layout_base_00b48c00(owner);
    BaseCleanup cleanup{owner, context.actual_owner};
    put(owner, 0x40, 0);
    put(owner, 0, 0x00d62af4);
    create_native_hardware_layout_from_streams_00b60790(owner, streams, context);
    cleanup.armed = false;
    return owner;
}
} // namespace bsp
