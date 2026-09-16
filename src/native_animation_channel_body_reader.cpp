#include "bsp/native_animation_channel_body_reader.hpp"

#include "bsp/native_resource_node_traversal.hpp"
#include "bsp/native_resource_reader_references.hpp"
#include "bsp/native_resource_value_reads.hpp"
#include "bsp/native_string.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <cstring>
#include <exception>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native animation channel bodies require MSVC Win32 x87 assembly.
#endif

namespace bsp {
namespace {
using U = std::uint32_t;
U bits(const void* p) noexcept { return static_cast<U>(reinterpret_cast<std::uintptr_t>(p)); }
void* pointer(U value) noexcept { return reinterpret_cast<void*>(value); }
void* at(const void* p, U offset = 0) noexcept { return pointer(bits(p) + offset); }
U word(const void* p, U offset = 0) noexcept { return *static_cast<const volatile U*>(at(p, offset)); }
void put(void* p, U offset, U value) noexcept { *static_cast<volatile U*>(at(p, offset)) = value; }
std::int32_t signed_word(U value) noexcept {
    std::int32_t result;
    std::memcpy(&result, &value, 4);
    return result;
}
bool named(const char* actual, const char* expected) { return actual && _stricmp(actual, expected) == 0; }
__declspec(naked) void __cdecl copy_key(void*, const void*) {
    __asm {
        push edi
        push esi
        mov edi, dword ptr [esp+12]
        mov esi, dword ptr [esp+16]
        mov ecx, 10
        rep movsd
        pop esi
        pop edi
        ret
    }
}
__declspec(naked) void __cdecl read_nine_floats(void*, void*, NativeResourceStreamReadContext&) {
    __asm {
        push ebp
        mov ebp, esp
        push ebx
        push esi
        push edi
        mov ebx, dword ptr [ebp+8]
        mov esi, dword ptr [ebp+12]
        mov edi, 9
    again:
        push dword ptr [ebp+16]
        push ebx
        call read_native_resource_node_float_00be99d0
        add esp, 8
        fstp dword ptr [esi]
        add esi, 4
        dec edi
        jnz again
        pop edi
        pop esi
        pop ebx
        pop ebp
        ret
    }
}
} // namespace

void reserve_native_animation_keys_00b76680(void* header, std::int32_t requested) {
    if (requested < 1) requested = 1;
    if (signed_word(word(header, 8)) >= requested) return;
    const U bytes = static_cast<U>(requested) * 0x28u;
    void* const allocation = singleton_lifetime_allocate({SingletonAllocationKind::object, bytes, bytes});
    U destination = bits(allocation);
    for (U i = 0; signed_word(i) < signed_word(word(header, 4)); ++i, destination += 0x28u) {
        if (destination) copy_key(pointer(destination), pointer(word(header) + i * 0x28u));
    }
    singleton_lifetime_free(pointer(word(header)));
    put(header, 0, bits(allocation));
    put(header, 8, static_cast<U>(requested));
}

void append_native_animation_key_00b77870(void* channel, const void* key) {
    void* const header = at(channel, 0x1c);
    const U capacity = word(header, 8);
    if (word(header, 4) == capacity) {
        auto requested = signed_word(capacity * 2u);
        if (requested <= 1) requested = 1;
        reserve_native_animation_keys_00b76680(header, requested);
    }
    const U destination = word(header) + word(header, 4) * 0x28u;
    if (destination) copy_key(pointer(destination), key);
    put(header, 4, word(header, 4) + 1u);
}

void read_native_animation_key_00b8a0a0(void* handle, void* channel,
    NativeResourceStreamReadContext& context) {
    NativeAnimationKeyStorage key;
    key.field_00 = read_native_resource_node_dword_00be9a00(handle, context);
    read_nine_floats(handle, key.fields_04_to_24, context);
    append_native_animation_key_00b77870(channel, &key);
}

__declspec(naked) void* __cdecl publish_native_animation_channel_00b771c0(void*, U, void*) {
    __asm {
        mov ecx, dword ptr [esp+4]
        mov eax, dword ptr [esp+12]
        fld dword ptr [ecx+1ch]
        mov edx, dword ptr [ecx+10h]
        fstp dword ptr [esp+12]
        push esi
        mov esi, dword ptr [esp+12]
        mov dword ptr [edx+esi*4], eax
        mov edx, dword ptr [eax+20h]
        mov eax, dword ptr [eax+1ch]
        lea edx, [edx+edx*4]
        fld dword ptr [eax+edx*8-20h]
        lea eax, [eax+edx*8-20h]
        fstp dword ptr [esp+12]
        pop esi
        fld dword ptr [esp+8]
        fld dword ptr [esp+12]
        fcomip st(0), st(1)
        fstp st(0)
        jbe candidate
        movss xmm0, dword ptr [esp+12]
        movss dword ptr [ecx+1ch], xmm0
        ret
    candidate:
        movss xmm0, dword ptr [esp+8]
        movss dword ptr [ecx+1ch], xmm0
        ret
    }
}

void read_native_animation_channel_00b8aad0(void* item, void* handle,
    void* group, NativeResourceStreamReadContext& context) {
    (void)item; // Captured native ECX is only forwarded to B8A0A0, which ignores it.
    void* const channel = singleton_lifetime_allocate({SingletonAllocationKind::object, 0x28, 0x28});
    if (channel) {
        put(channel, 0, 0x00ceb130); put(channel, 4, 1);
        put(channel, 0, 0x00d632b0);
        put(channel, 8, 0); put(channel, 0xc, 0);
        put(channel, 0x1c, 0); put(channel, 0x20, 0); put(channel, 0x24, 0);
    }
    // Native has NO channel/allocation cleanup guard, including before read-string.
    U name[2];
    void* const returned_name = read_native_resource_handle_string_00bea010(handle, name, context);
    try {
        void* const destination = at(channel, 8);
        if (destination != returned_name) {
            resize_native_string_header_0041dd40(destination, context.strings, word(returned_name), true);
            if (word(returned_name)) {
                // B8AB56/58/5B read count, source data, destination data in order.
                const U count = word(destination);
                const void* const source_data = pointer(word(returned_name, 4));
                void* const destination_data = pointer(word(destination, 4));
                std::memcpy(destination_data, source_data, count);
            }
        }
    } catch (...) {
        try { destroy_native_string_header_0041dd20(name, context.strings); }
        catch (...) { std::terminate(); }
        throw;
    }
    destroy_native_string_header_0041dd20(name, context.strings); // State disarmed first.
    put(channel, 0x14, read_native_resource_node_dword_00be9a00(handle, context));
    put(channel, 0x18, read_native_resource_node_dword_00be9a00(handle, context));
    constexpr const char* names[] = {"Position.X", "Position.Y", "Position.Z", "Rotation.H",
        "Rotation.P", "Rotation.B", "Scale.X", "Scale.Y", "Scale.Z", "ZoomFactor", "Dissolve"};
    U index = 0xffffffffu;
    for (U i = 0; i < 11; ++i) {
        if (named(static_cast<const char*>(pointer(word(channel, 0xc))), names[i])) { index = i; break; }
    }
    while (native_resource_node_has_remaining_00715bf0(handle)) {
        U child;
        create_native_resource_child_00bea680(handle, &child, context);
        try {
            if (named(static_cast<const char*>(pointer(word(pointer(child), 0x14))), "AnimationKey")) {
                read_native_animation_key_00b8a0a0(&child, channel, context);
                publish_native_animation_channel_00b771c0(group, index, channel);
            } else skip_native_resource_node_00be9c40(&child, context);
        } catch (...) {
            try { release_native_structured_node_handle_00be9ed0(&child, context.streams); }
            catch (...) { std::terminate(); }
            throw;
        }
        release_native_structured_node_handle_00be9ed0(&child, context.streams); // State disarmed first.
    }
}

void NativeAnimationChannelBodyReader::read_channel_00b8aad0(void* item, void* handle, void* group) {
    read_native_animation_channel_00b8aad0(item, handle, group, context_);
}
} // namespace bsp
