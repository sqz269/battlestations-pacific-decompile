#include "bsp/instance_upload.hpp"
#include "bsp/instance_geometry.hpp"
#include "bsp/render_sort.hpp"
#include <algorithm>
#include <cmath>
#include <limits>

namespace bsp {
namespace {
// Value/kernel projection of004142e0; no original object-layout assumptions.
// Preserve all three outputs and the original x87 product/add/spill order even
// though00b51a20 consumes only Z. Native input XYZ is staged before any writes.
void transform_point_004142e0(const std::array<float, 3>& point,
    const CameraMatrix& matrix, std::array<float, 3>& result) {
    const float* source = point.data();
    const float* transform = matrix.data();
    float* output = result.data();
    float values[3];
    __asm {
        mov ecx, source
        fld dword ptr [ecx + 4]
        fstp dword ptr values[0]
        fld dword ptr [ecx]
        fstp dword ptr values[4]
        fld dword ptr [ecx + 8]
        fstp dword ptr values[8]
        mov ecx, transform
        mov eax, output
        fld dword ptr [ecx + 16]
        fld dword ptr values[0]
        fld st(0)
        fmulp st(2), st(0)
        fld dword ptr [ecx]
        fld dword ptr values[4]
        fld st(0)
        fmulp st(2), st(0)
        fxch st(3)
        faddp st(1), st(0)
        fld dword ptr [ecx + 32]
        fld dword ptr values[8]
        fld st(0)
        fmulp st(2), st(0)
        fxch st(2)
        faddp st(1), st(0)
        fadd dword ptr [ecx + 48]
        fstp dword ptr [eax]
        fld dword ptr [ecx + 4]
        fmul st(0), st(3)
        fld dword ptr [ecx + 20]
        fmul st(0), st(3)
        faddp st(1), st(0)
        fld dword ptr [ecx + 36]
        fmul st(0), st(2)
        faddp st(1), st(0)
        fadd dword ptr [ecx + 52]
        fstp dword ptr [eax + 4]
        fld dword ptr [ecx + 8]
        fmulp st(3), st(0)
        fld dword ptr [ecx + 24]
        fmulp st(2), st(0)
        fxch st(2)
        faddp st(1), st(0)
        fld dword ptr [ecx + 40]
        fmulp st(2), st(0)
        faddp st(1), st(0)
        fadd dword ptr [ecx + 56]
        fstp dword ptr [eax + 8]
    }
}

bool entry_less(const InstanceRenderEntry* left, const InstanceRenderEntry* right) {
    return render_entry_material_value_less_00b51ab0(
        {left->sort_key, left->section->material_order, left->depth},
        {right->sort_key, right->section->material_order, right->depth});
}

bool sort_category_one(std::vector<InstanceRenderEntry*>& entries, std::string& error) {
    for (const auto* entry : entries) {
        if (!entry || !entry->section || !std::isfinite(entry->depth)) {
            error = "Instance category1 requires valid entries and finite sort depths";
            return false;
        }
    }
    if (entries.size() <= 32) {
        //00b1dce0 selects00b1d420 at<=32. Its strict comparisons and single
        // element00b1c210 rotations insert before the first larger item while
        // retaining the input order of equivalent material/depth records.
        for (std::size_t at = 1; at < entries.size(); ++at) {
            auto* value = entries[at];
            auto destination = at;
            while (destination && entry_less(value, entries[destination - 1])) {
                entries[destination] = entries[destination - 1];
                --destination;
            }
            entries[destination] = value;
        }
        return true;
    }
    // Distinct finite keys have one possible sorted permutation, independent
    // of the unported native partition/heapsort strategy. Ties are rejected
    // before replacing the original list; no native tie ordering is invented.
    auto sorted = entries;
    std::sort(sorted.begin(), sorted.end(), entry_less);
    for (std::size_t at = 1; at < sorted.size(); ++at) {
        if (!entry_less(sorted[at - 1], sorted[at])) {
            error = "Native category1 tie permutation above32 entries is not reconstructed";
            return false;
        }
    }
    entries.swap(sorted);
    return true;
}

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
            if (category_index && !sort_category_one(category.source_entries, error))
                return D3DERR_INVALIDCALL;

            void* mapped = nullptr;
            const auto lock_result = states.lock_vertex_stream_00b49980(*stream,
                category.instance_count, 0, false, mapped);
            if (FAILED(lock_result)) return lock_result;
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
