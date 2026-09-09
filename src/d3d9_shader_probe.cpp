// Diagnostic host shaders only: native shader loading/material execution is unported.
#include "bsp/d3d9_states.hpp"
#include "bsp/shader_source.hpp"
#include <d3dcompiler.h>
#include <cstdio>
#include <cstring>

bool probe_shader_bindings(IDirect3DDevice9& device) {
    // Use the installed compiler and the SDK declaration rather than inventing
    // shader bytecode or a private D3DX buffer/assembler ABI.
    HMODULE module = LoadLibraryExW(L"d3dcompiler_47.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
    if (!module) return false;
    const FARPROC address = GetProcAddress(module, "D3DCompile");
    pD3DCompile compile{};
    static_assert(sizeof(compile) == sizeof(address));
    std::memcpy(&compile, &address, sizeof(compile));
    if (!compile) { FreeLibrary(module); return false; }

    auto assemble_host_shader = [&](const char* source, const char* profile, ID3DBlob** code) {
        ID3DBlob* errors = nullptr;
        const HRESULT result = compile(source, std::strlen(source), "diagnostic shader",
            nullptr, nullptr, "main", profile, 0, 0, code, &errors);
        if (errors) {
            if (FAILED(result)) std::fwrite(errors->GetBufferPointer(), 1, errors->GetBufferSize(), stderr);
            errors->Release();
        }
        return result;
    };
    ID3DBlob* vertex_code = nullptr;
    ID3DBlob* pixel_code = nullptr;
    IDirect3DVertexShader9* vertex = nullptr;
    IDirect3DPixelShader9* pixel = nullptr;
    IDirect3DVertexShader9* saved_vertex = nullptr;
    IDirect3DPixelShader9* saved_pixel = nullptr;
    std::string declarations;
    bsp::ShaderField position;
    position.name = "Position"; position.component_count = 4;
    bsp::ShaderStructOptions options; options.include_semantics = true;
    const auto generated = bsp::append_shader_struct_00b38b50("ProbeInput", {position}, options, declarations);
    const std::string expected = "\nstruct ProbeInput\n{\n\tfloat4\t\tPosition\t\t : POSITION0;\n};\n\n";
    const bool declaration_matches = generated == bsp::ShaderSourceStatus::complete && declarations == expected;
    declarations += "float4 main(ProbeInput IN) : POSITION { return IN.Position; }";
    HRESULT result = declaration_matches
        ? assemble_host_shader(declarations.c_str(), "vs_2_0", &vertex_code) : E_FAIL;
    if (SUCCEEDED(result)) result = assemble_host_shader(
        "float4 main() : COLOR { return float4(0,1,0,1); }", "ps_2_0", &pixel_code);
    if (SUCCEEDED(result)) result = vertex_code
        ? device.CreateVertexShader(static_cast<const DWORD*>(vertex_code->GetBufferPointer()), &vertex) : E_FAIL;
    if (SUCCEEDED(result)) result = pixel_code
        ? device.CreatePixelShader(static_cast<const DWORD*>(pixel_code->GetBufferPointer()), &pixel) : E_FAIL;
    if (SUCCEEDED(result) && (!vertex || !pixel)) result = E_FAIL;
    if (SUCCEEDED(result)) result = device.GetVertexShader(&saved_vertex);
    bool captured = false;
    if (SUCCEEDED(result)) {
        result = device.GetPixelShader(&saved_pixel);
        captured = SUCCEEDED(result);
    }
    auto* lock = bsp::critical_section_create_00bd1860();
    bool matched = false;
    if (SUCCEEDED(result) && lock) {
        // Establish a known initial API state, then restore the caller's state.
        result = device.SetVertexShader(nullptr);
        if (SUCCEEDED(result)) result = device.SetPixelShader(nullptr);
        bsp::RendererSynchronization sync{};
        bsp::set_renderer_synchronization_00b33aa0(sync, true);
        const bsp::LogicalVertexShader first_vertex{vertex}, equivalent_vertex{vertex};
        const bsp::LogicalPixelShader first_pixel{pixel}, equivalent_pixel{pixel};
        bsp::D3D9StateCache state(device, sync, lock);
        bool skipped = state.bind_vertex_shader_00b21d10(nullptr) == S_FALSE;
        skipped = (state.bind_pixel_shader_00b21c20(nullptr) == S_FALSE) && skipped;
        if (SUCCEEDED(result)) result = state.bind_vertex_shader_00b21d10(&first_vertex);
        if (SUCCEEDED(result)) result = state.bind_pixel_shader_00b21c20(&first_pixel);
        skipped = (state.bind_vertex_shader_00b21d10(&equivalent_vertex) == S_FALSE) && skipped;
        skipped = (state.bind_pixel_shader_00b21c20(&equivalent_pixel) == S_FALSE) && skipped;
        IDirect3DVertexShader9* observed_vertex = nullptr;
        IDirect3DPixelShader9* observed_pixel = nullptr;
        if (SUCCEEDED(result)) result = device.GetVertexShader(&observed_vertex);
        if (SUCCEEDED(result)) result = device.GetPixelShader(&observed_pixel);
        matched = SUCCEEDED(result) && skipped && observed_vertex == vertex && observed_pixel == pixel
            && state.vertex_shader() == &first_vertex && state.pixel_shader() == &first_pixel
            && state.vertex_shader_calls() == 1 && state.pixel_shader_calls() == 1;
        if (observed_vertex) observed_vertex->Release();
        if (observed_pixel) observed_pixel->Release();
        result = state.bind_vertex_shader_00b21d10(nullptr);
        if (SUCCEEDED(result)) result = state.bind_pixel_shader_00b21c20(nullptr);
        observed_vertex = nullptr;
        observed_pixel = nullptr;
        if (SUCCEEDED(result)) result = device.GetVertexShader(&observed_vertex);
        if (SUCCEEDED(result)) result = device.GetPixelShader(&observed_pixel);
        matched = matched && SUCCEEDED(result) && !observed_vertex && !observed_pixel
            && !state.vertex_shader() && !state.pixel_shader()
            && state.vertex_shader_calls() == 2 && state.pixel_shader_calls() == 2
            && sync.nesting == 0 && lock->depth == 0;
        if (observed_vertex) observed_vertex->Release();
        if (observed_pixel) observed_pixel->Release();
    }
    if (captured) {
        const HRESULT restore_vertex = device.SetVertexShader(saved_vertex);
        const HRESULT restore_pixel = device.SetPixelShader(saved_pixel);
        matched = matched && SUCCEEDED(restore_vertex) && SUCCEEDED(restore_pixel);
    }
    std::printf("D3D9 shader bindings: hr=0x%08lx nonnull_identity_null_and_guard=%d\n",
        static_cast<unsigned long>(result), matched);
    if (lock) bsp::critical_section_destroy_owned_0041cc80(lock);
    if (saved_vertex) saved_vertex->Release();
    if (saved_pixel) saved_pixel->Release();
    if (vertex) vertex->Release();
    if (pixel) pixel->Release();
    if (vertex_code) vertex_code->Release();
    if (pixel_code) pixel_code->Release();
    FreeLibrary(module);
    return matched;
}
