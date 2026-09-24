#include "bsp/native_gui_page_construction.hpp"
#include "bsp/native_gui_widget_base_storage.hpp"
#include "bsp/native_node_construction.hpp"
#include "bsp/native_string_pool_storage.hpp"
#include <cstring>
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native GUI page construction requires MSVC Win32.
#endif

namespace bsp {
namespace {
using Word = std::uint32_t;
template<class T> volatile T& field(void* object, std::size_t offset) noexcept {
    return *reinterpret_cast<volatile T*>(static_cast<unsigned char*>(object) + offset);
}
template<class T> const volatile T& field(const void* object, std::size_t offset) noexcept {
    return *reinterpret_cast<const volatile T*>(static_cast<const unsigned char*>(object) + offset);
}
void* at(void* object, Word offset) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(object) + offset);
}
static_assert(offsetof(NativeNodeStorage, auxiliary_flags_138) == 0x138);
} // namespace

void bind_native_gui_widget_node_00aa6720(void* widget, NativeNodeStorage* node) noexcept {
    field<NativeNodeStorage*>(widget, 0x4c) = node;
    if (node) {
        volatile auto& flags = node->auxiliary_flags_138;
        flags = flags & 0xfffffffcu;
    }
}

void initialize_native_gui_page_00ac6600_prefix(void* page, const void* name,
    NativeNodeStorage* scene_node, Word share_scene,
    NativeGuiPageConstructionContext& c, NativeGuiPageConstructionAcquired& a) {
    if (a.phase != NativeGuiPageConstructionAcquired::Phase::not_started)
        throw std::logic_error("native GUI page construction prefix cannot replay");
    a.phase = NativeGuiPageConstructionAcquired::Phase::running;
    a.actual_page = page;
    a.actual_name = name;
    a.actual_scene_node = scene_node;
    a.share_scene_slot = share_scene;
    try {
        a.native_site = 0x00ac6627;
        construct_native_gui_widget_base_00aa9390(page, 1, c.actual_one_00d7a24c);
        a.base_returned = true;
        auto* const page_name = at(page, 0x100);
        const bool distinct_name = page_name != name; // AC663E, before page stores.
        field<Word>(page, 0) = 0x00d5be38;
        field<Word>(page, 0xec) = 0;
        field<Word>(page, 0xf0) = 0;
        field<unsigned char>(page, 0xf5) = 0;
        field<Word>(page, 0xf8) = 0;
        field<Word>(page, 0xfc) = 0;
        a.native_eh_state = 0; // AC6666, before fresh header initialization.
        field<Word>(page_name, 0) = 0;
        field<char*>(page_name, 4) = nullptr;
        if (distinct_name) {
            a.native_site = 0x00ac667c;
            resize_native_string_header_0041dd40(page_name, c.strings,
                field<Word>(name, 0), true);
            if (field<Word>(name, 0) != 0) {
                const auto count = field<Word>(page_name, 0);
                auto* const destination = field<char*>(page_name, 4);
                auto* const source = field<char*>(name, 4);
                a.native_site = 0x00ac6691;
                if (count != 0) std::memmove(destination, source, count);
            }
        }
        a.name_returned = true;
        const Word first_default = c.actual_00d7a2f0;
        field<Word>(page, 0x108) = 0;
        field<Word>(page, 0x10c) = first_default;
        field<Word>(page, 0x110) = c.actual_00ce3804;
        const Word one = c.actual_one_00d7a24c;
        field<Word>(page, 0x114) = one;
        field<Word>(page, 0x118) = 0;
        field<Word>(page, 0x11c) = 0;
        field<unsigned char>(page, 0x120) = static_cast<unsigned char>(share_scene);
        field<unsigned char>(page, 0x121) = 0;
        a.native_eh_state = 1; // AC66FD precedes the two size stores.
        field<Word>(page, 0x20) = one;
        field<Word>(page, 0x24) = one;
        a.native_site = 0x00ac670f;
        bind_native_gui_widget_node_00aa6720(page, scene_node);
        a.scene_bound = true;
        a.continuation = 0x00ac6714;
        a.phase = NativeGuiPageConstructionAcquired::Phase::awaiting_lua_continuation;
    } catch (...) {
        // No invented raw AA9730 implementation and no premature pool return.
        a.phase = NativeGuiPageConstructionAcquired::Phase::failed;
        throw;
    }
}
} // namespace bsp
