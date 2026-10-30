#include "bsp/native_gui_text_single_line_storage.hpp"
#include "bsp/native_mesh_remaining_fields.hpp"
#include "bsp/native_mesh_texture_field.hpp"
#include <cstddef>
#include <stdexcept>

namespace bsp {
namespace {
using Word = std::uint32_t;
using Scratch = NativeGuiTextSingleLineStorageScratch;
using Context = NativeGuiTextSingleLineStorageContext;
using Acquired = NativeGuiTextSingleLineStorageAcquired;
using Phase = NativeGuiTextSingleLineStoragePhase;
static_assert(sizeof(void*) == 4 && sizeof(Word) == 4);
static_assert(sizeof(float) == 4 && sizeof(double) == 8);
static_assert(sizeof(std::int64_t) == 8);
enum {
    sSaved = offsetof(Scratch, saved_control),
    sTruncate = offsetof(Scratch, truncation_control),
    sConverted = offsetof(Scratch, converted_width),
    sWidth = offsetof(Scratch, width_bits),
    sHeight = offsetof(Scratch, height),
    sPen = offsetof(Scratch, pen_bits),
    sCursor = offsetof(Scratch, cursor),
    sNext = offsetof(Scratch, next_unit),
    sAdvance = offsetof(Scratch, advance),
    sFirstVertex = offsetof(Scratch, first_vertex),
    sQuad = offsetof(Scratch, quad_index),
    sIndices = offsetof(Scratch, index_position),
    sFirstGlyph = offsetof(Scratch, first_glyph)
};

void require(bool condition, const char* message) {
    if (!condition) throw std::logic_error(message);
}
void location(Acquired& acquired, Acquired::Boundary boundary, Word site) noexcept {
    acquired.boundary = boundary;
    acquired.native_site = site;
}
void* pointer(Word value) noexcept {
    return reinterpret_cast<void*>(static_cast<std::uintptr_t>(value));
}
Word bits(const volatile void* value) noexcept {
    return static_cast<Word>(reinterpret_cast<std::uintptr_t>(value));
}
void* at(const volatile void* base, Word offset) noexcept {
    return pointer(bits(base) + offset);
}
Word load(const volatile void* base, Word offset = 0) noexcept {
    return *static_cast<const volatile Word*>(at(base, offset));
}
Word unit(const volatile void* address) noexcept {
    return *static_cast<const volatile std::uint16_t*>(address);
}
void store(void* base, Word offset, Word value) noexcept {
    *static_cast<volatile Word*>(at(base, offset)) = value;
}

void capture_control(Scratch& scratch) {
    auto* const actual_scratch = &scratch;
    __asm {
        mov edx, actual_scratch
        fnstcw word ptr [edx + sSaved] // ABA02E: BEFORE section stores
    }
}
void prepare_width(void* actual_text, Scratch& scratch,
    const volatile double& width_00cec380) {
    auto* const actual_scratch = &scratch;
    const volatile double* const width_value = &width_00cec380;
    // One live x87 value through multiply and FISTP64. Do not move FLDCW
    // ahead of the multiply, spill a double, mask exceptions or add a guard.
    __asm {
        mov esi, actual_text
        mov edx, actual_scratch
        fld dword ptr [esi + 020h]       // ABA055
        mov eax, dword ptr [esi + 0108h] // ABA058: current Font
        mov ecx, width_value
        fmul qword ptr [ecx]             // ABA05E
        mov dword ptr [esi + 0114h], 0   // exact +0 bits; no FP conversion
        movzx ecx, word ptr [eax + 014h] // unsigned Font height
        movzx eax, word ptr [edx + sSaved]
        or eax, 0c00h
        mov word ptr [edx + sTruncate], ax
        mov eax, dword ptr [esi + 0f0h]  // capture FIRST stored-text cursor
        mov dword ptr [edx + sHeight], ecx
        fldcw word ptr [edx + sTruncate] // ABA088
        mov dword ptr [edx + sQuad], 0
        mov dword ptr [edx + sPen], 0
        mov dword ptr [edx + sFirstGlyph], 1
        fistp qword ptr [edx + sConverted] // ABA09D, Native int64 indefinite too
        mov ecx, dword ptr [edx + sConverted]
        mov dword ptr [edx + sWidth], ecx // low DWORD only
        fldcw word ptr [edx + sSaved]    // ABA0A9, only after returned FISTP
        mov dword ptr [edx + sFirstVertex], 0
        mov dword ptr [edx + sCursor], eax
    }
}
void measure_advance(void* actual_text, void* captured_glyph, Scratch& scratch) {
    auto* const actual_scratch = &scratch;
    __asm {
        mov eax, captured_glyph
        mov edx, actual_scratch
        movzx ecx, word ptr [eax + 012h]
        mov dword ptr [edx + sAdvance], ecx
        add dword ptr [edx + sCursor], 2 // before FILD, like ABA0D7
        fild dword ptr [edx + sAdvance]
        mov esi, actual_text
        fmul dword ptr [esi + 01d8h]
        fadd dword ptr [esi + 0114h]
        fstp dword ptr [esi + 0114h]     // one float32 spill PER code unit
    }
}
void align_pen(void* actual_text, Scratch& scratch,
    const volatile float& unsigned_bias_00ce3978,
    const volatile double& half_00d7a280) {
    auto* const actual_scratch = &scratch;
    const volatile float* const bias_value = &unsigned_bias_00ce3978;
    const volatile double* const half_value = &half_00d7a280;
    __asm {
        mov esi, actual_text
        mov edx, actual_scratch
        mov eax, dword ptr [esi + 0100h]
        cmp eax, 2
        je single_right
        cmp eax, 1
        jne single_aligned
        mov ecx, dword ptr [edx + sWidth]
        test ecx, ecx
        fild dword ptr [edx + sWidth]
        jge single_center_unsigned
        mov ecx, bias_value
        fadd dword ptr [ecx]
    single_center_unsigned:
        fsub dword ptr [esi + 0114h]
        mov ecx, half_value
        fmul qword ptr [ecx]             // NO intermediate float/double spill
        jmp single_pen_store
    single_right:
        mov ecx, dword ptr [edx + sWidth]
        test ecx, ecx
        fild dword ptr [edx + sWidth]
        jge single_right_unsigned
        mov ecx, bias_value
        fadd dword ptr [ecx]
    single_right_unsigned:
        fsub dword ptr [esi + 0114h]
    single_pen_store:
        fstp dword ptr [edx + sPen]
    single_aligned:
    }
}
void emit_advance(void* actual_text, void* captured_glyph, Scratch& scratch) {
    auto* const actual_scratch = &scratch;
    __asm {
        mov eax, captured_glyph
        mov edx, actual_scratch
        movzx eax, word ptr [eax + 012h] // SAME captured glyph AFTER quad call
        add dword ptr [edx + sFirstVertex], 4
        add dword ptr [edx + sIndices], 0ch
        add dword ptr [edx + sQuad], 1
        mov dword ptr [edx + sAdvance], eax
        mov ecx, dword ptr [edx + sCursor]
        movzx eax, word ptr [ecx + 2]    // next unit BEFORE pen FP operations
        fild dword ptr [edx + sAdvance]
        add dword ptr [edx + sCursor], 2
        test ax, ax
        mov dword ptr [edx + sNext], eax
        mov esi, actual_text
        fmul dword ptr [esi + 01d8h]    // CURRENT scale after provider callback
        fadd dword ptr [edx + sPen]
        fstp dword ptr [edx + sPen]
    }
}

Word stream_target(void* actual_stream, Word offset, Context& context) {
    const Word profile = load(actual_stream);
    const volatile Word* const table = context.bindings.current_stream_table(
        actual_stream, profile);
    return table[offset / 4];
}
void map_vertex(void* vertex, Word target, Word count, Context& context) {
    if (target == 0x00b49980u)
        (void)lock_native_logical_vertex_stream_00b49980(vertex, context.mapping, count, 0, 0);
    else
        (void)context.bindings.other_vertex_map_10(vertex, target, count, 0, 0);
}
void* map_index(void* index, Word target, Word count, Context& context) {
    if (target == 0x00b49b60u)
        return lock_native_logical_index_stream_00b49b60(index, context.mapping, count, 0, 0);
    return context.bindings.other_index_map_0c(index, target, count, 0, 0);
}
void unmap_index(void* index, Word target, Context& context) {
    if (target == 0x00b49c70u)
        unlock_native_logical_index_stream_00b49c70(index, context.mapping);
    else context.bindings.other_index_unmap_10(index, target);
}
void unmap_vertex(void* vertex, Word target, Context& context) {
    if (target == 0x00b49a80u)
        unlock_native_logical_vertex_stream_00b49a80(vertex, context.mapping);
    else context.bindings.other_vertex_unmap_14(vertex, target);
}
} // namespace

NativeGuiTextSingleLineStorageFrame::NativeGuiTextSingleLineStorageFrame(
    void* captured_actual_text, volatile Word (&actual_homes)[2], Context& context,
    Acquired& acquired) noexcept
    : text_(captured_actual_text), homes_(actual_homes), context_(context), acquired_(acquired) {}

void build_native_gui_text_single_line_storage_00ab9fd0(
    NativeGuiTextSingleLineStorageFrame& frame) {
    auto& context = frame.context_;
    auto& acquired = frame.acquired_;
    auto& scratch = frame.scratch_;
    void* const text = frame.text_;
    try {
        require(frame.phase_ == Phase::fresh && acquired.phase == Phase::fresh &&
            !acquired.failure, "AB9FD0 requires one fresh retained invocation");
        frame.phase_ = acquired.phase = Phase::entered;
        acquired.text = text;
        acquired.model = pointer(load(text, 0x4c)); // BEFORE Text+110 store
        store(text, 0x110, 0);
        location(acquired, Acquired::Boundary::model_geometry, 0x00ab9fe5);
        void* const mesh = context.bindings.model_geometry_00b74640(acquired.model, 0);
        acquired.mesh = mesh;
        location(acquired, Acquired::Boundary::vertex_stream, 0x00ab9ff3);
        void* const vertex = native_mesh_vertex_stream_unchecked_00b73260(mesh, 0);
        acquired.vertex = vertex;
        acquired.vertex_count = load(text, 0xec) * 4u;
        location(acquired, Acquired::Boundary::vertex_map, 0x00aba003);
        acquired.vertex_map_target = stream_target(vertex, 0x10, context);
        location(acquired, Acquired::Boundary::vertex_map, 0x00aba00e);
        acquired.vertex_map_entered = true;
        map_vertex(vertex, acquired.vertex_map_target, acquired.vertex_count, context);
        acquired.vertex_map_returned = true;
        void* const index = pointer(load(mesh, 0x60)); // AFTER vertex map
        acquired.index = index;
        acquired.index_count = load(text, 0xec) * 6u;
        location(acquired, Acquired::Boundary::index_map, 0x00aba019);
        acquired.index_map_target = stream_target(index, 0x0c, context);
        location(acquired, Acquired::Boundary::index_map, 0x00aba02c);
        acquired.index_map_entered = true;
        void* const index_mapping = map_index(index, acquired.index_map_target,
            acquired.index_count, context);
        acquired.index_mapping = index_mapping;
        acquired.index_map_returned = true;

        capture_control(scratch);
        const Word section_vertex_count = load(text, 0xec);
        scratch.index_position = bits(index_mapping);
        void* const initial_section = pointer(frame.homes_[1]); // ABA03F
        store(initial_section, 0x10, section_vertex_count * 4u);
        store(initial_section, 0x18, load(text, 0xec) * 2u); // separate EC read
        location(acquired, Acquired::Boundary::width, 0x00aba055);
        prepare_width(text, scratch, context.width_00cec380);
        if (scratch.cursor == 0) scratch.cursor = bits(context.empty_wide_00f8be50);
        Word code_unit = unit(pointer(scratch.cursor));
        while (code_unit != 0) {
            location(acquired, Acquired::Boundary::measure_glyph, 0x00aba0ca);
            void* const current_font = pointer(load(text, 0x108));
            scratch.font_code_unit_home = static_cast<std::uint16_t>(code_unit);
            void* const glyph = select_native_font_glyph_storage_00ad4480(current_font,
                scratch.font_code_unit_home, scratch.font_glyph);
            acquired.measured_glyph = glyph;
            measure_advance(text, glyph, scratch);
            ++acquired.measured_units;
            code_unit = unit(pointer(scratch.cursor));
        }
        location(acquired, Acquired::Boundary::alignment, 0x00aba100);
        align_pen(text, scratch, context.unsigned_bias_00ce3978, context.half_00d7a280);

        scratch.cursor = load(text, 0xf0); // SECOND independent stored-text capture
        if (scratch.cursor == 0) scratch.cursor = bits(context.empty_wide_00f8be50);
        code_unit = unit(pointer(scratch.cursor));
        if (code_unit != 0) {
            scratch.position_bits[1] = 0; // once, not before every glyph
            scratch.position_bits[2] = 0;
            do {
                location(acquired, Acquired::Boundary::emit_glyph, 0x00aba17b);
                void* const current_font = pointer(load(text, 0x108));
                scratch.font_code_unit_home = static_cast<std::uint16_t>(code_unit);
                void* const glyph = select_native_font_glyph_storage_00ad4480(current_font,
                    scratch.font_code_unit_home, scratch.font_glyph);
                acquired.emitted_glyph = glyph;
                if (scratch.first_glyph != 0) {
                    void* const texture = pointer(load(glyph, 0x18));
                    void* const section0 = pointer(frame.homes_[1]); // current ABA18C
                    void* const material0 = pointer(load(section0, 0x20));
                    location(acquired, Acquired::Boundary::texture0, 0x00aba196);
                    set_native_material_texture_unchecked_00b189f0(material0, 0, texture,
                        context.owners);
                    void* const alpha = pointer(load(glyph, 0x1c)); // after slot0 callback
                    void* const section1 = pointer(frame.homes_[1]); // fresh ABA19E
                    void* const material1 = pointer(load(section1, 0x20));
                    location(acquired, Acquired::Boundary::texture1, 0x00aba1a8);
                    set_native_material_texture_unchecked_00b189f0(material1, 1, alpha,
                        context.owners);
                    scratch.first_glyph = 0;
                    store(text, 0x18c, scratch.pen_bits);
                }
                // Populate actual callee homes in original PUSH order. Glyph
                // and height stay separate; AB98F0 mutates homes0/7 itself.
                scratch.quad_homes[9] = unit(pointer(scratch.cursor)); // late ABA1C0
                scratch.quad_homes[8] = scratch.first_vertex;
                scratch.quad_homes[7] = scratch.height;
                scratch.quad_homes[6] = scratch.quad_index;
                const Word position = bits(&scratch.position_bits[0]);
                scratch.quad_homes[5] = position;
                scratch.quad_homes[4] = position;
                scratch.quad_homes[3] = scratch.index_position;
                scratch.quad_homes[2] = bits(vertex);
                scratch.quad_homes[1] = position;
                scratch.quad_homes[0] = bits(glyph);
                scratch.position_bits[0] = scratch.pen_bits; // ABA1F2 after PUSHes
                location(acquired, Acquired::Boundary::glyph_quad, 0x00aba1f8);
                context.bindings.write_glyph_quad_00ab98f0(text, scratch.quad_homes);
                emit_advance(text, glyph, scratch); // captured glyph, CURRENT scale
                ++acquired.emitted_units;
                code_unit = scratch.next_unit;
            } while (code_unit != 0);
        }

        location(acquired, Acquired::Boundary::index_unmap, 0x00aba23a);
        acquired.index_unmap_target = stream_target(index, 0x10, context);
        location(acquired, Acquired::Boundary::index_unmap, 0x00aba241);
        acquired.index_unmap_entered = true;
        unmap_index(index, acquired.index_unmap_target, context);
        acquired.index_unmap_returned = true;
        location(acquired, Acquired::Boundary::vertex_unmap, 0x00aba243);
        acquired.vertex_unmap_target = stream_target(vertex, 0x14, context);
        location(acquired, Acquired::Boundary::vertex_unmap, 0x00aba24a);
        acquired.vertex_unmap_entered = true;
        unmap_vertex(vertex, acquired.vertex_unmap_target, context);
        acquired.vertex_unmap_returned = true;
        void* const final_section = pointer(frame.homes_[1]); // AFTER both unmaps
        location(acquired, Acquired::Boundary::layout, 0x00aba255);
        acquired.layout_entered = true;
        context.bindings.rebuild_layout_00b865a0(final_section, mesh);
        acquired.layout_returned = true;
        require(frame.phase_ == Phase::entered && acquired.phase == Phase::entered &&
            !acquired.failure, "AB9FD0 cannot complete after caught invocation failure");
        frame.phase_ = acquired.phase = Phase::complete;
    } catch (...) {
        acquired.escaped_boundary = acquired.boundary;
        acquired.escaped_site = acquired.native_site;
        acquired.failure = std::current_exception();
        frame.phase_ = acquired.phase = Phase::failed;
        throw; // no auto-unlock, CW guard, rollback, retry or process policy
    }
}
} // namespace bsp
