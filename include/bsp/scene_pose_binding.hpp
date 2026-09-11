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

} // namespace bsp
