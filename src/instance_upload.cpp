#include "bsp/instance_upload.hpp"
#include "bsp/camera_affine.hpp"
#include "bsp/instance_geometry.hpp"
#include "bsp/instance_sort.hpp"
#include <algorithm>
#include <cmath>
#include <limits>

namespace bsp {
namespace {

struct StreamUnlock {
    D3D9StateCache& states;
    LogicalVertexStream& stream;
    bool active{true};
    ~StreamUnlock() { if (active) states.unlock_vertex_stream_00b49a80(stream); }
    void unlock() {
        states.unlock_vertex_stream_00b49a80(stream);
        active = false;
    }
};
}

void set_generated_section_instance_count_00b85590(GeneratedInstanceSection& section,
    std::uint32_t count) noexcept {
    section.instance_count = count;
}

bool initialize_instance_render_entry_00b51a20(InstanceRenderEntry& entry,
    float leading_value, GeneratedInstanceSection& section, GeneratedInstanceGeometry& geometry,
    InstanceUploadModel& model, CameraTransform& camera, float visibility, float depth_override,
    std::uint32_t flags, std::string& error) {
    entry.leading_value = leading_value;
    entry.visibility = visibility;
    entry.section = &section;
    entry.geometry = &geometry;
    entry.model = &model;
    entry.camera = &camera;
    entry.flags = flags;
    if (depth_override > 0.0f) {
        entry.depth = depth_override;
        return true;
    }
    if (!model.world_sphere_center) {
        error = "Render entry requires the actual model world-sphere callback";
        return false;
    }
    const auto& view = get_camera_view_00b6fcb0(camera);
    std::array<float, 3> world_center, view_center;
    if (!model.world_sphere_center(model.context, world_center, error)) {
        if (error.empty()) error = "Model world-sphere callback failed";
        return false;
    }
    transform_point_004142e0(world_center, view, view_center);
    const float bias = section.depth_bias;
    const float view_z = view_center[2];
    float depth;
    __asm {
        fld dword ptr bias
        fadd dword ptr view_z
        fstp dword ptr depth
    }
    entry.depth = depth;
    return true;
}

bool append_instance_render_entry_00b51cb0(InstanceRenderQueue& queue,
    InstanceRenderEntry& entry, std::string& error) {
    const auto maximum = static_cast<std::uint32_t>(std::numeric_limits<std::int32_t>::max());
    if (queue.entries_.size() > queue.capacity_ || queue.entries_.size() >= maximum) {
        error = "Render queue count exceeds the native valid signed-container domain";
        return false;
    }
    if (queue.entries_.size() == queue.capacity_) {
        if (queue.capacity_ > maximum / 2) {
            error = "Render queue capacity doubling would overflow the native signed domain";
            return false;
        }
        const auto requested = std::max<std::uint32_t>(256, queue.capacity_ * 2);
        if (requested > std::numeric_limits<std::uint32_t>::max() / 4) {
            error = "Render queue pointer allocation exceeds the native32-bit byte domain";
            return false;
        }
        queue.entries_.reserve(requested);
        queue.capacity_ = requested;
    }
    queue.entries_.push_back(&entry);
    return true;
}

HRESULT upload_instance_groups_00b1e990_fragment(D3D9StateCache& states,
    InstanceUploadContext& context, InstanceUploadStats& stats, std::string& error) {
    stats = {};
    error.clear();
    const auto maximum = static_cast<std::size_t>(std::numeric_limits<std::int32_t>::max());
    if (context.groups.size() > maximum) {
        error = "Instance group count exceeds the valid native signed-container domain";
        return D3DERR_INVALIDCALL;
    }
    for (auto* group : context.groups) {
        if (!group) {
            error = "Instance group pointer is null";
            return D3DERR_INVALIDCALL;
        }
        ++stats.groups_visited;
        for (std::uint32_t category_index = 0; category_index < 2; ++category_index) {
            auto& category = group->categories[category_index];
            if (!category.instance_count) continue;
            auto* model = category.generated_model;
            auto* generator = group->generator;
            if (!context.camera || !model || !model->geometry || !model->attach_scene
                || !model->world_sphere_center || !category.output_entry || !generator
                || !generator->declaration || !generator->write_record
                || category.source_entries.size() != category.instance_count
                || category.source_entries.size() > maximum) {
                error = "Active instance category has incomplete bindings or inconsistent counts";
                return D3DERR_INVALIDCALL;
            }
            auto& geometry = *model->geometry;
            auto& section = geometry.section;
            const auto stream = geometry.instance_stream;
            const auto stride = generator->declaration->stride;
            if (!stream || !stream->declaration || !stream->physical || !stride
                || stream->declaration->stride != stride
                || category.instance_count > std::numeric_limits<std::uint32_t>::max() / stride) {
                error = "Generated stream1 and generator declaration have invalid byte bounds";
                return D3DERR_INVALIDCALL;
            }
            const auto bytes = category.instance_count * stride;
            if (stream->physical->cursor > stream->physical->capacity
                || bytes > stream->physical->capacity - stream->physical->cursor) {
                error = "Instance upload exceeds the remaining shared vertex capacity";
                return D3DERR_INVALIDCALL;
            }
            for (auto* entry : category.source_entries) {
                if (!entry || entry == category.output_entry) {
                    error = "Instance source entries must be valid and distinct from the output entry";
                    return D3DERR_INVALIDCALL;
                }
            }
            //00b72110 is RET: the earlier PUSH0 survives for virtual+50's
            // second argument. This is attach(scene,false), not getter(0).
            if (!model->attach_scene(model->context, context.scene_binding, false, error)) {
                if (error.empty()) error = "Generated model scene attachment failed";
                return E_FAIL;
            }
            if (category_index && !sort_instance_entries_00b1dce0(category.source_entries, error))
                return D3DERR_INVALIDCALL;

            void* mapped = nullptr;
            const auto lock_depth_before = stream->physical->lock_depth;
            const auto lock_result = states.lock_vertex_stream_00b49980(*stream,
                category.instance_count, 0, false, mapped);
            if (FAILED(lock_result)) {
                // The physical helper increments depth after an attempted COM
                // Lock even on failure. Preflight failures do not increment it.
                // Balance only the attempted pair; preserve native cursor and
                // dynamic-lock counters instead of inventing a rewind.
                if (stream->physical->lock_depth != lock_depth_before)
                    states.unlock_vertex_stream_00b49a80(*stream);
                return lock_result;
            }
            StreamUnlock unlock{states, *stream};
            if (!mapped) {
                error = "Generated instance stream lock returned no mapped address";
                return E_FAIL;
            }
            auto* destination = static_cast<std::uint8_t*>(mapped);
            std::size_t remaining = static_cast<std::size_t>(category.instance_count) * stride;
            for (auto* entry : category.source_entries) {
                if (!generator->write_record(generator->context, *entry, destination, remaining, error)) {
                    if (error.empty()) error = "Instance generator callback failed";
                    return E_FAIL;
                }
                ++stats.records_written;
                // Native re-reads generator declaration after each callback.
                const auto advance = generator->declaration->stride;
                if (advance != stride || advance > remaining) {
                    error = "Instance generator changed the declaration stride during upload";
                    return D3DERR_INVALIDCALL;
                }
                destination += advance;
                remaining -= advance;
            }
            unlock.unlock();
            ++stats.categories_uploaded;
            set_generated_section_instance_count_00b85590(section, category.instance_count);
            if (!initialize_instance_render_entry_00b51a20(*category.output_entry, 0.0f,
                section, geometry, *model, *context.camera, category_index ? 0.5f : 1.0f,
                0.0f, 0x555u, error)) return E_FAIL;
            const auto queue_index = category_index ? 1u : section.material_queue_index;
            if (queue_index >= context.queues.size() || !context.queues[queue_index]) {
                error = "Native material/category queue selection has no bound destination";
                return D3DERR_INVALIDCALL;
            }
            if (!append_instance_render_entry_00b51cb0(*context.queues[queue_index],
                *category.output_entry, error)) return D3DERR_INVALIDCALL;
            ++stats.entries_queued;
        }
    }
    return S_OK;
}
}
