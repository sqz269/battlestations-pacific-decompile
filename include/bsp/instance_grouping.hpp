#pragma once
#include "bsp/instance_upload.hpp"
#include <memory>

namespace bsp {
// Projected input to native00b73770: the caller resolves object+50's optional
// index to object+4+16*index. Default threshold is the native 0x3f7eb852.
float compute_stream_threshold_fade_00b73770(float input, float fraction,
    float threshold = 0.995f) noexcept;

struct InstanceVisibilityInputs {
    bool enabled{}; // Effect descriptor+16, not an inferred transparency flag.
    std::array<float, 3> camera_world_position{}, model_world_position{};
    std::uint32_t camera_mode{}; // Camera+198, shift count masked to five bits.
    float camera_distance_limit{}; // Camera+178; modes selected by mask0x17.
    float descriptor_fraction{}; // Effect descriptor+18.
    float stream_threshold{0.995f};
};
// Positions must already have been refreshed through native world-matrix
// validity rules. Writes entry.visibility even if the result is rejected.
// This finite-position port retains native x87 float stores and comparisons;
// native CRT sqrt domain/error handling and world refresh remain separate.
bool apply_instance_visibility_00b1dff0_fragment(InstanceRenderEntry&,
    const InstanceVisibilityInputs&, bool& rejected, std::string& error);

// Native FISTP qword under temporary truncate rounding, low DWORD min(1).
// Result0 selects full-opacity category0; result1 selects faded category1.
std::uint32_t instance_category_00b1e1c7(float visibility) noexcept;

struct InstanceGroupingBinding {
    std::uint32_t id{}; // Native section+5C -> binding+8, signed valid index.
    std::shared_ptr<InstanceUploadGenerator> generator;
};
struct OwnedInstanceGroup {
    std::shared_ptr<const InstanceGroupingBinding> binding;
    InstanceUploadGroup upload;
    std::array<InstanceRenderEntry, 2> output_entries;
    std::array<std::shared_ptr<GeneratedInstanceGeometry>, 2> geometries;
    // Callback contexts are destroyed before their borrowed geometry targets.
    std::array<InstanceUploadModel, 2> models;
};

// Owns stable host group/output addresses. Native uses its 4Ch group allocator
// and global InterlockedIncrement entry pool; those allocators are not overlaid.
// Entries, binding callbacks, scene and queues must outlive consumption. A
// failed factory leaves the published group visible, as native has no rollback;
// abandon that frame after error instead of attempting to resume the group.
class InstanceGroupingState {
public:
    std::vector<std::unique_ptr<OwnedInstanceGroup>> by_binding;
    std::vector<InstanceUploadGroup*> ordered_groups;
};
class InstanceGroupFactory {
public:
    virtual ~InstanceGroupFactory() = default;
    // Implement the actual generated geometry/model service. category0 then1;
    // tint is the native mutable diffuse write (id%5), alpha always one.
    // The geometry retains its real cloned material; model.context_owner must
    // retain any nonnull callback context after the factory itself is destroyed.
    virtual bool create(const InstanceRenderEntry& source,
        const InstanceGroupingBinding&, std::uint32_t category,
        const std::array<float, 4>& tint, InstanceUploadModel&,
        std::shared_ptr<GeneratedInstanceGeometry>&, std::string& error) = 0;
};
enum class InstanceGroupingResult { rejected, queued_directly, grouped };

// Typed control/data fragment of00b1dff0. Applies visibility, routes entries
// without a binding to the material queue (or queue1 when faded), otherwise
// creates two categories once per binding ID and appends borrowed source entry
// pointers in encounter order. Group diagnostic strings, exact native pool
// allocation and intrusive reference ABI are outside this interface.
bool group_instance_render_entry_00b1dff0_fragment(InstanceGroupingState&,
    InstanceRenderEntry&, const InstanceVisibilityInputs&,
    std::shared_ptr<const InstanceGroupingBinding>, InstanceGroupFactory&,
    const std::vector<InstanceRenderQueue*>& queues, InstanceGroupingResult&,
    std::string& error);
}
