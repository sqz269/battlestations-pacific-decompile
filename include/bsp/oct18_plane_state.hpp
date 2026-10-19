#pragma once
#include <cstdint>

namespace bsp {
// Required raw math entries, called from assembly so their ST0 results are not
// rounded through a C++ return temporary. These are dependency bindings, not new
// bodies for the already reconstructed helpers. See the report for register ABI.
using PlaneStateClampEntry = float (__fastcall*)(const float*,const float*,const float*);
using PlaneStateLengthSquaredEntry = float (__fastcall*)(const float*);
using PlaneStateWrappedAngleEntry = float (__stdcall*)(float,float);
struct RecoveredPlaneStateMath {
    PlaneStateClampEntry clamp_00415620;
    PlaneStateLengthSquaredEntry length_squared_00427e30;
    PlaneStateWrappedAngleEntry wrapped_angle_00438b10;
};

struct RecoveredPlaneStateCalls {
    void* context;
    bool (*local_role_00927f30)(void*,void* unit,std::uint32_t role);
    void (*enqueue_id_007cf380)(void*,void* queue,const std::uint32_t* id_bits);
    void* (*find_state_0077c6b0)(void*,void* unit,std::uint32_t id);
    void (*serialize_007c2af0)(void*,void* message,void* unit,void* found_state);
    // Existing cc9 physics range: dependency only, never replaced or wired here.
    void (*gravity_007d8180)(void*,void* controller,std::uint32_t force);
    void (*invalidate_0042ed50)(void*,void* node);
    void (*refresh_world_00414db0)(void*,void* unit);
    const float* (*velocity_slot34)(void*,void* unit,float* scratch);
    void (*angles_0042d2e0)(void*,const void* matrix,float* x,float* y,float* z);
    void (*set_state_007c78a0)(void*,void* unit,std::uint8_t state,std::uint8_t mode);
    void* (*identity_007bdf50)(void*,void* message,float* matrix);
    void* (*copy_state_007baaf0)(void*,void* destination,const void* source);
    void (*apply_controller_007db1f0)(void*,void* controller,const void* state);
    void (*update_007c18b0)(void*,void* unit,float dt);
    void (*fixed_steps_008746a0)(void*,void* embedded_sim,std::int32_t negative_ticks);
    void (*simulation_slot04)(void*,void* embedded_sim,float dt);
    bool (*controller_slot38)(void*,void* embedded_controller);
    const void* (*world_matrix_0042d7e0)(void*,void* unit);
    void (*restore_slot1dc)(void*,void* unit,float dt);
};

struct RecoveredPlaneStateAccess {
    const RecoveredPlaneStateCalls* calls;
    const RecoveredPlaneStateMath* math;
    void* volatile* game_00e188a8;
    void* received_ids_00f87284;
    const volatile std::uint32_t* clock_ticks_00f876b0;
    const volatile std::uint32_t* zero_vector_00f87574; // three current DWORDs
    const volatile float* restore_cutoff_00d7a218;
    const volatile float* one_00d7a24c;
    const volatile double* correction_offset_00d7a308;
    const volatile float* correction_distance_sq_00ce3d64;
};

// Whole 007D1360..007D1B4B source driver, new explicit dependency interface.
// Original __thiscall ECX plane, stack mutable message, RET4; no result.
// No new helper body or host wiring, including the cc9-owned physics interval.
void apply_recovered_plane_state_007d1360(void* plane,void* message,
                                         const RecoveredPlaneStateAccess&);
} // namespace bsp
