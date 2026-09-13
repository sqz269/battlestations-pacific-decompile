#include "bsp/native_vertex_shader_compilation.hpp"
#include "bsp/native_physical_file_date.hpp"
#include "bsp/native_shader_device_reset.hpp"
#include "bsp/native_vfs_runtime_bindings.hpp"
#include <d3dx9shader.h>
#include <cstring>
#include <exception>
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native vertex shader compilation requires MSVC Win32.
#endif

namespace bsp {
namespace {
std::uint32_t word(const void* p, std::uint32_t offset = 0) noexcept {
    return *reinterpret_cast<const volatile std::uint32_t*>(static_cast<const char*>(p) + offset);
}
void put(void* p, std::uint32_t offset, std::uint32_t value) noexcept {
    *reinterpret_cast<volatile std::uint32_t*>(static_cast<char*>(p) + offset) = value;
}
void* pointer(const void* p, std::uint32_t offset = 0) noexcept {
    return *reinterpret_cast<void* const volatile*>(static_cast<const char*>(p) + offset);
}
template<class T> T import_function(FARPROC raw) noexcept {
    static_assert(sizeof(T) == sizeof(raw));
    T result;
    std::memcpy(&result, &raw, sizeof(result));
    return result;
}
void release_name(NativeVertexShaderCompilationOperation& a, NativeString& name,
    std::uint32_t bit, std::uint32_t site) noexcept {
    a.native_site = site;
    // Inline caller cleanup leaves the header's stale words untouched.
    destroy_native_string_header_0041dd20(&name, a.context->strings);
    a.names_released |= bit;
}
}

std::uint32_t write_native_physical_stream_00bf4f50(void* stream, const void* data,
    std::uint32_t requested, std::uint32_t* actual_count) {
    // WriteFile receives the original requested stack cell as its output.
    DWORD written = requested;
    (void)WriteFile(static_cast<HANDLE>(pointer(stream, 8)), data, requested, &written, nullptr);
    const auto low = word(stream, 0x10);
    const auto sum = low + written;
    put(stream, 0x10, sum);
    put(stream, 0x14, word(stream, 0x14) + (sum < low ? 1u : 0u));
    if (actual_count) *actual_count = written;
    return written;
}

std::uint32_t write_native_shader_diagnostic_string_00be4430(void* stream,
    const void* name, std::uint32_t* actual_count) {
    const auto length = word(name);
    const auto* data = static_cast<const char*>(pointer(name, 4));
    // Original fallback is the empty literal at 0109DB64.
    if (!data) data = reinterpret_cast<const char*>(0x0109db64);
    const auto table = word(stream);
    const auto target = word(reinterpret_cast<const void*>(table), 0x28);
    if (target != 0x00bf4f50)
        throw std::invalid_argument("BE4430 requires reconstructed physical write slot28 BF4F50");
    return write_native_physical_stream_00bf4f50(stream, data, length, actual_count);
}

NativeVertexShaderD3dxImports::NativeVertexShaderD3dxImports(HMODULE module) {
    if (!module) throw std::invalid_argument("actual d3dx9_40 module is required");
    compile_ = GetProcAddress(module, "D3DXCompileShader");
    disassemble_ = GetProcAddress(module, "D3DXDisassembleShader");
    if (!compile_ || !disassemble_)
        throw std::invalid_argument("required d3dx9_40 compile/disassemble exports are absent");
}
HRESULT NativeVertexShaderD3dxImports::compile(const char* source, std::uint32_t length,
    const char* profile, ID3DXBuffer** code, ID3DXBuffer** messages) const {
    return import_function<decltype(&D3DXCompileShader)>(compile_)(source, length, nullptr,
        nullptr, "main", profile, 0x1200, code, messages, nullptr);
}
HRESULT NativeVertexShaderD3dxImports::disassemble(const DWORD* code, ID3DXBuffer** text) const {
    return import_function<decltype(&D3DXDisassembleShader)>(disassemble_)(code, FALSE, nullptr, text);
}

NativeVertexShaderCompilationOperation::~NativeVertexShaderCompilationOperation() {
    if (phase == Phase::running || phase == Phase::failed || code || messages || disassembly || stream)
        std::terminate();
}
void NativeVertexShaderCompilationOperation::acknowledge_diagnostic_cleanup() noexcept {
    if (phase == Phase::running || code || messages || disassembly || stream ||
        names_entered != names_released) std::terminate();
    phase = Phase::diagnostic_retired;
}

HRESULT compile_native_vertex_shader_00b60f60(const void* engine_name, const char* profile,
    const char* source, IDirect3DVertexShader9** output, NativeVertexShaderCompilationContext& c,
    NativeVertexShaderCompilationOperation& a) {
    if (a.phase != NativeVertexShaderCompilationOperation::Phase::fresh)
        throw std::logic_error("B60F60 operation is one-shot");
    a.phase = NativeVertexShaderCompilationOperation::Phase::running;
    a.engine_name = engine_name; a.profile = profile; a.source = source;
    a.output = output; a.context = &c;
    try {
        a.native_site = 0x00b60f86;
        a.captured_device = static_cast<IDirect3DDevice9*>(
            get_native_renderer_device_00b1fef0(c.actual_renderer_00f8d394));
        a.native_site = 0x00b60fc7;
        a.compile_result = c.d3dx.compile(source, static_cast<std::uint32_t>(std::strlen(source)),
            profile, &a.code, &a.messages);
        if (!a.code) {
            a.phase = NativeVertexShaderCompilationOperation::Phase::complete;
            return a.compile_result; // Native does not release returned messages here.
        }
        a.native_site = 0x00b60fe9;
        const auto* bytecode = static_cast<const DWORD*>(a.code->GetBufferPointer());
        a.native_site = 0x00b60fec;
        a.disassembly_result = c.d3dx.disassemble(bytecode, &a.disassembly);
        a.captured_manager = c.actual_manager_0109ceec; // FF1: before ANY name allocation.
        a.native_site = 0x00b61000; a.names_entered |= 1u;
        construct_native_string_cstring_0041e870(&a.suffix, ".vsa", c.strings);
        a.names_returned |= 1u;
        a.native_site = 0x00b61018; a.names_entered |= 2u;
        construct_native_string_cstring_0041e870(&a.prefix, "shaderfx/debug/", c.strings);
        a.names_returned |= 2u;
        a.native_site = 0x00b6102a; a.names_entered |= 4u;
        concatenate_native_string_headers_004261a0(&a.prefix, &a.stem, engine_name, c.strings);
        a.names_returned |= 4u;
        a.native_site = 0x00b6103d; a.names_entered |= 8u;
        concatenate_native_string_headers_004261a0(&a.stem, &a.path, &a.suffix, c.strings);
        a.names_returned |= 8u;
        a.native_site = 0x00b61051;
        const auto open_table = word(a.captured_manager);
        const auto open_target = word(reinterpret_cast<const void*>(open_table), 4);
        a.stream = c.vfs.open_manager_entry(open_target, a.captured_manager, &a.path, 0x35);
        release_name(a, a.path, 8, 0x00b61075);
        release_name(a, a.stem, 4, 0x00b61099);
        release_name(a, a.prefix, 2, 0x00b610bd);
        release_name(a, a.suffix, 1, 0x00b610e3);
        a.native_site = 0x00b610f2;
        if (!a.disassembly)
            throw std::runtime_error("B60F60 native disassembly dereference lacks actual COM output");
        const auto* assembly = static_cast<const char*>(a.disassembly->GetBufferPointer());
        a.native_site = 0x00b610f9; a.names_entered |= 16u;
        construct_native_string_cstring_0041e870(&a.text, assembly, c.strings);
        a.names_returned |= 16u;
        a.native_site = 0x00b6110f;
        const auto write_table = word(a.stream);
        const auto write_target = word(reinterpret_cast<const void*>(write_table), 0x5c);
        if (write_target != 0x00be4430)
            throw std::invalid_argument("B60F60 requires diagnostic string slot5C BE4430");
        (void)write_native_shader_diagnostic_string_00be4430(a.stream, &a.text, nullptr);
        release_name(a, a.text, 16, 0x00b6112f);
        a.native_site = 0x00b61138;
        const auto remaining = InterlockedDecrement(
            reinterpret_cast<volatile LONG*>(static_cast<char*>(a.stream) + 4));
        a.stream_reference_released = true;
        if (!remaining) {
            a.native_site = 0x00b61148;
            c.vfs.zero_reference(word(a.stream), a.stream);
        }
        a.stream = nullptr; // Sidecar only, after native terminal returns.
        a.native_site = 0x00b61158;
        if (a.disassembly) { (void)a.disassembly->Release(); a.disassembly = nullptr; }
        // B61166 captures device vtable BEFORE bytecode GetBufferPointer.
        const auto device_table = word(a.captured_device);
        a.native_site = 0x00b61173;
        bytecode = static_cast<const DWORD*>(a.code->GetBufferPointer());
        using Create = HRESULT (WINAPI*)(IDirect3DDevice9*, const DWORD*, IDirect3DVertexShader9**);
        const auto create = reinterpret_cast<Create>(word(reinterpret_cast<const void*>(device_table), 0x16c));
        a.native_site = 0x00b6117d;
        a.creation_result = create(a.captured_device, bytecode, output);
        a.shader_creation_returned = true;
        a.native_site = 0x00b6118f;
        if (a.code) { (void)a.code->Release(); a.code = nullptr; }
        a.native_site = 0x00b611a3;
        if (a.messages) { (void)a.messages->Release(); a.messages = nullptr; }
        a.phase = NativeVertexShaderCompilationOperation::Phase::complete;
        return a.creation_result;
    } catch (...) {
        a.phase = NativeVertexShaderCompilationOperation::Phase::failed;
        throw;
    }
}
} // namespace bsp
