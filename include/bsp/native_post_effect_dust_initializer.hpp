#pragma once
#include "bsp/native_post_effect_construction.hpp"

namespace bsp {
// Original D620AC bytes and the same actual pools, canonical owners and native
// constant cells already used by the child B4E840 constructor. Borrowed for the
// complete lifetime of every surviving child and host companion.
struct NativePostEffectDustContext {
    NativePostEffectConstructionContext& construction;
    const char* effect_name_00d620ac;
};

struct NativePostEffectDustAcquired {
    void* actual_receiver{};
    void* raw_child{};
    void* returned_child{};
    NativeString* effect_name_header{};
    bool name_constructed{};
    bool name_return_started{};
    bool raw_free_started{};
    bool completed_child_preserved_after_host_failure{};
    bool child_published{};
    bool texture_bind_started{};
    bool normal_body_completed{};
    int native_state{-1};
    void* captured_texture{};
    void* current_post_effect{};
    void* current_material{};
};

// Caller-owned and immovable. Preparation reserves the existing child's host
// records before B52860 writes the actual receiver. Settled failures keep the
// exact child companions and name header available for external disposition.
// Diagnostics never grant permission to replay a consumed return or free.
class NativePostEffectDustBlock final {
public:
    enum class Phase { idle, preparing, prepared, executing, settled };
    NativePostEffectDustBlock() noexcept = default;
    ~NativePostEffectDustBlock() noexcept;
    NativePostEffectDustBlock(const NativePostEffectDustBlock&) = delete;
    NativePostEffectDustBlock& operator=(const NativePostEffectDustBlock&) = delete;
    NativePostEffectDustBlock(NativePostEffectDustBlock&&) = delete;
    NativePostEffectDustBlock& operator=(NativePostEffectDustBlock&&) = delete;
    void prepare(NativePostEffectDustContext&);
    void cancel_preparation() noexcept;
    Phase phase() const noexcept { return phase_; }
    const NativePostEffectDustAcquired& acquired() const noexcept { return acquired_; }
    NativePostEffectConstructionBlock& child() noexcept { return child_; }
    // Releases nothing. Caller first disposes every surviving native/host child
    // and residual name allocation, establishes quiescence, and resets child().
    // This resets only diagnostic storage and the inactive raw name header.
    void reset_after_host_quiescence() noexcept;
private:
    friend void initialize_native_post_effect_dust_00b52860(void*, std::size_t,
        NativePostEffectDustBlock&);
    void settle() noexcept;
    void return_name();
    void unwind(int&, bool&);
    NativePostEffectDustContext* context_{};
    Phase phase_{Phase::idle};
    NativePostEffectDustAcquired acquired_;
    NativePostEffectConstructionBlock child_;
    NativeString effect_name_;
};

// B4CBF0 is exactly MOV EAX,[ECX+14h]; RET. The private native leaf has no
// profile check or ownership effect. Valid receiver storage is required.
void* __fastcall get_native_post_effect_material_00b4cbf0(const void*) noexcept;

// Complete306-byte B52860 normal body and its logical caller cleanup schedule.
// Original ECX existing receiver (at least CCh bytes), no stack arguments, RET;
// EBX/ESI/EDI saved. New source interface receives extent and prepared host block.
// It creates one24h D61EC8 child, binds CURRENT receiver+3C to CURRENT +68's
// material slot0, then performs only the evidenced field stores. It does not
// release a prior+68 value or initialize the entire outer receiver.
// B4E840 host binding failure after native completion preserves the child raw
// allocation and leaves the unexecuted parent publication untouched. Ordinary
// C++ secondary cleanup policy is explicit; no native FH3/ABI/gameplay claim.
void initialize_native_post_effect_dust_00b52860(void* actual_receiver,
    std::size_t receiver_bytes, NativePostEffectDustBlock&);
} // namespace bsp
