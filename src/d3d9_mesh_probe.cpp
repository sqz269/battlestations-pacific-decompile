// One installed-model diagnostic. Scene/camera inputs are explicit host inputs;
// no native world, shadow-buffer generation or gameplay is implied by this draw.
#include "asset_stream_probe.hpp"
#include "installed_model_probe.hpp"
#include "bsp/compiled_material.hpp"
#include "bsp/resource_path.hpp"
#include "bsp/vertex_format.hpp"
#include "bsp/mesh_gpu_streams.hpp"
#include "bsp/mesh_decode_bindings.hpp"
#include "bsp/building_instance.hpp"
#include "bsp/material_lighting.hpp"
#include "bsp/material_constants.hpp"
#include "bsp/material_textures.hpp"
#include "bsp/d3d9_texture.hpp"
#include "bsp/texture_load_policy.hpp"
#include <algorithm>
#include <cstring>
#include <set>
#include <cstdio>
#include <fstream>
#include <filesystem>

namespace {
template<class T> struct OwnedCom {
    T* p{};
    ~OwnedCom() { if (p) p->Release(); }
    OwnedCom() = default;
    OwnedCom(const OwnedCom&) = delete;
    OwnedCom& operator=(const OwnedCom&) = delete;
};
struct TextureImports {
    HMODULE module = LoadLibraryExW(L"d3dx9_40.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
    bsp::ReadImageInfoFromMemory info{};
    bsp::CreateTextureFromMemory create{};
    TextureImports() {
        if (!module) return;
        auto address = GetProcAddress(module, "D3DXGetImageInfoFromFileInMemory");
        std::memcpy(&info, &address, sizeof(info));
        address = GetProcAddress(module, "D3DXCreateTextureFromFileInMemoryEx");
        std::memcpy(&create, &address, sizeof(create));
    }
    ~TextureImports() { if (module) FreeLibrary(module); }
};

bool load_texture(IDirect3DDevice9& device, AssetStreamProbe& assets,
    const TextureImports& imports, const std::string& requested,
    std::unique_ptr<bsp::D3D9RetainedTexture2D>& output, std::string& error) {
    std::shared_ptr<bsp::MemoryStream> source;
    auto name = requested;
    bsp::lowercase_resource_name_004bcc00(name);
    std::string logical;
    if (!imports.info || !imports.create || !assets.read(name, source, error, &logical)) return false;
    const auto result = bsp::load_retained_texture_2d_00b2c2d0_fragment(device,
        imports.info, imports.create, source,
        {static_cast<std::uint32_t>(logical.size()), logical.c_str()}, 0, output);
    std::printf("Mounted mesh texture: %s -> %s hr=0x%08lx\n", requested.c_str(), logical.c_str(),
        static_cast<unsigned long>(result));
    return SUCCEEDED(result) && output && output->texture();
}

bool scene_constants(const std::vector<bsp::ReflectedShaderConstant>& reflection,
    std::vector<float>& words, bool vertex, std::string& error) {
    // Deliberate host scene: fixed orthographic camera, uniform ambient cube,
    // one directional light, no fog/time effect. These are explicit scene
    // inputs, not guessed native initialization of the world or renderer.
    const float view_projection[]{.24f,-.14f,-.1f,0, 0,.28f,-.1f,0,
        -.24f,-.14f,-.1f,0, 0,0,.5f,1};
    const float screen_texture[]{.5f,0,0,0, 0,-.5f,0,0, 0,0,1,0, .5f,.5f,0,1};
    for (const auto& constant : reflection) {
        if (constant.register_set == 3) continue;
        if (constant.register_set != 2 || constant.parameter_type != 3
            || constant.register_index + constant.register_count > words.size()/4) return false;
        auto* value = words.data() + 4 * constant.register_index;
        std::fill(value, value + constant.register_count * 4, 0.0f);
        const auto& name = constant.name;
        if (name == "cVtxElemScale" || name == "cVtxElemOffset") continue; // Recovered binder below.
        if (vertex && (name == "cViewProjMat" || name == "cScreenToTextureMat")) {
            if (constant.rows != 4 || constant.columns != 4 || constant.register_count != 4) return false;
            bsp::write_system_matrix_00b404a0(value,
                name == "cViewProjMat" ? view_projection : screen_texture);
        } else if (vertex && name == "cAmbientCube" && constant.register_count == 6) {
            for (unsigned i = 0; i < 6; ++i) {
                value[i*4] = value[i*4+1] = value[i*4+2] = .35f; value[i*4+3] = 1;
            }
        } else if (vertex && name == "cDirLightDiffuseColor") {
            value[0] = value[1] = value[2] = .65f; value[3] = 1;
        } else if (vertex && name == "cDirLightWorldSpaceDir") {
            value[0] = value[1] = value[2] = .577350269f;
        } else if (vertex && name == "cWorldSpaceEyePos") {
            value[0] = value[1] = value[2] = 8;
        } else if (vertex && name == "cFogParams") {
            value[0] = 100; value[1] = 200;
        } else if (vertex && name == "cFogHeightParams") value[0] = 1;
        else if ((vertex && (name == "cFogColor" || name == "cFogDirColor4" || name == "cPointLightsData"))
            || (!vertex && name == "cElapsedTime")) {}
        else { error = "Unresolved installed mesh scene constant: " + name; return false; }
    }
    return true;
}

bool write_bitmap(const D3DLOCKED_RECT& locked, UINT width, UINT height) {
    BITMAPFILEHEADER file{}; BITMAPINFOHEADER info{};
    file.bfType = 0x4d42;
    file.bfOffBits = sizeof(file) + sizeof(info);
    file.bfSize = file.bfOffBits + width * height * 4;
    info.biSize = sizeof(info); info.biWidth = static_cast<LONG>(width);
    info.biHeight = -static_cast<LONG>(height); info.biPlanes = 1;
    info.biBitCount = 32; info.biCompression = BI_RGB;
    std::ofstream output("local/installed_mesh/render.bmp", std::ios::binary);
    output.write(reinterpret_cast<const char*>(&file), sizeof(file));
    output.write(reinterpret_cast<const char*>(&info), sizeof(info));
    for (UINT row = 0; row < height; ++row)
        output.write(static_cast<const char*>(locked.pBits) + row * locked.Pitch, width * 4);
    return static_cast<bool>(output);
}

bool draw_mesh(IDirect3DDevice9& device, AssetStreamProbe& assets,
    const InstalledModelProbe& model, const bsp::CompiledMaterialPass& shaders, std::string& error) {
    if (shaders.base.options.instance_generator != "building" || !model.hierarchy.matrix
        || shaders.shadow_samplers.shadow_texture_slot != 1 || shaders.shadow_samplers.shadow_map_slot != -1)
        return false;
    const auto& mesh = model.mesh;
    const auto& subset = mesh.subsets[0];
    bsp::MaterialLighting lighting;
    TextureImports imports;
    std::vector<std::unique_ptr<bsp::D3D9RetainedTexture2D>> textures;
    bsp::MaterialTextureSlots slots;
    std::vector<const bsp::MeshVertexStreamPayload*> ordered_streams;
    for (const auto& event : subset.events) {
        if (const auto* selected = std::get_if<bsp::MeshVertexStreamReference>(&event)) {
            if (selected->index >= mesh.vertex_streams.size()) return false;
            ordered_streams.push_back(&mesh.vertex_streams[selected->index]);
        } else if (const auto* texture = std::get_if<bsp::MeshTextureRequest>(&event)) {
            std::unique_ptr<bsp::D3D9RetainedTexture2D> owner;
            if (!load_texture(device, assets, imports, texture->name, owner, error)) return false;
            auto logical = std::make_shared<bsp::LogicalTexture>(); logical->texture = owner->texture();
            if (!bsp::set_material_texture_00b189f0(slots, texture->slot, logical)) return false;
            textures.push_back(std::move(owner));
        } else if (const auto* record = std::get_if<bsp::MeshLightingRecord>(&event))
            lighting.set_lighting_record_00b179d0(record->ignored_slot, record->values);
    }
    if (ordered_streams.size() != 1 || !mesh.indices) return false;
    // This white image is a controlled unoccluded ShadowTexture scene input.
    // Native shadow generation/global ownership is a separate remaining task.
    std::unique_ptr<bsp::D3D9RetainedTexture2D> shadow_image;
    if (!load_texture(device, assets, imports, "white.tga", shadow_image, error)) return false;
    auto shadow = std::make_shared<bsp::LogicalTexture>(); shadow->texture = shadow_image->texture();
    std::vector<float> vertex_words(256 * 4), pixel_words(224 * 4);
    bsp::MeshDecodeBindingStats decoded;
    if (!scene_constants(shaders.vr, vertex_words, true, error)
        || !scene_constants(shaders.pr, pixel_words, false, error)
        || !bsp::pack_mesh_vertex_decode_constants_00b428c0(ordered_streams, shaders.vb,
            {static_cast<std::uint32_t>(shaders.base.options.compressed_element_count), {}},
            0, vertex_words, decoded, error)
        || decoded.records_from_metadata != 3 || decoded.records_written != 3) return false;
    bsp::BuildingInstanceData instance;
    if (!bsp::write_building_instance_data_00b55780(*model.hierarchy.matrix, {}, 1,
        lighting.diffuse_color_00b179f0(0)[3], instance, error)) return false;
    auto declaration = std::make_shared<bsp::VertexDeclaration>();
    auto instance_declaration = std::make_shared<bsp::VertexDeclaration>();
    if (!bsp::decode_vertex_format_00b2dbd0(ordered_streams[0]->format_name, *declaration, error)
        || !bsp::decode_vertex_format_00b2dbd0(bsp::building_instance_vertex_format, *instance_declaration, error)) return false;

    OwnedCom<IDirect3DStateBlock9> saved;
    OwnedCom<IDirect3DSurface9> old_target, old_depth, target, depth, readback;
    HRESULT hr = device.CreateStateBlock(D3DSBT_ALL, &saved.p);
    if (SUCCEEDED(hr)) hr = device.GetRenderTarget(0, &old_target.p);
    if (SUCCEEDED(hr)) {
        const auto result = device.GetDepthStencilSurface(&old_depth.p);
        if (FAILED(result) && result != D3DERR_NOTFOUND) hr = result;
    }
    if (FAILED(hr)) return false;
    unsigned visible = 0; std::set<DWORD> colors;
    bool buffers_match = false, constants_match = false, layout_match = false, bindings_match = false;
    try {
        bsp::RendererSynchronization sync;
        bsp::D3D9StateCache states(device, sync, nullptr);
        std::shared_ptr<bsp::LogicalVertexStream> vertices;
        std::shared_ptr<bsp::LogicalIndexStream> indices;
        hr = bsp::create_mesh_vertex_stream_00b4bc00_fragment(device, states,
            *ordered_streams[0], declaration, vertices);
        if (SUCCEEDED(hr)) hr = bsp::create_mesh_index_stream_00b4bf30_fragment(device, states, *mesh.indices, indices);
        // Explicit host backing for the recovered instance record. The native
        // per-frame instance allocator is not reconstructed by this diagnostic.
        auto instances = std::make_shared<bsp::LogicalVertexStream>();
        instances->physical = std::make_shared<bsp::VertexBufferBinding>();
        instances->physical->flags = 1; instances->physical->capacity = sizeof(instance);
        instances->declaration = instance_declaration; instances->vertex_count = 1;
        instances->flags = 1; instances->tag = static_cast<DWORD>(D3DSTREAMSOURCE_INSTANCEDATA);
        if (SUCCEEDED(hr)) hr = bsp::vertex_buffer_recreate_00b492b0(*instances->physical, device);
        void* mapped = nullptr;
        if (SUCCEEDED(hr)) hr = states.lock_vertex_stream_00b49980(*instances, 1, 0, false, mapped);
        if (SUCCEEDED(hr) && mapped) {
            std::memcpy(mapped, instance.data(), sizeof(instance)); states.unlock_vertex_stream_00b49a80(*instances);
        } else if (SUCCEEDED(hr)) hr = E_FAIL;
        auto layout = std::make_shared<bsp::D3D9VertexLayout>();
        layout->append_stream_00b48a00(declaration); layout->append_stream_00b48a00(instance_declaration);
        if (SUCCEEDED(hr)) hr = layout->create_if_missing_00b60a10(device);
        const auto elements = layout->elements();
        layout_match = elements.size() == 13 && layout->stride() == 160;
        for (UINT i = 0; layout_match && i < 9; ++i)
            layout_match = elements[i+3].Stream == 1 && elements[i+3].Offset == i*16
                && elements[i+3].Type == D3DDECLTYPE_FLOAT4 && elements[i+3].Usage == D3DDECLUSAGE_TEXCOORD
                && elements[i+3].UsageIndex == i+1;
        if (SUCCEEDED(hr)) {
            D3DVERTEXBUFFER_DESC vd{}; D3DINDEXBUFFER_DESC id{};
            hr = vertices->physical->buffer->GetDesc(&vd);
            if (SUCCEEDED(hr)) hr = indices->physical->buffer->GetDesc(&id);
            buffers_match = SUCCEEDED(hr) && vd.Size == 192 && vd.Usage == 0 && vd.Pool == D3DPOOL_MANAGED
                && id.Size == 48 && id.Usage == 0 && id.Pool == D3DPOOL_MANAGED && id.Format == D3DFMT_INDEX16;
            if (SUCCEEDED(hr)) hr = vertices->physical->buffer->Lock(0, 0, &mapped, D3DLOCK_READONLY);
            if (SUCCEEDED(hr)) {
                buffers_match = buffers_match && std::memcmp(mapped, ordered_streams[0]->bytes.data(), vd.Size) == 0;
                hr = vertices->physical->buffer->Unlock();
            }
            if (SUCCEEDED(hr)) hr = indices->physical->buffer->Lock(0, 0, &mapped, D3DLOCK_READONLY);
            if (SUCCEEDED(hr)) {
                buffers_match = buffers_match && std::memcmp(mapped, mesh.indices->bytes.data(), id.Size) == 0;
                hr = indices->physical->buffer->Unlock();
            }
            if (SUCCEEDED(hr)) hr = instances->physical->buffer->Lock(0, 0, &mapped, D3DLOCK_READONLY);
            if (SUCCEEDED(hr)) {
                buffers_match = buffers_match && std::memcmp(mapped, instance.data(), sizeof(instance)) == 0;
                hr = instances->physical->buffer->Unlock();
            }
        }
        if (SUCCEEDED(hr)) hr = device.CreateRenderTarget(256,256,D3DFMT_A8R8G8B8,D3DMULTISAMPLE_NONE,0,FALSE,&target.p,nullptr);
        if (SUCCEEDED(hr)) hr = device.CreateDepthStencilSurface(256,256,D3DFMT_D24S8,D3DMULTISAMPLE_NONE,0,TRUE,&depth.p,nullptr);
        if (SUCCEEDED(hr)) hr = device.CreateOffscreenPlainSurface(256,256,D3DFMT_A8R8G8B8,D3DPOOL_SYSTEMMEM,&readback.p,nullptr);
        if (SUCCEEDED(hr)) hr = device.SetDepthStencilSurface(nullptr);
        if (SUCCEEDED(hr)) hr = device.SetRenderTarget(0,target.p);
        if (SUCCEEDED(hr)) hr = device.SetDepthStencilSurface(depth.p);
        const D3DVIEWPORT9 viewport{0,0,256,256,0,1};
        if (SUCCEEDED(hr)) hr = device.SetViewport(&viewport);
        for (const auto& setting : {std::pair<D3DRENDERSTATETYPE,DWORD>{D3DRS_CULLMODE,D3DCULL_NONE},
            {D3DRS_ZENABLE,TRUE},{D3DRS_ZWRITEENABLE,TRUE},{D3DRS_ZFUNC,D3DCMP_LESSEQUAL},
            {D3DRS_ALPHATESTENABLE,FALSE},{D3DRS_ALPHABLENDENABLE,FALSE},{D3DRS_SCISSORTESTENABLE,FALSE},
            {D3DRS_SRGBWRITEENABLE,FALSE},{D3DRS_COLORWRITEENABLE,15},{D3DRS_CLIPPLANEENABLE,0}})
            if (SUCCEEDED(hr)) hr = device.SetRenderState(setting.first,setting.second);
        for (UINT slot = 0; slot < 2; ++slot)
            if (SUCCEEDED(hr)) hr = device.SetSamplerState(slot,D3DSAMP_SRGBTEXTURE,FALSE);
        bsp::LogicalVertexShader vertex_shader{shaders.vertex}; bsp::LogicalPixelShader pixel_shader{shaders.pixel};
        if (SUCCEEDED(hr)) {
            states.bind_render_state_block_00b27a80(shaders.states);
            states.bind_sampler_state_block_00b27b90(std::make_shared<bsp::SamplerStateBlock>(shaders.pass.sampler_states));
            hr = states.bind_vertex_shader_00b21d10(&vertex_shader);
        }
        if (SUCCEEDED(hr)) hr = states.bind_pixel_shader_00b21c20(&pixel_shader);
        if (SUCCEEDED(hr)) hr = states.bind_vertex_layout_00b23f20(layout);
        if (SUCCEEDED(hr)) {
            states.bind_vertex_stream_00b24840(0, vertices); states.bind_vertex_stream_00b24840(1, instances);
            states.set_stream_frequency_00b24a40(0, vertices->tag | 1);
            states.set_stream_frequency_00b24a40(1, instances->tag | 1);
            states.bind_index_stream_00b24b00(indices, static_cast<INT>(vertices->base_vertex));
            hr = bsp::bind_material_textures_00b43470(states, shaders.pass, slots.textures(), shaders.pb.sampler_mask);
        }
        if (SUCCEEDED(hr)) hr = bsp::bind_material_shadow_samplers_00b430cf_fragment(states, shaders.shadow_samplers, {shadow,false,false,{}});
        if (SUCCEEDED(hr)) hr = states.set_vertex_shader_constants_f_00b21820(0,vertex_words.data(),shaders.vb.end_register);
        if (SUCCEEDED(hr)) hr = states.set_pixel_shader_constants_f_00b218c0(0,pixel_words.data(),shaders.pb.end_register);
        if (SUCCEEDED(hr)) {
            std::array<float,24> values{};
            hr = device.GetVertexShaderConstantF(shaders.vb.registers[24],values.data(),6);
            constants_match = SUCCEEDED(hr) && std::memcmp(values.data(),
                vertex_words.data()+shaders.vb.registers[24]*4,sizeof(values)) == 0;
            UINT f0=0,f1=0; device.GetStreamSourceFreq(0,&f0); device.GetStreamSourceFreq(1,&f1);
            OwnedCom<IDirect3DBaseTexture9> t0,t1;
            device.GetTexture(0,&t0.p); device.GetTexture(1,&t1.p);
            bindings_match = f0 == 0x40000001 && f1 == 0x80000001
                && t0.p == slots.textures()[0]->texture && t1.p == shadow->texture;
        }
        if (SUCCEEDED(hr) && !(buffers_match && constants_match && layout_match && bindings_match)) hr = E_FAIL;
        if (SUCCEEDED(hr)) hr = device.Clear(0,nullptr,D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER,0xff000000,1,0);
        if (SUCCEEDED(hr)) hr = device.BeginScene();
        if (SUCCEEDED(hr)) {
            hr = states.draw_indexed_00b24010({},static_cast<D3DPRIMITIVETYPE>(subset.native_primitive),
                subset.range_words[0],subset.range_words[1],subset.range_words[2]+indices->base_index,subset.range_words[3]);
            const auto ended = device.EndScene(); if (SUCCEEDED(hr)) hr = ended;
        }
        if (SUCCEEDED(hr)) hr = device.GetRenderTargetData(target.p,readback.p);
        D3DLOCKED_RECT pixels{};
        if (SUCCEEDED(hr)) hr = readback.p->LockRect(&pixels,nullptr,D3DLOCK_READONLY);
        if (SUCCEEDED(hr)) {
            struct UnlockSurface {
                IDirect3DSurface9* surface;
                ~UnlockSurface() { if (surface) surface->UnlockRect(); }
            } unlock{readback.p};
            for (UINT y=0;y<256;++y) for (UINT x=0;x<256;++x) {
                DWORD color; std::memcpy(&color,static_cast<const char*>(pixels.pBits)+y*pixels.Pitch+x*4,4);
                if (color & 0xffffff) { ++visible; colors.insert(color); }
            }
            if (!write_bitmap(pixels,256,256)) hr = E_FAIL;
            const auto unlocked = readback.p->UnlockRect(); unlock.surface = nullptr;
            if (SUCCEEDED(hr)) hr = unlocked;
        }
        states.bind_vertex_shader_00b21d10(nullptr); states.bind_pixel_shader_00b21c20(nullptr);
        states.bind_texture_00b24710(0,{}); states.bind_texture_00b24710(1,{});
        states.bind_vertex_stream_00b24840(0,{}); states.bind_vertex_stream_00b24840(1,{});
        states.bind_index_stream_00b24b00({},0); states.bind_vertex_layout_00b23f20({});
        states.set_stream_frequency_00b24a40(0,1); states.set_stream_frequency_00b24a40(1,1);
        device.SetVertexDeclaration(nullptr); states.invalidate();
    } catch (const std::exception& exception) { error = exception.what(); hr = E_FAIL; }
    bool restored = SUCCEEDED(device.SetDepthStencilSurface(nullptr));
    restored = SUCCEEDED(device.SetRenderTarget(0,old_target.p)) && restored;
    restored = SUCCEEDED(device.SetDepthStencilSurface(old_depth.p)) && restored;
    restored = SUCCEEDED(saved.p->Apply()) && restored;
    const bool checked = SUCCEEDED(hr) && buffers_match && constants_match && layout_match && bindings_match
        && visible > 32 && visible < 16384 && colors.size() > 10 && restored;
    std::printf("Installed mesh draw: hr=0x%08lx vertices=%u primitives=%u GPU_bytes=%d decode_records=%zu constant_readback=%d instance_layout=%d textures_frequencies=%d visible=%u colors=%zu restored=%d checked=%d error=%s\n",
        static_cast<unsigned long>(hr),ordered_streams[0]->count,subset.range_words[3],buffers_match,
        decoded.records_from_metadata,constants_match,layout_match,bindings_match,visible,colors.size(),restored,checked,error.c_str());
    return checked;
}
}

bool probe_installed_mesh(IDirect3DDevice9& device, AssetStreamProbe& assets) {
    InstalledModelProbe model;
    if (!probe_model_metadata(assets, &model)) return false;
    std::string descriptor, error;
    if (!bsp::shader_descriptor_name_00b2ebb0_fragment(model.mesh.subsets[0].effect_name, descriptor)) return false;
    const bsp::ShaderScriptResolver resolver = [&](const std::string& requested, std::string& bytes, std::string& message) {
        std::shared_ptr<bsp::MemoryStream> stream;
        std::string logical;
        if (!assets.read(requested, stream, message, &logical)) return false;
        std::printf("Mounted mesh shader lookup: %s -> %s\n", requested.c_str(), logical.c_str());
        bytes.assign(reinterpret_cast<const char*>(stream->data_00bef610()),
            static_cast<std::size_t>(stream->size_00bef600()));
        return true;
    };
    std::shared_ptr<bsp::CompiledMaterialPass> shaders;
    if (!bsp::compile_material_pass(device, resolver, descriptor, {0,3,false,false}, shaders, error)) {
        std::fprintf(stderr, "Installed mesh compiler: %s\n", error.c_str()); return false;
    }
    std::filesystem::create_directories("local/installed_mesh");
    std::ofstream("local/installed_mesh/vertex.hlsl", std::ios::binary) << shaders->vertex_source;
    std::ofstream("local/installed_mesh/pixel.hlsl", std::ios::binary) << shaders->pixel_source;
    for (const auto& stage : {std::pair<const char*, const std::vector<bsp::ReflectedShaderConstant>*>{"VS", &shaders->vr}, {"PS", &shaders->pr}})
        for (const auto& constant : *stage.second)
            std::printf("Installed mesh %s constant %s: set=%u type=%u index=%u count=%u rows=%u columns=%u\n",
                stage.first, constant.name.c_str(), constant.register_set, constant.parameter_type,
                constant.register_index, constant.register_count, constant.rows, constant.columns);
    std::printf("Installed mesh compiled: descriptor=%s combiner=%s samplers=%u/%u usage=%u instance=%s\n",
        descriptor.c_str(), shaders->base.combiners[0].c_str(), shaders->sampler_counts.pixel,
        shaders->sampler_counts.vertex, shaders->pb.sampler_mask, shaders->base.options.instance_generator.c_str());
    return draw_mesh(device, assets, model, *shaders, error);
}
