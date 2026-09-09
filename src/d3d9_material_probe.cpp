// One bounded material fixture in the existing diagnostic host. Native pass
// construction, metadata loading and the complete constant builder are unported.
#include "bsp/d3d9_states.hpp"
#include "bsp/material_constants.hpp"
#include <cstring>
#include <cstdio>

bool probe_material_states_and_constants(IDirect3DDevice9& device) {
    IDirect3DStateBlock9* saved = nullptr;
    HRESULT result = device.CreateStateBlock(D3DSBT_ALL, &saved);
    if (FAILED(result)) return false;
    auto* lock = bsp::critical_section_create_00bd1860();
    bsp::RendererSynchronization sync{};
    bsp::set_renderer_synchronization_00b33aa0(sync, true);
    bool matched = false;
    {
        bsp::D3D9StateCache state(device, sync, lock);
        auto render = std::make_shared<bsp::RenderStateBlock>();
        render->states = {{D3DRS_ZENABLE, FALSE}, {D3DRS_CULLMODE, D3DCULL_CW},
            {D3DRS_CULLMODE, D3DCULL_CCW}};
        auto sampler = std::make_shared<bsp::SamplerStateBlock>();
        sampler->states = {{0, D3DSAMP_MINFILTER, D3DTEXF_POINT},
            {0, D3DSAMP_MAGFILTER, D3DTEXF_POINT}};
        std::weak_ptr<bsp::RenderStateBlock> retained_render = render;
        std::weak_ptr<bsp::SamplerStateBlock> retained_sampler = sampler;
        state.bind_render_state_block_00b27a80(render);
        state.bind_sampler_state_block_00b27b90(sampler);
        state.bind_render_state_block_00b27a80(render);
        state.bind_sampler_state_block_00b27b90(sampler);
        render.reset(); sampler.reset();
        matched = !retained_render.expired() && !retained_sampler.expired()
            && state.render_block_calls() == 1 && state.sampler_block_calls() == 1
            && state.render_calls() == 3 && state.sampler_calls() == 2;
        state.bind_render_state_block_00b27a80(nullptr);
        state.bind_sampler_state_block_00b27b90(nullptr);
        DWORD cull{}, filter{};
        result = device.GetRenderState(D3DRS_CULLMODE, &cull);
        if (SUCCEEDED(result)) result = device.GetSamplerState(0, D3DSAMP_MINFILTER, &filter);
        matched = matched && SUCCEEDED(result) && cull == D3DCULL_CCW && filter == D3DTEXF_POINT
            && retained_render.expired() && retained_sampler.expired()
            && state.render_block_calls() == 2 && state.sampler_block_calls() == 2
            && state.render_calls() == 3 && state.sampler_calls() == 2;

        bsp::MaterialConstantParameter matrix, scalar;
        const float matrix_values[]{1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16};
        const float scalar_values[]{21,22,23};
        matrix.source_words.resize(16); scalar.source_words.resize(3);
        std::memcpy(matrix.source_words.data(), matrix_values, sizeof(matrix_values));
        std::memcpy(scalar.source_words.data(), scalar_values, sizeof(scalar_values));
        matrix.matrix = true; matrix.vertex_registers = {2}; matrix.pixel_registers = {1};
        scalar.vertex_registers = {0}; scalar.pixel_registers = {-1};
        std::vector<float> vertex(32, -99), pixel(32, -99);
        const auto packed = bsp::pack_material_parameter_constants_00b423c5(
            {matrix, scalar}, 0, {{2,4}, {2,2}}, vertex, pixel);
        std::vector<float> expected_vertex(32, -99), expected_pixel(32, -99);
        const float transposed[]{1,5,9,13,2,6,10,14,3,7,11,15,4,8,12,16};
        float system_matrix[16]{};
        bsp::write_system_matrix_00b404a0(system_matrix, matrix_values);
        matched = matched && std::memcmp(system_matrix, transposed, sizeof(transposed)) == 0;
        std::memcpy(expected_vertex.data(), scalar_values, sizeof(scalar_values));
        std::memcpy(expected_vertex.data() + 8, transposed, 8 * sizeof(float));
        std::memcpy(expected_pixel.data() + 4, transposed, sizeof(transposed));
        matched = matched && packed == bsp::MaterialConstantPackStatus::complete
            && vertex == expected_vertex && pixel == expected_pixel;
        if (matched) result = state.set_vertex_shader_constants_f_00b21820(0, vertex.data(), 8);
        if (matched && SUCCEEDED(result)) result = state.set_pixel_shader_constants_f_00b218c0(0, pixel.data(), 8);
        float observed_vertex[32]{}, observed_pixel[32]{};
        if (matched && SUCCEEDED(result)) result = device.GetVertexShaderConstantF(0, observed_vertex, 8);
        if (matched && SUCCEEDED(result)) result = device.GetPixelShaderConstantF(0, observed_pixel, 8);
        matched = matched && SUCCEEDED(result)
            && std::memcmp(observed_vertex, expected_vertex.data(), sizeof(observed_vertex)) == 0
            && std::memcmp(observed_pixel, expected_pixel.data(), sizeof(observed_pixel)) == 0
            && sync.nesting == 0 && lock->depth == 0;
    }
    if (FAILED(saved->Apply())) matched = false;
    saved->Release();
    bsp::critical_section_destroy_owned_0041cc80(lock);
    std::printf("D3D9 material: hr=0x%08lx ordered_blocks_retention_and_packed_registers=%d\n",
        static_cast<unsigned long>(result), matched);
    return matched;
}
