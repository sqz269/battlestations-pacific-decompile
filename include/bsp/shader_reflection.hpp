#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <array>
#include "bsp/shader_source.hpp"

namespace bsp {
struct ReflectedShaderConstant {
    std::string name;
    std::uint32_t register_set{}, register_index{}, register_count{};
    std::uint32_t parameter_class{}, parameter_type{}, rows{}, columns{}, elements{};
};
// D3DX enumeration projection of00b3aea0; bytecode must be a valid compiled
// shader including its terminating token. No native metadata/allocator ABI.
bool reflect_shader_constants_00b3aea0(const std::uint32_t* bytecode,
    std::vector<ReflectedShaderConstant>& output, std::string& error);
struct ShaderConstantBindings {
    std::array<std::uint8_t, 54> registers;
    std::array<std::uint8_t, 54> counts{}; // Host initialization; valid only if register !=FF.
    std::uint8_t end_register{};
    std::uint32_t sampler_mask{};
    std::vector<ReflectedShaderConstant> material_constants;
    ShaderConstantBindings() { registers.fill(0xff); }
};
// Native mapping portion of00b3aea0. Preserves prior system slots and appends
// unknown float records. Register/count/end truncate to bytes as in native.
// Invalid registry semantic IDs fail atomically instead of corrupting memory.
bool map_shader_constants_00b3aea0(const std::vector<ReflectedShaderConstant>&,
    const std::vector<ShaderSystemConstant>& registry, ShaderConstantBindings&);
}
