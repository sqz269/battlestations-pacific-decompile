#pragma once
#include "bsp/native_string.hpp"
#include <array>
#include <cstddef>
#include <cstdint>

namespace bsp {
struct NativeMaterialEffectDescriptorContext;
struct NativeShaderDescriptorArray {
    void* data_00;
    std::int32_t count_04;
    std::int32_t capacity_08;
};
// Actual 110h descriptor. Unestablished scalar meanings remain opaque. This
// owner has NO intrusive counter: +04 is parser-written PipeID. No implicit
// cleanup, parser defaults, or initialization of the opaque bytes.
struct NativeShaderDescriptorStorage {
    std::uintptr_t vtable_00;
    std::uint32_t pipe_id_04,priority_08;
    NativeString name_0c;
    std::byte opaque_14[0x14];
    NativeString name_28;
    std::uint32_t word_30;
    NativeString name_34,name_3c;
    std::byte flags_44[4];
    NativeString mode_names_48[14];
    NativeShaderDescriptorArray pairs_b8,virtual_owners_c4,string_owners_d0,string_owners_dc;
    NativeString name_e8,name_f0,name_f8,name_100;
    std::uint32_t rt_count_108;
    std::byte opaque_10c[4];
};
static_assert(sizeof(void*)==4);
static_assert(sizeof(NativeShaderDescriptorArray)==12);
static_assert(sizeof(NativeShaderDescriptorStorage)==0x110);
static_assert(offsetof(NativeShaderDescriptorStorage,name_0c)==0x0c);
static_assert(offsetof(NativeShaderDescriptorStorage,name_28)==0x28);
static_assert(offsetof(NativeShaderDescriptorStorage,name_34)==0x34);
static_assert(offsetof(NativeShaderDescriptorStorage,mode_names_48)==0x48);
static_assert(offsetof(NativeShaderDescriptorStorage,pairs_b8)==0xb8);
static_assert(offsetof(NativeShaderDescriptorStorage,virtual_owners_c4)==0xc4);
static_assert(offsetof(NativeShaderDescriptorStorage,string_owners_d0)==0xd0);
static_assert(offsetof(NativeShaderDescriptorStorage,string_owners_dc)==0xdc);
static_assert(offsetof(NativeShaderDescriptorStorage,name_100)==0x100);
static_assert(offsetof(NativeShaderDescriptorStorage,rt_count_108)==0x108);

// B43700: ECX fresh aligned110h, EAX same, RET. Sets D61A44, zeros22
// actual strings and four12-byte headers; scalar fields and padding survive.
NativeShaderDescriptorStorage* initialize_native_shader_descriptor_00b43700(void* fresh);
// B458A0: ECX actual110h, RET. Direct current virtual0(flags1) for C4
// entries; D0/DC entries are heap objects beginning with an actual8h string.
// Capture each entry address before callbacks; reload unsigned live count.
// Then reverse13 member cleanups, also on exception. Array counts become0;
// data/capacity and string headers stay stale. Valid nonnegative extents and
// shared singleton heap / supplied actual string storage are preconditions.
void destroy_native_shader_descriptor_00b458a0(NativeShaderDescriptorStorage&,NativeStringStorage&);
void destroy_native_shader_descriptor_00b458a0(
    NativeShaderDescriptorStorage&,NativeMaterialEffectDescriptorContext&);
// B46930: ECX descriptor, stack flags; EAX old address, RET4. Bit0 frees
// the actual object through the shared heap; other flag bits do not free it.
NativeShaderDescriptorStorage* delete_native_shader_descriptor_00b46930(
    NativeShaderDescriptorStorage*,NativeStringStorage&,std::uint32_t flags);
NativeShaderDescriptorStorage* delete_native_shader_descriptor_00b46930(
    NativeShaderDescriptorStorage*,NativeMaterialEffectDescriptorContext&,std::uint32_t flags);

// Host-only callable virtual0 bridge for actual effect+C4 direct deletion.
// The external table binds the SAME110h allocation to its string lifetime;
// it adds no private counter or ownership registry. Keep this binding alive
// until virtual deletion or detach(). Those actions restore native D61A44.
// Detach transfers no ownership. Flags0 leaves already-destroyed raw storage.
class NativeShaderDescriptorCallableBinding final {
public:
    NativeShaderDescriptorCallableBinding(NativeShaderDescriptorStorage&,NativeStringStorage&);
    ~NativeShaderDescriptorCallableBinding();
    NativeShaderDescriptorCallableBinding(const NativeShaderDescriptorCallableBinding&)=delete;
    NativeShaderDescriptorCallableBinding& operator=(const NativeShaderDescriptorCallableBinding&)=delete;
    void detach() noexcept;
    bool bound() const noexcept {return storage_!=nullptr;}
private:
    static void* __fastcall invoke(void*,void*,std::uint32_t);
    std::array<std::uintptr_t,2> table_;
    NativeShaderDescriptorStorage* storage_;
    NativeStringStorage& strings_;
};
} // namespace bsp
