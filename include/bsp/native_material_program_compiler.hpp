#pragma once
#include "bsp/native_material_effect_programs.hpp"
#include "bsp/native_shader_field_reader.hpp"

namespace bsp {
// Actual B0h builder, produced by B354D0 and B3C3A0/B3B3C0. Unknown bytes
// remain opaque and unwritten. These are the actual pooled string/array
// headers, not a ShaderSourceBuilder projection.
struct NativeMaterialProgramBuilderStorage {
    std::uint32_t vtable_00;
    NativeShaderDescriptorArray fields_04, fields_10, fields_1c, fields_28, fields_34;
    NativeShaderDescriptorArray virtual_owners_40;
    NativeString source_4c;
    NativeShaderDescriptorArray words_54, words_60;
    std::uint8_t byte_6c, byte_6d;
    std::byte preserved_6e[2];
    NativeShaderDescriptorStorage* descriptor_70;
    NativeShaderDescriptorStorage* mode_descriptor_74;
    NativeMaterialEffectStorage* effect_78;
    std::byte unproduced_7c[0x1c];
    std::uint8_t legacy_generation_98;
    std::byte preserved_99[3];
    NativeString name_9c;
    std::uint32_t render_target_count_a4;
    std::uint8_t mode_a8, descriptor_flag_a9, policy_aa;
    std::byte preserved_ab;
    std::uint32_t generation_ac;
};
static_assert(sizeof(NativeMaterialProgramBuilderStorage)==0xb0);
static_assert(offsetof(NativeMaterialProgramBuilderStorage,fields_28)==0x28);
static_assert(offsetof(NativeMaterialProgramBuilderStorage,source_4c)==0x4c);
static_assert(offsetof(NativeMaterialProgramBuilderStorage,descriptor_70)==0x70);
static_assert(offsetof(NativeMaterialProgramBuilderStorage,legacy_generation_98)==0x98);
static_assert(offsetof(NativeMaterialProgramBuilderStorage,name_9c)==0x9c);
static_assert(offsetof(NativeMaterialProgramBuilderStorage,generation_ac)==0xac);

// B354D0 ECX fresh B0h storage, EAX same, RET. Publishes D5F314; clears
// DWORDs04..68 and name9C/A0; writes FF at6C/6D. Other bytes preserve preimage.
NativeMaterialProgramBuilderStorage* initialize_native_material_program_builder_00b354d0(void*);
// B3A7E0 normal destruction through RET B3AE9B (stored Ghidra body is short).
// Delete current40 children with current0(flags1), then field owners in native
// order04/10/28/1C/34; clear each captured slot AFTER deletion. Release name,
// words60, words54, source, and arrays40/34/28/1C/10/04. Count0 publication
// precedes each final array free; stale data/capacity remain. No root free.
// Valid accessible nonnegative counts are required; original FH3 is not emitted.
void destroy_native_material_program_builder_00b3a7e0(
    NativeMaterialProgramBuilderStorage&,NativeStringStorage&);

struct NativeMaterialProgramVertexInputsAcquired {
    NativeShaderFieldStorage* unpublished_field{};
    bool name_constructed{};
    std::uint32_t descriptor_offset{}, index{}, native_site{};
    bool entered{}, complete{};
};
// Actual B35930 ECX builder, RET. Copy current root then mode D0/D4 fields
// into builder04. Capture scalar inputs after raw allocation and BEFORE name
// copying; publish independent1Ch records with mask0, no dedup or clear.
// Same B34680 reserve/string services; retain unpublished field on host error.
void append_native_material_program_vertex_inputs_00b35930(
    NativeMaterialProgramBuilderStorage&,NativeStringStorage&,
    NativeMaterialProgramVertexInputsAcquired&);

enum class NativeMaterialProgramCompilePhase {
    fresh, builder, wrapper_name, pass_prefix, vertex_inputs, required_tail,
    builder_cleanup, complete, failed
};
enum class NativeMaterialProgramCompilerResume : std::uint32_t {
    source_after_vertex_inputs_00b3b513=0x00b3b513,
    cached_before_pass_allocation_00b3b536=0x00b3b536
};
struct NativeMaterialProgramCompilerFrame {
    NativeMaterialProgramCompilerFrame() {} // Do not zero unproduced builder bytes.
    NativeMaterialProgramBuilderStorage builder;
    NativeString local_name, substring;
    NativeMaterialProgramVertexInputsAcquired vertex_inputs;
    // Copies values/identities, never retains the caller's request address.
    NativeMaterialEffectStorage* effect{};
    NativeShaderDescriptorStorage* descriptor{};
    NativeShaderDescriptorStorage* mode_descriptor{};
    const NativeString* original_name{};
    std::uint32_t render_target_count{}, generation{};
    std::uint8_t mode{}, descriptor_flag{}, policy{};
    NativeMaterialProgramCompilePhase phase{NativeMaterialProgramCompilePhase::fresh};
    NativeMaterialProgramCompilerResume resume{};
    std::uint32_t native_site{};
    bool builder_live{}, local_name_live{}, substring_live{};
    NativeMaterialPassStorage* result{};
    NativeMaterialProgramChild tail_child;
};

// Required unimplemented NATIVE continuation, not a D3DX-result conversion.
// Starts at the supplied exact B3B3C0 instruction. It borrows stable actual
// builder/local_name and shared request identities. Attach acquisitions to
// tail_child before later calls. Do not repeat the already-executed prefix.
// A returned pass must be fully constructed/registered in the SAME canonical
// domain and transfers its held reference. nullptr means actual native compiler
// failure. A normal return must have completed native local-name cleanup and
// set local_name_live=false; the stale header itself is preserved. Any native
// orphaned allocations on null return must remain in that canonical domain,
// not be destroyed by the child frame. A failure retains this exact frame.
class NativeMaterialProgramCompilerTail {
public:
    virtual ~NativeMaterialProgramCompilerTail() = default;
    virtual NativeMaterialPassStorage* continue_material_pass_00b3b3c0(
        NativeMaterialProgramCompilerFrame&,NativeMaterialProgramCompilerResume,
        NativeMaterialProgramChild&) = 0;
};
struct NativeMaterialProgramCompileContext {
    NativeStringStorage& strings;
    const volatile std::uint8_t& load_variants_0108d6f0;
    const volatile std::uint8_t& source_mode_0108d6f1;
    NativeMaterialProgramCompilerTail& tail;
};
class NativeMaterialProgramCompileOperation final : public NativeMaterialProgramChildFrame {
public:
    NativeMaterialProgramCompileOperation();
    ~NativeMaterialProgramCompileOperation() override;
    NativeMaterialProgramCompileOperation(const NativeMaterialProgramCompileOperation&) = delete;
    NativeMaterialProgramCompileOperation& operator=(const NativeMaterialProgramCompileOperation&) = delete;
    bool complete() const noexcept;
    bool retains_native_state() const noexcept;
    const NativeMaterialProgramCompilerFrame& frame() const noexcept;
private:
    std::unique_ptr<NativeMaterialProgramCompilerFrame> frame_;
    friend NativeMaterialPassStorage* compile_native_material_program_00b3c3a0(
        const NativeMaterialProgramRequest&,NativeMaterialProgramCompileContext&,
        NativeMaterialProgramCompileOperation&);
};
// B3C3A0: ECX effect, EDX descriptor; seven stack arguments, RET1C, EAX pass.
// Actual wrapper plus B3B3C0 prefix through native B35930. Remaining compiler
// body is REQUIRED above. No default provider or successful semantic/raw bridge.
// Allocate/adopt operation before call. Normal return destroys builder after
// the compiler returns. C++ failure retains storage instead of native FH3 unwind.
NativeMaterialPassStorage* compile_native_material_program_00b3c3a0(
    const NativeMaterialProgramRequest&,NativeMaterialProgramCompileContext&,
    NativeMaterialProgramCompileOperation&);
} // namespace bsp
