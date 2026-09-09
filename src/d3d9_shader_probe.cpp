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
    const std::vector<bsp::ShaderSystemConstant> constants{
        {"ProbeTransform", 4, 4, 1}, {"ProbeTint", 1, 4, 2}, {"ProbeBias", 1, 1, 1}};
    bsp::append_system_constant_header_00b38ff0(constants, true, 6, declarations);
    const bool constant_header_matches = declarations ==
        "float4x4 ProbeTransform : register(c0);\nfloat4 ProbeTint[2] : register(c4);\nfloat ProbeBias;\n";
    std::string samplers;
    bsp::append_vertex_samplers_00b38080(
        {{"Skipped", false, 2}, {"Unknown", true, 0}, {"ProbeSampler", true, 2}},
        {{"ProbeCube", true, 3}}, samplers);
    const bool samplers_match = samplers ==
        "sampler2D\tProbeSampler\t\t: register(s1);\nsamplerCUBE\tProbeCube\t\t: register(s2);\n";
    declarations += samplers;
    bsp::ShaderField position;
    position.name = "Position"; position.component_count = 4;
    bsp::ShaderStructOptions options; options.include_semantics = true;
    const auto generated = bsp::append_shader_struct_00b38b50("ProbeInput", {position}, options, declarations);
    const std::string expected = "\nstruct ProbeInput\n{\n\tfloat4\t\tPosition\t\t : POSITION0;\n};\n\n";
    const bool declaration_matches = generated == bsp::ShaderSourceStatus::complete
        && constant_header_matches && samplers_match && declarations.size() >= expected.size()
        && declarations.compare(declarations.size() - expected.size(), expected.size(), expected) == 0;
    HRESULT result = declaration_matches ? S_OK : E_FAIL;
    // Exercise native component packing across a register boundary, followed
    // by generated declarations and unpack code in the existing compile probe.
    bsp::ShaderField uv; uv.name = "UV"; uv.component_count = 4;
    uv.semantic = bsp::ShaderSemantic::texcoord; uv.component_mask = 5;
    bsp::ShaderField extra = uv; extra.name = "Extra"; extra.component_mask = 15;
    bsp::ShaderField color; color.name = "Color"; color.component_count = 4;
    color.semantic = bsp::ShaderSemantic::color;
    const std::vector<bsp::ShaderField> fields{position, uv, extra, color};
    bsp::ShaderInterpolatorLayout interpolators;
    bsp::append_interpolator_mapping_00b34aa0(fields, interpolators);
    bsp::ShaderInterpolatorOptions interpolator_options;
    bsp::ShaderStructOptions pixel_options; pixel_options.first_field = 1;
    std::string pixel_source;
    bsp::append_pixel_samplers_00b37ef0(
        {{"PixelTexture", false, 2}, {"VertexOnly", true, 2}, {"UnknownPixel", false, 0}},
        {{"PixelVolume", false, 4}}, pixel_source);
    const bool pixel_samplers_match = pixel_source ==
        "sampler2D\tPixelTexture\t\t: register(s0);\nsampler3D\tPixelVolume\t\t: register(s2);\n";
    const bool pixel_generated = bsp::append_shader_struct_00b38b50("sPixelIn", fields,
        pixel_options, pixel_source) == bsp::ShaderSourceStatus::complete
        && bsp::append_interpolator_struct_00b36e30(interpolators, interpolator_options,
            pixel_source) == bsp::ShaderSourceStatus::complete
        && bsp::append_interpolator_unpack_00b37000(fields, interpolators, interpolator_options,
            pixel_source) == bsp::ShaderSourceStatus::complete;
    const bool packing_matches = interpolators.texcoords.size() == 6
        && interpolators.colors.size() == 4 && interpolators.texcoord_registers == 2
        && interpolators.texcoord_last_width == 2 && interpolators.color_registers == 1
        && interpolators.color_last_width == 4
        && pixel_source.find("PixelIn.UV.z = INT.TexCoord0.y;") != std::string::npos
        && pixel_source.find("PixelIn.Extra.z = INT.TexCoord1.x;") != std::string::npos;
    pixel_source += "float4 main(sInterpolators INT) : COLOR { sPixelIn IN = UnpackInterpolators(INT); "
        "return float4(IN.UV.x, IN.UV.z, IN.Extra.z, IN.Color.w); }";
    auto vertex_fields = fields;
    vertex_fields[0].name = "ScreenSpacePos";
    bsp::ShaderStructOptions vertex_output_options;
    bsp::ShaderInterpolatorOptions vertex_interpolator_options;
    vertex_interpolator_options.include_position = true;
    const bool vertex_generated = bsp::append_shader_struct_00b38b50("sVertexOut", vertex_fields,
        vertex_output_options, declarations) == bsp::ShaderSourceStatus::complete
        && bsp::append_interpolator_struct_00b36e30(interpolators, vertex_interpolator_options,
            declarations) == bsp::ShaderSourceStatus::complete
        && bsp::append_interpolator_pack_00b35540(vertex_fields, interpolators,
            declarations) == bsp::ShaderSourceStatus::complete;
    const bool pack_matches = declarations.find("INT.TexCoord0.y = OUT.UV.z;") != std::string::npos
        && declarations.find("INT.TexCoord1.x = OUT.Extra.z;") != std::string::npos;
    bsp::append_system_constant_header_00b38ff0(
        {{"cVtxElemScale", 1, 4, 2}, {"cVtxElemOffset", 1, 4, 2}}, false, 0, declarations);
    std::string initialization;
    bsp::append_zero_shader_fields_00b357d0("OUT", vertex_fields, initialization);
    bsp::append_vertex_input_decode_00b35820({position}, true, 3, initialization);
    const bool initialization_matches = initialization ==
        "\t\tOUT.ScreenSpacePos=0;\n\t\tOUT.UV=0;\n\t\tOUT.Extra=0;\n\t\tOUT.Color=0;\n"
        "\tIN.Position=IN.Position * cVtxElemScale[0].xyzw + cVtxElemOffset[0].xyzw;\n";
    declarations += "sInterpolators main(ProbeInput IN) { sVertexOut OUT;\n";
    declarations += initialization;
    declarations += "OUT.ScreenSpacePos=mul(IN.Position,ProbeTransform); OUT.UV=ProbeTint[1]+ProbeBias; "
        "OUT.Color=float4(0,0,0,1); return PackInterpolators(OUT); }";
    if (SUCCEEDED(result)) result = vertex_generated && pack_matches && initialization_matches
        ? assemble_host_shader(declarations.c_str(), "vs_2_0", &vertex_code) : E_FAIL;
    if (SUCCEEDED(result)) result = pixel_generated && packing_matches && pixel_samplers_match
        ? assemble_host_shader(pixel_source.c_str(), "ps_2_0", &pixel_code) : E_FAIL;
    // Installed debugshader.shfx VS projection; field/constant registry is
    // explicit here, pending native descriptor loading and registry recovery.
    bsp::ShaderVertexProgram debug_program;
    debug_program.inputs = {position, color};
    auto system_position = position; system_position.name = "ObjectSpacePos";
    debug_program.system_values.push_back(system_position);
    system_position.name = "WorldSpacePos"; debug_program.system_values.push_back(system_position);
    system_position.name = "ScreenSpacePos"; debug_program.system_values.push_back(system_position);
    debug_program.outputs = {system_position, color};
    debug_program.packing_fields = debug_program.outputs;
    bsp::append_interpolator_mapping_00b34aa0(debug_program.outputs, debug_program.interpolators);
    debug_program.constants = {{"cViewProjMat", 4, 4, 1}, {"cAmbientCube", 1, 4, 6},
        {"cFogDirColor4", 1, 4, 4}, {"cFogColor", 1, 4, 1}};
    debug_program.register_limit = 256; // Diagnostic registry limit, not recovered global.
    debug_program.base.vertex_code = "SYS.ObjectSpacePos=IN.Position;\n"
        "SYS.WorldSpacePos=IN.Position;\nSYS.ScreenSpacePos=mul(SYS.WorldSpacePos,cViewProjMat);\n"
        "OUT.Color = IN.Color;";
    std::string debug_source;
    const bool full_vertex_generated = bsp::generate_vertex_source_00b39110(debug_program,
        debug_source) == bsp::ShaderSourceStatus::complete;
    ID3DBlob* debug_code = nullptr;
    if (SUCCEEDED(result)) result = full_vertex_generated
        ? assemble_host_shader(debug_source.c_str(), "vs_2_0", &debug_code) : E_FAIL;
    if (SUCCEEDED(result)) {
        vertex_code->Release(); vertex_code = debug_code; debug_code = nullptr;
    }
    if (debug_code) debug_code->Release();
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
