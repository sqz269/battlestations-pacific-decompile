// Host fixture for the installed alphablend/dummy shader pair, not a material loader.
#include "bsp/d3d9_states.hpp"
#include "bsp/shader_lua.hpp"
#include "bsp/shader_reflection.hpp"
#include "bsp/material_constants.hpp"
#include "bsp/material_samplers.hpp"
#include <d3dcompiler.h>
#include <cstdio>
#include <cstring>

namespace {
template<class T> struct ComOwner {
    T* p{};
    ~ComOwner() { if (p) p->Release(); }
    ComOwner() = default;
    ComOwner(const ComOwner&) = delete;
    ComOwner& operator=(const ComOwner&) = delete;
};
bool prepare_constants(const std::vector<bsp::ReflectedShaderConstant>& reflection,
    const bsp::ShaderConstantBindings& bindings, std::vector<float>& words, bool vertex) {
    for (const auto& c : reflection) {
        if (c.register_set == 3) continue; // Samplers are handled by material binding.
        if (c.register_set != 2 || c.parameter_type != 3 || c.register_count == 0
            || c.register_index >= words.size() / 4
            || c.register_count > words.size() / 4 - c.register_index) return false;
        auto* destination = words.data() + c.register_index * 4;
        int semantic = -1;
        if (c.name == "cWorldMat" || c.name == "cViewProjMat") {
            if (!vertex || c.register_count != 4 || c.rows != 4 || c.columns != 4) return false;
            const float identity[16]{1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1};
            bsp::write_system_matrix_00b404a0(destination, identity);
            semantic = c.name == "cWorldMat" ? 0 : 5;
        } else if (c.name == "cVtxElemScale" || c.name == "cVtxElemOffset") {
            if (!vertex || c.register_count < 3 || c.register_count > 8 || c.rows != 1 || c.columns != 4) return false;
            const bool scale = c.name == "cVtxElemScale";
            for (std::uint32_t i = 0; i < c.register_count * 4; ++i) destination[i] = scale ? 1.0f : 0.0f;
            semantic = scale ? 24 : 25;
        } else if (c.name == "cVisibility") {
            if (c.register_count != 1) return false;
            destination[0] = 1; // Explicit fully visible fixture entry.
            semantic = 43;
        } else if (c.name == "cElapsedTime") {
            if (c.register_count != 1) return false;
            semantic = 39; // Explicit zero-time fixture disables the conditional transform.
        } else if (c.name == "cAlpha") {
            if (c.register_count != 1 || c.rows != 1 || c.columns != 1) return false;
            // Filled through the reconstructed material parameter packer below.
        } else {
            std::fprintf(stderr, "Alpha fixture has an unresolved constant: %s\n", c.name.c_str());
            return false;
        }
        if (semantic >= 0 && (bindings.registers[semantic] != c.register_index
            || bindings.counts[semantic] != c.register_count)) return false;
    }
    return true;
}
}

