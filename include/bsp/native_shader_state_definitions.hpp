#pragma once
#include "bsp/native_shader_sampler_owner.hpp"
#include "bsp/singleton_lifetime.hpp"

namespace bsp {
// Actual 16-byte registry entry. Only the name is initialized by resize growth;
// state and conversion remain preimage until a record is copied/registered.
struct NativeShaderStateDefinition {
    NativeString name_00;
    std::uint32_t state_08, conversion_0c;
};
using NativeShaderStateDefinitionArray=NativeShaderDescriptorArray;
struct NativeShaderStateDefinitionsStorage {
    std::uintptr_t vtable_00;
    NativeShaderStateDefinitionArray render_04, sampler_10, texture_stage_1c;
};
static_assert(sizeof(NativeShaderStateDefinition)==0x10);
static_assert(offsetof(NativeShaderStateDefinition,state_08)==8);
static_assert(sizeof(NativeShaderStateDefinitionsStorage)==0x28);
static_assert(offsetof(NativeShaderStateDefinitionsStorage,render_04)==4);
static_assert(offsetof(NativeShaderStateDefinitionsStorage,sampler_10)==0x10);
static_assert(offsetof(NativeShaderStateDefinitionsStorage,texture_stage_1c)==0x1c);

// Full native bodies; new explicit C++ dependencies, not drop-in original ABI.
// Fresh storage, valid counts/extents and non-dangling copy inputs are required.
NativeShaderStateDefinition* copy_native_shader_state_definition_00b57630(
    void* fresh, const NativeShaderStateDefinition&, NativeStringStorage&);
void reserve_native_shader_state_definitions_00b57800(
    NativeShaderStateDefinitionArray&, std::int32_t request, NativeStringStorage&);
void resize_native_shader_state_definitions_00b57930(
    NativeShaderStateDefinitionArray&, std::int32_t request, NativeStringStorage&);
void append_native_shader_state_definition_00b58050(
    NativeShaderStateDefinitionArray&, const NativeShaderStateDefinition&, NativeStringStorage&);
void register_native_shader_state_definition_00b58200(
    NativeShaderStateDefinitionArray&, const NativeString&, std::uint32_t state,
    std::uint32_t conversion, NativeStringStorage&);
// Resize(0), then free current data. Count becomes zero; pointer/capacity stale.
void destroy_native_shader_state_definition_array_00b58300(
    NativeShaderStateDefinitionArray&, NativeStringStorage&);
// First matching state wins; existing payload is never replaced. Growth +5,
// minimum10, through the already reconstructed B40CF0 eight-byte row reserve.
void append_unique_native_shader_state_pair_00b567b0(
    NativeShaderStateListStorage&, std::uint32_t state, std::uint32_t payload);

// Base lifetime functions touch only word00 plus the supplied ACTUAL global
// 0108FE90 and shared01090AA0 domain. Registration/unregistration reload that
// global after a second manager getter, under the originally captured section.
NativeShaderStateDefinitionsStorage* construct_native_shader_state_definition_base_00b56610(
    NativeShaderStateDefinitionsStorage&, NativeShaderStateDefinitionsStorage* volatile&,
    SingletonLifetimeDomain&);
void destroy_native_shader_state_definition_base_00b566b0(
    NativeShaderStateDefinitionsStorage&, NativeShaderStateDefinitionsStorage* volatile&,
    SingletonLifetimeDomain&);
NativeShaderStateDefinitionsStorage* delete_native_shader_state_definition_base_00b56750(
    NativeShaderStateDefinitionsStorage*, NativeShaderStateDefinitionsStorage* volatile&,
    SingletonLifetimeDomain&, std::uint32_t flags);
// Full B585A0: actual 40-byte owner, 31/13/18 ordered entries; constructor
// publishes in its base before arrays exist. No hidden reference count.
NativeShaderStateDefinitionsStorage* construct_native_shader_state_definitions_00b585a0(
    void* fresh, NativeShaderStateDefinitionsStorage* volatile&,
    SingletonLifetimeDomain&, NativeStringStorage&);
void destroy_native_shader_state_definitions_00b58320(
    NativeShaderStateDefinitionsStorage&, NativeShaderStateDefinitionsStorage* volatile&,
    SingletonLifetimeDomain&, NativeStringStorage&);
NativeShaderStateDefinitionsStorage* delete_native_shader_state_definitions_00b59e50(
    NativeShaderStateDefinitionsStorage*, NativeShaderStateDefinitionsStorage* volatile&,
    SingletonLifetimeDomain&, NativeStringStorage&, std::uint32_t flags);

// Host callback composition for the SAME lifetime domain and published slot.
// Construct binding, then domain from callbacks(), then bind(domain, strings)
// before constructing any definitions. Keep all three alive through shutdown.
class NativeShaderStateDefinitionsLifetimeBinding final {
public:
    NativeShaderStateDefinitionsLifetimeBinding(
        NativeShaderStateDefinitionsStorage* volatile&, SingletonLifetimeCallbacks next);
    NativeShaderStateDefinitionsLifetimeBinding(const NativeShaderStateDefinitionsLifetimeBinding&)=delete;
    NativeShaderStateDefinitionsLifetimeBinding& operator=(const NativeShaderStateDefinitionsLifetimeBinding&)=delete;
    SingletonLifetimeCallbacks callbacks() noexcept;
    void bind(SingletonLifetimeDomain&, NativeStringStorage&);
private:
    static void destroy_registered(void*,void*,std::uint32_t) noexcept;
    static void invalid_parameter(void*);
    NativeShaderStateDefinitionsStorage* volatile& published_;
    SingletonLifetimeCallbacks next_;
    SingletonLifetimeDomain* lifetime_=nullptr;
    NativeStringStorage* strings_=nullptr;
};
} // namespace bsp
