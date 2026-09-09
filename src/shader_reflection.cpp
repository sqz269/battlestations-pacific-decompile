#include "bsp/shader_reflection.hpp"
#include <d3dx9shader.h>
#include <cstring>

namespace bsp {
bool map_shader_constants_00b3aea0(const std::vector<ReflectedShaderConstant>& constants,
    const std::vector<ShaderSystemConstant>& registry, ShaderConstantBindings& output) {
    auto mapped = output;
    std::int32_t highest_start = -1;
    std::uint32_t highest_count = 0, sampler_mask = 0;
    for (const auto& constant : constants) {
        if (constant.parameter_type == 3) { // D3DXPT_FLOAT, not RegisterSet.
            const ShaderSystemConstant* definition = nullptr;
            for (const auto& candidate : registry) {
                if (candidate.name.size() == constant.name.size()
                    && _stricmp(candidate.name.c_str(), constant.name.c_str()) == 0) {
                    definition = &candidate; break;
                }
            }
            bool accepted = true;
            if (definition) {
                const auto id = definition->semantic_id;
                if (id >= mapped.registers.size()) return false;
                accepted = mapped.registers[id] == 0xff;
                if (accepted) {
                    mapped.registers[id] = static_cast<std::uint8_t>(constant.register_index);
                    mapped.counts[id] = static_cast<std::uint8_t>(constant.register_count);
                }
            } else mapped.material_constants.push_back(constant);
            std::int32_t start{};
            std::memcpy(&start, &constant.register_index, sizeof(start)); // Native signed JGE.
            if (accepted && highest_start < start) {
                highest_start = start; highest_count = constant.register_count;
            }
        }
        if (constant.register_set == 3) sampler_mask |= 1u << (constant.register_index & 31u);
    }
    mapped.end_register = highest_start < 0 ? 0 : static_cast<std::uint8_t>(
        static_cast<std::uint32_t>(highest_start) + highest_count);
    mapped.sampler_mask = sampler_mask;
    output = std::move(mapped);
    return true;
}
bool reflect_shader_constants_00b3aea0(const std::uint32_t* bytecode,
    std::vector<ReflectedShaderConstant>& output, std::string& error) {
    if (!bytecode) { error = "Missing shader bytecode"; return false; }
    HMODULE module = LoadLibraryExW(L"d3dx9_40.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
    if (!module) { error = "Cannot load installed D3DX9_40 for reflection"; return false; }
    decltype(&D3DXGetShaderConstantTable) get_table{};
    const FARPROC address = GetProcAddress(module, "D3DXGetShaderConstantTable");
    static_assert(sizeof(address) == sizeof(get_table));
    std::memcpy(&get_table, &address, sizeof(get_table));
    ID3DXConstantTable* table = nullptr;
    HRESULT result = get_table ? get_table(reinterpret_cast<const DWORD*>(bytecode), &table) : E_NOINTERFACE;
    D3DXCONSTANTTABLE_DESC description{};
    if (SUCCEEDED(result)) result = table ? table->GetDesc(&description) : E_FAIL;
    std::vector<ReflectedShaderConstant> constants;
    for (UINT i = 0; SUCCEEDED(result) && i < description.Constants; ++i) {
        const D3DXHANDLE handle = table->GetConstant(nullptr, i);
        D3DXCONSTANT_DESC entries[256]{};
        UINT count = 256;
        result = handle ? table->GetConstantDesc(handle, entries, &count) : E_FAIL;
        if (SUCCEEDED(result)) {
            if (!count || !entries[0].Name) { result = E_FAIL; break; }
            const auto& entry = entries[0]; // Native consumes the first returned descriptor.
            constants.push_back({entry.Name, static_cast<std::uint32_t>(entry.RegisterSet),
                entry.RegisterIndex, entry.RegisterCount, static_cast<std::uint32_t>(entry.Class),
                static_cast<std::uint32_t>(entry.Type), entry.Rows, entry.Columns, entry.Elements});
        }
    }
    if (table) table->Release();
    FreeLibrary(module);
    if (FAILED(result)) { error = "D3DX shader constant enumeration failed"; return false; }
    output = std::move(constants); error.clear(); return true;
}
}
