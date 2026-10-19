#pragma once
#include "bsp/directional_light_reference.hpp"
#include "bsp/lighting_configuration_apply.hpp"
#include "bsp/mission_camera.hpp"
#include "bsp/native_gui_scene_fields.hpp"
#include "bsp/native_invalid_parameter_noinfo_call.hpp"
#include "bsp/native_scene_registration_gates.hpp"
#include "bsp/native_scene_resource_storage.hpp"

// Private application composition is in game_native_mission_lighting.inc.
// Its providers borrow the renderer's SAME actual owners, pools, node/type
// domain and raw strings. No alternate lighting constants are uploaded.
