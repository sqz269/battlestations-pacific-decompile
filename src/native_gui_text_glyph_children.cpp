#include "bsp/native_gui_text_glyph_children.hpp"
#include "bsp/native_gui_widget_child_links.hpp"
#include "bsp/native_invalid_parameter_noinfo_call.hpp"
#include <cstring>
#include <initializer_list>
#include <stdexcept>
#include <type_traits>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native Text glyph-child clearing requires MSVC Win32.
#endif

namespace bsp {
namespace {
using Word = std::uint32_t;
using Boundary = NativeGuiTextGlyphChildrenAcquired::Boundary;
static_assert(sizeof(void*) == sizeof(Word));
static_assert(std::is_trivially_destructible_v<NativeGuiTextGlyphChildrenAcquired>);
Word bits(const volatile void* p) noexcept { return reinterpret_cast<Word>(p); }
void* ptr(Word p) noexcept { return reinterpret_cast<void*>(p); }
Word read(Word p, Word offset = 0) noexcept {
    return *reinterpret_cast<const volatile Word*>(p + offset);
}
void write(Word p, Word offset, Word value) noexcept {
    *reinterpret_cast<volatile Word*>(p + offset) = value;
}
Word sar_two(Word value) noexcept {
    return (value >> 2u) | ((value & 0x80000000u) ? 0xc0000000u : 0u);
}
struct Region { const volatile void* begin; std::size_t size; };
bool disjoint(Region left, Region right) noexcept {
    const auto l = static_cast<std::uint64_t>(bits(left.begin));
    const auto r = static_cast<std::uint64_t>(bits(right.begin));
    return l + left.size <= r || r + right.size <= l;
}
void preflight(void* text, NativeGuiTextGlyphChildrenContext& c,
    NativeGuiTextGlyphChildrenAcquired& a) {
    if (!text || (bits(text) & 3u) || a.started || a.complete ||
        a.boundary != Boundary::none || a.active_call_site ||
        a.detach_calls_entered || a.delete_calls_entered)
        throw std::invalid_argument("Raw Text glyph clear requires fresh persistent diagnostics and aligned storage");
    const Region writes[] = {{text, 0x1f4}, {&a, sizeof(a)}};
    if (!disjoint(writes[0], writes[1]))
        throw std::invalid_argument("Raw Text glyph clear diagnostics must be disjoint from Text");
    for (const auto region : writes) {
        for (const Region borrowed : {Region{&c, sizeof(c)},
            {&c.nodes, sizeof(c.nodes)}, {&c.children, sizeof(c.children)}})
            if (!disjoint(region, borrowed))
                throw std::invalid_argument("Raw Text glyph clear cannot alias its provider graph");
    }
}
void enter(NativeGuiTextGlyphChildrenAcquired& a, Boundary boundary, Word site) noexcept {
    a.boundary = boundary;
    a.active_call_site = site;
    a.boundary_returned = false;
}
void invalid(NativeGuiTextGlyphChildrenAcquired& a, Word site) {
    enter(a, Boundary::invalid_parameter, site);
    invoke_native_invalid_parameter_00bf6713();
    a.boundary_returned = true;
}
} // namespace

void clear_native_gui_text_glyph_children_00ab80c0(void* actual,
    NativeGuiTextGlyphChildrenContext& c, NativeGuiTextGlyphChildrenAcquired& a) {
    preflight(actual, c, a);
    a.started = true;
    const Word text = bits(actual);
    Word index = 0;
    Word loop_begin;
    for (;;) {
        loop_begin = read(text, 0x198); // AB80D0: also retained for exit check
        a.index = index;
        a.loop_begin = loop_begin;
        if (loop_begin == 0 || index >= sar_two(read(text, 0x19c) - loop_begin))
            break;
        // Reuse captured ECX; the second length is CURRENT. No callback yet.
        if (loop_begin == 0 || index >= sar_two(read(text, 0x19c) - loop_begin))
            invalid(a, 0x00ab8100u);
        const Word detach_child = read(read(text, 0x198) + index * 4u);
        const Word offset = index * 4u; // AB810E, after incoming child capture
        a.detach_child = detach_child;
        enter(a, Boundary::detach, 0x00ab8118u);
        ++a.detach_calls_entered;
        detach_native_gui_widget_child_00aa83a0(actual, ptr(detach_child), c.nodes);
        a.boundary_returned = true;
        ++a.detach_calls_returned;

        const Word current_begin = read(text, 0x198); // AB811D, AFTER detach
        if (current_begin == 0 || index >= sar_two(read(text, 0x19c) - current_begin))
            invalid(a, 0x00ab8136u);
        const Word array = read(text, 0x198); // fresh AB813B, including diagnostic return
        const Word child = read(array + offset);
        const Word slot = array + offset;
        a.captured_slot = slot;
        a.captured_child = child;
        if (child != 0) {
            const Word profile = read(child);
            a.captured_profile = profile;
            enter(a, Boundary::current_table, 0x00ab814au);
            const volatile Word* const table = c.children.current_table(ptr(child), profile);
            const Word target = table[1]; // required actual current numeric table
            a.boundary_returned = true;
            a.captured_target = target;
            enter(a, Boundary::child_delete, 0x00ab8151u);
            ++a.delete_calls_entered;
            c.children.call_child_delete_04(ptr(child), target, 1u);
            a.boundary_returned = true;
            ++a.delete_calls_returned;
            write(slot, 0, 0); // AB8153: ORIGINAL captured slot, only after return
            ++a.slot_clears;
        }
        ++index;
    }
    const Word captured_end = read(text, 0x19c); // AB8161, before first diagnostic
    a.captured_end = captured_end;
    if (loop_begin > captured_end) invalid(a, 0x00ab816bu);
    const Word begin = read(text, 0x198); // AB8170, after possible diagnostic
    a.erase_begin = begin;
    if (begin > read(text, 0x19c)) invalid(a, 0x00ab817eu);
    if (begin != captured_end) {
        const Word count = sar_two(read(text, 0x19c) - captured_end); // AB8187
        const Word bytes = count * 4u;
        const Word new_end = begin + bytes; // AB819B, before memmove
        a.erase_bytes = bytes;
        a.erase_new_end = new_end;
        if (count != 0 && (count & 0x80000000u) == 0) {
            enter(a, Boundary::memmove, 0x00ab81a4u);
            (void)::memmove_s(ptr(begin), bytes, ptr(captured_end), bytes);
            a.boundary_returned = true;
        }
        write(text, 0x19c, new_end); // AB81AC; no current-header recapture
    }
    a.complete = true;
}
} // namespace bsp
