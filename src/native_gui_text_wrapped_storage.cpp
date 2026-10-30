#include "bsp/native_gui_text_wrapped_storage.hpp"
#include "bsp/native_mesh_remaining_fields.hpp"
#include "bsp/native_mesh_texture_field.hpp"
#include "bsp/native_wide_string.hpp"
#include <cstddef>
#include <stdexcept>

namespace bsp {
namespace {
using Word = std::uint32_t;
using Scratch = NativeGuiTextWrappedStorageScratch;
using Context = NativeGuiTextWrappedStorageContext;
using Acquired = NativeGuiTextWrappedStorageAcquired;
using Boundary = Acquired::Boundary;
static_assert(sizeof(void*) == 4 && sizeof(Word) == 4 && sizeof(float) == 4 && sizeof(double) == 8);
static_assert(sizeof(Scratch) == 0x64);
static_assert(offsetof(Scratch, conversion_and_position_54) == 0x44);
static_assert(offsetof(Scratch, height_60) == 0x50);
static_assert(offsetof(Scratch, index_stream_64) == 0x54);
static_assert(offsetof(Scratch, glyph_position_68) == 0x58);
// All offsets are relative to the actual S+10 image, not to a new projection.
enum {
    wFirst = 0x03, wPen = 0x04, wSpaces = 0x08, wSum = 0x0c,
    wTop = 0x10, wSpaceGlyph = 0x14, wSpaceStep = 0x18, wScan = 0x1c,
    wWidth = 0x20, wQuad = 0x24, wIndices = 0x28, wMax = 0x2c,
    wMin = 0x30, wAlias = 0x34, wFirstVertex = 0x38, wHeightFloat = 0x3c,
    wVertex = 0x40, wConversion = 0x44, wHeight = 0x50, wIndex = 0x54,
    wPosition = 0x58
};
Word bits(const volatile void* p) noexcept {
    return static_cast<Word>(reinterpret_cast<std::uintptr_t>(p));
}
void* pointer(Word p) noexcept { return reinterpret_cast<void*>(static_cast<std::uintptr_t>(p)); }
void* address(const volatile void* p, Word offset = 0) noexcept { return pointer(bits(p) + offset); }
// Integer asm accesses object representations directly. Ordinary float3
// objects remain legal output objects for genuine helpers; no typed DWORD
// lvalue aliases a float/header and no volatile qualification is stripped.
Word load(const volatile void* p, Word offset = 0) {
    const volatile void* q = address(p, offset);
    Word value;
    __asm {
        mov edx, q
        mov eax, dword ptr [edx]
        mov value, eax
    }
    return value;
}
Word unit(const volatile void* p) {
    Word value;
    __asm {
        mov edx, p
        movzx eax, word ptr [edx]
        mov value, eax
    }
    return value;
}
Word byte(const volatile void* p) {
    Word value;
    __asm {
        mov edx, p
        movzx eax, byte ptr [edx]
        mov value, eax
    }
    return value;
}
Word signed_unit(const volatile void* p) {
    Word value;
    __asm {
        mov edx, p
        movsx eax, word ptr [edx]
        mov value, eax
    }
    return value;
}
void store(volatile void* p, Word offset, Word value) {
    void* q = address(p, offset);
    __asm {
        mov edx, q
        mov eax, value
        mov dword ptr [edx], eax
    }
}
void store_byte(volatile void* p, Word value) {
    __asm {
        mov edx, p
        mov eax, value
        mov byte ptr [edx], al
    }
}
void store_unit(void* p, Word value) {
    __asm {
        mov edx, p
        mov eax, value
        mov word ptr [edx], ax
    }
}
void location(Acquired& a, Boundary boundary, Word site) noexcept {
    a.boundary = boundary;
    a.native_site = site;
}
Word target(void* stream, Word slot, Context& c, Acquired& a) {
    const Word profile = load(stream);
    a.captured_profile = profile;
    const volatile Word* const table = c.bindings.current_stream_table(stream, profile);
    const Word selected = table[slot / 4u];
    a.captured_target = selected;
    return selected;
}
void map_vertex(void* stream, Word selected, Word count, Context& c) {
    if (selected == 0x00b49980)
        (void)lock_native_logical_vertex_stream_00b49980(stream, c.mapping, count, 0, 0);
    else (void)c.bindings.other_vertex_map_10(stream, selected, count, 0, 0);
}
void* map_index(void* stream, Word selected, Word count, Context& c) {
    if (selected == 0x00b49b60)
        return lock_native_logical_index_stream_00b49b60(stream, c.mapping, count, 0, 0);
    return c.bindings.other_index_map_0c(stream, selected, count, 0, 0);
}
void unmap_index(void* stream, Word selected, Context& c) {
    if (selected == 0x00b49c70) unlock_native_logical_index_stream_00b49c70(stream, c.mapping);
    else c.bindings.other_index_unmap_10(stream, selected);
}
void unmap_vertex(void* stream, Word selected, Context& c) {
    if (selected == 0x00b49a80) unlock_native_logical_vertex_stream_00b49a80(stream, c.mapping);
    else c.bindings.other_vertex_unmap_14(stream, selected);
}
void height_to_float(Word height, Scratch& s) {
    auto* const p = &s;
    __asm {
        mov edx, p
        mov eax, height
        cvtsi2ss xmm0, eax // ABA294; full MOVSX result, not unsigned WORD
        mov dword ptr [edx + wHeight], eax
        movss dword ptr [edx + wHeightFloat], xmm0
    }
}
void capture_control(volatile Word (&homes)[2]) {
    volatile Word* const h = homes;
    __asm {
        mov edx, h
        fnstcw word ptr [edx] // ABA32A, preserves upperWORD
    }
}
void prepare_width(void* text, Scratch& s, volatile Word (&homes)[2], Word saved_control, Context& c) {
    auto* const p = &s;
    volatile Word* const h = homes;
    const volatile double* const width = &c.glyph_quad.width_00cec380;
    __asm {
        mov esi, p
        mov edi, h
        mov ebx, text
        mov edx, width
        mov eax, saved_control
        fld dword ptr [ebx + 0x20]
        or eax, 0xc00
        fmul qword ptr [edx]
        mov dword ptr [esi + wAlias], eax
        xorps xmm0, xmm0
        fldcw word ptr [esi + wAlias]
        mov dword ptr [ebx + 0x110], 0
        mov dword ptr [esi + wQuad], 0
        movss dword ptr [esi + wPen], xmm0
        movss dword ptr [esi + wTop], xmm0
        mov byte ptr [esi + wFirst], 1
        fistp qword ptr [esi + wConversion]
        mov eax, dword ptr [esi + wConversion]
        mov dword ptr [esi + wWidth], eax
        fldcw word ptr [edi]
    }
}
// Keeps candidate ST0 live through EXACT Native integer/UTF16 operations;
// no C++ helper or callback runs until it is popped on either path.
bool scan_glyph(void* text, void* glyph, Scratch& s, volatile Word (&homes)[2],
    Word& cursor, Word& break_width, Context& c) {
    auto* const p = &s;
    volatile Word* const h = homes;
    Word* const scan_home = &cursor;
    Word* const break_home = &break_width;
    const volatile float* const bias = &c.unsigned_bias_00ce3978;
    Word more;
    __asm {
        mov esi, p
        mov edi, h
        mov ebx, text
        mov eax, glyph
        fld dword ptr [ebx + 0x1d8]
        movzx eax, word ptr [eax + 0x12]
        mov ecx, dword ptr [esi + wWidth]
        test ecx, ecx
        mov dword ptr [esi + wConversion], eax
        fild dword ptr [esi + wConversion]
        fmulp st(1), st(0)
        fadd dword ptr [esi + wScan]
        fild dword ptr [esi + wWidth]
        jge wrapped_scan_unsigned_ready
        mov edx, bias
        fadd dword ptr [edx]
    wrapped_scan_unsigned_ready:
        fxch st(1)
        fcomi st(0), st(1)
        fstp st(1)
        ja wrapped_scan_overflow
        mov edx, scan_home
        mov eax, dword ptr [edx]
        cmp word ptr [eax], 0x20
        jne wrapped_scan_not_space
        add dword ptr [esi + wSpaces], 1
        cvttss2si ecx, dword ptr [esi + wScan]
        mov ebx, break_home
        mov dword ptr [ebx], ecx
        mov dword ptr [esi + wAlias], eax
        jmp wrapped_scan_advance
    wrapped_scan_not_space:
        mov byte ptr [edi], 0
    wrapped_scan_advance:
        add eax, 2
        mov dword ptr [edx], eax
        fstp dword ptr [esi + wScan]
        cmp word ptr [eax], 0
        setne al
        movzx eax, al
        mov more, eax
        jmp wrapped_scan_done
    wrapped_scan_overflow:
        mov edx, scan_home
        mov eax, dword ptr [edx]
        cmp word ptr [eax], 0x20
        fstp st(0)
        jne wrapped_scan_no_backup
        sub dword ptr [edx], 2
    wrapped_scan_no_backup:
        mov more, 0
    wrapped_scan_done:
    }
    return more != 0;
}
Word truncate_scan(Scratch& s) {
    auto* const p = &s;
    Word value;
    __asm {
        mov edx, p
        cvttss2si eax, dword ptr [edx + wScan]
        mov value, eax
    }
    return value;
}
void align_line(void* text, Scratch& s, Word stop, Word spaces, Word break_width, Context& c) {
    auto* const p = &s;
    const volatile float* const bias = &c.unsigned_bias_00ce3978;
    const volatile double* const half = &c.half_00d7a280;
    __asm {
        mov esi, p
        mov ebx, text
        mov ecx, stop
        mov edx, spaces
        xorps xmm0, xmm0
        mov eax, dword ptr [ebx + 0x100]
        test eax, eax
        jz wrapped_align_left
        cmp eax, 3
        jne wrapped_align_right_test
        test cx, cx
        jz wrapped_align_left
        cmp cx, 0xa
        je wrapped_align_left
        test edx, edx
        jz wrapped_align_no_spaces
        mov ecx, dword ptr [esi + wWidth]
        sub ecx, break_width
        test ecx, ecx
        mov dword ptr [esi + wConversion], ecx
        fild dword ptr [esi + wConversion]
        jge wrapped_align_justify_unsigned
        mov eax, bias
        fadd dword ptr [eax]
    wrapped_align_justify_unsigned:
        add edx, -1
        mov dword ptr [esi + wConversion], edx
        fidiv dword ptr [esi + wConversion] // spaces1 deliberately divides by0
        mov edx, dword ptr [esi + wSpaceGlyph]
        movzx eax, word ptr [edx + 0x12]
        mov dword ptr [esi + wConversion], eax
        fild dword ptr [esi + wConversion]
        faddp st(1), st(0)
        fstp dword ptr [esi + wSpaceStep]
        jmp wrapped_align_zero_pen
    wrapped_align_no_spaces:
        movss dword ptr [esi + wSpaceStep], xmm0
        jmp wrapped_align_zero_pen
    wrapped_align_right_test:
        cmp eax, 2
        jne wrapped_align_center_test
        mov ecx, dword ptr [esi + wSpaceGlyph]
        movzx edx, word ptr [ecx + 0x12]
        mov eax, dword ptr [esi + wWidth]
        sub eax, break_width
        test eax, eax
        cvtsi2ss xmm1, edx
        mov dword ptr [esi + wConversion], eax
        fild dword ptr [esi + wConversion]
        movss dword ptr [esi + wSpaceStep], xmm1
        jge wrapped_align_right_unsigned
        mov ecx, bias
        fadd dword ptr [ecx]
    wrapped_align_right_unsigned:
        fstp dword ptr [esi + wPen]
        jmp wrapped_align_done
    wrapped_align_center_test:
        cmp eax, 1
        jne wrapped_align_done // actual S+28/preexisting pen unchanged
        mov ecx, dword ptr [esi + wSpaceGlyph]
        movzx edx, word ptr [ecx + 0x12]
        mov eax, dword ptr [esi + wWidth]
        sub eax, break_width
        test eax, eax
        cvtsi2ss xmm1, edx
        mov dword ptr [esi + wConversion], eax
        fild dword ptr [esi + wConversion]
        movss dword ptr [esi + wSpaceStep], xmm1
        jge wrapped_align_center_unsigned
        mov ecx, bias
        fadd dword ptr [ecx]
    wrapped_align_center_unsigned:
        mov eax, half
        fmul qword ptr [eax]
        fstp dword ptr [esi + wPen]
        jmp wrapped_align_done
    wrapped_align_left:
        mov ecx, dword ptr [esi + wSpaceGlyph]
        movzx edx, word ptr [ecx + 0x12]
        cvtsi2ss xmm1, edx
        movss dword ptr [esi + wSpaceStep], xmm1
    wrapped_align_zero_pen:
        movss dword ptr [esi + wPen], xmm0
    wrapped_align_done:
    }
}
Word begin_line(Scratch& s) {
    auto* const p = &s;
    Word first;
    __asm {
        mov edx, p
        fld dword ptr [edx + wTop]
        movss xmm1, dword ptr [edx + wTop]
        fadd dword ptr [edx + wHeightFloat]
        mov eax, dword ptr [edx + wFirstVertex]
        xorps xmm0, xmm0
        movss dword ptr [edx + wPosition + 4], xmm1
        movss dword ptr [edx + wPosition + 8], xmm0
        fstp dword ptr [edx + wAlias]
        mov first, eax
    }
    return first;
}
void update_bounds(Scratch& s) {
    auto* const p = &s;
    __asm {
        mov edx, p
        fld dword ptr [edx + wMin]
        fld dword ptr [edx + wTop]
        fcomip st(0), st(1)
        fstp st(0)
        ja wrapped_bounds_min_done
        movss xmm0, dword ptr [edx + wTop]
        movss dword ptr [edx + wMin], xmm0
    wrapped_bounds_min_done:
        fld dword ptr [edx + wAlias]
        fld dword ptr [edx + wMax]
        fcomip st(0), st(1)
        fstp st(0)
        ja wrapped_bounds_max_done
        movss xmm0, dword ptr [edx + wAlias]
        movss dword ptr [edx + wMax], xmm0
    wrapped_bounds_max_done:
    }
}
bool advance_glyph(void* text, void* glyph, Scratch& s, volatile Word (&homes)[2],
    Word& cursor, Word is_space) {
    auto* const p = &s;
    volatile Word* const h = homes;
    Word* const current = &cursor;
    Word more;
    __asm {
        mov esi, p
        mov edi, h
        mov ebx, text
        cmp is_space, 0
        jne wrapped_advance_space
        mov eax, glyph
        movzx ecx, word ptr [eax + 0x12]
        mov dword ptr [esi + wConversion], ecx
        fild dword ptr [esi + wConversion]
        fld dword ptr [ebx + 0x1d8]
        fmul st(0), st(1)
        jmp wrapped_advance_add
    wrapped_advance_space:
        fld dword ptr [esi + wSpaceStep]
        fld st(0)
    wrapped_advance_add:
        fadd dword ptr [esi + wPen]
        add dword ptr [esi + wQuad], 1
        mov edx, current
        mov eax, dword ptr [edx]
        add eax, 2
        mov dword ptr [edx], eax
        cmp eax, dword ptr [edi]
        fstp dword ptr [esi + wPen]
        fadd dword ptr [esi + wSum]
        fstp dword ptr [esi + wSum]
        setne al
        movzx eax, al
        mov more, eax
    }
    return more != 0;
}
Word finish_line(void* text, Word cursor, Scratch& s, Context& c) {
    auto* const p = &s;
    const volatile double* const one = &c.one_00d7a210;
    const volatile double* const spacing = &c.line_spacing_00ce6618;
    Word code;
    __asm {
        mov esi, p
        mov ebx, text
        fld dword ptr [ebx + 0x114]
        fld dword ptr [esi + wSum]
        fcomip st(0), st(1)
        fstp st(0)
        jbe wrapped_line_width_done
        movss xmm0, dword ptr [esi + wSum]
        movss dword ptr [ebx + 0x114], xmm0
    wrapped_line_width_done:
        fld dword ptr [ebx + 0x10c]
        add dword ptr [ebx + 0x110], 1
        mov edx, one
        fadd qword ptr [edx]
        mov eax, cursor
        movzx eax, word ptr [eax]
        test ax, ax
        mov edx, spacing
        fsub qword ptr [edx]
        fmul dword ptr [esi + wHeightFloat]
        fadd dword ptr [esi + wTop]
        fstp dword ptr [esi + wTop]
        mov code, eax
    }
    return code;
}
void vertical_metrics(void* text, Scratch& s, volatile Word (&homes)[2], Context& c) {
    auto* const p = &s;
    volatile Word* const h = homes;
    const volatile double* const height = &c.glyph_quad.height_00cef1b8;
    const volatile float* const scale = &c.glyph_quad.vertical_scale_00e12fd4;
    const volatile double* const half = &c.half_00d7a280;
    __asm {
        mov esi, p
        mov edi, h
        mov ebx, text
        fld dword ptr [esi + wMax]
        mov eax, dword ptr [ebx + 0x104]
        sub eax, 1
        fsub dword ptr [esi + wMin]
        xorps xmm0, xmm0
        fstp dword ptr [edi + 4]
        fld dword ptr [edi + 4]
        movss dword ptr [edi + 4], xmm0
        fst dword ptr [ebx + 0x178]
        mov edx, height
        fdiv qword ptr [edx]
        mov edx, scale
        fld dword ptr [edx]
        fld st(0)
        fmulp st(2), st(0)
        fxch st(1)
        fstp dword ptr [edi]
        jz wrapped_vertical_center
        sub eax, 1
        jnz wrapped_vertical_other
        fmul dword ptr [ebx + 0x24]
        fsub dword ptr [edi]
        fstp dword ptr [edi + 4]
        jmp wrapped_vertical_done
    wrapped_vertical_center:
        fmul dword ptr [ebx + 0x24]
        fsub dword ptr [edi]
        mov edx, half
        fmul qword ptr [edx]
        fstp dword ptr [edi + 4]
        jmp wrapped_vertical_done
    wrapped_vertical_other:
        fstp st(0)
    wrapped_vertical_done:
    }
}
void shift_position(void* vertex, Scratch& s, volatile Word (&homes)[2], Word& index) {
    auto* const p = &s;
    volatile Word* const h = homes;
    Word* const index_home = &index;
    __asm {
        mov esi, p
        mov edi, h
        mov edx, vertex
        mov ecx, index_home
        mov ebx, dword ptr [ecx]
        fld dword ptr [esi + wConversion + 4]
        mov eax, dword ptr [edx + 0xc]
        fadd dword ptr [edi + 4]
        movss xmm0, dword ptr [esi + wConversion]
        imul eax, ebx
        fstp dword ptr [esi + wConversion + 4]
        fld dword ptr [esi + wConversion + 4]
        add eax, dword ptr [edx + 0x10]
        add ebx, 1
        mov dword ptr [ecx], ebx
        add eax, dword ptr [edx + 8]
        movss dword ptr [eax], xmm0
        fstp dword ptr [eax + 4]
        movss xmm0, dword ptr [esi + wConversion + 8]
        movss dword ptr [eax + 8], xmm0
    }
}
} // namespace

NativeGuiTextWrappedStorageFrame::NativeGuiTextWrappedStorageFrame(void* text,
    volatile Word (&homes)[2], Scratch& scratch, Context& context, Acquired& acquired,
    NativeVertexPositionStorageScratch& position_scratch) noexcept
    : text_(text), homes_(homes), scratch_(scratch), context_(context), acquired_(acquired),
      position_scratch_(position_scratch) {}

void build_native_gui_text_wrapped_storage_00aba270(NativeGuiTextWrappedStorageFrame& f) {
    using Phase = NativeGuiTextWrappedStorageFrame::Phase;
    auto& s = f.scratch_;
    auto& c = f.context_;
    auto& a = f.acquired_;
    void* const text = f.text_;
    if (f.phase_ != Phase::fresh || a.phase != Phase::fresh || a.failure)
        throw std::logic_error("ABA270 requires one fresh retained invocation");
    f.phase_ = a.phase = Phase::entered;
    a.actual_text = text;
    a.home_identity = f.homes_;
    a.private_image = &s;
    try {
        location(a, Boundary::clear, 0x00aba279);
        clear_native_gui_text_glyph_children_00ab80c0(text, c.children, a.clear);
        location(a, Boundary::height, 0x00aba27e);
        const Word height = signed_unit(address(pointer(load(text, 0x108)), 0x14));
        f.ebx_ = load(f.homes_); // ABA288: AFTER whole clear and height read
        f.esi_ = bits(address(text, 0xec));
        height_to_float(height, s);
        if (f.esi_ != f.ebx_) {
            location(a, Boundary::copy, 0x00aba2a4);
            const Word length = load(pointer(f.ebx_));
            resize_native_wide_string_header_004c53e0(pointer(f.esi_), c.glyph_quad.strings, length, true);
            if (load(pointer(f.ebx_)) != 0) { // CURRENT source header AFTER resize
                Word from = load(pointer(f.ebx_), 4);
                Word to = load(pointer(f.esi_), 4);
                Word copied;
                do {
                    copied = unit(pointer(from));
                    store_unit(pointer(to), copied);
                    from += 2u;
                    to += 2u;
                } while (copied != 0);
            }
        }
        location(a, Boundary::model_geometry, 0x00aba2d1);
        void* const model = pointer(load(text, 0x4c));
        a.captured_model = model;
        // Whole existing B74640: actual +180 load; stacked0 is unused.
        f.ebx_ = load(model, 0x180);
        a.captured_mesh = pointer(f.ebx_);
        location(a, Boundary::vertex_stream, 0x00aba2e1);
        f.esi_ = bits(native_mesh_vertex_stream_unchecked_00b73260(pointer(f.ebx_), 0));
        a.captured_vertex = pointer(f.esi_);
        const Word vertex_count = load(text, 0xec) * 4u; // BEFORE current table
        location(a, Boundary::vertex_map, 0x00aba2ee);
        const Word vertex_target = target(pointer(f.esi_), 0x10, c, a);
        store(&s, wVertex, f.esi_); // ABA2FE: BEFORE entered map
        a.map_count = vertex_count;
        location(a, Boundary::vertex_map, 0x00aba302);
        a.vertex_map_entered = true;
        map_vertex(pointer(f.esi_), vertex_target, vertex_count, c);
        a.vertex_map_returned = true;
        const Word index_length = load(text, 0xec); // ABA304: AFTER vertex map
        void* const index = pointer(load(pointer(f.ebx_), 0x60));
        a.captured_index = index;
        location(a, Boundary::index_map, 0x00aba30d);
        const Word index_target = target(index, 0x0c, c, a);
        const Word index_count = index_length * 6u;
        store(&s, wFirstVertex, 0); // Native ABA31C precedes callback
        store(&s, wIndex, bits(index));
        a.map_count = index_count;
        location(a, Boundary::index_map, 0x00aba328);
        a.index_map_entered = true;
        void* const indices = map_index(index, index_target, index_count, c);
        a.index_mapping = indices;
        a.index_map_returned = true;
        capture_control(f.homes_); // LOWWORD of incoming source home
        f.ebp_ = load(address(f.homes_, 4)); // ABA32E, actual section home
        store(&s, wIndices, bits(indices));
        store(pointer(f.ebp_), 0x10, load(text, 0xec) * 4u);
        const Word initial_primitive_count = load(text, 0xec);
        const Word saved_control = unit(f.homes_); // ABA34C, before section18
        store(pointer(f.ebp_), 0x18, initial_primitive_count * 2u);
        void* const space_font = pointer(load(text, 0x108)); // ABA356 BEFORE FLD20
        f.ebx_ = 0;
        f.font_key_ = 0x20;
        location(a, Boundary::width, 0x00aba35c);
        prepare_width(text, s, f.homes_, saved_control, c);
        location(a, Boundary::space_glyph, 0x00aba3a1);
        f.glyph_ = bits(select_native_font_glyph_storage_00ad4480(space_font,
            f.font_key_, f.font_scratch_));
        const Word minimum = load(&c.minimum_00ce4970);
        store(&s, wSpaceGlyph, f.glyph_);
        f.ebx_ = load(text, 0xf0);
        store(&s, wMin, minimum);
        const Word maximum = load(&c.maximum_00ce4adc);
        store(&s, wMax, maximum);
        if (f.ebx_ == 0) f.ebx_ = bits(c.glyph_quad.actual_empty_utf16_00f8be50);
        f.code_ = unit(pointer(f.ebx_));
        if (f.code_ != 0) {
            do {
                if (f.code_ == 0x20) {
                    bool skip = true; // Native control flow, not a domain admission
                    if (load(text, 0x100) == 0) {
                        Word current_begin = load(text, 0xf0);
                        if (current_begin == 0) current_begin = bits(c.glyph_quad.actual_empty_utf16_00f8be50);
                        if (f.ebx_ == current_begin) skip = false;
                        else {
                            current_begin = load(text, 0xf0); // distinct CURRENT reload
                            if (current_begin == 0) current_begin = bits(c.glyph_quad.actual_empty_utf16_00f8be50);
                            if (f.ebx_ != current_begin && unit(pointer(f.ebx_ - 2u)) == 0xa)
                                skip = false;
                        }
                    }
                    if (skip) {
                        do { f.ebx_ += 2u; } while (unit(pointer(f.ebx_)) == 0x20);
                    }
                }
                f.ebp_ = 0;
                const Word scan_initial = unit(pointer(f.ebx_)); // ABA42E BEFORE stores
                store(&s, wScan, 0);
                store(&s, wSpaces, 0);
                store_byte(f.homes_, 1); // LOWBYTE only, saved upper bytes survive
                store(&s, wAlias, 0);
                f.esi_ = f.ebx_;
                if (scan_initial != 0) {
                    for (;;) {
                        f.code_ = unit(pointer(f.esi_));
                        if (f.code_ == 0xa) break;
                        void* const font = pointer(load(text, 0x108));
                        f.font_key_ = static_cast<std::uint16_t>(f.code_);
                        location(a, Boundary::scan_glyph, 0x00aba460);
                        f.glyph_ = bits(select_native_font_glyph_storage_00ad4480(font,
                            f.font_key_, f.font_scratch_));
                        a.selected_glyph = pointer(f.glyph_);
                        location(a, Boundary::scan, 0x00aba465);
                        if (!scan_glyph(text, pointer(f.glyph_), s, f.homes_, f.esi_, f.ebp_, c)) break;
                    }
                    if (byte(f.homes_) == 0 && unit(pointer(f.esi_)) == 0x20) {
                        const Word space_advance = unit(address(pointer(load(&s, wSpaceGlyph)), 0x12));
                        Word spaces = load(&s, wSpaces);
                        do {
                            f.esi_ -= 2u;
                            spaces -= 1u;
                            f.ebp_ -= space_advance;
                        } while (unit(pointer(f.esi_)) == 0x20);
                        store(&s, wSpaces, spaces);
                    }
                }
                const Word stop = unit(pointer(f.esi_));
                const Word spaces = load(&s, wSpaces);
                if (stop == 0) store(f.homes_, 0, f.esi_);
                else if (spaces == 0 || stop == 0xa) {
                    f.esi_ += 2u;
                    store(f.homes_, 0, f.esi_);
                } else store(f.homes_, 0, load(&s, wAlias) + 2u);
                if (f.ebp_ == 0 || stop == 0 || stop == 0xa) f.ebp_ = truncate_scan(s);
                location(a, Boundary::alignment, 0x00aba53a);
                align_line(text, s, stop, spaces, f.ebp_, c); // NO mode gate; stale28 untouched
                const Word line_end = load(f.homes_); // ABA635
                store(&s, wSum, 0);
                if (f.ebx_ != line_end) {
                    f.esi_ = begin_line(s);
                    do {
                        f.code_ = unit(pointer(f.ebx_));
                        void* const font = pointer(load(text, 0x108));
                        f.font_key_ = static_cast<std::uint16_t>(f.code_);
                        location(a, Boundary::emit_glyph, 0x00aba67a);
                        f.ebp_ = bits(select_native_font_glyph_storage_00ad4480(font,
                            f.font_key_, f.font_scratch_));
                        a.selected_glyph = pointer(f.ebp_);
                        if (byte(address(&s, wFirst)) != 0) {
                            void* const texture0 = pointer(load(pointer(f.ebp_), 0x18));
                            void* const section0 = pointer(load(address(f.homes_, 4)));
                            void* const material0 = pointer(load(section0, 0x20));
                            location(a, Boundary::texture0, 0x00aba695);
                            set_native_material_texture_unchecked_00b189f0(material0, 0, texture0, c.owners);
                            void* const texture1 = pointer(load(pointer(f.ebp_), 0x1c));
                            void* const section1 = pointer(load(address(f.homes_, 4)));
                            void* const material1 = pointer(load(section1, 0x20));
                            location(a, Boundary::texture1, 0x00aba6a7);
                            set_native_material_texture_unchecked_00b189f0(material1, 1, texture1, c.owners);
                            const Word initial_pen = load(&s, wPen);
                            store_byte(address(&s, wFirst), 0);
                            store(text, 0x18c, initial_pen);
                        }
                        update_bounds(s);
                        const Word code_unit = unit(pointer(f.ebx_));
                        const Word full_signed_height = load(&s, wHeight);
                        const Word quad = load(&s, wQuad);
                        const Word pen = load(&s, wPen);
                        f.quad_homes_[9] = code_unit;
                        f.quad_homes_[8] = f.esi_;
                        f.quad_homes_[7] = full_signed_height;
                        f.quad_homes_[6] = quad;
                        const Word current_indices = load(&s, wIndices);
                        f.quad_homes_[5] = bits(s.glyph_position_68);
                        const Word current_vertex = load(&s, wVertex);
                        f.quad_homes_[4] = bits(s.glyph_position_68);
                        f.quad_homes_[3] = current_indices;
                        f.quad_homes_[2] = current_vertex;
                        f.quad_homes_[1] = bits(s.glyph_position_68);
                        f.quad_homes_[0] = f.ebp_;
                        store(&s, wPosition, pen); // Native ABA727, after all ten PUSHes
                        location(a, Boundary::glyph_record, 0x00aba730);
                        auto& quad_record = c.bindings.prepared_glyph_quad_record(a);
                        a.selected_quad = &quad_record;
                        location(a, Boundary::glyph_quad, 0x00aba730);
                        write_native_gui_text_glyph_quad_storage_00ab98f0(text,
                            f.quad_homes_, c.glyph_quad, quad_record);
                        store(&s, wIndices, load(&s, wIndices) + 12u);
                        f.esi_ += 4u;
                        const Word is_space = unit(pointer(f.ebx_)) == 0x20;
                        location(a, Boundary::advance, 0x00aba743);
                        if (!advance_glyph(text, pointer(f.ebp_), s, f.homes_, f.ebx_, is_space)) break;
                    } while (true);
                    store(&s, wFirstVertex, f.esi_);
                }
                location(a, Boundary::line_metrics, 0x00aba785);
                f.code_ = finish_line(text, f.ebx_, s, c);
            } while (f.code_ != 0);
            f.esi_ = load(&s, wVertex); // ABA7D4, only on processed-text arm
            f.ebp_ = load(address(f.homes_, 4));
        }
        location(a, Boundary::vertical_metrics, 0x00aba7dc);
        vertical_metrics(text, s, f.homes_, c);
        const Word quads_low_word = unit(address(&s, wQuad));
        const Word vertices = quads_low_word * 4u;
        const Word primitives = quads_low_word * 2u;
        f.ebx_ = 0; // Source home for Native final EDI index
        store(pointer(f.ebp_), 0x10, vertices);
        store(pointer(f.ebp_), 0x18, primitives);
        if (vertices != 0) {
            do {
                a.position_index = f.ebx_;
                f.position_homes_[1] = f.ebx_; // ABA860: independent Native PUSH-copy
                f.position_homes_[0] = bits(s.conversion_and_position_54); // ABA865: SAME S54
                location(a, Boundary::position_record, 0x00aba868);
                auto& position_record = c.bindings.prepared_position_record(a);
                a.selected_position = &position_record;
                location(a, Boundary::position, 0x00aba868);
                (void)read_native_vertex_position_storage_004768d0(pointer(f.esi_),
                    f.position_homes_, f.position_scratch_, c.position, position_record);
                location(a, Boundary::position_store, 0x00aba86d);
                shift_position(pointer(f.esi_), s, f.homes_, f.ebx_);
            } while (f.ebx_ < load(pointer(f.ebp_), 0x10)); // unsigned CURRENT count
        }
        void* const captured_index = pointer(load(&s, wIndex));
        location(a, Boundary::index_unmap, 0x00aba8ad);
        const Word index_unmap = target(captured_index, 0x10, c, a);
        location(a, Boundary::index_unmap, 0x00aba8b2);
        a.index_unmap_entered = true;
        unmap_index(captured_index, index_unmap, c);
        a.index_unmap_returned = true;
        location(a, Boundary::vertex_unmap, 0x00aba8b4);
        const Word vertex_unmap = target(pointer(f.esi_), 0x14, c, a); // AFTER index return
        location(a, Boundary::vertex_unmap, 0x00aba8bb);
        a.vertex_unmap_entered = true;
        unmap_vertex(pointer(f.esi_), vertex_unmap, c);
        a.vertex_unmap_returned = true;
        f.phase_ = a.phase = Phase::complete;
    } catch (...) {
        a.failure = std::current_exception();
        a.escaped_boundary = a.boundary;
        a.escaped_site = a.native_site;
        f.phase_ = a.phase = Phase::failed;
        throw; // retention ONLY: no Native cleanup/unlock/origin inference
    }
}
} // namespace bsp
