#pragma once
#include "bsp/native_shader_descriptor_owner.hpp"
#include "bsp/native_material_pools.hpp"

namespace bsp {
using NativeShaderStateListStorage=NativeShaderDescriptorArray;
// Actual2Ch sampler descriptor. No intrusive counter. Names are actual8h
// headers;24/28 point to0Ch headers allocated in DISTINCT0108FEE4 pool slots.
struct NativeShaderSamplerStorage {
    std::uintptr_t vtable_00;
    NativeString name_04;
    std::uint8_t vertex_sampler_0c;
    std::byte padding_0d[3];
    std::int32_t type_10,texture_source_14;
    NativeString source_name_18;
    std::int32_t index_20;
    NativeShaderStateListStorage* sampler_states_24;
    NativeShaderStateListStorage* texture_stage_states_28;
};
static_assert(sizeof(NativeShaderSamplerStorage)==0x2c);
static_assert(offsetof(NativeShaderSamplerStorage,name_04)==4);
static_assert(offsetof(NativeShaderSamplerStorage,vertex_sampler_0c)==0xc);
static_assert(offsetof(NativeShaderSamplerStorage,type_10)==0x10);
static_assert(offsetof(NativeShaderSamplerStorage,source_name_18)==0x18);
static_assert(offsetof(NativeShaderSamplerStorage,sampler_states_24)==0x24);
// B621A0 full13-byte header constructor; ECX header, EAX same, RET. It
// initializes only12 bytes and preserves the physical slot's slab ID at+0C.
NativeShaderStateListStorage* initialize_native_shader_state_list_00b621a0(void* fresh);
// B40CF0 full104-byte reserve; ECX header, stack signed request, RET4.
// Minimum10 rows of8 bytes; signed capacity comparison, live count/data copy,
// free old storage before publishing new pointer then capacity. No rollback.
void reserve_native_shader_state_pairs_00b40cf0(NativeShaderStateListStorage&,std::int32_t request);
// B56DE0 full77-byte pointer-holder cleanup; ECX address of a header pointer,
// RET. Capture header, reserve(0) if cap<0, zero count/free rows, return SAME
// header slot to canonical pool, THEN clear holder. Header pointer/cap stay stale.
void destroy_native_shader_state_list_00b56de0(NativeShaderStateListStorage*& holder,NativeShaderStateListPool&);
void destroy_native_shader_state_list_00b56de0(NativeShaderStateListStorage*& holder,NativeShaderStateListPoolStorage&);
// Allocation-initialization fragment B57B80..B57BA0 only. Sets D621F4,
// names04/18 empty, vertex byte0/source14/index20/list pointers24/28 zero.
// Type10 and padding0D..0F remain unwritten. Full Lua readerB57B50 is NOT here.
NativeShaderSamplerStorage* initialize_native_shader_sampler_00b57b50_fragment(void* fresh);
// B56EA0 full146-byte destructor: publish D621F4, cleanup24 then28, then
// strings18 and04. Its two unwind states cover ONLY these strings, so failure
// during24 cleanup does not also destroy28. Strings leave stale headers.
void destroy_native_shader_sampler_00b56ea0(NativeShaderSamplerStorage&,NativeShaderStateListPool&,NativeStringStorage&);
void destroy_native_shader_sampler_00b56ea0(NativeShaderSamplerStorage&,NativeShaderStateListPoolStorage&,NativeStringStorage&);
// B56FC0 full30-byte scalar deletion: ECX owner, stack flags, EAX old owner,
// RET4; free2Ch object on bit0 only after successful member destruction.
NativeShaderSamplerStorage* delete_native_shader_sampler_00b56fc0(
    NativeShaderSamplerStorage*,NativeShaderStateListPool&,NativeStringStorage&,std::uint32_t flags);
NativeShaderSamplerStorage* delete_native_shader_sampler_00b56fc0(
    NativeShaderSamplerStorage*,NativeShaderStateListPoolStorage&,NativeStringStorage&,std::uint32_t flags);
// Host-only external callable table for actual descriptor C4 direct deletion.
// Keep binding and shared pool/string lifetime alive until virtual deletion
// or detach(); no private counter/registry and no implicit owner destruction.
class NativeShaderSamplerCallableBinding final {
public:
    NativeShaderSamplerCallableBinding(NativeShaderSamplerStorage&,NativeShaderStateListPool&,NativeStringStorage&);
    ~NativeShaderSamplerCallableBinding();
    NativeShaderSamplerCallableBinding(const NativeShaderSamplerCallableBinding&)=delete;
    NativeShaderSamplerCallableBinding& operator=(const NativeShaderSamplerCallableBinding&)=delete;
    void detach() noexcept;
    bool bound() const noexcept{return storage_!=nullptr;}
private:
    static void* __fastcall invoke(void*,void*,std::uint32_t);
    std::array<std::uintptr_t,2> table_;
    NativeShaderSamplerStorage* storage_;
    NativeShaderStateListPool& pool_;
    NativeStringStorage& strings_;
};
// Shared host callable table for any number of actual sampler objects from the
// same pool/string domain. It owns no objects and stores no per-object registry.
// Binding, pool and strings must outlive all attached samplers and their users.
class NativeShaderSamplerClassBinding final {
public:
    NativeShaderSamplerClassBinding(NativeShaderStateListPool&,NativeStringStorage&) noexcept;
    NativeShaderSamplerClassBinding(const NativeShaderSamplerClassBinding&)=delete;
    NativeShaderSamplerClassBinding& operator=(const NativeShaderSamplerClassBinding&)=delete;
    void bind(NativeShaderSamplerStorage&);
    void detach(NativeShaderSamplerStorage&) noexcept;
    NativeShaderStateListPool& pool() const noexcept{return pool_;}
    NativeStringStorage& strings() const noexcept{return strings_;}
private:
    static void* __fastcall invoke(void*,void*,std::uint32_t);
    std::array<std::uintptr_t,2> table_;
    NativeShaderStateListPool& pool_;
    NativeStringStorage& strings_;
};
} // namespace bsp
