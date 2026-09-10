#pragma once
#include "bsp/system_lighting_owners.hpp"

namespace bsp {
// A borrowed view of the SAME light's node and native +178/+17C/+180 array.
// The array's elements each own a scene reference; the reused array helpers
// themselves neither retain nor release. This is distinct from node.scene+170.
struct LightSceneRetention {
    SceneNodeAttachment& node;
    SystemAmbientBacklinks& scenes_178;
};

// Native ECX light; stack scene/recurse byte; RET8. Valid array storage and
// live bindings are required. Attach requires a nonnull scene. Duplicate
// attach and absent detach still recurse through current child virtuals.
void attach_light_scene_00b7c020(SceneAttachmentRuntime&, LightSceneRetention&,
    SceneResource* requested, bool recurse);
void remove_light_scene_00b7bd60(SceneAttachmentRuntime&, LightSceneRetention&,
    SceneResource* requested, bool recurse);

// Concrete adapters for SceneNodeAttachment's actual current virtual50/54.
// The caller explicitly binds node.context to its stable LightSceneRetention.
// Native node-base destruction switches these callbacks to the node versions;
// these functions do not infer a vtable phase or create an owning light.
void dispatch_light_scene_attach_00b7c020(SceneAttachmentRuntime&,
    SceneNodeAttachment&, SceneResource*, bool recurse);
void dispatch_light_scene_remove_00b7bd60(SceneAttachmentRuntime&,
    SceneNodeAttachment&, SceneResource*, bool recurse);
}
