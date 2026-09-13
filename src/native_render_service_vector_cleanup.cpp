#include "bsp/native_render_service_vector_cleanup.hpp"
#include "bsp/native_renderer_begin_frame.hpp"
#include "bsp/native_string.hpp"
#include "bsp/singleton_lifetime.hpp"

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Actual service vector cleanup requires MSVC Win32.
#endif

namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == 4);

Word word(const void* base, Word byte_offset) noexcept {
    Word result;
    __asm { mov eax, base }
    __asm { mov edx, byte_offset }
    __asm { mov eax, dword ptr [eax + edx] }
    __asm { mov result, eax }
    return result;
}
void put(void* base, Word byte_offset, Word value) noexcept {
    __asm { mov eax, base }
    __asm { mov edx, byte_offset }
    __asm { mov ecx, value }
    __asm { mov dword ptr [eax + edx], ecx }
}
Word bits(const void* pointer) noexcept { return reinterpret_cast<Word>(pointer); }
void* pointer(Word value) noexcept { return reinterpret_cast<void*>(value); }
void clear_fields(void* vector) noexcept {
    put(vector, 4, 0);
    put(vector, 8, 0);
    put(vector, 12, 0);
}
} // namespace

void destroy_native_renderer_record_vector_00b14500(void* vector,
    NativeStringRawPoolContext& strings) {
    Word current = word(vector, 4);
    if (current != 0) {
        const Word end = word(vector, 8);
        while (current != end) {
            destroy_native_renderer_record_00b10740(pointer(current), strings);
            current += 0x12cu;
        }
        singleton_lifetime_free(pointer(word(vector, 4))); // Fresh after destruction.
    }
    clear_fields(vector);
}

void destroy_native_renderer_record_vector_thunk_00b14590(void* vector,
    NativeStringRawPoolContext& strings) {
    destroy_native_renderer_record_vector_00b14500(vector, strings);
}

void destroy_native_string_header_range_00432050(void* begin, void* end,
    NativeStringRawPoolContext& strings) {
    Word current = bits(begin);
    const Word captured_end = bits(end);
    while (current != captured_end) {
        destroy_native_string_header_0041dd20(pointer(current), strings);
        current += 8u;
    }
}

void destroy_native_string_header_vector_004324a0(void* vector,
    NativeStringRawPoolContext& strings) {
    const Word begin = word(vector, 4);
    if (begin != 0) {
        const Word end = word(vector, 8);
        destroy_native_string_header_range_00432050(pointer(begin), pointer(end), strings);
        singleton_lifetime_free(pointer(word(vector, 4))); // Fresh after range call.
    }
    clear_fields(vector);
}
} // namespace bsp
