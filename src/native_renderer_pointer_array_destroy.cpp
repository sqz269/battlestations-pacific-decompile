#include "bsp/native_renderer_pointer_array_destroy.hpp"
#include "bsp/native_logical_vertex_owner.hpp"
#include "bsp/native_logical_index_owner.hpp"
#include "bsp/singleton_lifetime.hpp"

namespace bsp {
namespace {
static_assert(sizeof(void*) == 4);
using Word = std::uint32_t;
volatile Word& word(void* base, Word offset = 0) noexcept {
    return *reinterpret_cast<volatile Word*>(reinterpret_cast<Word>(base) + offset);
}
std::int32_t signed_word(void* base, Word offset) noexcept {
    return static_cast<std::int32_t>(word(base, offset));
}
template<auto Reserve>
void resize_pointers(void* header, std::int32_t requested) {
    if (requested > signed_word(header, 8)) Reserve(header, static_cast<Word>(requested));
    Word index = word(header, 4);
    while (static_cast<std::int32_t>(index) < requested) {
        const Word destination = word(header) + index * 4u;
        if (destination != 0) word(reinterpret_cast<void*>(destination)) = 0;
        index += 1u;
    }
    while (requested < signed_word(header, 4)) word(header, 4) = word(header, 4) - 1u;
    word(header, 4) = static_cast<Word>(requested);
}
void reserve_query(void* header, Word requested) {
    reserve_native_renderer_query_pointers_00b22530(header, 0, static_cast<std::int32_t>(requested));
}
template<auto Resize>
void destroy_pointers(void* header) {
    Resize(header, 0, 0);
    singleton_lifetime_free(reinterpret_cast<void*>(word(header)));
}
} // namespace

void __fastcall reserve_native_renderer_query_pointers_00b22530(
    void* header, Word, std::int32_t capacity) {
    reserve_native_renderer_pointer_array_00b22d10(header, static_cast<Word>(capacity));
}
void __fastcall resize_native_renderer_query_pointers_00b22cc0(
    void* header, Word, std::int32_t count) {
    resize_pointers<reserve_query>(header, count);
}
void __fastcall resize_native_renderer_vertex_pointers_00b25940(
    void* header, Word, std::int32_t count) {
    resize_pointers<reserve_native_renderer_pointer_array_00b22d10>(header, count);
}
void __fastcall resize_native_renderer_index_pointers_00b259d0(
    void* header, Word, std::int32_t count) {
    resize_pointers<reserve_native_renderer_index_pointer_array_00b22d70>(header, count);
}
void __fastcall destroy_native_renderer_query_pointers_00b27f50(void* header) {
    destroy_pointers<resize_native_renderer_query_pointers_00b22cc0>(header);
}
void __fastcall destroy_native_renderer_vertex_pointers_00b29b20(void* header) {
    destroy_pointers<resize_native_renderer_vertex_pointers_00b25940>(header);
}
void __fastcall destroy_native_renderer_index_pointers_00b29b40(void* header) {
    destroy_pointers<resize_native_renderer_index_pointers_00b259d0>(header);
}
} // namespace bsp
