#include "bsp/native_font_tree_insert.hpp"
#include "bsp/native_effect_handle_acquisition.hpp"
#include <cstddef>

namespace bsp {
static_assert(sizeof(NativeIntPointerTree18Iterator) == 8);
static_assert(offsetof(NativeIntPointerTree18Iterator, owner) == 0);
static_assert(offsetof(NativeIntPointerTree18Iterator, node) == 4);
static_assert(sizeof(NativeIntPointerTree18Pair) == 8);
static_assert(offsetof(NativeIntPointerTree18Pair, key_00) == 0);
static_assert(offsetof(NativeIntPointerTree18Pair, value_04) == 4);

NativeIntPointerTree18Iterator* __cdecl link_native_font_tree_node_00ad4600(
    void* actual_font_base, NativeIntPointerTree18Iterator* output8,
    std::uint8_t insert_left, void* parent,
    const NativeIntPointerTree18Pair* pair8) {
    return link_native_int_pointer_tree18_node_0086ebe0(
        actual_font_base, output8, insert_left, parent, pair8);
}
} // namespace bsp

#include "bsp/detail/native_tree_insert_storage.hpp"
#include "bsp/native_font_tree_iterator.hpp"

namespace bsp {
namespace {
using FontWordAccess185 = detail::TreeInsertAccess<0x14, 0x15>;

// Actual WORD operand over a genuine DWORD object/raw node address, without
// manufacturing an incompatible C++ uint16_t lvalue. Ordinary Source helper
// ABI only; hardware faults/private Native register behavior are not supplied.
__declspec(noinline) std::uint16_t read_font_word185(
    const void* actual_storage) noexcept {
    std::uint16_t loaded_word;
    __asm {
        mov eax, actual_storage
        movzx eax, word ptr [eax]
        mov loaded_word, ax
    }
    return loaded_word;
}

struct FontWordSearch185 final {
    const NativeIntPointerTree18Pair* pair;
    std::uint16_t captured_key{};
    bool capture_complete{};

    bool operator()(void* node) {
        if (!capture_complete) {
            captured_key = read_font_word185(&pair->key_00);
            capture_complete = true;
        }
        const auto current_node_key = read_font_word185(
            static_cast<const unsigned char*>(node) + 0x0c);
        return static_cast<unsigned>(captured_key) <
            static_cast<unsigned>(current_node_key);
    }
};
} // namespace

NativeIntPointerTree18InsertResult* __cdecl
insert_native_font_tree_unique_00ad4aa0(void* actual_font_base,
    NativeIntPointerTree18InsertResult* actual_output12,
    const NativeIntPointerTree18Pair* actual_pair8) {
    static_assert(sizeof(NativeIntPointerTree18InsertResult) == 12);
    static_assert(offsetof(NativeIntPointerTree18InsertResult, iterator_00) == 0);
    static_assert(offsetof(NativeIntPointerTree18InsertResult, inserted_08) == 8);
    return detail::insert_unique_tree_pair<FontWordAccess185,
        NativeIntPointerTree18Iterator>(actual_font_base, actual_output12,
        actual_pair8, FontWordSearch185{actual_pair8},
        [actual_pair8](void* candidate) {
            const auto candidate_key = read_font_word185(
                static_cast<const unsigned char*>(candidate) + 0x0c);
            const auto current_incoming_key = read_font_word185(
                &actual_pair8->key_00);
            return static_cast<unsigned>(candidate_key) <
                static_cast<unsigned>(current_incoming_key);
        },
        [](NativeIntPointerTree18Iterator& iterator) {
            decrement_native_font_tree_iterator_00ad4040(iterator);
        },
        [](void* font_base, NativeIntPointerTree18Iterator* output8,
            std::uint8_t insert_left, void* parent, const void* pair8) {
            return link_native_font_tree_node_00ad4600(font_base, output8,
                insert_left, parent,
                static_cast<const NativeIntPointerTree18Pair*>(pair8));
        },
        [](NativeIntPointerTree18InsertResult* output12, void* owner,
            void* node, std::uint8_t inserted) {
            volatile NativeIntPointerTree18InsertResult& publication = *output12;
            publication.iterator_00.owner = owner;
            publication.iterator_00.node = node;
            publication.inserted_08 = inserted;
        });
}
} // namespace bsp
