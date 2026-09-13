#pragma once
#include "bsp/native_vertex_shader_compilation.hpp"

namespace bsp {
// Concrete exports from the borrowed, installed d3dx9_40 module.
class NativePixelShaderD3dxImports final {
public:
    explicit NativePixelShaderD3dxImports(HMODULE actual_d3dx9_40);
    HRESULT compile(const char*, std::uint32_t length, const char* profile,
        std::uint32_t flags, ID3DXBuffer** code, ID3DXBuffer** messages) const;
    HRESULT disassemble(const DWORD*, ID3DXBuffer**) const;
private:
    FARPROC compile_{}, disassemble_{};
};
struct NativePixelShaderCompilationContext {
    NativeStringStorage& strings;
    void* const volatile& actual_renderer_00f8d394;
    void* const volatile& actual_manager_0109ceec;
    NativeVfsRuntimeBindings& vfs;
    const NativePixelShaderD3dxImports& d3dx;
};
// One-shot retained call frame, not a replacement native owner or FH3 frame.
// Borrowed inputs/context/module must survive the call; caller excludes native
// owner retirement while running/failed. Child string helpers retain their
// established exception domains, including cleanup of a failed substring's
// temporary. An unfinished header is a preimage, not proof of buffer ownership.
struct NativePixelShaderCompilationOperation final {
    enum class Phase { fresh, running, complete, failed, diagnostic_retired };
    enum Name : std::uint32_t { suffix, prefix, stem, path, text, full, line, index, substring, name_count };
    Phase phase{Phase::fresh};
    std::uint32_t native_site{};
    const void* engine_name{};
    const char* profile{};
    const char* source{};
    IDirect3DPixelShader9** output{};
    std::uint32_t* texcoord_masks{};
    std::uint32_t* color_masks{};
    NativePixelShaderCompilationContext* context{};
    IDirect3DDevice9* captured_device{};
    void* captured_manager{};
    void* stream{};
    ID3DXBuffer* code{};
    ID3DXBuffer* messages{};
    ID3DXBuffer* disassembly{};
    HRESULT compile_result{}, creation_result{}, disassembly_result{};
    std::uint32_t compile_flags{}, cursor{}, match{}, newline{}, parsed_index{};
    char* captured_line_data{};
    char* captured_full_data{};
    NativeString names[name_count];
    // Active includes an entered helper that did not return. Returned only
    // describes the current instance of each reusable native stack header.
    std::uint32_t active_names{}, returned_names{}, strings_constructed{}, strings_released{};
    bool shader_creation_returned{}, stream_reference_released{}, suffix_psa2{}, parsing_started{};
    NativePixelShaderCompilationOperation() = default;
    ~NativePixelShaderCompilationOperation();
    NativePixelShaderCompilationOperation(const NativePixelShaderCompilationOperation&) = delete;
    NativePixelShaderCompilationOperation& operator=(const NativePixelShaderCompilationOperation&) = delete;
    // Only after independently resolving COM, stream, native strings and any
    // failed child. Does not resume the operation or claim native completion.
    void acknowledge_diagnostic_cleanup() noexcept;
};

// Full normal B61280 path over actual 8h strings, real D3DX/HAL/VFS owners.
// Native fastcall ECX name, EDX profile, stack source/outShader/texcoord/color;
// EAX HRESULT, RET10h. New C++ interface, not a drop-in native ABI replacement.
// Flags1400 unless case-sensitive name substring "shore"; create precedes
// disassembly. Both masks enable diagnostics; suffix and post-stream parser
// independently read texcoord[0]==500. Disassembly HRESULT replaces create HR.
// Parser assumes the native valid domain: readable LF-terminated declarations
// and writable mask indices; it neither clears masks nor bounds their extent.
// Missing disassembly and unsupported write profiles fail explicitly instead
// of reproducing native invalid dereferences. Code-null messages stay retained.
HRESULT compile_native_pixel_shader_00b61280(const void* actual_engine_name,
    const char* profile, const char* source, IDirect3DPixelShader9** actual_output,
    std::uint32_t* actual_texcoord_masks, std::uint32_t* actual_color_masks,
    NativePixelShaderCompilationContext&, NativePixelShaderCompilationOperation&);
} // namespace bsp
