#pragma once
#include "bsp/camera_frame_state.hpp"

namespace bsp {
// Borrowed views of one actual camera. The caller binds its +2E0 plane and
// existing frame, whose frustum is +2F4 and shared projection flags are +2F0.
// Construction reads/writes nothing and creates no independent camera state.
struct CameraClipPlaneState {
    CameraFrameState& frame;
    CameraPlane& extra_plane_2e0;
};

// Native thiscall ECX=plane set, stack=count, RET4. Stores every count bit;
// neither validates the count nor initializes/clears any plane records.
void set_camera_plane_count_00b65070(CameraPlaneSet&, std::uint32_t count) noexcept;
// Native thiscall ECX=plane set, stack=source,flags, RET8. Caller must provide
// count<16 and 16 readable source bytes. Increments count before source reads,
// then copies forward with x87 FLD/FSTP and stores flags last. Overlap is live.
// Native unchecked out-of-storage indexes are outside this typed API contract.
void append_camera_plane_00b65990(CameraPlaneSet&, const CameraPlane&,
    std::uint32_t flags) noexcept;
// Native thiscall ECX=camera, stack=source, RET4. Copies +2E0 first, sets count6,
// appends the live source again at record6 with flags3, then ORs camera flags40.
void set_camera_extra_clip_plane_00b70410(CameraClipPlaneState&,
    const CameraPlane&) noexcept;
// Native ECX=camera, RET. Resets count6 and clears camera flag40. Plane bytes,
// record flags, frustum-cache validity and all other camera flags survive.
void clear_camera_extra_clip_plane_00b70470(CameraClipPlaneState&) noexcept;
}
