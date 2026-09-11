#pragma once

#include "bsp/native_camera_frustum.hpp"

namespace bsp {
// Raw original camera/node storage only. Parent+30 is an actual 32-bit native
// node address, not a NativeCameraOwner companion token. Ancestors reached by
// the world provider must support flags+5C, local+B0 and world+F0 accesses.
// No parent-null shortcut, lifetime operation or cache-hit-only substitute.
// All five originals take ECX camera, return EAX cache address, and use RET.
// The first four fastcall entries retain those argument/return locations.

// B6FCB0[58]: view+60; flags+5C bit8; refresh raw world when bit2 is clear.
void* __fastcall get_native_camera_view_00b6fcb0(void* actual_camera);

// B6FCF0[112]: flags+2F0 bit8; projection+1E0 then copy+2A0; returns+2A0.
// Original ordered far/near/aspect/fov FLD/FSTP argument preparation is kept.
// The projection provider receives writable temporary CALLEE argument copies;
// owner fov+1C4/aspect+1C8/near+1D4/far+1D8 are not its argument-slot storage.
void* __fastcall get_native_camera_projection_00b6fcf0(void* actual_camera);

// B70490[116]: flags+2F0 bit10; full view/projection product into cache+220.
void* __fastcall get_native_camera_view_projection_00b70490(void* actual_camera);

// B70510[61]: flags+2F0 bit20; full general inverse of VP into cache+260.
void* __fastcall get_native_camera_inverse_view_projection_00b70510(void* actual_camera);

// B70710[69]: publish flags+2F0 bit4 BEFORE obtaining VP/extracting planes;
// assign six planes with flags7 to+2F4; count+434 is untouched. EDX adds the
// existing fixed CRT/frustum context, retained across the full VP getter.
// The cache-hit path does not dereference the context. No error rollback.
void* __fastcall get_native_camera_frustum_00b70710(
    void* actual_camera, const NativeCameraFrustumContext* actual_context);

// Raw storage must cover all reached accesses (frustum record5 ends at+36C);
// retained records6..15/count and all other bytes are not initialized here.
// Native floating controls, faults and ordered current flag stores remain.
// These names are hypotheses; no original caller integration or gameplay claim.
} // namespace bsp
