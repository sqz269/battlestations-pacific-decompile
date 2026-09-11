#pragma once
#include "bsp/point_effect_instance.hpp"

namespace bsp {

// Concrete native node virtual+34: D62CBC contains B6E870. Runs the canonical
// world setter against this SAME actual backing and dispatches current +40
// through the existing scene binding, reloading it after the first callback.
// Caller explicitly binds this to scene_attachment.set_world_matrix for native
// c3dNode. Other node types require their actual recovered +34 implementation.
void set_native_node_world_matrix_00b6e870(SceneAttachmentRuntime&,
    SceneNodeAttachment&, const CameraMatrix&);

// Native ECX=effect, stack matrix*, RET4. Executes current node+110 virtual+34
// FIRST with the original source reference, then reloads effect parent+8C.
// With a parent stores source * current parent inverse-world into relative+D0;
// otherwise performs the canonical x87 copy. Never snapshots source across
// the virtual call: source can alias fields that callback changes.
void set_point_effect_world_matrix_0053d9c0(PointEffectInstanceStorage&,
    const CameraMatrix& source, SceneAttachmentRuntime&);

// Native ECX=effect, stack matrix*, RET4. First x87-copies source into+D0,
// captures parent, conditionally refreshes its world, then invokes node+110
// virtual+34 with relative * parent-world (or actual+D0 for a null parent).
// No copy is made of+D0 before its native operation. Actual node/scene bindings
// and input/parent storage must survive callbacks; concurrent mutation is not
// supported. These are new C++ interfaces, not drop-in native replacements.
void set_point_effect_relative_matrix_0072aa80(PointEffectInstanceStorage&,
    const CameraMatrix& source, SceneAttachmentRuntime&);

} // namespace bsp
