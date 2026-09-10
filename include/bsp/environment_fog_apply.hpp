#pragma once
#include "bsp/system_fog_owner.hpp"
#include <array>
#include <string>

namespace bsp {
// Borrowed regions of the ACTUAL environment object, with native offsets.
// No environment constructor, owner, default values or copied field snapshot.
// References and the referenced storage remain valid throughout the call.
struct EnvironmentFogFields {
    const std::array<float, 11>& scalars_08; // native08..30, read in setter order
    const std::array<float, 4>& color_34;
    const std::array<std::array<float, 4>, 4>& directional_colors_44;
    const std::array<float, 4>& underwater_color_84;
};

// Interior[0078D076,0078D180) of0078CFF0; native EDI=environment, ESI=camera.
// New host ABI, not a native entrypoint. Pass the SAME live camera.fog_184 slot
// by reference. Every nonnull view must originate in a live SystemFogOwner;
// no standalone SystemFogState, alternate fog domain or default owner is valid.
// Native does not retain during these calls; owners must survive their writes.
//
// Writes color, four directionals, six scalars, underwater color, five scalars.
// Every setter reloads the camera owner. Scalar updates keep FLD(source), owner
// reload, FSTP(argument) in that order; color setters preserve raw forward words.
// No scaling/clamping/normalization or camera mode gate occurs in this interior.
// Returns false on a null live owner, retaining prior writes and x87 effects;
// this checked host error is not a native null-owner fallback. Original code
// would dereference null. Environment values remain live through each read.
bool apply_environment_fog_0078d076(const EnvironmentFogFields&,
    const SystemFogState* const& actual_camera_fog_184, std::string& error);
}
