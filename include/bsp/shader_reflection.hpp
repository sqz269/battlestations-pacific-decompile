#pragma once
#include <cstdint>
#include <string>
#include <vector>

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
}
