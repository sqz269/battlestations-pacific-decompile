#pragma once
#include "bsp/native_material_pass_copy.hpp"

namespace bsp {
// Persistent host state, not embedded native storage. The parent B3B280 is
// analyzed only: its actual singleton/cache loader remains unimplemented.
struct NativeDescriptorSamplerOperation final {
    enum class Phase {fresh,running,complete,failed,diagnostic_retired};
    Phase phase{Phase::fresh};
    std::uint32_t function{},native_site{},index{},value{},stage_word{};
    void* owner{};
    void* retained_owner{};
    NativeMaterialPassDestructionAccess* binding_access{};
    NativeStringStorage* strings{};
    const NativeString* source_name{};
    NativeString* destination_name{};
    volatile std::uint32_t* actual_stage_local{};
    NativeMaterialPassBindingStorage* binding{};
    bool raw_binding_freed{};
    NativeDescriptorSamplerOperation()=default;
    ~NativeDescriptorSamplerOperation();
    NativeDescriptorSamplerOperation(const NativeDescriptorSamplerOperation&)=delete;
    NativeDescriptorSamplerOperation& operator=(const NativeDescriptorSamplerOperation&)=delete;
    // Caller first resolves any retained owner/storage obligations; frees nothing.
    void acknowledge_diagnostic_cleanup() noexcept;
};

// B5F100 ECX actual pass; stack index and stage argument (only its low byte),
// RET8. Append to actual pass+24 eight-byte rows using existing B40C80 reserve.
// stage_local is the caller-supplied ACTUAL four-byte scratch at native ESP-4:
// write only byte0 before reserve, then load the whole DWORD after reserve.
// Upper bytes are preexisting native stack bytes, not zero padding or bool.
// Caller retains this storage across allocation/failure and supplies its preimage.
void append_native_material_texture_reference_00b5f100(NativeMaterialPassBaseStorage&,
    std::uint32_t index,std::uint8_t stage,volatile std::uint32_t& stage_local,
    NativeDescriptorSamplerOperation&);

// B44CF0 ECX actual88h pass; stack binding word, retained owner, counted name;
// RET Ch. Allocate actual10h binding and reuse B44690 in the same lifetime.
// Publish at CURRENT pass+6C after construction, then increment CURRENT count.
// Four-slot accessible extent required; no replacement release or private registry.
// Constructor failure frees raw allocation, as the one-state native unwind does;
// binding constructor already cleans its name. The parent frame remains failed.
void append_native_material_pass_binding_00b44cf0(NativeMaterialPassStorage&,
    std::uint32_t word,void* retained_owner,const NativeString&,
    NativeMaterialPassDestructionAccess&,NativeDescriptorSamplerOperation&);

// B18FB0 ECX actual effect base; stack unsigned index, actual8h name; RET8.
// Compare index unsigned with sign-extended +94 count; publish low16(index+1)
// before copying actual name+3C+index*8. Same-header skips copy AFTER count.
// Valid index0..10 required. Reuse existing actual-header resize and string pool.
void set_native_material_effect_texture_name_00b18fb0(NativeMaterialEffectBaseStorage&,
    std::uint32_t index,const NativeString&,NativeStringStorage&,
    NativeDescriptorSamplerOperation&);

// Explicit C++ ABIs; descriptive names are hypotheses. No native FH3 encoding,
// rollback or external retirement interception. Borrowed owners, rows, scratch
// and shared contexts stay alive and retirement is excluded while running/failed.
} // namespace bsp
