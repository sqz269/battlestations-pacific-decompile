#pragma once
#include "bsp/camera_frame_state.hpp"
#include <cstddef>
#include <string>

namespace bsp {
// Borrow the actual matrix field of the service captured from00F8D39C at
//00B46A7B. No owned matrix, default service, or service-lifecycle replacement.
// This is a field projection, not the original service memory layout.
struct SystemCameraMatrixService {
    const CameraMatrix& matrix_1d8;
};

// Native ECX service, no stack arguments, EAX=ECX+1D8, plain RET.
// Returns the borrowed field identity; semantic matrix name remains provisional.
const CameraMatrix& get_system_camera_service_matrix_00b0d100(
    const SystemCameraMatrixService&);

// Interior00B46A84..00B46C4F of00B46A70; new C++ interface, not native ABI.
// Uses the actual frame/camera caches and the already captured service owner.
// live_words_0108fc30 must point to the sixteen actual consecutive global
// words0108FC30..0108FC6C. Their raw bits are copied without transposing.
// Every input remains live until its native read. No second camera/cache copy.
//
// Patch caller-initialized words only: c0..3,c6.xyz,c7..30. Capacity124 suffices;
// do not resize, clear, or supply native-unwritten padding. Check capacity at
// each matrix/eye group or individual global store, after the preceding getter
// and refresh. A bounds error is a new host failure: earlier writes and cache
// changes remain; the rejected group is untouched. Pointers must be valid for
// the stated capacity; a null output is permitted only with zero capacity.
// Matrix stores use sequential x87 FLD/FSTP; eye snapshots three raw words;
// globals perform sixteen ordered raw load/store pairs after all camera calls.
// Output must not overlap camera, service or global storage. Borrowed inputs
// must remain alive; the API provides no synchronization for concurrent edits.
bool pack_system_camera_constants_00b46a84(CameraFrameState&,
    const SystemCameraMatrixService& captured_service,
    const float* live_words_0108fc30,
    float* initialized_prefix_words, std::size_t word_count, std::string& error);
}