bool draw_alpha_material_probe(IDirect3DDevice9& device, const bsp::ShaderLuaCode& base,
    const bsp::ShaderLuaCode& effect, ID3DBlob* vertex_code, ID3DBlob* pixel_code) {
    if (!vertex_code || !pixel_code) return false;
    std::vector<bsp::ReflectedShaderConstant> vr, pr;
    bsp::ShaderConstantBindings vb, pb;
    std::string error;
    const auto registry = bsp::make_system_constant_registry_00b5bf70();
    if (!bsp::reflect_shader_constants_00b3aea0(static_cast<const std::uint32_t*>(vertex_code->GetBufferPointer()), vr, error)
        || !bsp::reflect_shader_constants_00b3aea0(static_cast<const std::uint32_t*>(pixel_code->GetBufferPointer()), pr, error)
        || !bsp::map_shader_constants_00b3aea0(vr, registry, vb)
        || !bsp::map_shader_constants_00b3aea0(pr, registry, pb)) {
        std::fprintf(stderr, "Alpha reflection: %s\n", error.c_str()); return false;
    }
    std::vector<float> vertex_words(256 * 4), pixel_words(224 * 4);
    if (!prepare_constants(vr, vb, vertex_words, true) || !prepare_constants(pr, pb, pixel_words, false)) return false;
    bsp::MaterialConstantParameter alpha;
    alpha.source_words = {0x3f800000}; alpha.vertex_registers = {-1}; alpha.pixel_registers = {-1};
    for (const auto& c : vb.material_constants) if (c.name == "cAlpha") alpha.vertex_registers[0] = static_cast<std::int32_t>(c.register_index);
    for (const auto& c : pb.material_constants) if (c.name == "cAlpha") alpha.pixel_registers[0] = static_cast<std::int32_t>(c.register_index);
    if (alpha.pixel_registers[0] < 0 || bsp::pack_material_parameter_constants_00b423c5(
        {alpha}, 0, {}, vertex_words, pixel_words) != bsp::MaterialConstantPackStatus::complete) return false;
    bsp::MaterialSamplerPass pass;
    bsp::MaterialSamplerCounters counters;
    if (!bsp::append_material_samplers_00b3b280(base.samplers, pass, counters)
        || !bsp::append_material_samplers_00b3b280(effect.samplers, pass, counters)
        || counters.references != 1 || counters.pixel != 1 || counters.vertex != 0 || pb.sampler_mask != 1) return false;
    bsp::prune_material_sampler_states(pass, pb.sampler_mask);
    ComOwner<IDirect3DStateBlock9> saved;
    ComOwner<IDirect3DSurface9> old_target, old_depth, target, readback;
    ComOwner<IDirect3DTexture9> texture;
    ComOwner<IDirect3DVertexShader9> vertex;
    ComOwner<IDirect3DPixelShader9> pixel;
    HRESULT result = device.CreateStateBlock(D3DSBT_ALL, &saved.p);
    if (SUCCEEDED(result)) result = device.GetRenderTarget(0, &old_target.p);
    if (SUCCEEDED(result)) {
        const HRESULT depth = device.GetDepthStencilSurface(&old_depth.p);
        if (FAILED(depth) && depth != D3DERR_NOTFOUND) result = depth;
    }
    if (SUCCEEDED(result)) result = device.CreateRenderTarget(64, 64, D3DFMT_A8R8G8B8, D3DMULTISAMPLE_NONE, 0, FALSE, &target.p, nullptr);
    if (SUCCEEDED(result)) result = device.CreateOffscreenPlainSurface(64, 64, D3DFMT_A8R8G8B8, D3DPOOL_SYSTEMMEM, &readback.p, nullptr);
    if (SUCCEEDED(result)) result = device.CreateTexture(1, 1, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &texture.p, nullptr);
    D3DLOCKED_RECT texels{};
    if (SUCCEEDED(result)) result = texture.p->LockRect(0, &texels, nullptr, 0);
    if (SUCCEEDED(result)) {
        const DWORD red = 0xffff0000;
        std::memcpy(texels.pBits, &red, sizeof(red));
        result = texture.p->UnlockRect(0);
    }
    if (SUCCEEDED(result)) result = device.CreateVertexShader(static_cast<const DWORD*>(vertex_code->GetBufferPointer()), &vertex.p);
    if (SUCCEEDED(result)) result = device.CreatePixelShader(static_cast<const DWORD*>(pixel_code->GetBufferPointer()), &pixel.p);
    if (SUCCEEDED(result)) result = device.SetDepthStencilSurface(nullptr);
    if (SUCCEEDED(result)) result = device.SetRenderTarget(0, target.p);
    const D3DVIEWPORT9 viewport{0, 0, 64, 64, 0, 1};
    if (SUCCEEDED(result)) result = device.SetViewport(&viewport);
    // Explicit isolated-host state, not inferred native descriptor defaults.
    for (const auto& entry : {std::pair<D3DRENDERSTATETYPE, DWORD>{D3DRS_ZENABLE, FALSE},
        {D3DRS_ZWRITEENABLE, FALSE}, {D3DRS_ALPHATESTENABLE, FALSE}, {D3DRS_FOGENABLE, FALSE},
        {D3DRS_CULLMODE, D3DCULL_NONE}, {D3DRS_SCISSORTESTENABLE, FALSE},
        {D3DRS_SRGBWRITEENABLE, FALSE}, {D3DRS_COLORWRITEENABLE, 15},
        {D3DRS_SEPARATEALPHABLENDENABLE, FALSE}, {D3DRS_BLENDOP, D3DBLENDOP_ADD}})
        if (SUCCEEDED(result)) result = device.SetRenderState(entry.first, entry.second);
    for (const auto& entry : {std::pair<D3DSAMPLERSTATETYPE, DWORD>{D3DSAMP_MINFILTER, D3DTEXF_POINT},
        {D3DSAMP_MAGFILTER, D3DTEXF_POINT}, {D3DSAMP_MIPFILTER, D3DTEXF_NONE}, {D3DSAMP_SRGBTEXTURE, FALSE}})
        if (SUCCEEDED(result)) result = device.SetSamplerState(0, entry.first, entry.second);
    auto* lock = bsp::critical_section_create_00bd1860();
    if (!lock && SUCCEEDED(result)) result = E_OUTOFMEMORY;
    DWORD center{}, outside{};
    {
        bsp::RendererSynchronization sync{};
        bsp::set_renderer_synchronization_00b33aa0(sync, true);
        const bsp::LogicalVertexShader logical_vertex{vertex.p};
        const bsp::LogicalPixelShader logical_pixel{pixel.p};
        bsp::D3D9StateCache state(device, sync, lock);
        if (SUCCEEDED(result)) result = state.bind_vertex_shader_00b21d10(&logical_vertex);
        if (SUCCEEDED(result)) result = state.bind_pixel_shader_00b21c20(&logical_pixel);
        auto states = std::make_shared<bsp::RenderStateBlock>();
        for (const auto* script : {&base, &effect}) for (const auto& entry : script->render_states)
            states->states.push_back({static_cast<D3DRENDERSTATETYPE>(entry.state), entry.value});
        auto logical_texture = std::make_shared<bsp::LogicalTexture>(); logical_texture->texture = texture.p;
        if (SUCCEEDED(result)) {
            state.bind_render_state_block_00b27a80(states);
            state.bind_sampler_state_block_00b27b90(std::make_shared<bsp::SamplerStateBlock>(pass.sampler_states));
            result = bsp::bind_material_textures_00b43470(state, pass, {logical_texture}, pb.sampler_mask);
        }
        // Upload only reflected live spans; no assumed autoallocated registers.
        for (const auto& c : vr) if (c.register_set == 2 && SUCCEEDED(result))
            result = state.set_vertex_shader_constants_f_00b21820(c.register_index, vertex_words.data() + c.register_index * 4, c.register_count);
        for (const auto& c : pr) if (c.register_set == 2 && SUCCEEDED(result))
            result = state.set_pixel_shader_constants_f_00b218c0(c.register_index, pixel_words.data() + c.register_index * 4, c.register_count);
        struct Vertex { float position[4], normal[3], uv[2]; };
        const Vertex vertices[] = {{{-.75f,-.75f,.5f,1},{0,0,1},{0,1}},
            {{0,.75f,.5f,1},{0,0,1},{.5f,0}}, {{.75f,-.75f,.5f,1},{0,0,1},{1,1}}};
        auto physical = std::make_shared<bsp::VertexBufferBinding>();
        physical->flags = 0x1000; physical->capacity = sizeof(vertices);
        auto stream = std::make_shared<bsp::LogicalVertexStream>();
        stream->physical = physical; stream->flags = 0x1000; stream->tag = 0x40000001;
        stream->declaration = std::make_shared<bsp::VertexDeclaration>();
        stream->declaration->append_00b48330(D3DDECLTYPE_FLOAT4, D3DDECLUSAGE_POSITION);
        stream->declaration->append_00b48330(D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_NORMAL);
        stream->declaration->append_00b48330(D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD);
        auto layout = std::make_shared<bsp::D3D9VertexLayout>(); layout->append_stream_00b48a00(stream->declaration);
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
        if (SUCCEEDED(result)) result = device.GetRenderTargetData(target.p, readback.p);
        D3DLOCKED_RECT pixels{};
        if (SUCCEEDED(result)) result = readback.p->LockRect(&pixels, nullptr, D3DLOCK_READONLY);
        if (SUCCEEDED(result)) {
            const auto* bytes = static_cast<const unsigned char*>(pixels.pBits);
            std::memcpy(&center, bytes + 32 * pixels.Pitch + 32 * 4, 4);
            std::memcpy(&outside, bytes + 2 * pixels.Pitch + 2 * 4, 4);
            result = readback.p->UnlockRect();
        }
        state.invalidate(); // Discard borrowed shader identities before wrapper destruction.
    }
    if (lock) bsp::critical_section_destroy_owned_0041cc80(lock);
    bool restored = true;
    if (old_target.p) {
        restored = SUCCEEDED(device.SetRenderTarget(0, old_target.p));
        restored = SUCCEEDED(device.SetDepthStencilSurface(old_depth.p)) && restored;
    }
    if (saved.p) restored = SUCCEEDED(saved.p->Apply()) && restored;
    const bool matched = SUCCEEDED(result) && center == 0xffff0000 && outside == 0xff000000;
    std::printf("Installed alpha material draw: hr=0x%08lx center=0x%08lx outside=0x%08lx checked=%d restored=%d\n",
        static_cast<unsigned long>(result), center, outside, matched, restored);
    return matched && restored;
}
