#pragma once
#include "bsp/native_traceline_render.hpp"
#include <cstddef>
#include <cstdint>

namespace bsp {
struct CameraAxesCrtAccess;
class ActualNativeStringPoolStorage;
class NativeRenderActualOwners;
struct NativeInstanceGeometryAccess;
struct NativeInstanceGeometryAcquired;
struct NativeRenderBatchStorage;

// Borrow the application's actual publications and canonical domains. No
// replacement queue, group cache, model map, reference count or string pool.
struct NativeInstanceCollectionAccess {
    const CameraAxesCrtAccess* crt;
    const volatile float* one_00d7a24c;
    const volatile double* distance_width_00ce4d70;
    const volatile double* cutoff_00ceb690;
    const volatile float* default_threshold_00ce77dc;
    ActualNativeStringPoolStorage* strings;
    NativeRenderActualOwners* owners;
    NativeInstanceGeometryAccess* geometry;
    void* const volatile* entry_cache_0108fe88;
    volatile std::uint32_t* color_guard_00f8d4b0;
    volatile std::uint32_t* colors_00f8d450; // Six actual float4 rows.
    const char* separator_00d21d00;
    const char* instance_prefix_00d5e5e4;
    void* (__cdecl* allocate_00bf681b)(std::size_t);
    // External diagnostic for the current cold creation; retain partial
    // creators after an exception. Cleared only before the next creation.
    NativeInstanceGeometryAcquired* acquired_geometry;
};

// Full normal B1DFF0..B1E6A7, original ECX actual command, stack actual28h
// entry, RET4. New C++ interface adds explicit access. Uses existing actual
// group/array/model/material/batch operations; numeric x87/SSE crossings and
// current raw pointer publications are preserved. Producer-created valid
// storage, successful allocations and canonical completed companions required.
// Native C++ temporary cleanup is retained; hardware-fault/SEH unwind and
// application renderer composition are not established by this interface.
// The CRT/math-error callback must return without throwing through the naked
// visibility helper; its explicit dummy frame has no native EH registration.
void collect_native_instance_entry_00b1dff0(void* actual_command,
    void* actual_entry, NativeInstanceCollectionAccess&);

// Concrete collection route for the existing actual Traceline/mesh renderer.
// The application supplies its remaining current-profile virtual services by
// deriving from this class. This adds no owner registration or entry copy.
class NativeInstanceCollectingRenderServices : public NativeTracelineRenderServices {
public:
    explicit NativeInstanceCollectingRenderServices(NativeInstanceCollectionAccess& a) noexcept
        : collection_(a) {}
    void collect_entry_00b1dff0(void* command, void* entry) final;
private:
    NativeInstanceCollectionAccess& collection_;
};

// Complete raw effect getters: ECX owner, EAX actual+ B8 / DWORD+AC, RET.
const void* __fastcall native_effect_name_00b172d0(const void*) noexcept;
std::uint32_t __fastcall native_effect_batch_index_00b17300(const void*) noexcept;

// Complete B73770; ECX actual mesh/threshold owner, original two floats on
// stack, RET8, ST0 result. EDX supplies current constants; no threshold copy.
float __fastcall native_stream_threshold_fade_00b73770(const void*,
    const NativeInstanceCollectionAccess*, float leading, float fraction);

// Complete B51CB0. Actual batch storage borrows the same raw28h entry pointer;
// it never interprets that entry as the older semantic InstanceRenderEntry.
void append_native_render_batch_entry_00b51cb0(NativeRenderBatchStorage&, void*);

// Complete 43C130. Original ECX output8h, EDX C-string prefix, stack right8h,
// RET4/EAX output. Construct prefix, concatenate then release current prefix.
// Native ownership-bit unwind can release a completed output if prefix
// cleanup throws; this interface admits nonthrowing current pool cleanup.
void* prefix_native_string_header_0043c130(void* output, const char* prefix,
    const void* right, ActualNativeStringPoolStorage&);
} // namespace bsp
