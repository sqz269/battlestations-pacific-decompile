#include "bsp/shader_reflection.hpp"
#include <d3dx9shader.h>
#include <cstring>

namespace bsp {
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
