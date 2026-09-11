#pragma once
#include "bsp/native_shader_state_reader.hpp"
namespace bsp {
// B34620, ECX pointer-array header, stack signed request, RET4. Minimum six;
// copy current live count/data, free old storage, then publish pointer/capacity.
void reserve_native_shader_sampler_pointers_00b34620(NativeShaderDescriptorArray&,std::int32_t request);
// B57B50 full reader, ECX definitions, stack actual20-byte entry, EAX newly
// allocated2Ch sampler, RET4. Numeric native vtable profile is preserved here.
// Names require nonnull Lua string/number coercions. State-list headers are
// always allocated, and published only after any table read/temporary cleanup.
// Native unwind owns temporary Lua objects/names only; no sampler rollback.
NativeShaderSamplerStorage* read_native_shader_sampler_00b57b50(
    NativeShaderStateDefinitionsStorage&,NativeLuaObjectStorage& entry,
    NativeShaderStateListPool&,NativeStringStorage&,const bool& crt_sse2_conversion);
// B41830 full traversal, ECX actual110h descriptor, stack Shader object, RET4.
// Explicit host adaptation installs the shared callable table in each parsed
// sampler. The supplied published slot models live0108FE90 and is reloaded for
// every accepted key. Entries append toC4 in lua_next order; no clear or sort.
// Binding and shared domains must outlive the descriptor and every sampler.
void read_native_shader_samplers_00b41830(NativeShaderDescriptorStorage&,
    NativeLuaObjectStorage& shader,NativeShaderStateDefinitionsStorage* volatile& published,
    NativeShaderSamplerClassBinding&,const bool& crt_sse2_conversion);
} // namespace bsp
