#pragma once
#include "bsp/native_material_effect_programs.hpp"
#include "bsp/native_lua_bootstrap.hpp"
#include "bsp/native_lua_file_loading.hpp"
#include "bsp/native_shader_combiner_reader.hpp"
#include "bsp/native_shader_field_reader.hpp"
#include "bsp/native_shader_sampler_reader.hpp"
#include <memory>

namespace bsp {
// All borrowed inputs belong to the SAME actual Lua/string/VFS/descriptor
// lifetime. Bootstrap and file services must install the SAME DoFile callback.
// Sampler binding/pool and published definitions outlive every parsed sampler.
// Stack preimages remain explicit inputs for the existing ordinal readers; no
// defaults are invented for missing combiner/field ordinals.
struct NativeShaderDescriptorReadContext {
    NativeStringStorage& strings;
    const NativeLuaBootstrapInputs& bootstrap;
    const NativeLuaFileServices& files;
    NativeShaderStateDefinitionsStorage* volatile& definitions_0108fe90;
    NativeShaderSamplerClassBinding& sampler_binding;
    const bool& crt_sse2_conversion;
    const NativeShaderCombinerStackInputs& combiner_stack;
    const NativeShaderFieldStackInputs& field_stack;
};

enum class NativeShaderDescriptorReadPhase {
    fresh, bootstrap, script, globals, fields, render_states, combiners,
    samplers, vertex_input, interpolators, source_strings, cleanup, complete, failed
};
// A program child frame over actual4C8h Lua state and actual14h stack objects.
// Construct and retain before calling the parser. No descriptor copy, vtable
// replacement, refcount or automatic native rollback is added. The caller keeps
// its original name8h argument at a stable address through a retained failure.
// On normal return Lua temporaries/state have been destroyed and this frame's
// destruction is metadata-only. A failed live frame cannot be discarded/retried.
class NativeShaderDescriptorReadOperation final : public NativeMaterialProgramChildFrame {
public:
    NativeShaderDescriptorReadOperation();
    ~NativeShaderDescriptorReadOperation() override;
    NativeShaderDescriptorReadOperation(const NativeShaderDescriptorReadOperation&) = delete;
    NativeShaderDescriptorReadOperation& operator=(const NativeShaderDescriptorReadOperation&) = delete;
    bool complete() const noexcept;
    bool retains_native_state() const noexcept;
    NativeShaderDescriptorReadPhase phase() const noexcept;
    std::uint32_t active_call_site() const noexcept;
    NativeLuaStateStorage* retained_lua_state() noexcept;
    NativeShaderDescriptorStorage* descriptor() const noexcept;
    const void* original_name_header() const noexcept;
private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
    friend void read_native_shader_descriptor_00b43b00(NativeShaderDescriptorStorage&,
        const void*,std::uint32_t,NativeShaderDescriptorReadContext&,NativeShaderDescriptorReadOperation&);
};

// B43B00: ECX already-constructed actual110h descriptor, stack actual8h name
// and unsigned generation, RET8; no return-value contract. Runs B69D40(name,0)
// after real Lua bootstrap(mask1), then writes scalar/string fields in native
// order and invokes the actual render/combiner/sampler/field readers. Existing
// arrays append; absent mode names and unproduced bytes preserve their preimage.
// Name/descriptor inputs stay borrowed identities; the existing callable
// descriptor binding is preserved. The normal native path is composed here;
// retained C++ failure is deliberately not the original local SEH unwind.
void read_native_shader_descriptor_00b43b00(NativeShaderDescriptorStorage&,
    const void* actual_name_header,std::uint32_t generation,
    NativeShaderDescriptorReadContext&,NativeShaderDescriptorReadOperation&);
} // namespace bsp
