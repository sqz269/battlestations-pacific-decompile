#include "bsp/native_gui_text_field_storage.hpp"
#include "bsp/game_native_readonly_data.hpp"
#include "bsp/gui_material_binding.hpp"
#include "bsp/gui_widget_owner.hpp"
#include "bsp/native_gui_widget_color_storage.hpp"
#include "bsp/native_physical_file_date.hpp"
#include <cstring>
#include <initializer_list>
#include <stdexcept>
#include <type_traits>

namespace bsp {
namespace {
using Word = std::uint32_t;
using Acquired = NativeGuiTextFieldStorageAcquired;
static_assert(sizeof(void*) == 4);
static_assert(std::is_trivially_destructible_v<Acquired>);
Word bits(const volatile void* p) noexcept { return reinterpret_cast<Word>(p); }
void* pointer(Word p) noexcept { return reinterpret_cast<void*>(p); }
void* at(const void* p, Word offset = 0) noexcept { return pointer(bits(p) + offset); }
Word read(const void* p, Word offset = 0) noexcept {
    return *reinterpret_cast<const volatile Word*>(bits(p) + offset);
}
void write(void* p, Word offset, Word value) noexcept {
    *reinterpret_cast<volatile Word*>(bits(p) + offset) = value;
}
void require(bool condition, const char* message) {
    if (!condition) throw std::invalid_argument(message);
}
struct Region { const volatile void* begin; std::size_t bytes; };
bool representable(Region r) noexcept {
    return r.begin && static_cast<std::uint64_t>(bits(r.begin)) + r.bytes <= 0x100000000ull;
}
bool separate(Region a, Region b) noexcept {
    const auto aa = static_cast<std::uint64_t>(bits(a.begin));
    const auto bb = static_cast<std::uint64_t>(bits(b.begin));
    return aa + a.bytes <= bb || bb + b.bytes <= aa;
}
void separate_scratch(Region region, NativeGuiTextFieldStorageFrame& frame, Acquired& a) {
    require(representable(region) && separate(region, {&frame, sizeof(frame)}) &&
        separate(region, {&a, sizeof(a)}), "Text field scratch must be separate from live borrowed backing");
}
void* preflight(NativeGuiTextReference& reference, NativeGuiTextFieldStorageContext& context,
    NativeGuiTextFieldStorageFrame& frame, Acquired& a, const void* input, std::size_t input_bytes) {
    const auto& c = reference.storage_context();
    void* const raw = reference.storage();
    require(reference.phase() == NativeGuiTextReference::Phase::bound && raw &&
        (bits(raw) & 3u) == 0 && c.owners.find(raw) == &reference,
        "Text fields require the SAME live full-storage canonical122 binding");
    require(c.construction.complete && c.construction.destination == raw &&
        !c.construction.exception_cleanup_started && !c.construction.base_lifetime_ended,
        "Text fields require the SAME genuine completed93 construction");
    require(a.phase == Acquired::Phase::fresh && a.entry == Acquired::Entry::none &&
        !a.destination && !a.active_call_site && !a.shader_decrement_entered &&
        !a.state_entered && !a.color_entered,
        "Text field invocation requires a fresh retained diagnostic record");
    const Region scratch[] = {{&frame, sizeof(frame)}, {&a, sizeof(a)}};
    require(representable(scratch[0]) && representable(scratch[1]) &&
        (bits(&frame) & 3u) == 0 && separate(scratch[0], scratch[1]),
        "Text field frame/diagnostics require separate representable backing");
    const auto& r = c.retirement.resources;
    for (const Region borrowed : {Region{raw, 0x1f8}, {&reference, sizeof(reference)},
        {&c, sizeof(c)}, {&c.owners, sizeof(c.owners)}, {&c.data, sizeof(c.data)},
        {&c.construction_context, sizeof(c.construction_context)},
        {&c.construction_frame, sizeof(c.construction_frame)}, {&c.construction, sizeof(c.construction)},
        {&c.retirement, sizeof(c.retirement)}, {&r, sizeof(r)},
        {&r.actual, sizeof(r.actual)}, {&r.actual.models, sizeof(r.actual.models)},
        {&r.strings, sizeof(r.strings)}, {&r.effects, sizeof(r.effects)},
        {&r.parenting, sizeof(r.parenting)}, {&r.decrement_00ce2220, sizeof(r.decrement_00ce2220)},
        {&c.construction_context.one_00d7a24c, sizeof(Word)},
        {&context, sizeof(context)}, {&context.fonts, sizeof(context.fonts)},
        {&context.current, sizeof(context.current)}})
        separate_scratch(borrowed, frame, a);
    if (input_bytes) separate_scratch({input, input_bytes}, frame, a);
    return raw; // No Native count load/gate/retain or current-table read.
}
template<class Operation>
void enter(NativeGuiTextReference& reference, NativeGuiTextFieldStorageContext& c,
    NativeGuiTextFieldStorageFrame& frame, Acquired& a, Acquired::Entry entry,
    const void* input, std::size_t input_bytes, Operation operation) {
    void* const raw = preflight(reference, c, frame, a, input, input_bytes);
    a.destination = raw;
    a.entry = entry;
    a.phase = Acquired::Phase::entered;
    try {
        operation(raw, reference.storage_context());
        a.phase = Acquired::Phase::complete;
    } catch (...) {
        a.phase = Acquired::Phase::failed;
        throw; // Preserve reached raw fields/credits; no cleanup or retry.
    }
}
Word current_target(void* raw, unsigned slot, Word& profile,
    NativeGuiTextFieldStorageContext& c, NativeGuiTextFieldStorageFrame& frame, Acquired& a) {
    profile = read(raw); // AFTER the preceding native field/arithmetic effects.
    const auto* table = c.current.current_text_table(raw, profile); // Pure metadata only.
    require(table && bits(table) == profile && (profile & 3u) == 0,
        "Text current dispatch requires its genuine captured actual-profile table");
    separate_scratch({table, 0x84}, frame, a);
    return table[slot / 4]; // Numeric selector, never a numeric host call.
}
const NativeModelTailStorage& model_tail(const void* model) noexcept {
    return *reinterpret_cast<const NativeModelTailStorage*>(at(model, 0x174));
}
bool has_geometry_00b74650(const void* model) noexcept {
    return read(model, 0x180) != 0; // Genuine whole B74650 field read.
}
void color_body(void* raw, const void* original_rgba,
    NativeGuiTextFieldStorageFrame& frame, Acquired& a) {
    a.active_call_site = 0x00ab6b59;
    set_native_gui_widget_color_storage_00aa6870(raw, original_rgba);
    a.base_color_returned = true;
    void* model = pointer(read(raw, 0x188));
    if (!model) return;
    a.active_call_site = 0x00ab6b68;
    if (!has_geometry_00b74650(model)) return;
    model = pointer(read(raw, 0x188));
    a.active_call_site = 0x00ab6b79;
    void* mesh = gui_model_geometry_00b74640(model_tail(model), 0);
    a.active_call_site = 0x00ab6b80;
    if (!gui_mesh_element_count_00b72b40(mesh)) return;
    model = pointer(read(raw, 0x188));
    a.active_call_site = 0x00ab6b93;
    mesh = gui_model_geometry_00b74640(model_tail(model), 0);
    a.active_call_site = 0x00ab6b9a;
    void* const element = gui_geometry_element_00b732c0(mesh, 0);

    const auto* shadow = static_cast<const volatile float*>(at(raw, 0x174));
    const auto* incoming = static_cast<const volatile float*>(at(original_rgba, 0xc));
    auto* product = &frame.shadow_alpha;
    void* material;
    // Native loads/multiplies before capturing material, then spills before
    // B179F0. This is scratch only, never a parallel Text/Material field.
    __asm {
        mov eax, shadow
        mov edx, incoming
        fld dword ptr [eax]
        fmul dword ptr [edx]
        mov eax, element
        mov eax, dword ptr [eax + 0x20]
        mov material, eax
        mov eax, product
        fstp dword ptr [eax]
    }
    a.active_call_site = 0x00ab6bb3;
    auto* const diffuse = native_material_diffuse_00b179f0(
        *static_cast<NativeMaterialStorage*>(material), 0);
    __asm {
        mov eax, product
        mov edx, diffuse
        fld dword ptr [eax]
        fstp dword ptr [edx + 0xc]
    }
    a.shadow_alpha_published = true;
}
void brighten(NativeGuiTextFieldStorageFrame& frame, const NativeGuiTextReferenceContext& c,
    Acquired& a) {
    const auto* rgb = static_cast<const volatile double*>(c.data.data_at(0x00d5c5d0, 8));
    const auto* alpha = static_cast<const volatile double*>(c.data.data_at(0x00d7a258, 8));
    separate_scratch({rgb, 8}, frame, a);
    separate_scratch({alpha, 8}, frame, a);
    auto* rgba = frame.rgba;
    __asm {
        mov eax, rgba
        mov edx, rgb
        fld dword ptr [eax]
        fld qword ptr [edx]
        fadd st(1), st(0)
        fxch st(1)
        fstp dword ptr [eax]
        fld dword ptr [eax + 4]
        fadd st(0), st(1)
        fstp dword ptr [eax + 4]
        fadd dword ptr [eax + 8]
        fstp dword ptr [eax + 8]
        fld dword ptr [eax + 0xc]
        mov edx, alpha
        fadd qword ptr [edx]
        fstp dword ptr [eax + 0xc]
    }
}
void state_body(void* raw, std::int32_t state, const NativeGuiTextReferenceContext& owner,
    NativeGuiTextFieldStorageContext& c, NativeGuiTextFieldStorageFrame& frame, Acquired& a) {
    const auto flag = *reinterpret_cast<const volatile std::uint8_t*>(at(raw, 0x77));
    const Word offset = flag ? 0x14cu : (static_cast<Word>(state) << 4) + 0x11cu;
    const void* const row = at(raw, offset); // Native DWORD wrap, no state clamp.
    frame.rgba[0] = read(row);
    frame.rgba[1] = read(row, 4);
    const Word third = read(row, 8);
    const Word fourth = read(row, 0xc);
    frame.rgba[2] = third;
    frame.rgba[3] = fourth;
    a.state_row_copied = true;
    if (state == 1) {
        brighten(frame, owner, a);
        a.state_brightened = true;
    }
    a.active_call_site = 0x00ab727c;
    a.color_target = current_target(raw, 0x50, a.color_profile, c, frame, a);
    a.color_entered = true;
    const void* rgba = const_cast<const Word*>(frame.rgba);
    if (a.color_target == 0x00ab6b50)
        color_body(raw, rgba, frame, a);
    else
        c.current.call_unknown_color_50(raw, a.color_profile, a.color_target, rgba);
    a.color_returned = true;
}
} // namespace

void set_native_gui_text_font_name_storage_00ab8c30(NativeGuiTextReference& reference,
    const void* name, NativeGuiTextFieldStorageContext& c,
    NativeGuiTextFieldStorageFrame& frame, Acquired& a) {
    enter(reference, c, frame, a, Acquired::Entry::font_name, name, 8,
        [&](void* raw, const NativeGuiTextReferenceContext& owner) {
        void* const cached = at(raw, 0x1c8);
        a.active_call_site = 0x00ab8c42;
        if (equal_native_string_headers_00435c40(cached, name)) {
            a.equal_name = true;
            return;
        }
        if (cached != name) {
            const Word length = read(name);
            a.active_call_site = 0x00ab8c5a;
            resize_native_string_header_0041dd40(cached, owner.retirement.resources.strings, length, true);
            a.name_resized = true;
            if (read(name) != 0) {
                const Word bytes = read(cached);
                const void* const source = pointer(read(name, 4));
                void* const destination = pointer(read(cached, 4));
                a.active_call_site = 0x00ab8c6f;
                if (bytes) std::memmove(destination, source, bytes); // Stock BF7680; omit zero-byte copy.
                a.name_copied = true;
            }
        }
        a.active_call_site = 0x00ab8c7f;
        auto* const font = c.fonts.find_native_font_00ac3570(cached);
        void* image = nullptr;
        if (font) {
            require(font->live() && font->native_image() && &font->actual_owners() == &owner.owners,
                "Raw Text Font lookup must use SAME image/actual owner domain");
            image = font->native_image();
            separate_scratch({font, sizeof(*font)}, frame, a);
            separate_scratch({image, 0xac}, frame, a);
        }
        a.selected_font = image;
        write(raw, 0x108, bits(image)); // Borrow only; no Font retain.
        a.font_published = true;
        const Word alpha = image ? read(image, 0x1c) : owner.construction_context.one_00d7a24c;
        write(raw, 0x1d4, alpha);
        a.alpha_published = true;
        a.shader = pointer(read(raw, 0x1ec));
        if (!a.shader) return;
        a.active_call_site = 0x00ab8cb3;
        const auto decrement = owner.retirement.resources.decrement_00ce2220;
        require(decrement != nullptr, "Raw Text shader release requires CURRENT CE2220 import");
        a.captured_decrement = decrement;
        a.shader_decrement_entered = true;
        const long result = decrement(reinterpret_cast<volatile long*>(at(a.shader, 4)));
        a.shader_result = result;
        a.shader_decremented = true;
        if (result == 0) {
            a.active_call_site = 0x00ab8cc3;
            a.shader_zero_entered = true;
            dispatch_native_render_actual_owner_zero(owner.owners, a.shader);
            a.shader_zero_returned = true;
        }
        write(raw, 0x1ec, 0); // AFTER callback, only the originally nonnull arm.
        a.shader_slot_cleared = true;
    });
}
void set_native_gui_text_flag77_storage_00ab6a70(NativeGuiTextReference& reference,
    std::uint8_t flag, NativeGuiTextFieldStorageContext& c,
    NativeGuiTextFieldStorageFrame& frame, Acquired& a) {
    enter(reference, c, frame, a, Acquired::Entry::flag77, nullptr, 0,
        [&](void* raw, const NativeGuiTextReferenceContext& owner) {
        *reinterpret_cast<volatile std::uint8_t*>(at(raw, 0x77)) = flag;
        a.flag_published = true;
        a.active_call_site = flag ? 0x00ab6a8b : 0x00ab6a95;
        a.state_target = current_target(raw, 0x80, a.state_profile, c, frame, a);
        const std::int32_t state = flag ? 3 : 0;
        a.state_entered = true;
        if (a.state_target == 0x00ab7200)
            state_body(raw, state, owner, c, frame, a);
        else
            c.current.call_unknown_state_80(raw, a.state_profile, a.state_target, state);
        a.state_returned = true;
    });
}
void apply_native_gui_text_state_color_storage_00ab7200(NativeGuiTextReference& reference,
    std::int32_t state, NativeGuiTextFieldStorageContext& c,
    NativeGuiTextFieldStorageFrame& frame, Acquired& a) {
    enter(reference, c, frame, a, Acquired::Entry::state_color, nullptr, 0,
        [&](void* raw, const NativeGuiTextReferenceContext& owner) {
        state_body(raw, state, owner, c, frame, a);
    });
}
void set_native_gui_text_color_storage_00ab6b50(NativeGuiTextReference& reference,
    const void* rgba, NativeGuiTextFieldStorageContext& c,
    NativeGuiTextFieldStorageFrame& frame, Acquired& a) {
    enter(reference, c, frame, a, Acquired::Entry::color, rgba, 16,
        [&](void* raw, const NativeGuiTextReferenceContext&) {
        color_body(raw, rgba, frame, a);
    });
}
} // namespace bsp
