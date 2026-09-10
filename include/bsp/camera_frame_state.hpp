#pragma once
#include "bsp/camera_transform.hpp"
#include "bsp/d3d9_states.hpp"
#include "bsp/system_fog_constants.hpp"

namespace bsp {
using CameraPlane = std::array<float, 4>;
struct CameraPlaneRecord {
    CameraPlane coefficients{};
    std::uint32_t flags{};
};
struct CameraPlaneSet {
    std::array<CameraPlaneRecord, 16> planes;
    std::uint32_t count{}; // native +140; must be <=16, independent of frustum refresh
};
struct CameraViewport {
    DWORD x{}, y{}, width{}, height{}; // native wrapper +08,+0C,+10,+14
    std::uint8_t scissor_enabled{}; // +20; preserve exact byte as render-state value
    RECT scissor{}; // +24
};

// One stable companion per CameraState, sharing its projection.valid_flags.
// New storage/lifetime API, not the original camera layout or constructor.
// Borrowed viewport/fog/context objects must remain alive through their last use.
struct CameraFrameState {
    explicit CameraFrameState(CameraState& value) : camera(value) {}
    CameraFrameState(const CameraFrameState&) = delete;
    CameraFrameState& operator=(const CameraFrameState&) = delete;
    CameraState& camera;
    std::uint8_t byte_174{}; // native +174; system prefix c75.x
    std::uint8_t enabled{}; // +17C
    const CameraViewport* viewport{}; // +180; required when enabled
    const SystemFogState* fog_184{}; // one actual +184 owner, including ambient +08
    DWORD clear_flags{}; // +188
    float clear_depth{}; // +18C
    D3DCOLOR clear_color{}; // +190
    DWORD clear_stencil{}; // +194
    std::uint32_t render_mode{}; // +198; material pass selector, not batch index
    CameraMatrix inverse_view_projection{}; // +260; projection flag20
    CameraPlaneSet frustum; // +2F4; projection flag4
    const CameraPlane* context_depth_scale_43c{}; // native +43C, optional c71
    std::array<float, 3> axis_y{}, axis_x{}; // native +440,+44C; shared projection flag100
};

// Original fastcall ECX=dst, EDX=src, RET. Exact x87/SSE operation schedule;
// no singularity check and no fallback. Destination may alias source.
void invert_camera_general_00b632d0(CameraMatrix&, const CameraMatrix&);
// Original thiscall ECX=plane, stack=destination,matrix, RET8; alias-safe result.
void transform_camera_plane_00b65ba0(CameraPlane&, const CameraPlane&, const CameraMatrix&);
// Six planes: col3+col0, col3-col0, col3-col1, col3+col1, 2*col2, col3-col2;
// native negates each D then normalizes all four coefficients by normal length.
// Finite nonnegative CRT sqrt core recovered; exceptional CRT diagnostics open.
void extract_camera_frustum_00b653f0(std::array<CameraPlane, 6>&, const CameraMatrix&);
const CameraMatrix& get_camera_inverse_view_projection_00b70510(CameraFrameState&);
const CameraPlaneSet& get_camera_frustum_00b70710(CameraFrameState&);
// Native runtime global0109EEA4 chooses SSE2 double spill/CVTTSD2SI or x87
// signed64 truncation low32. This explicit argument projects that actual mode.
D3DCOLOR convert_camera_ambient_004fb850(const CameraPlane&, bool sse2_truncation);

// One stable companion for the actual cache, device, synchronization and lock.
// No independent renderer cache, COM ownership, or native memory-layout claim.
class D3D9CameraFrameAccess {
public:
    explicit D3D9CameraFrameAccess(D3D9StateCache& value) : cache_(value) {}
    D3D9CameraFrameAccess(const D3D9CameraFrameAccess&) = delete;
    D3D9CameraFrameAccess& operator=(const D3D9CameraFrameAccess&) = delete;
    bool user_clip_planes_supported{}; // renderer+1B51
    bool sse2_color_truncation{}; // explicit projection of global0109EEA4
    // Native ignores HRESULTs. New APIs expose the viewport/rectangle/Clear
    // result; attempted calls still update counters and cached state on failure.
    HRESULT bind_viewport_00b26770(const CameraViewport&);
    HRESULT clear_00b21430(DWORD count, const D3DRECT*, DWORD flags,
        const D3DCOLOR* color, float depth, DWORD stencil);
    HRESULT set_clip_plane_00b23e50(UINT index, const CameraPlane&);
    void prepare_camera_00b285a0(CameraFrameState&);
    void execute_camera_command_00b71360(CameraFrameState&);
    void restore_pending_planes_00b25080();
    void append_plane_00b25040(const CameraPlane&);
    D3D9StateCache& cache() const noexcept { return cache_; } // Host companion identity check.
    const CameraViewport* viewport() const { return viewport_; }
    const CameraPlaneSet& plane_set() const { return plane_set_; }
    const std::array<CameraPlane, 14>& device_clip_planes() const { return clip_planes_; }
    std::uint32_t active_plane_count() const { return active_plane_count_; }
    std::uint32_t pending_plane_count() const { return pending_plane_count_; }
    std::uint32_t viewport_calls() const { return viewport_calls_; }
    std::uint32_t clear_calls() const { return clear_calls_; }
private:
    struct Guard;
    D3D9StateCache& cache_;
    CameraPlaneSet plane_set_; // +17C0
    const CameraViewport* viewport_{}; // +1904; borrowed identity, never retained
    // Native +190C..+19EB, followed immediately by the two count fields.
    // Callers must keep indexes <14; actual device limits may be smaller.
    std::array<CameraPlane, 14> clip_planes_{};
    std::uint32_t active_plane_count_{}; // +19EC
    std::uint32_t pending_plane_count_{}; // +19F0
    std::uint32_t viewport_calls_{}; // +1BD0, modulo2^32
    std::uint32_t clear_calls_{}; // +1BD4, modulo2^32
};
}
