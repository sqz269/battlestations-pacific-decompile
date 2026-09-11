#pragma once

#include "bsp/pose_refresh.hpp"

namespace bsp {

// Interior fragment of 0046C550: native EBP is the captured nonnull parent
// argument; X/Z are float locals at ESP+1Ch/+24h. Resolve only that owner to
// its existing pose, refresh its actual C8/worldCC if dirty, then add world
// indices12/14 in native x87 load/add/store order. Does not retain any owner.
// X/Z represent caller locals, distinct from pose/hierarchy/flag storage.
void add_scene_parent_world_offset_0046c6b7(void* captured_parent_identity,
    PoseRefreshResolver&, float& x, float& z);

// Interior 0046C6E5..0046C70E, reached only with a null parent identity.
// Local is the captured live argument4; parent_argument is the gate's inline
// by-value matrix snapshot. The product has distinct scratch storage. X/Z are
// distinct caller locals, outside either input matrix, written in native order.
void compose_scene_null_parent_offset_0046c6e5(const CameraMatrix& local,
    const CameraMatrix& parent_argument, float& x, float& z);

} // namespace bsp
