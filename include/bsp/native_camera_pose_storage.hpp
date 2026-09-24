#pragma once
#include "bsp/native_camera_raw_world_edit.hpp"
#include "bsp/system_camera_axes.hpp"
#include <cstdint>

namespace bsp {
// Pure disjoint immutable metadata borrowing live initialized DWORD arrays.
// All pointers/backing remain stable through nested calls. No typed aggregate
// is created over overlapping native scratch; no preimage is synthesized.
struct NativeCameraLookAtFrame {
    volatile std::uint32_t* locals;    // 13 words, native entryESP-34h
    volatile std::uint32_t* arguments; // 4 words at entryESP+4: target, up.xyz
};
struct NativeCameraLookAtBindings {
    const CameraAxesCrtAccess& crt;
    const volatile std::uint32_t& negative_zero_00d7a208;
    const volatile std::uint64_t& parallel_threshold_00d62ba0;
    const volatile std::uint64_t& up_perturbation_00d7a3a0;
    const volatile std::uint32_t& one_00d7a24c;
};
class NativeCameraPoseDispatch : public NativeNodeBaseWorldDispatch {
public:
    // resolve_profile additionally MUST preserve live x87 state/status/control
    // and perform integer-only lookup: B6DAE0 resolves34 while z is in ST0.
    // Invocation executes the exact captured target on actual storage with the
    // actual mutable pushed argument cell. Concrete admitted camera bindings
    // reach B71400/B71460; base bindings may reach B6DAE0/B6E870. No fallback.
    // Every reached invocation borrows prepared persistent frames; acquisition
    // is pure host metadata, with no native writes/zero-preimage allocation.
    virtual void invoke_virtual30(std::uint32_t, void*, volatile std::uint32_t&) = 0;
    virtual void invoke_virtual34(std::uint32_t, void*, volatile std::uint32_t&) = 0;
};
struct NativeCameraPositionFrame {
    volatile std::uint32_t* base_argument; // B71400 entryESP-8h, overwritten
    NativeCameraDirectionFrame direction; // backing at B71400 entryESP-20h
};
struct NativeCameraPoseFrame {
    volatile std::uint32_t* locals; // 39 words at B700E0 entryESP-9Ch
    // reciprocal[0], delta[1..3], normalized[4..6], view[7..22], world[23..38]
    NativeCameraLookAtFrame builder;
    volatile std::uint32_t* position_argument; // native entryESP-ACh
    volatile std::uint32_t* world_argument;    // same physical entryESP-ACh
    // Camera's nested B71400 base argument and builder.arguments share E-B8h;
    // builder.locals starts E-F0h; world_argument also aliases builder up.z.
    // Nested provider frames may overlap dead regions, never metadata or the
    // excluded saved-register/return gaps. Only live callee outputs apply.
};

// Complete978B normal body. ECX output, EDX eye, target/up stack block, RET10
// and EAX output become this explicit source interface. Repeated overwrites of
// actual arguments and all local/output alias read points are retained.
void* build_native_camera_look_at_00b63f10(void* actual_output64,
    const void* actual_eye3, const NativeCameraLookAtFrame&,
    const NativeCameraLookAtBindings&);

// Complete47B. Capture incoming pointer; store x, overwrite SAME argument with
// actual+F0 before y/z reads; capture current34 while z is live, store z, invoke
// captured target with that same argument word. Source projects the tail call.
void set_native_raw_world_position_00b6dae0(void* actual_camera,
    volatile std::uint32_t& position_argument, NativeCameraPoseDispatch&);
// Complete34B. Capture incoming source before2F0 clear, initialize pushed base
// argument, call B6DAE0 and then genuine B70660 using supplied direction view.
void set_native_raw_camera_position_00b71400(void* actual_camera,
    const volatile std::uint32_t& position_argument,
    const NativeCameraPositionFrame&, NativeCameraPoseDispatch&);
// Complete277B. Capture eye once/current30 before flag clear; target argument
// is current after30. Final table is captured before inverse; slot34 only after
// writing the returned pointer into the actual world argument. Constants are
// independently current at the pose and builder read sites.
void set_native_camera_look_at_storage_00b700e0(void* actual_camera,
    const volatile std::uint32_t& eye_argument,
    const volatile std::uint32_t& target_argument,
    const NativeCameraPoseFrame&, NativeCameraPoseDispatch&,
    const NativeCameraLookAtBindings&);

// Reuse genuine419440/4F9B30/inverse/world/CRT providers. Exclude aliases into
// their unexposed private frames, saved/return gaps, asynchronous mutation and
// native fault/FH3/private CRT-record identity. A real owning CRT runtime must
// remain bound. Valid addressed actual storage and raw provider domains apply.
// These bodies have no EH/rollback and do not admit a full B71A80 constructor,
// camera companion or game integration. Pure source-interface checks add no
// native success/default behavior; provider failures retain completed writes.
} // namespace bsp
