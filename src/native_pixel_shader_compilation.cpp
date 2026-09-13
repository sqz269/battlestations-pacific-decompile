#include "bsp/native_pixel_shader_compilation.hpp"
#include "bsp/native_physical_file_date.hpp"
#include "bsp/native_pooled_string_substring.hpp"
#include "bsp/native_shader_device_reset.hpp"
#include "bsp/native_vfs_runtime_bindings.hpp"
#include <d3dx9shader.h>
#include <cstring>
#include <cstdlib>
#include <exception>
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native pixel shader compilation requires MSVC Win32.
#endif
namespace bsp {
namespace {
using Op = NativePixelShaderCompilationOperation;
using U = std::uint32_t;
U word(const void* p, U offset = 0) noexcept {
    return *reinterpret_cast<const volatile U*>(static_cast<const char*>(p) + offset);
}
char* data(const void* p) noexcept { return reinterpret_cast<char*>(word(p, 4)); }
template<class T> T import_function(FARPROC raw) noexcept {
    static_assert(sizeof(T) == sizeof(raw));
    T result; std::memcpy(&result, &raw, sizeof(result)); return result;
}
void entered(Op& a, Op::Name id, U site) noexcept {
    a.native_site = site; a.active_names |= 1u << id; a.returned_names &= ~(1u << id);
}
void returned(Op& a, Op::Name id) noexcept {
    a.returned_names |= 1u << id; ++a.strings_constructed;
}
void released(Op& a, Op::Name id) noexcept {
    a.active_names &= ~(1u << id); a.returned_names &= ~(1u << id); ++a.strings_released;
}
void release(Op& a, Op::Name id, U site) noexcept {
    a.native_site = site;
    destroy_native_string_header_0041dd20(&a.names[id], a.context->strings);
    released(a, id); // Native header words remain stale.
}
void construct(Op& a, Op::Name id, const char* value, U site) {
    entered(a, id, site);
    construct_native_string_cstring_0041e870(&a.names[id], value, a.context->strings);
    returned(a, id);
}
bool contains(const char* start, const char* needle) {
    if (!start) return false;
    const auto* found = std::strstr(start, needle);
    return found && static_cast<U>(reinterpret_cast<U>(found) - reinterpret_cast<U>(start)) != 0xffffffffu;
}
U find_from(const void* header, const char* needle, U cursor, bool subtract_current_base = false) {
    const auto* base = data(header);
    if (!base) return 0xffffffffu;
    if (cursor & 0x80000000u) cursor = 0;
    else if (cursor > word(header)) return 0xffffffffu;
    const auto* found = std::strstr(reinterpret_cast<const char*>(reinterpret_cast<U>(base) + cursor), needle);
    // Declaration searches subtract captured ESI/EDI; newline searches at
    // B61714/B6192E reload the full header's CURRENT data after strstr.
    return found ? reinterpret_cast<U>(found) - reinterpret_cast<U>(subtract_current_base ? data(header) : base) : 0xffffffffu;
}
void parse_declarations(Op& a, bool color) {
    const U index_site = color ? 0x00b618b0u : 0x00b6169cu;
    const U body_site = color ? 0x00b61950u : 0x00b61730u;
    auto& c = *a.context;
    auto& full = a.names[Op::full];
    auto& line = a.names[Op::line];
    auto& index = a.names[Op::index];
    auto& part = a.names[Op::substring];
    auto* masks = color ? a.color_masks : a.texcoord_masks;
    a.cursor = 0;
    for (;;) {
        // Color EDI persists into final full-string cleanup across line release.
        a.captured_full_data = data(&full);
        a.native_site = color ? 0x00b61884 : 0x00b61670;
        a.match = find_from(&full, color ? "dcl_color" : "dcl_texcoord", a.cursor);
        if (a.match == 0xffffffffu) break;
        entered(a, Op::index, index_site);
        construct_native_string_substring_00469840(&full, &index,
            a.match + (color ? 9u : 12u), 1, c.strings);
        returned(a, Op::index);
        a.native_site = color ? 0x00b618c2 : 0x00b616ae;
        const auto* index_data = data(&index);
        a.parsed_index = static_cast<U>(std::atol(index_data ? index_data : ""));
        release(a, Op::index, color ? 0x00b618e6 : 0x00b616d2);
        const U body_start = a.match + (color ? 11u : 14u);
        a.native_site = color ? 0x00b61902 : 0x00b616ee;
        a.newline = find_from(&full, "\n", body_start, true);
        entered(a, Op::substring, body_site);
        construct_native_string_substring_00469840(&full, &part,
            body_start, a.newline - body_start, c.strings);
        returned(a, Op::substring);
        a.native_site = color ? 0x00b61969 : 0x00b61749;
        resize_native_string_header_0041dd40(&line, c.strings, word(&part), true);
        // TEXCOORD EBX is captured immediately after resize; COLOR ESI is
        // captured before memcpy (or in the empty branch). Neither is reloaded
        // after substring cleanup, which can observe/change the line header.
        a.captured_line_data = data(&line);
        if (word(&part)) {
            const U count = word(&line);
            a.native_site = color ? 0x00b61981 : 0x00b61761;
            if (count) std::memmove(a.captured_line_data, data(&part), count);
        }
        release(a, Op::substring, color ? 0x00b619ae : 0x00b61788);
        bool any = false;
        static constexpr const char* components[] = {"x", "y", "z", "w"};
        static constexpr U tex_sites[] = {0xb6179b, 0xb617c4, 0xb617e6, 0xb61808};
        static constexpr U color_sites[] = {0xb619c1, 0xb619e3, 0xb61a05, 0xb61a27};
        for (U component = 0; component != 4; ++component) {
            a.native_site = color ? color_sites[component] : tex_sites[component];
            if (contains(a.captured_line_data, components[component])) {
                *reinterpret_cast<volatile U*>(reinterpret_cast<U>(masks) + a.parsed_index * 4u) |= 1u << component;
                any = true; // Each native OR occurs before the next strstr.
            }
        }
        if (!any)
            *reinterpret_cast<volatile U*>(reinterpret_cast<U>(masks) + a.parsed_index * 4u) |= 15u;
        a.cursor = a.newline; // Native miss -1 restarts; no invented progress guard.
    }
}
} // namespace

NativePixelShaderD3dxImports::NativePixelShaderD3dxImports(HMODULE module) {
    if (!module) throw std::invalid_argument("actual d3dx9_40 module is required");
    compile_ = GetProcAddress(module, "D3DXCompileShader");
    disassemble_ = GetProcAddress(module, "D3DXDisassembleShader");
    if (!compile_ || !disassemble_) throw std::invalid_argument("required d3dx9_40 exports absent");
}
HRESULT NativePixelShaderD3dxImports::compile(const char* source, U length,
    const char* profile, U flags, ID3DXBuffer** code, ID3DXBuffer** messages) const {
    return import_function<decltype(&D3DXCompileShader)>(compile_)(source, length,
        nullptr, nullptr, "main", profile, flags, code, messages, nullptr);
}
HRESULT NativePixelShaderD3dxImports::disassemble(const DWORD* code, ID3DXBuffer** out) const {
    return import_function<decltype(&D3DXDisassembleShader)>(disassemble_)(code, FALSE, nullptr, out);
}
NativePixelShaderCompilationOperation::~NativePixelShaderCompilationOperation() {
    if (phase == Phase::running || phase == Phase::failed || code || messages || disassembly || stream || active_names)
        std::terminate();
}
void NativePixelShaderCompilationOperation::acknowledge_diagnostic_cleanup() noexcept {
    if (phase == Phase::running || code || messages || disassembly || stream || active_names)
        std::terminate();
    phase = Phase::diagnostic_retired;
}
HRESULT compile_native_pixel_shader_00b61280(const void* engine_name, const char* profile,
    const char* source, IDirect3DPixelShader9** output, U* texcoord, U* color,
    NativePixelShaderCompilationContext& c, NativePixelShaderCompilationOperation& a) {
    if (a.phase != Op::Phase::fresh) throw std::logic_error("B61280 operation is one-shot");
    a.phase = Op::Phase::running;
    a.engine_name = engine_name; a.profile = profile; a.source = source; a.output = output;
    a.texcoord_masks = texcoord; a.color_masks = color; a.context = &c;
    try {
        a.native_site = 0x00b612a6;
        a.captured_device = static_cast<IDirect3DDevice9*>(get_native_renderer_device_00b1fef0(c.actual_renderer_00f8d394));
        a.native_site = 0x00b612c4;
        const auto* initial_name_data = data(engine_name);
        const auto* shore = initial_name_data ? std::strstr(initial_name_data, "shore") : nullptr;
        // B612D0 reloads name+04 AFTER the CRT call, unlike parser searches
        // whose offsets subtract the earlier captured base register.
        a.compile_flags = shore && reinterpret_cast<U>(shore) - reinterpret_cast<U>(data(engine_name)) != 0xffffffffu
            ? 0u : 0x1400u;
        a.native_site = 0x00b6132b;
        a.compile_result = c.d3dx.compile(source, static_cast<U>(std::strlen(source)), profile,
            a.compile_flags, &a.code, &a.messages);
        if (!a.code) { a.phase = Op::Phase::complete; return a.compile_result; }
        const U device_table = word(a.captured_device); // Captured before GetBufferPointer.
        a.native_site = 0x00b61352;
        const auto* bytecode = static_cast<const DWORD*>(a.code->GetBufferPointer());
        using Create = HRESULT (WINAPI*)(IDirect3DDevice9*, const DWORD*, IDirect3DPixelShader9**);
        const auto create = reinterpret_cast<Create>(word(reinterpret_cast<void*>(device_table), 0x1a8));
        a.native_site = 0x00b61358;
        a.creation_result = create(a.captured_device, bytecode, output);
        a.shader_creation_returned = true;
        HRESULT result = a.creation_result;
        if (texcoord && color) {
            a.native_site = 0x00b6139b;
            bytecode = static_cast<const DWORD*>(a.code->GetBufferPointer());
            a.native_site = 0x00b6139e;
            result = a.disassembly_result = c.d3dx.disassemble(bytecode, &a.disassembly);
            a.captured_manager = c.actual_manager_0109ceec;
            a.suffix_psa2 = word(texcoord) == 500;
            const bool second = a.suffix_psa2;
            construct(a, Op::suffix, second ? ".psa2" : ".psa1", second ? 0xb614af : 0xb613c9);
            construct(a, Op::prefix, "shaderfx/debug/", second ? 0xb614c7 : 0xb613e1);
            entered(a, Op::stem, second ? 0xb614d9 : 0xb613f3);
            concatenate_native_string_headers_004261a0(&a.names[Op::prefix], &a.names[Op::stem], engine_name, c.strings);
            returned(a, Op::stem);
            entered(a, Op::path, second ? 0xb614ec : 0xb61406);
            concatenate_native_string_headers_004261a0(&a.names[Op::stem], &a.names[Op::path], &a.names[Op::suffix], c.strings);
            returned(a, Op::path);
            a.native_site = second ? 0xb61500 : 0xb6141a;
            const U open_table = word(a.captured_manager);
            a.stream = c.vfs.open_manager_entry(word(reinterpret_cast<void*>(open_table), 4), a.captured_manager, &a.names[Op::path], 0x35);
            release(a, Op::path, second ? 0xb61522 : 0xb6143c);
            release(a, Op::stem, second ? 0xb61546 : 0xb61460);
            release(a, Op::prefix, second ? 0xb6156a : 0xb61484);
            release(a, Op::suffix, 0xb61591);
            a.native_site = 0xb615a3;
            if (!a.disassembly) throw std::runtime_error("B61280 native disassembly dereference lacks actual COM output");
            const auto* assembly = static_cast<const char*>(a.disassembly->GetBufferPointer());
            construct(a, Op::text, assembly, 0xb615aa);
            a.native_site = 0xb615c1;
            const U write_table = word(a.stream);
            if (word(reinterpret_cast<void*>(write_table), 0x5c) != 0xbe4430)
                throw std::invalid_argument("B61280 requires diagnostic slot5C BE4430");
            (void)write_native_shader_diagnostic_string_00be4430(a.stream, &a.names[Op::text], nullptr);
            release(a, Op::text, 0xb615e5);
            a.native_site = 0xb615ee;
            const auto remaining = InterlockedDecrement(reinterpret_cast<volatile LONG*>(static_cast<char*>(a.stream) + 4));
            a.stream_reference_released = true;
            if (!remaining) { a.native_site = 0xb615fe; c.vfs.zero_reference(word(a.stream), a.stream); }
            a.stream = nullptr;
            if (word(texcoord) != 500) { // Independent live read AFTER stream cleanup.
                a.parsing_started = true; a.native_site = 0xb61620;
                assembly = static_cast<const char*>(a.disassembly->GetBufferPointer());
                construct(a, Op::full, assembly, 0xb61627);
                entered(a, Op::line, 0xb61638);
                *reinterpret_cast<volatile U*>(&a.names[Op::line]) = 0;
                *reinterpret_cast<volatile U*>(reinterpret_cast<char*>(&a.names[Op::line]) + 4) = 0;
                returned(a, Op::line);
                parse_declarations(a, false);
                parse_declarations(a, true);
                release(a, Op::line, 0xb61a80);
                a.native_site = 0xb61aa3;
                if (a.captured_full_data) c.strings.release(a.captured_full_data, word(&a.names[Op::full]) + 1u);
                released(a, Op::full);
            }
            a.native_site = 0xb61ab9;
            if (a.disassembly) { (void)a.disassembly->Release(); a.disassembly = nullptr; }
        }
        a.native_site = 0xb61ad4;
        if (a.code) { (void)a.code->Release(); a.code = nullptr; }
        a.native_site = 0xb61aec;
        if (a.messages) { (void)a.messages->Release(); a.messages = nullptr; }
        a.phase = Op::Phase::complete;
        return result;
    } catch (...) { a.phase = Op::Phase::failed; throw; }
}
} // namespace bsp
