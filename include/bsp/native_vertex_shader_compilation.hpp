#pragma once
#include "bsp/native_string.hpp"
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <d3d9.h>
#include <cstdint>

struct ID3DXBuffer;
namespace bsp {
class NativeVfsRuntimeBindings;

// BF4F50: ECX actual BF50D0 20h stream, stack data/requested/count, RET0C.
// Real synchronous WriteFile, ignored BOOL, add actual bytes to CURRENT
// position10/14, then publish optional count. Cached size18/1C is untouched.
std::uint32_t write_native_physical_stream_00bf4f50(void* actual_stream,
    const void* data, std::uint32_t requested, std::uint32_t* actual_count);
// BE4430: ECX stream, stack actual8h string/count, RET8. Captures length/data
// before current table; forwards to CURRENT28. Concrete BF4F50 binding only;
// other numeric methods are explicit source boundaries. No owner conversion.
std::uint32_t write_native_shader_diagnostic_string_00be4430(void* actual_stream,
    const void* actual_string, std::uint32_t* actual_count);

// Borrow the installed d3dx9_40 module, resolved by the actual export names.
// No replacement compiler callbacks, fallback API, module ownership or tables.
class NativeVertexShaderD3dxImports final {
public:
    explicit NativeVertexShaderD3dxImports(HMODULE actual_d3dx9_40);
    HRESULT compile(const char* source, std::uint32_t length, const char* profile,
        ID3DXBuffer** code, ID3DXBuffer** messages) const;
    HRESULT disassemble(const DWORD* code, ID3DXBuffer** text) const;
private:
    FARPROC compile_{};
    FARPROC disassemble_{};
};

struct NativeVertexShaderCompilationContext {
    NativeStringStorage& strings; // SAME actual pool used by the VFS binding.
    void* const volatile& actual_renderer_00f8d394;
    void* const volatile& actual_manager_0109ceec;
    NativeVfsRuntimeBindings& vfs; // SAME manager, pool, stream/lifetime domains.
    const NativeVertexShaderD3dxImports& d3dx;
};

// Stable one-shot source call frame, not another native shader/stream owner.
// Caller retains all borrowed inputs/context/module and excludes terminal
// admission until completion or explicit diagnostic resolution. No FH3 unwind
// or automatic COM/string cleanup is invented. Existing concat helper keeps
// its established child exception domain; a failed child header is a preimage,
// not proof that its buffer remains owned. See the document's failure table.
struct NativeVertexShaderCompilationOperation final {
    enum class Phase { fresh, running, complete, failed, diagnostic_retired };
    Phase phase{Phase::fresh};
    std::uint32_t native_site{};
    const void* engine_name{};
    const char* profile{};
    const char* source{};
    IDirect3DVertexShader9** output{};
    NativeVertexShaderCompilationContext* context{};
    IDirect3DDevice9* captured_device{};
    void* captured_manager{};
    void* stream{};
    ID3DXBuffer* code{};
    ID3DXBuffer* messages{};
    ID3DXBuffer* disassembly{};
    HRESULT compile_result{}, disassembly_result{}, creation_result{};
    NativeString suffix, prefix, stem, path, text;
    // Bits correspond to suffix,prefix,stem,path,text. Entered includes a
    // failed helper; returned records completion; released records cleanup.
    std::uint32_t names_entered{}, names_returned{}, names_released{};
    bool stream_reference_released{}, shader_creation_returned{};
    NativeVertexShaderCompilationOperation() = default;
    ~NativeVertexShaderCompilationOperation();
    NativeVertexShaderCompilationOperation(const NativeVertexShaderCompilationOperation&) = delete;
    NativeVertexShaderCompilationOperation& operator=(const NativeVertexShaderCompilationOperation&) = delete;
    // Diagnostic only AFTER manually resolving all native resources/children.
    // Never resumes the call or marks a failed operation as native completion.
    void acknowledge_diagnostic_cleanup() noexcept;
};

// B60F60 normal readable/output domain, plus exact code-null return behavior.
// Original fastcall: ECX actual8h engine name, EDX profile, stack source and
// actual IDirect3DVertexShader9**; EAX HRESULT; RET8. New C++ service interface.
// Captures device before compile1200; code presence selects diagnostic work;
// .vsa VFS open35, write, string/stream/disassembly cleanup precedes actual
// captured-device CreateVertexShader. Code/messages cleanup follows creation.
// Code-null return leaves messages retained, matching native missing cleanup.
// A missing disassembly output is an explicit source failure instead of the
// native invalid dereference. Unsupported stream write targets fail visibly.
HRESULT compile_native_vertex_shader_00b60f60(const void* actual_engine_name,
    const char* profile, const char* source, IDirect3DVertexShader9** actual_output,
    NativeVertexShaderCompilationContext&, NativeVertexShaderCompilationOperation&);
} // namespace bsp
