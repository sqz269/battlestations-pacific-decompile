// Diagnostic host shaders only: native shader loading/material execution is unported.
#include "bsp/d3d9_states.hpp"
#include "bsp/shader_source.hpp"
#include "bsp/material_constants.hpp"
#include "bsp/camera_projection.hpp"
#include <d3dcompiler.h>
#include <cstdio>
#include <cstring>

namespace {
bool draw_generated_debug_pair(IDirect3DDevice9& device, bsp::D3D9StateCache& state) {
    IDirect3DStateBlock9* saved = nullptr;
    IDirect3DSurface9* old_target = nullptr;
    IDirect3DSurface9* old_depth = nullptr;
    IDirect3DSurface9* target = nullptr;
    IDirect3DSurface9* readback = nullptr;
    HRESULT result = device.CreateStateBlock(D3DSBT_ALL, &saved);
    if (SUCCEEDED(result)) result = device.GetRenderTarget(0, &old_target);
    if (SUCCEEDED(result)) {
        const HRESULT depth_result = device.GetDepthStencilSurface(&old_depth);
        if (FAILED(depth_result) && depth_result != D3DERR_NOTFOUND) result = depth_result;
    }
    if (SUCCEEDED(result)) result = device.CreateRenderTarget(64, 64, D3DFMT_A8R8G8B8,
        D3DMULTISAMPLE_NONE, 0, FALSE, &target, nullptr);
    if (SUCCEEDED(result)) result = device.CreateOffscreenPlainSurface(64, 64, D3DFMT_A8R8G8B8,
        D3DPOOL_SYSTEMMEM, &readback, nullptr);
    if (SUCCEEDED(result)) result = device.SetDepthStencilSurface(nullptr);
    if (SUCCEEDED(result)) result = device.SetRenderTarget(0, target);
    const D3DVIEWPORT9 viewport{0, 0, 64, 64, 0, 1};
    if (SUCCEEDED(result)) result = device.SetViewport(&viewport);
    for (const auto& setting : {std::pair<D3DRENDERSTATETYPE, DWORD>{D3DRS_ZENABLE, FALSE},
        {D3DRS_ALPHABLENDENABLE, FALSE}, {D3DRS_ALPHATESTENABLE, FALSE}, {D3DRS_FOGENABLE, FALSE},
        {D3DRS_CULLMODE, D3DCULL_NONE}, {D3DRS_SCISSORTESTENABLE, FALSE},
        {D3DRS_SRGBWRITEENABLE, FALSE}, {D3DRS_COLORWRITEENABLE, 15}}) {
        if (SUCCEEDED(result)) result = device.SetRenderState(setting.first, setting.second);
    }
    float vertex_constants[77 * 4]{};
    bsp::CameraProjection camera;
    bsp::set_camera_fov_00b6fbb0(camera, 1.57079637f);
    bsp::set_camera_aspect_00b6fbd0(camera, 1);
    bsp::set_camera_near_00b6fbf0(camera, .1f);
    bsp::set_camera_far_00b6fc10(camera, 100);
    const bsp::CameraMatrix supplied{1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1};
    bsp::set_camera_projection_00b6fd60(camera, supplied);
    bool camera_checked = bsp::get_camera_projection_00b6fcf0(camera) == supplied;
    bsp::set_camera_aspect_00b6fbd0(camera, 1); // Equal scalar still invalidates custom projection.
    camera_checked = camera_checked && !(camera.valid_flags & 8);
    const auto& view_projection = bsp::get_camera_projection_00b6fcf0(camera);
    camera_checked = camera_checked && (camera.valid_flags & 8)
        && view_projection == camera.original && view_projection[11] == 1 && view_projection[15] == 0
        && view_projection[10] > 1 && view_projection[10] < 1.002f
        && view_projection[14] < -.1f && view_projection[14] > -.101f;
    if (!camera_checked) result = E_FAIL;
    bsp::write_system_matrix_00b404a0(vertex_constants + 60, view_projection.data());
    float pixel_constants[77 * 4]{}; // Native cElapsedTime at c34: conditional transform disabled.
    if (SUCCEEDED(result)) result = state.set_vertex_shader_constants_f_00b21820(0, vertex_constants, 77);
    if (SUCCEEDED(result)) result = state.set_pixel_shader_constants_f_00b218c0(0, pixel_constants, 77);
    struct Vertex { float position[4], color[4]; };
    const Vertex vertices[] = {{{-.75f, -.75f, 1, 1}, {.25f, .5f, .75f, 1}},
        {{0, .75f, 1, 1}, {.25f, .5f, .75f, 1}}, {{.75f, -.75f, 1, 1}, {.25f, .5f, .75f, 1}}};
    auto physical = std::make_shared<bsp::VertexBufferBinding>();
    physical->flags = 0x1000; physical->capacity = sizeof(vertices);
    auto stream = std::make_shared<bsp::LogicalVertexStream>();
    stream->physical = physical; stream->flags = 0x1000; stream->tag = 0x40000001;
    stream->declaration = std::make_shared<bsp::VertexDeclaration>();
    stream->declaration->append_00b48330(D3DDECLTYPE_FLOAT4, D3DDECLUSAGE_POSITION);
    stream->declaration->append_00b48330(D3DDECLTYPE_FLOAT4, D3DDECLUSAGE_COLOR);
    auto layout = std::make_shared<bsp::D3D9VertexLayout>();
    layout->append_stream_00b48a00(stream->declaration);
    if (SUCCEEDED(result)) result = bsp::vertex_buffer_recreate_00b492b0(*physical, device);
    if (SUCCEEDED(result)) result = layout->create_if_missing_00b60a10(device);
    void* mapped = nullptr;
    if (SUCCEEDED(result)) result = state.lock_vertex_stream_00b49980(*stream, 3, 0, true, mapped);
    if (SUCCEEDED(result)) {
        std::memcpy(mapped, vertices, sizeof(vertices));
        state.unlock_vertex_stream_00b49a80(*stream);
        state.bind_vertex_stream_00b24840(0, stream);
        result = state.bind_vertex_layout_00b23f20(layout);
    }
    if (SUCCEEDED(result)) result = device.Clear(0, nullptr, D3DCLEAR_TARGET, 0xff000000, 1, 0);
    if (SUCCEEDED(result)) result = device.BeginScene();
    if (SUCCEEDED(result)) {
        result = state.draw_primitive_00b21b40({}, D3DPT_TRIANGLELIST, 0, 1);
        const HRESULT ended = device.EndScene();
        if (SUCCEEDED(result)) result = ended;
    }
    if (SUCCEEDED(result)) result = device.GetRenderTargetData(target, readback);
    D3DLOCKED_RECT pixels{};
    DWORD center = 0, outside = 0;
    if (SUCCEEDED(result)) result = readback->LockRect(&pixels, nullptr, D3DLOCK_READONLY);
    if (SUCCEEDED(result)) {
        const auto* bytes = static_cast<const unsigned char*>(pixels.pBits);
        std::memcpy(&center, bytes + 32 * pixels.Pitch + 32 * 4, 4);
        std::memcpy(&outside, bytes + 2 * pixels.Pitch + 2 * 4, 4);
        result = readback->UnlockRect();
    }
    const bool matched = SUCCEEDED(result) && outside == 0xff000000
        && (center >> 24) == 255 && ((center >> 16) & 255) >= 63 && ((center >> 16) & 255) <= 64
        && ((center >> 8) & 255) >= 127 && ((center >> 8) & 255) <= 128
        && (center & 255) >= 191 && (center & 255) <= 192;
    bool restored = true;
    if (old_target) {
        restored = SUCCEEDED(device.SetRenderTarget(0, old_target));
        restored = SUCCEEDED(device.SetDepthStencilSurface(old_depth)) && restored;
    }
    if (saved) restored = SUCCEEDED(saved->Apply()) && restored;
    std::printf("Generated debug shader draw: hr=0x%08lx center=0x%08lx outside=0x%08lx checked=%d restored=%d\n",
        static_cast<unsigned long>(result), center, outside, matched, restored);
    if (readback) readback->Release(); if (target) target->Release();
    if (old_depth) old_depth->Release(); if (old_target) old_target->Release();
    if (saved) saved->Release();
    return matched && restored;
}
}

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
    bsp::append_system_constant_header_00b38ff0({{"cShadowMapSizeData", 1, 4, 1}},
        false, 0, pixel_source);
    bsp::append_shadow_helper_00b38230(false, pixel_source);
    bsp::append_map_shadow_helper_00b382b0(true, pixel_source);
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
    bsp::append_vertex_inputs_00b35930({position, color}, {}, debug_program.inputs);
    bsp::append_vertex_system_fields_00b35be0(debug_program.system_values);
    const bool selected_debug_fields = bsp::append_selected_interpolators_00b36800(
        {color}, {}, nullptr, nullptr, debug_program.outputs) == bsp::ShaderSourceStatus::complete;
    debug_program.packing_fields = debug_program.outputs;
    bsp::append_interpolator_mapping_00b34aa0(debug_program.outputs, debug_program.interpolators);
    debug_program.constants = bsp::make_system_constant_registry_00b5bf70();
    debug_program.register_limit = bsp::system_constant_annotation_limit;
    debug_program.base.vertex_code = "SYS.ObjectSpacePos=IN.Position;\n"
        "SYS.WorldSpacePos=IN.Position;\nSYS.ScreenSpacePos=mul(SYS.WorldSpacePos,cViewProjMat);\n"
        "OUT.Color = IN.Color;";
    std::string debug_source;
    const auto debug_profiles = bsp::select_shader_profiles_00b43b00(3);
    const bool full_vertex_generated = selected_debug_fields && bsp::generate_vertex_source_00b39110(debug_program,
        debug_source) == bsp::ShaderSourceStatus::complete;
    ID3DBlob* debug_code = nullptr;
    if (SUCCEEDED(result)) result = full_vertex_generated
        ? assemble_host_shader(debug_source.c_str(), debug_profiles.vertex.c_str(), &debug_code) : E_FAIL;
    if (SUCCEEDED(result)) {
        vertex_code->Release(); vertex_code = debug_code; debug_code = nullptr;
    }
    if (debug_code) { debug_code->Release(); debug_code = nullptr; }
    bsp::ShaderPixelProgram debug_pixel;
    debug_pixel.inputs = debug_program.outputs;
    debug_pixel.unpack_fields = debug_program.outputs;
    bsp::append_pixel_system_fields_00b372d0(debug_pixel.system_values);
    debug_pixel.interpolators = debug_program.interpolators;
    debug_pixel.constants = debug_program.constants;
    debug_pixel.register_limit = bsp::system_constant_annotation_limit;
    debug_pixel.base.pixel_code = "SYS.DiffuseColor = IN.Color;";
    debug_pixel.effect.pixel_code = "FinalColor[0] = SYS.DiffuseColor;";
    std::string debug_pixel_source;
    const bool full_pixel_generated = bsp::generate_pixel_source_00b39880(debug_pixel,
        debug_pixel_source) == bsp::ShaderSourceStatus::complete;
    ID3DBlob* debug_pixel_code = nullptr;
    if (SUCCEEDED(result)) result = full_pixel_generated
        ? assemble_host_shader(debug_pixel_source.c_str(), debug_profiles.pixel.c_str(), &debug_pixel_code) : E_FAIL;
    if (SUCCEEDED(result)) {
        pixel_code->Release(); pixel_code = debug_pixel_code; debug_pixel_code = nullptr;
    }
    if (debug_pixel_code) debug_pixel_code->Release();
    // Native00b61280 obtains these masks from pixel disassembly. The installed
    // compiler is an adapter; the native D3DX compiler/cache wrapper is unported.
    decltype(&D3DDisassemble) disassemble{};
    const FARPROC disassembly_address = GetProcAddress(module, "D3DDisassemble");
    static_assert(sizeof(disassemble) == sizeof(disassembly_address));
    std::memcpy(&disassemble, &disassembly_address, sizeof(disassemble));
    ID3DBlob* disassembly = nullptr;
    if (SUCCEEDED(result)) result = disassemble ? disassemble(pixel_code->GetBufferPointer(),
        pixel_code->GetBufferSize(), 0, nullptr, &disassembly) : E_FAIL;
    std::vector<std::uint32_t> texcoord_usage(10), color_usage(2);
    std::vector<bsp::ShaderField> filtered_fields;
    if (SUCCEEDED(result)) {
        const std::string text(static_cast<const char*>(disassembly->GetBufferPointer()));
        const bool parsed = bsp::parse_pixel_usage_00b61280(text, texcoord_usage, color_usage)
            == bsp::ShaderSourceStatus::complete;
        const bool selected = parsed && bsp::append_selected_interpolators_00b36800(
            {color}, {}, &texcoord_usage, &color_usage, filtered_fields) == bsp::ShaderSourceStatus::complete;
        if (!selected || color_usage[0] != 15 || filtered_fields.size() != 2
            || filtered_fields[1].component_mask != 15) result = E_FAIL;
        if (SUCCEEDED(result)) {
            debug_program.outputs = filtered_fields;
            debug_program.packing_fields = filtered_fields;
            debug_program.interpolators = {};
            bsp::append_interpolator_mapping_00b34aa0(filtered_fields, debug_program.interpolators);
            result = bsp::generate_vertex_source_00b39110(debug_program, debug_source)
                == bsp::ShaderSourceStatus::complete ? S_OK : E_FAIL;
        }
        if (SUCCEEDED(result)) result = assemble_host_shader(debug_source.c_str(),
            debug_profiles.vertex.c_str(), &debug_code);
        if (SUCCEEDED(result)) {
            vertex_code->Release(); vertex_code = debug_code; debug_code = nullptr;
        }
        std::printf("Pixel disassembly usage: COLOR0=%lu filtered_fields=%zu hr=0x%08lx\n",
            static_cast<unsigned long>(color_usage[0]), filtered_fields.size(), static_cast<unsigned long>(result));
    }
    if (disassembly) disassembly->Release();
    if (debug_code) { debug_code->Release(); debug_code = nullptr; }
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
        if (matched) matched = draw_generated_debug_pair(device, state);
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
