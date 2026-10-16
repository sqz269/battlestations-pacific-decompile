#include "bsp/native_gui_frame_box_state_storage.hpp"
#include "bsp/game_native_readonly_data.hpp"
#include "bsp/game_native_string_process.hpp"
#include "bsp/gui_text_material.hpp"
#include "bsp/gui_widget_owner.hpp"
#include "bsp/native_gui_state_vector_access.hpp"
#include "bsp/native_gui_widget_color_storage.hpp"
#include "bsp/native_invalid_parameter_noinfo_call.hpp"
#include "bsp/native_mesh_buffer_fields.hpp"
#include "bsp/native_mesh_remaining_fields.hpp"
#include "bsp/native_mesh_subset_loading.hpp"
#include "bsp/native_physical_file_date.hpp"
#include "bsp/native_string_pool_storage.hpp"
#include <cstring>
#include <exception>
#include <initializer_list>
#include <new>
#include <stdexcept>
#include <utility>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Raw FrameBox state storage requires MSVC Win32.
#endif

namespace bsp {
namespace {
using Word = std::uint32_t;
using Frame = NativeGuiFrameBoxStateStorageFrame;
using Context = NativeGuiFrameBoxStateStorageContext;
using Acquired = NativeGuiFrameBoxStateStorageAcquired;
void require(bool condition, const char* message) {
    if (!condition) throw std::invalid_argument(message);
}
void* at(void* p, std::size_t offset) noexcept {
    return static_cast<std::byte*>(p) + offset;
}
volatile Word& word(void* p, std::size_t offset = 0) noexcept {
    return *static_cast<volatile Word*>(at(p, offset));
}
volatile std::uint8_t& byte(void* p, std::size_t offset = 0) noexcept {
    return *static_cast<volatile std::uint8_t*>(at(p, offset));
}
void* pointer(void* p, std::size_t offset = 0) noexcept {
    return reinterpret_cast<void*>(word(p, offset));
}
const volatile Word* profile(void* p) noexcept {
    return reinterpret_cast<const volatile Word*>(word(p));
}
void* header(Frame& f, std::size_t index) noexcept {
    return const_cast<Word*>(&f.locals[index]);
}
volatile std::uint8_t& argument_flag(Frame& f) noexcept {
    return *reinterpret_cast<volatile std::uint8_t*>(&f.argument_slot);
}
void call(Acquired& a, Word site) noexcept {
    a.active_call_site = site;
    a.boundary = Acquired::Boundary::provider_call;
}
void metadata(Acquired& a) noexcept { a.boundary = Acquired::Boundary::host_metadata; }
struct Region { const volatile void* begin; std::size_t size; };
bool separate(Region l, Region r) noexcept {
    const auto x = static_cast<std::uint64_t>(reinterpret_cast<std::uintptr_t>(l.begin));
    const auto y = static_cast<std::uint64_t>(reinterpret_cast<std::uintptr_t>(r.begin));
    return x + l.size <= y || y + r.size <= x;
}
struct Views {
    const volatile Word* box;
    const volatile Word* renderer;
    const volatile Word* stream;
    const volatile Word* material;
    const volatile Word* one;
    const char* declaration;
    const char* empty;
    const char* fade;
    const char* textured;
    const char* overbright;
    const char* low;
    const char* high;
    const char* blend;
};
Views preflight(void* box, Frame& f, Context& c, Acquired& a) {
    require(box && (reinterpret_cast<std::uintptr_t>(box) & 3u) == 0 &&
        !a.started && !a.complete && !a.failed && a.native_eh_state == -1 &&
        a.boundary == Acquired::Boundary::none && !a.captured_mesh &&
        !a.declaration.reference && !a.section.started &&
        a.vertex.phase == NativeStreamClonePhase::empty &&
        a.material.phase == NativeMaterialFactoryAcquired::Phase::empty &&
        f.geometry.diagnostics().phase == NativeGuiFrameBoxGeometryPhase::fresh &&
        !a.section_creator && !a.material_creator && !a.clip_operation &&
        !a.vertex_release.entered && !a.declaration_release.entered &&
        !a.material_release.entered && !a.section_release.entered,
        "Raw FrameBox state requires aligned actual storage and fresh persistent diagnostics");
    const Region writes[] = {{box, 0x120}, {&f, sizeof(f)}, {&a, sizeof(a)},
        {&f.locals[0], sizeof(f.locals)}, {&f.argument_slot, sizeof(f.argument_slot)},
        {&f.geometry, sizeof(f.geometry)}, {&f.transform, sizeof(f.transform)},
        {&f.clip_storage, sizeof(f.clip_storage)}};
    const Region borrowed[] = {{&c, sizeof(c)}, {&c.geometry, sizeof(c.geometry)},
        {&c.materials, sizeof(c.materials)}, {&c.effects, sizeof(c.effects)},
        {&c.strings, sizeof(c.strings)}, {&c.string_process, sizeof(c.string_process)},
        {&c.owners, sizeof(c.owners)}, {&c.box_reference, sizeof(c.box_reference)},
        {&c.data, sizeof(c.data)}, {&c.renderer, sizeof(c.renderer)},
        {&c.layout, sizeof(c.layout)}, {&c.mapping, sizeof(c.mapping)},
        {&c.geometry_storage, sizeof(c.geometry_storage)}, {&c.transform, sizeof(c.transform)},
        {&c.transform_dispatch, sizeof(c.transform_dispatch)}, {&c.clip, sizeof(c.clip)},
        {&c.clip.materials, sizeof(c.clip.materials)},
        {&c.increment_00ce221c, sizeof(c.increment_00ce221c)},
        {&c.decrement_00ce2220, sizeof(c.decrement_00ce2220)},
        {&c.transform.one_00d7a24c, 4}, {&c.transform.negative_zero_00d7a208, 4},
        {&c.transform.y_scale_00e12fc4, 4}, {&c.clip.actual_aspect_ratio_00e12fc0, 4}};
    for (std::size_t i = 0; i != 8; ++i) {
        for (std::size_t j = i + 1; j != 8; ++j)
            require(separate(writes[i], writes[j]), "Raw FrameBox state writable regions must be separate");
        for (const auto r : borrowed)
            require(separate(writes[i], r), "Raw FrameBox state must preserve known provider storage");
    }
    require(c.owners.find(box) == &c.box_reference &&
        c.box_reference.phase() == NativeGuiFrameBoxIdentityPhase::bound,
        "Raw FrameBox state requires the SAME admitted canonical163 identity without a count gate");
    require(&c.geometry.actual_owners() == &c.owners &&
        &c.materials.retained_owners == &c.owners &&
        &c.effects.effects.owners.actual_owners() == &c.owners &&
        &c.effects.strings == &c.string_process.strings() &&
        &c.materials.parameter_names == &c.string_process.strings() &&
        &c.effects.effects.construction.strings == &c.string_process.strings() &&
        &c.strings == &c.string_process.raw_context() && c.effects.effects.cache == &c.effects &&
        &c.renderer.streams.geometry == &c.geometry && &c.renderer.streams.mapping == &c.mapping &&
        &c.transform.dispatch == &c.transform_dispatch &&
        &c.clip.actual_read_only_data == &c.data && &c.clip.strings == &c.strings &&
        &c.clip.materials.parameter_names == &c.string_process.strings() &&
        static_cast<NativeMaterialParameterSlots*>(&c.clip.materials.parameter_slots) == &c.materials.parameter_slots &&
        static_cast<const volatile void*>(&c.mapping.actual_renderer_00f8d394) ==
            static_cast<const volatile void*>(&c.effects.effects.construction.current_renderer_00f8d394),
        "Raw FrameBox state requires the SAME actual owner/string/material/renderer/clip domains");
    require_gui_text_native_renderer_domain(c.renderer,
        c.effects.effects.construction.current_renderer_00f8d394, c.string_process.strings(), c.owners);
    const Views v{
        static_cast<const volatile Word*>(c.data.data_at(0x00d5d130, 0x9c)),
        static_cast<const volatile Word*>(c.data.data_at(0x00d5f0a8, 0x64)),
        static_cast<const volatile Word*>(c.data.data_at(0x00d61d6c, 0x28)),
        static_cast<const volatile Word*>(c.data.data_at(0x00d5e520, 8)),
        static_cast<const volatile Word*>(c.data.data_at(0x00d7a24c, 4)),
        static_cast<const char*>(c.data.data_at(0x00cf0078, sizeof("SimpleColor.mvfm"))),
        static_cast<const char*>(c.data.data_at(0x00ce3a0c, 1)),
        static_cast<const char*>(c.data.data_at(0x00d5c4a8, sizeof("guifade.mshd"))),
        static_cast<const char*>(c.data.data_at(0x00d5c3d8, sizeof("GuiDefault.mshd"))),
        static_cast<const char*>(c.data.data_at(0x00d5c47c, 18)),
        static_cast<const char*>(c.data.data_at(0x00d5c470, 10)),
        static_cast<const char*>(c.data.data_at(0x00d5c464, 11)),
        static_cast<const char*>(c.data.data_at(0x00d5c454, 13))};
    require(c.effects.effects.renderer_profile_00d5f0a8 == v.renderer &&
        &c.clip.actual_one_00d7a24c == v.one &&
        static_cast<const volatile void*>(&c.transform.one_00d7a24c) == v.one &&
        static_cast<const volatile void*>(&c.transform.negative_zero_00d7a208) == c.data.data_at(0x00d7a208, 4),
        "Raw FrameBox state requires already-held genuine aliases");
    for (const auto w : writes) {
        for (const auto r : {Region{v.box, 0x9c}, {v.renderer, 0x64}, {v.stream, 0x28},
            {v.material, 8}, {v.one, 4}, {v.declaration, sizeof("SimpleColor.mvfm")},
            {v.empty, 1}, {v.fade, sizeof("guifade.mshd")},
            {v.textured, sizeof("GuiDefault.mshd")}, {v.overbright, 18}, {v.low, 10},
            {v.high, 11}, {v.blend, 13}})
            require(separate(w, r), "Raw FrameBox state writable storage must preserve held aliases");
    }
    return v;
}
void return_captured_name(void* data, std::size_t index, Frame& f, Context& c, Acquired& a) {
    if (data) {
        const Word bytes = f.locals[index] + 1u;
        auto* pool = native_string_pool_get_or_create_00419cc0(
            c.strings.actual_published_01090aa8, c.strings.actual_manager_publication_01090aa0);
        return_native_string_pool_00bd1510(pool, data, bytes, c.strings.actual_small_returns_disabled_01090aa4);
    }
    ++a.completed_name_returns;
}
void normal_name_return(std::size_t index, Frame& f, Context& c, Acquired& a, Word site) {
    void* const data = reinterpret_cast<void*>(f.locals[index + 1]); // DATA first
    a.native_eh_state = -1;
    call(a, site);
    return_captured_name(data, index, f, c, a); // conditional CURRENT length
}
void cleanup(Frame& f, Context& c, Acquired& a) noexcept {
    a.exception_cleanup_started = true;
    const auto state = a.native_eh_state;
    if (state < 0) return;
    a.native_eh_state = -1; // every one of the seven native actions goes to-1
    try {
        std::size_t index;
        if (state == 0) index = 2;
        else if (state == 1) index = 5;
        else if (state == 2) index = 7;
        else if (state >= 3 && state <= 6) index = 0;
        else std::terminate();
        destroy_native_string_header_0041dd20(header(f, index), c.strings);
        ++a.completed_name_returns;
    } catch (...) { std::terminate(); }
}
template<class Resource>
void release(Resource*& creator, NativeTextureSurfaceReferenceIncrement decrement,
    NativeGuiFrameBoxStateStorageRelease& r, Context& c) {
    require(creator && decrement && !r.entered, "Raw FrameBox release requires one unconsumed reference");
    void* const raw = std::exchange(creator, nullptr);
    r.identity = raw;
    r.captured_decrement = decrement;
    r.entered = true;
    r.result = decrement(static_cast<volatile long*>(at(raw, 4)));
    r.decremented = true;
    if (r.result == 0) {
        r.zero_entered = true;
        dispatch_native_render_actual_owner_zero(c.owners, raw);
        r.zero_returned = true;
    }
    r.complete = true; // independent metadata only after possibly ended raw lifetime
}
void parameter(const char* literal, Word length, std::size_t source_offset,
    bool quartet, std::int32_t state, Word resize_site, Word register_site,
    Word return_site, void* box, Frame& f, Context& c, Acquired& a) {
    f.locals[0] = 0;
    f.locals[1] = 0;
    call(a, resize_site);
    resize_native_string_header_0041dd40(header(f, 0), c.strings, length, true);
    void* const data = reinterpret_cast<void*>(f.locals[1]);
    if (data) {
        const Word bytes = f.locals[0] + 1u;
        // Same qualified BF7680 Source boundary as raw41E870/166: overlap is
        // admitted; zero-byte copy is omitted. No new CRT leaf/ABI credit.
        if (bytes) std::memmove(data, literal, bytes);
    }
    a.native_eh_state = state;
    call(a, register_site);
    if (quartet)
        register_native_material_float4_00b18aa0(*a.material_creator, header(f, 0),
            at(box, source_offset), c.clip.materials);
    else
        register_native_material_float_00b18b20(*a.material_creator, header(f, 0),
            at(box, source_offset), c.clip.materials);
    normal_name_return(0, f, c, a, return_site);
}
__declspec(noinline) void copy_first_record(void* box, const void* record) noexcept {
    __asm {
        mov edx, box
        mov eax, record
        fld dword ptr [eax+14h]
        fstp dword ptr [edx+20h]
        fld dword ptr [eax+18h]
        fstp dword ptr [edx+24h]
        fld dword ptr [eax+0ch]
        fstp dword ptr [edx+18h]
        fld dword ptr [eax+10h]
        fstp dword ptr [edx+1ch]
    }
}
__declspec(noinline) void* copy_second_record(void* box, void* record) noexcept {
    void* const actual_size_pointer = at(record, 0x14);
    __asm {
        mov edx, box
        mov eax, actual_size_pointer
        fld dword ptr [eax-8]
        fstp dword ptr [edx+18h]
        fld dword ptr [eax-4]
        fstp dword ptr [edx+1ch]
        fld dword ptr [eax]
        fstp dword ptr [edx+20h]
        fld dword ptr [eax+4]
        fstp dword ptr [edx+24h]
    }
    return actual_size_pointer;
}
// Exact AD12B0..AD14F7 current-layout white stores, six groups of nine.
// No layout snapshot or helper calls; one live D7A24C MOVSS before the loop.
__declspec(noinline) void fill_white(void* stream, const volatile Word* packed,
    const volatile Word* one) noexcept {
    __asm {
        mov esi, stream
        mov eax, packed
        mov edx, dword ptr [eax]
        mov eax, one
        movss xmm0, dword ptr [eax]
        mov eax, 2
    white_00ad12b0:
        MOV ECX,dword ptr [ESI + 0x34]
        TEST ECX,ECX
        JL white_00ad12c6
        LEA EDI,[EAX + -0x2]
        IMUL EDI,dword ptr [ESI + 0xc]
        ADD EDI,dword ptr [ESI + 0x8]
        MOV dword ptr [EDI + ECX*0x1],EDX
        JMP white_00ad12f0
    white_00ad12c6:
        MOV EDI,dword ptr [ESI + 0x38]
        LEA ECX,[EAX + -0x2]
        IMUL ECX,dword ptr [ESI + 0xc]
        ADD ECX,dword ptr [ESI + 0x8]
        MOVSS dword ptr [ECX + EDI*0x1],XMM0
        MOV EDI,dword ptr [ESI + 0x3c]
        MOVSS dword ptr [ECX + EDI*0x1],XMM0
        MOV EDI,dword ptr [ESI + 0x40]
        MOVSS dword ptr [ECX + EDI*0x1],XMM0
        MOV EDI,dword ptr [ESI + 0x44]
        MOVSS dword ptr [ECX + EDI*0x1],XMM0
    white_00ad12f0:
        MOV ECX,dword ptr [ESI + 0x34]
        TEST ECX,ECX
        JL white_00ad1306
        LEA EDI,[EAX + -0x1]
        IMUL EDI,dword ptr [ESI + 0xc]
        ADD EDI,dword ptr [ESI + 0x8]
        MOV dword ptr [EDI + ECX*0x1],EDX
        JMP white_00ad1330
    white_00ad1306:
        MOV EDI,dword ptr [ESI + 0x38]
        LEA ECX,[EAX + -0x1]
        IMUL ECX,dword ptr [ESI + 0xc]
        ADD ECX,dword ptr [ESI + 0x8]
        MOVSS dword ptr [ECX + EDI*0x1],XMM0
        MOV EDI,dword ptr [ESI + 0x3c]
        MOVSS dword ptr [ECX + EDI*0x1],XMM0
        MOV EDI,dword ptr [ESI + 0x40]
        MOVSS dword ptr [ECX + EDI*0x1],XMM0
        MOV EDI,dword ptr [ESI + 0x44]
        MOVSS dword ptr [ECX + EDI*0x1],XMM0
    white_00ad1330:
        MOV ECX,dword ptr [ESI + 0x34]
        TEST ECX,ECX
        JL white_00ad1345
        MOV EDI,dword ptr [ESI + 0xc]
        IMUL EDI,EAX
        ADD EDI,dword ptr [ESI + 0x8]
        MOV dword ptr [EDI + ECX*0x1],EDX
        JMP white_00ad136e
    white_00ad1345:
        MOV ECX,dword ptr [ESI + 0xc]
        MOV EDI,dword ptr [ESI + 0x38]
        IMUL ECX,EAX
        ADD ECX,dword ptr [ESI + 0x8]
        MOVSS dword ptr [ECX + EDI*0x1],XMM0
        MOV EDI,dword ptr [ESI + 0x3c]
        MOVSS dword ptr [ECX + EDI*0x1],XMM0
        MOV EDI,dword ptr [ESI + 0x40]
        MOVSS dword ptr [ECX + EDI*0x1],XMM0
        MOV EDI,dword ptr [ESI + 0x44]
        MOVSS dword ptr [ECX + EDI*0x1],XMM0
    white_00ad136e:
        MOV ECX,dword ptr [ESI + 0x34]
        TEST ECX,ECX
        JL white_00ad1384
        LEA EDI,[EAX + 0x1]
        IMUL EDI,dword ptr [ESI + 0xc]
        ADD EDI,dword ptr [ESI + 0x8]
        MOV dword ptr [EDI + ECX*0x1],EDX
        JMP white_00ad13ae
    white_00ad1384:
        MOV EDI,dword ptr [ESI + 0x38]
        LEA ECX,[EAX + 0x1]
        IMUL ECX,dword ptr [ESI + 0xc]
        ADD ECX,dword ptr [ESI + 0x8]
        MOVSS dword ptr [ECX + EDI*0x1],XMM0
        MOV EDI,dword ptr [ESI + 0x3c]
        MOVSS dword ptr [ECX + EDI*0x1],XMM0
        MOV EDI,dword ptr [ESI + 0x40]
        MOVSS dword ptr [ECX + EDI*0x1],XMM0
        MOV EDI,dword ptr [ESI + 0x44]
        MOVSS dword ptr [ECX + EDI*0x1],XMM0
    white_00ad13ae:
        MOV ECX,dword ptr [ESI + 0x34]
        TEST ECX,ECX
        JL white_00ad13c4
        LEA EDI,[EAX + 0x2]
        IMUL EDI,dword ptr [ESI + 0xc]
        ADD EDI,dword ptr [ESI + 0x8]
        MOV dword ptr [EDI + ECX*0x1],EDX
        JMP white_00ad13ee
    white_00ad13c4:
        MOV EDI,dword ptr [ESI + 0x38]
        LEA ECX,[EAX + 0x2]
        IMUL ECX,dword ptr [ESI + 0xc]
        ADD ECX,dword ptr [ESI + 0x8]
        MOVSS dword ptr [ECX + EDI*0x1],XMM0
        MOV EDI,dword ptr [ESI + 0x3c]
        MOVSS dword ptr [ECX + EDI*0x1],XMM0
        MOV EDI,dword ptr [ESI + 0x40]
        MOVSS dword ptr [ECX + EDI*0x1],XMM0
        MOV EDI,dword ptr [ESI + 0x44]
        MOVSS dword ptr [ECX + EDI*0x1],XMM0
    white_00ad13ee:
        MOV ECX,dword ptr [ESI + 0x34]
        TEST ECX,ECX
        JL white_00ad1404
        LEA EDI,[EAX + 0x3]
        IMUL EDI,dword ptr [ESI + 0xc]
        ADD EDI,dword ptr [ESI + 0x8]
        MOV dword ptr [EDI + ECX*0x1],EDX
        JMP white_00ad142e
    white_00ad1404:
        MOV EDI,dword ptr [ESI + 0x38]
        LEA ECX,[EAX + 0x3]
        IMUL ECX,dword ptr [ESI + 0xc]
        ADD ECX,dword ptr [ESI + 0x8]
        MOVSS dword ptr [ECX + EDI*0x1],XMM0
        MOV EDI,dword ptr [ESI + 0x3c]
        MOVSS dword ptr [ECX + EDI*0x1],XMM0
        MOV EDI,dword ptr [ESI + 0x40]
        MOVSS dword ptr [ECX + EDI*0x1],XMM0
        MOV EDI,dword ptr [ESI + 0x44]
        MOVSS dword ptr [ECX + EDI*0x1],XMM0
    white_00ad142e:
        MOV ECX,dword ptr [ESI + 0x34]
        TEST ECX,ECX
        JL white_00ad1444
        LEA EDI,[EAX + 0x4]
        IMUL EDI,dword ptr [ESI + 0xc]
        ADD EDI,dword ptr [ESI + 0x8]
        MOV dword ptr [EDI + ECX*0x1],EDX
        JMP white_00ad146e
    white_00ad1444:
        MOV EDI,dword ptr [ESI + 0x38]
        LEA ECX,[EAX + 0x4]
        IMUL ECX,dword ptr [ESI + 0xc]
        ADD ECX,dword ptr [ESI + 0x8]
        MOVSS dword ptr [ECX + EDI*0x1],XMM0
        MOV EDI,dword ptr [ESI + 0x3c]
        MOVSS dword ptr [ECX + EDI*0x1],XMM0
        MOV EDI,dword ptr [ESI + 0x40]
        MOVSS dword ptr [ECX + EDI*0x1],XMM0
        MOV EDI,dword ptr [ESI + 0x44]
        MOVSS dword ptr [ECX + EDI*0x1],XMM0
    white_00ad146e:
        MOV ECX,dword ptr [ESI + 0x34]
        TEST ECX,ECX
        JL white_00ad1484
        LEA EDI,[EAX + 0x5]
        IMUL EDI,dword ptr [ESI + 0xc]
        ADD EDI,dword ptr [ESI + 0x8]
        MOV dword ptr [EDI + ECX*0x1],EDX
        JMP white_00ad14ae
    white_00ad1484:
        MOV EDI,dword ptr [ESI + 0x38]
        LEA ECX,[EAX + 0x5]
        IMUL ECX,dword ptr [ESI + 0xc]
        ADD ECX,dword ptr [ESI + 0x8]
        MOVSS dword ptr [ECX + EDI*0x1],XMM0
        MOV EDI,dword ptr [ESI + 0x3c]
        MOVSS dword ptr [ECX + EDI*0x1],XMM0
        MOV EDI,dword ptr [ESI + 0x40]
        MOVSS dword ptr [ECX + EDI*0x1],XMM0
        MOV EDI,dword ptr [ESI + 0x44]
        MOVSS dword ptr [ECX + EDI*0x1],XMM0
    white_00ad14ae:
        MOV ECX,dword ptr [ESI + 0x34]
        TEST ECX,ECX
        JL white_00ad14c4
        LEA EDI,[EAX + 0x6]
        IMUL EDI,dword ptr [ESI + 0xc]
        ADD EDI,dword ptr [ESI + 0x8]
        MOV dword ptr [EDI + ECX*0x1],EDX
        JMP white_00ad14ee
    white_00ad14c4:
        MOV EDI,dword ptr [ESI + 0x38]
        LEA ECX,[EAX + 0x6]
        IMUL ECX,dword ptr [ESI + 0xc]
        ADD ECX,dword ptr [ESI + 0x8]
        MOVSS dword ptr [ECX + EDI*0x1],XMM0
        MOV EDI,dword ptr [ESI + 0x3c]
        MOVSS dword ptr [ECX + EDI*0x1],XMM0
        MOV EDI,dword ptr [ESI + 0x40]
        MOVSS dword ptr [ECX + EDI*0x1],XMM0
        MOV EDI,dword ptr [ESI + 0x44]
        MOVSS dword ptr [ECX + EDI*0x1],XMM0
    white_00ad14ee:
        ADD EAX,0x9
        LEA ECX,[EAX + -0x2]
        CMP ECX,0x36
        JL white_00ad12b0
    }
}

} // namespace

void rebuild_native_gui_frame_box_state_storage_00ad0d80(void* box,
    Frame& f, Context& c, Acquired& a) {
    const auto views = preflight(box, f, c, a);
    a.started = true;
    try {
        const auto incoming = *reinterpret_cast<const volatile std::uint16_t*>(&f.argument_slot);
        a.captured_model = pointer(box, 0x4c); // BEFORE native EC store
        *reinterpret_cast<volatile std::uint16_t*>(at(box, 0xec)) = incoming;
        call(a, 0x00ad0daf);
        const auto& tail = *static_cast<const NativeModelTailStorage*>(at(a.captured_model, 0x174));
        a.captured_mesh = static_cast<NativeMeshStorage*>(gui_model_geometry_00b74640(tail, 0));
        f.locals[4] = reinterpret_cast<Word>(a.captured_mesh);
        call(a, 0x00ad0dbc);
        if (native_mesh_vertex_stream_count_00b72b20(a.captured_mesh) == 0) {
            call(a, 0x00ad0dd2);
            construct_native_string_header_0041e870(header(f, 2), c.strings, views.declaration);
            metadata(a);
            a.declaration_renderer = c.effects.effects.construction.current_renderer_00f8d394;
            auto* table = profile(a.declaration_renderer);
            require(table == views.renderer, "Raw FrameBox renderer38 requires its genuine current profile");
            a.declaration_target = table[0x38 / 4];
            require(a.declaration_target == 0x00b317e0, "Unsupported current renderer38 target");
            a.native_eh_state = 0;
            call(a, 0x00ad0def);
            a.declaration.reference = load_native_renderer_vertex_declaration_00b317e0(
                a.declaration_renderer, header(f, 2), c.renderer.declarations);
            metadata(a);
            c.geometry.register_native_declaration_reference(a.declaration, c.renderer.declarations.declarations);
            normal_name_return(2, f, c, a, 0x00ad0e0e);

            metadata(a);
            a.vertex_renderer = c.effects.effects.construction.current_renderer_00f8d394;
            table = profile(a.vertex_renderer);
            require(table == views.renderer, "Raw FrameBox renderer5C requires its genuine current profile");
            a.vertex_target = table[0x5c / 4];
            call(a, 0x00ad0e2a);
            create_native_vertex_from_captured_renderer_5c(a.vertex_renderer, a.vertex_target,
                54, 1, a.declaration.reference, c.renderer.streams, a.vertex);
            call(a, 0x00ad0e33);
            set_native_mesh_vertex_stream_00b73bb0(*a.captured_mesh, c.owners, 0, a.vertex.creator);
            a.stream_published = true;
            call(a, 0x00ad0e3c);
            const auto vertex_decrement = c.decrement_00ce2220;
            release(a.vertex.creator, vertex_decrement, a.vertex_release, c);
            call(a, 0x00ad0e52);
            const auto declaration_decrement = c.decrement_00ce2220;
            release(a.declaration.reference, declaration_decrement, a.declaration_release, c);
        }
        call(a, 0x00ad0e68);
        a.captured_stream = native_mesh_vertex_stream_unchecked_00b73260(a.captured_mesh, 0);
        metadata(a);
        auto* table = profile(a.captured_stream);
        require(table == views.stream, "Raw FrameBox mapping requires the genuine current logical vertex profile");
        a.map_target = table[0x10 / 4];
        require(a.map_target == 0x00b49980, "Unsupported current stream10 target");
        call(a, 0x00ad0e7c);
        a.map_entered = true;
        lock_native_logical_vertex_stream_00b49980(a.captured_stream, c.mapping, 54, 0, 0);
        a.map_returned = true;
        argument_flag(f) = 1;
        call(a, 0x00ad0e85);
        if (native_mesh_section_count_00b72b40(a.captured_mesh) == 0) {
            call(a, 0x00ad0e8e);
            a.section_creator = c.geometry.create_native_section_00533fa0(a.section);
            f.locals[2] = reinterpret_cast<Word>(a.section_creator);
            call(a, 0x00ad0ea0);
            construct_native_string_header_0041e870(header(f, 0), c.strings, views.empty);
            call(a, 0x00ad0eb2);
            const bool override_name = unequal_native_string_headers_00449af0(at(box, 0x104), header(f, 0));
            void* const empty_data = reinterpret_cast<void*>(f.locals[1]);
            call(a, 0x00ad0ecc);
            return_captured_name(empty_data, 0, f, c, a); // no armed EH state here
            const void* effect_name;
            std::size_t temporary_index = 0;
            if (override_name) effect_name = at(box, 0x104);
            else {
                const bool textured = byte(box, 0x100) != 0;
                temporary_index = textured ? 7u : 5u;
                call(a, textured ? 0x00ad11f2 : 0x00ad11a1);
                construct_native_string_header_0041e870(header(f, temporary_index), c.strings,
                    textured ? views.textured : views.fade);
                a.native_eh_state = textured ? 2 : 1;
                effect_name = header(f, temporary_index);
            }
            call(a, override_name ? 0x00ad0ee2 : temporary_index == 7 ? 0x00ad1203 : 0x00ad11b2);
            a.material_creator = create_native_material_from_effect_cache_00535320(effect_name,
                c.materials.material_slots, c.owners, c.effects, a.material);
            metadata(a);
            if (a.material_creator)
                c.geometry.register_native_material_creator(a.material, c.materials, views.material);
            if (!override_name)
                normal_name_return(temporary_index, f, c, a, 0x00ad11d8);
        } else {
            call(a, 0x00ad0eef);
            a.section_creator = static_cast<NativeMeshSectionStorage*>(
                gui_geometry_element_00b732c0(a.captured_mesh, 0));
            const auto increment = c.increment_00ce221c; // ONCE for both existing refs
            a.existing_pair_increment = increment;
            f.locals[2] = reinterpret_cast<Word>(a.section_creator);
            call(a, 0x00ad0f04);
            increment(static_cast<volatile long*>(at(a.section_creator, 4)));
            a.section_retained = true;
            a.material_creator = static_cast<NativeMaterialStorage*>(pointer(a.section_creator, 0x20));
            argument_flag(f) = 0;
            call(a, 0x00ad0f12);
            increment(static_cast<volatile long*>(at(a.material_creator, 4)));
            a.material_retained = true;
        }
        metadata(a);
        require(a.section_creator && a.material_creator,
            "Native null section/material dereference is outside valid FrameBox storage");
        parameter(views.overbright, 17, 0x94, false, 3, 0x00ad0f26, 0x00ad0f5f, 0x00ad0f7f, box, f, c, a);
        parameter(views.low, 9, 0xa4, true, 4, 0x00ad0f9b, 0x00ad0fd5, 0x00ad0ff5, box, f, c, a);
        parameter(views.high, 10, 0xb4, true, 5, 0x00ad1013, 0x00ad104c, 0x00ad106c, box, f, c, a);
        parameter(views.blend, 12, 0xc4, false, 6, 0x00ad108a, 0x00ad10c3, 0x00ad10e3, box, f, c, a);
        metadata(a);
        a.clip_preparation_entered = true;
        a.clip_operation = ::new (static_cast<void*>(f.clip_storage.bytes))
            NativeGuiWidgetClipOperation(box, *a.material_creator, c.clip);
        a.clip_constructed = true;
        call(a, 0x00ad10f2);
        a.clip_entered = true;
        register_native_gui_widget_clip_storage_00aa9f10(box, *a.material_creator, c.clip, *a.clip_operation);
        a.clip_returned = true;
        call(a, 0x00ad10fc);
        set_native_material_parameter_owner_00b18a40(*a.material_creator, box, 1, c.owners);
        a.owner_published = true;
        auto* const ranges = reinterpret_cast<NativeMeshSectionStorage*>(f.locals[2]);
        word(ranges, 0x08) = 4;
        word(ranges, 0x0c) = 0;
        word(ranges, 0x14) = 0;
        word(ranges, 0x10) = 54;
        word(ranges, 0x18) = 18;
        if (byte(box, 0x100) != 0) {
            const Word begin = word(box, 0xf4); // BEFORE current EC capture
            const auto index = static_cast<std::int32_t>(
                *reinterpret_cast<const volatile std::int16_t*>(at(box, 0xec)));
            bool valid = false;
            if (begin) {
                const Word difference = word(box, 0xf8) - begin;
                std::int32_t signed_difference;
                std::memcpy(&signed_difference, &difference, 4);
                valid = static_cast<Word>(index) < static_cast<Word>(signed_difference / 44);
            }
            if (!valid) {
                call(a, 0x00ad1158);
                invoke_native_invalid_parameter_00bf6713();
            }
            const Word selected = static_cast<Word>(index) * 44u + word(box, 0xf4);
            void* const record = reinterpret_cast<void*>(selected);
            a.captured_first_record = record;
            a.first_record_selected = true;
            void* const texture = pointer(record);
            call(a, 0x00ad116d);
            set_native_material_texture_00b189f0(*a.material_creator, 0, texture, c.owners);
            copy_first_record(box, record); // SAME saved record AFTER texture callbacks
        } else {
            call(a, 0x00ad1228);
            set_native_material_texture_00b189f0(*a.material_creator, 0, nullptr, c.owners);
        }
        const bool geometry_disabled = byte(box, 0x74) != 0;
        byte(header(f, 0), 2) = 0xff;
        byte(header(f, 0), 1) = 0xff;
        byte(header(f, 0), 0) = 0xff;
        byte(header(f, 0), 3) = 0xff;
        if (!geometry_disabled) {
            const void* size;
            const bool textured_geometry = byte(box, 0x100) != 0;
            if (textured_geometry) {
                const auto index = static_cast<Word>(static_cast<std::int32_t>(
                    *reinterpret_cast<const volatile std::int16_t*>(at(box, 0xec))));
                call(a, 0x00ad1260);
                void* const record = native_gui_state_vector_record_00acec40(at(box, 0xf0), index);
                a.captured_second_record = record;
                a.second_record_selected = true;
                size = copy_second_record(box, record);
            } else size = at(box, 0x20);
            metadata(a);
            table = profile(box);
            require(table == views.box, "Raw FrameBox geometry requires its genuine current profile");
            a.geometry_target = table[0x7c / 4];
            require(a.geometry_target == 0x00acf9b0, "Unsupported current FrameBox7C target");
            call(a, textured_geometry ? 0x00ad1289 : 0x00ad129a);
            a.geometry_entered = true;
            write_native_gui_frame_box_geometry_00acf9b0(box, a.captured_stream, size, c.geometry_storage, f.geometry);
            a.geometry_returned = true;
            fill_white(a.captured_stream, &f.locals[0], views.one);
        }
        metadata(a);
        table = profile(a.captured_stream);
        require(table == views.stream, "Raw FrameBox unlock requires its genuine current logical vertex profile");
        a.unlock_target = table[0x14 / 4];
        require(a.unlock_target == 0x00b49a80, "Unsupported current stream14 target");
        call(a, 0x00ad1504);
        a.unlock_entered = true;
        unlock_native_logical_vertex_stream_00b49a80(a.captured_stream, c.mapping);
        a.unlocked = true;
        call(a, 0x00ad1508);
        a.transform_entered = true;
        recompose_native_gui_widget_transform_00aa7220(box, f.transform, c.transform);
        a.transform_returned = true;
        auto* const section = reinterpret_cast<NativeMeshSectionStorage*>(f.locals[2]);
        call(a, 0x00ad1514);
        set_native_mesh_section_material_00b864c0(*section, c.owners, a.material_creator);
        a.material_published = true;
        const auto final_decrement = c.decrement_00ce2220; // AD1519 ONCE
        a.final_pair_decrement = final_decrement;
        call(a, 0x00ad1523);
        release(a.material_creator, final_decrement, a.material_release, c);
        auto* const mesh = reinterpret_cast<NativeMeshStorage*>(f.locals[4]);
        call(a, 0x00ad1538);
        rebuild_native_mesh_section_vertex_layout_00b865a0(*section, c.owners, mesh, c.layout);
        if (argument_flag(f) != 0) {
            call(a, 0x00ad1547);
            append_native_mesh_draw_section_00b73c60(*mesh, section);
            a.section_appended = true;
        }
        call(a, 0x00ad1550);
        release(a.section_creator, final_decrement, a.section_release, c);
        metadata(a);
        table = profile(box);
        require(table == views.box, "Raw FrameBox color requires its genuine current profile");
        a.color_target = table[0x50 / 4];
        require(a.color_target == 0x00aa6870, "Unsupported current FrameBox50 target");
        call(a, 0x00ad156a);
        a.color_entered = true;
        set_native_gui_widget_color_storage_00aa6870(box, at(box, 0x50));
        a.color_returned = true;
        a.complete = true;
        a.boundary = Acquired::Boundary::none;
    } catch (...) {
        a.failed = true;
        if (a.boundary != Acquired::Boundary::host_metadata) cleanup(f, c, a);
        throw;
    }
}
} // namespace bsp
