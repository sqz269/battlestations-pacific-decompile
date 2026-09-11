#include "bsp/gui_clip_box.hpp"
#include "bsp/gui_lua_reader.hpp"
#include <cstddef>
#include <cstring>
#include <stdexcept>

namespace bsp {
namespace {
void require_clip_box(GuiWidgetOwner& owner) {
    if (owner.layout().type != GuiWidgetType::ClipBox || owner.layout().transform.type_id != 16)
        throw std::invalid_argument("ClipBox requires the same type16 base widget");
}
}

void update_gui_clip_box_00ace120(GuiClipBoxFields& fields,
    const GuiWidgetTransform& transform) noexcept {
    static_assert(sizeof(void*) == 4, "ClipBox x87 schedule requires MSVC Win32");
    static_assert(offsetof(GuiClipBoxFields, border_f4) == 8);
    static_assert(offsetof(GuiClipBoxFields, border_width_104) == 24);
    static_assert(sizeof(GuiClipBoxFields) == 32);
    static_assert(offsetof(GuiWidgetTransform, pivot_y) == offsetof(GuiWidgetTransform, pivot_x) + 4);
    const auto position = resolved_position(transform); //00ACE12B ->00AA6750
    float native_frame[9];
    std::memcpy(&native_frame[7], &position.x, sizeof(float));
    std::memcpy(&native_frame[8], &position.y, sizeof(float));
    const auto* size_values = &transform.size.width;
    const auto* pivot = &transform.pivot_x;
    float* frame = native_frame;
    auto* output = &fields;
    const double half = 0.5; // verified00D7A280
    // Offsets in frame match native ESP after PUSH ESI. Output is native+EC.
    // Preserve retained x87 values across every spill; algebraic simplification
    // changes halfway rounding, NaN payload choices and signed-zero results.
    __asm {
        mov ebx, frame
        mov ecx, size_values
        mov edx, pivot
        mov edi, output
        fld dword ptr [ecx]
        fmul dword ptr [edx]
        fstp dword ptr [ebx + 4]
        fld dword ptr [ecx + 4]
        fmul dword ptr [edx + 4]
        fstp dword ptr [ebx + 8]
        fld dword ptr [ecx]
        movss xmm0, dword ptr [ecx + 4]
        fstp dword ptr [ebx + 14h]
        movss dword ptr [ebx + 18h], xmm0
        fld dword ptr [ebx + 1ch]
        fsub dword ptr [ebx + 4]
        fstp dword ptr [ebx + 0ch]
        fld dword ptr [ebx + 20h]
        fsub dword ptr [ebx + 8]
        fstp dword ptr [ebx + 10h]
        fld dword ptr [ebx + 0ch]
        fld st(0)
        fld dword ptr [ebx + 14h]
        fld st(0)
        faddp st(2), st(0)
        fxch st(1)
        fstp dword ptr [ebx + 4]
        fld dword ptr [ebx + 10h]
        fld st(0)
        fld dword ptr [ebx + 18h]
        fld st(0)
        faddp st(2), st(0)
        fxch st(1)
        fstp dword ptr [ebx + 8]
        fld dword ptr [ebx + 4]
        faddp st(4), st(0)
        fxch st(3)
        fstp dword ptr [ebx + 14h]
        fadd dword ptr [ebx + 8]
        fstp dword ptr [ebx + 18h]
        fld dword ptr [ebx + 14h]
        fld qword ptr half
        fmul st(1), st(0) //00ACE1BA DC C9: retain the half in ST0
        fxch st(1)
        fstp dword ptr [ebx + 0ch]
        fld dword ptr [ebx + 18h]
        fmul st(0), st(1)
        fstp dword ptr [ebx + 10h]
        fld dword ptr [ebx + 0ch]
        fstp dword ptr [edi]
        fld dword ptr [ebx + 10h]
        fstp dword ptr [edi + 4]
        movss xmm0, dword ptr [edi + 18h]
        movss xmm1, dword ptr [edi + 1ch]
        fmul st(1), st(0) //00ACE1F0 DC C9, unlike00ACE1C6 D8 C9
        movss dword ptr [edi + 10h], xmm0
        fxch st(1)
        movss dword ptr [edi + 14h], xmm1
        fstp dword ptr [ebx + 1ch]
        fmulp st(1), st(0)
        fstp dword ptr [ebx + 20h]
        fld dword ptr [ebx + 1ch]
        fstp dword ptr [edi + 8]
        fld dword ptr [ebx + 20h]
        fstp dword ptr [edi + 0ch]
    }
}

void read_gui_clip_box_properties_00ace650(GuiClipBoxFields& fields,
    const GuiWidgetTransform& transform, const GuiTable& table,
    const bool& crt_sse2_conversion) {
    const auto* value = table.find("BorderWidth");
    if (!value || value->kind() == GuiValue::Kind::Nil) {
        const std::uint32_t default_bits = 0x3dcccccd; //00D7A2F0
        std::memcpy(&fields.border_width_104[0], &default_bits, sizeof(default_bits));
        std::memcpy(&fields.border_width_104[1], &default_bits, sizeof(default_bits));
    } else if (!gui_lua_store_value_00bd63b0(*value,
        gui_lua_field(GuiLuaFieldType::Vec2, fields.border_width_104), nullptr,
        crt_sse2_conversion)) {
        throw std::invalid_argument("ClipBox BorderWidth must be an actual Vec2 table");
    }
    update_gui_clip_box_00ace120(fields, transform);
}

void copy_gui_clip_box_00ace0f0(GuiWidgetOwner& destination,
    const GuiWidgetOwner& source, const GuiClipBoxBaseCopy& base_copy) {
    require_clip_box(destination);
    if (!base_copy) throw std::invalid_argument("ClipBox copy requires actual base00AA9520");
    base_copy(destination, source);
    // The native derived table stamp has no field-copy continuation. The
    // destination companion's C++ type supplies this same derived dispatch.
    require_clip_box(destination);
}

GuiClipBoxTypeImplementation::GuiClipBoxTypeImplementation(GuiWidgetOwner& owner,
    const bool& crt_sse2_conversion)
    : owner_(owner), crt_sse2_conversion_(crt_sse2_conversion) { require_clip_box(owner); }
void GuiClipBoxTypeImplementation::properties_bound(GuiWidgetOwner& owner, const GuiTable& table) {
    if (&owner != &owner_) throw std::invalid_argument("ClipBox reader changed its base owner");
    read_gui_clip_box_properties_00ace650(fields_, owner.layout().transform, table,
        crt_sse2_conversion_);
}
void GuiClipBoxTypeImplementation::update24_00ace120(GuiWidgetOwner& owner) {
    if (&owner != &owner_) throw std::invalid_argument("ClipBox update changed its base owner");
    update_gui_clip_box_00ace120(fields_, owner.layout().transform);
}
GuiClipBoxParameterSources gui_clip_box_parameter_sources(GuiWidgetOwner& owner) {
    require_clip_box(owner);
    auto* clip = dynamic_cast<GuiClipBoxTypeImplementation*>(&owner.implementation());
    if (!clip) throw std::invalid_argument("ClipBox sources require the existing type16 implementation");
    return {clip->fields().center_ec, clip->fields().border_f4};
}
} // namespace bsp
