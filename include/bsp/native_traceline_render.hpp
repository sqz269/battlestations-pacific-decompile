#pragma once
#include <cstdint>

namespace bsp {
struct CameraAxesCrtAccess;
struct NativeCameraFrustumContext;
struct NativeLogicalBufferMappingContext;
struct FrameClock;
struct ClockTimestamp;
class SystemTimeTimerVirtuals;
struct NativeTracelineRenderAccess;

// Borrow the application's canonical native profile and render-command domains.
// profile() resolves the CURRENT raw +00 table without changing the owner.
// Its result contains at least the requested slot and remains valid through the
// call. Numeric image addresses are inspected, never called as host pointers.
// Known Model/Traceline slots dispatch concrete bodies below. Other derived
// slots require the actual service, including all original stack arguments.
class NativeTracelineRenderServices {
public:
    virtual ~NativeTracelineRenderServices() = default;
    virtual const volatile std::uint32_t* profile(const void* actual_owner) = 0;
    virtual const void* call_virtual48(void* actual_owner, std::uint32_t native_target) = 0;
    virtual std::uint32_t call_virtual58(void* actual_owner, std::uint32_t native_target,
        void* actual_camera, float lod) = 0;
    virtual void call_virtual20(void* actual_owner, std::uint32_t native_target,
        void* actual_context, float lod, float visibility, std::uint32_t flags) = 0;
    // Actual B1DFF0 context is render-context+10 (the existing command owner),
    // not the outer 18h context. The entry is the original 28h cache row.
    // This boundary must execute actual visibility/group/batch collection;
    // an empty callback, copied InstanceRenderEntry, or second queue is invalid.
    virtual void collect_entry_00b1dff0(void* actual_command, void* actual_entry) = 0;
};

struct NativeTracelineRenderAccess {
    const CameraAxesCrtAccess* crt;
    const NativeCameraFrustumContext* frustum;
    NativeLogicalBufferMappingContext* mapping;
    FrameClock* const volatile* clock_01090ab0;
    SystemTimeTimerVirtuals* clock_virtuals;
    NativeTracelineRenderServices* services;
    const volatile float* negative_zero_00d7a208;
    const volatile float* zero_00d7a218;
    const volatile float* one_00d7a24c;
    const volatile float* epsilon_00d7a238;
    const volatile double* alpha_00ce4b48;
    const volatile double* half_00d7a280;
    void* const volatile* entry_cache_0108fe88;
    // +14 service on the SAME current FrameClock, returning its real signed
    // int64 pair. The source performs FILD/FILD/FDIVP/FSTP32 itself.
    const ClockTimestamp* (__fastcall* clock_current_14)(FrameClock&, SystemTimeTimerVirtuals&);
};

// Complete AF26A0..AF3168. Original ECX actual1BCh Traceline, four stack
// words(context,lod,visibility,flags), RET10. EDX adds access; original stack
// arguments stay in order. All641 reachable instructions, 48-byte vertices,
// optional ten flare vertices, section0 range stores, actual map/unmap and
// full B748E0 submission are retained. No allocation, owner, clock or queue.
void __fastcall render_native_traceline_00af26a0(void* actual_node,
    const NativeTracelineRenderAccess*, void* actual_context,
    float lod, float visibility, std::uint32_t flags);

// Complete B748E0..B74B5A: visibility/distance/cull/LOD/mesh/child traversal.
// Same original four stack words/RET10; adds only EDX access.
void __fastcall render_native_generated_model_00b748e0(void* actual_model,
    const NativeTracelineRenderAccess*, void* actual_context,
    float lod, float visibility, std::uint32_t flags);

// Complete B72F80..B7325A normal paths. ECX actualBCh mesh; original stack
// context,model,lod,visibility,flags; RET14. All matching LOD, section-chain,
// native cache InterlockedIncrement and entry initialization paths retained.
// Actual B1DFF0 collection and other-derived virtual dispatch remain required
// external services, explicitly not reconstructed or validated by this file.
void __fastcall render_native_mesh_entries_00b72f80(void* actual_mesh,
    const NativeTracelineRenderAccess*, void* actual_context, void* actual_model,
    float lod, float visibility, std::uint32_t flags);

// Complete camera cache miss/hit and no-call classifier. Raw sphere=float4;
// plane set has 20-byte rows and signed count+140. Flags are all input bits;
// test distances spill float32 before subtracting D, preserving x87/SSE and
// unordered branches. Mask=AAA is outside; otherwise OR one bit per plane.
std::uint32_t __fastcall classify_native_camera_sphere_00b71530(void* actual_camera,
    const NativeTracelineRenderAccess*, const void* actual_sphere);
std::uint32_t __fastcall classify_native_plane_sphere_00b651e0(const void* actual_plane_set,
    const NativeTracelineRenderAccess*, const void* actual_sphere, std::uint32_t flags);

// Complete ordered11 x87 copies plus raw color DWORD+18; forward overlap
// effects and exceptional float encodings are retained. ECX dest, stack src,
// RET4/EAXdest; EDX unused. This is not memcpy of the 48-byte vertex.
void* __fastcall copy_native_traceline_vertex_00af1c20(void* destination,
    void* unused_edx, const void* source);

// Complete raw Model world-sphere/cache and sphere/point numerical kernels.
// Source sphere ECX; destination,matrix stacked; RET8. Context is EDX for
// the sphere getter/transform. No typed CameraTransform/ModelBounds overlay.
const void* __fastcall get_native_model_world_sphere_00b6e8c0(void* actual_model,
    const NativeTracelineRenderAccess*);
void* __fastcall transform_native_sphere_007c1180(const void* source,
    const NativeTracelineRenderAccess*, void* destination, const void* matrix);
void* __fastcall transform_native_point_004142e0(const void* source,
    void* unused_edx, void* destination, const void* matrix);
float __fastcall maximum_native_basis_squared_007bb620(const void* matrix);

// Complete B51A20..B51AAA, raw28h entry, eight original stack words/RET20.
// Preserves sort key+20/+24, computes native view-space depth when override
// does not exceed original zero. Source adds EDX access, no entry companion.
void __fastcall initialize_native_render_entry_00b51a20(void* actual_entry,
    const NativeTracelineRenderAccess*, float leading, void* actual_section,
    void* actual_geometry, void* actual_model, void* actual_camera,
    float visibility, float depth_override, std::uint32_t flags);

// Exact BEE050 service projection: original LEA EAX,[ECX+20]; RET. Returns
// the canonical FrameClock's CURRENT pair, not its interval or a double time.
const ClockTimestamp* __fastcall frame_clock_current_00bee050(FrameClock&) noexcept;

// New explicit bindings; source compilation/fixtures do not establish original
// object ABI, hardware-fault unwind, render-domain composition or gameplay.
// Live valid producer-created storage and nonthrowing real services required.
} // namespace bsp
