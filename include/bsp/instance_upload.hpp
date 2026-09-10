#pragma once
#include "bsp/camera_transform.hpp"
#include "bsp/d3d9_states.hpp"
#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace bsp {
struct GeneratedInstanceGeometry;
struct GeneratedInstanceSection;

// Borrowed projections of model virtual+50 (attach scene, recurse) and +48
// (cached world bounding sphere). The latter supplies the sphere CENTER, not
// necessarily the model's translation. Both callbacks are required; they must
// invoke the caller's actual scene/model operations. Bound object identities,
// generator declarations and source lists must remain stable during callbacks.
struct InstanceUploadModel {
    GeneratedInstanceGeometry* geometry{};
    void* context{};
    bool (*attach_scene)(void*, const void* scene, bool recurse, std::string&){};
    bool (*world_sphere_center)(void*, std::array<float, 3>&, std::string&){};
};

// Native entry fields +00..+24 in semantic storage. Every pointer is borrowed.
// Source entry/model/section and output entry addresses must remain stable until
// frame queues finish consuming them. This is not a native ABI/layout overlay.
struct InstanceRenderEntry {
    float leading_value{};
    GeneratedInstanceSection* section{};
    GeneratedInstanceGeometry* geometry{};
    InstanceUploadModel* model{};
    CameraTransform* camera{};
    float depth{};
    float visibility{};
    std::uint32_t flags{};
    std::uint64_t sort_key{}; // Constructor00b51a20 leaves this field unchanged.
};

// Actual generator virtual+8 and getter00b556b0 projection. The callback writes
// into the real mapped stream; declaration->stride is read after each call.
// Callback, context and declaration must stay valid throughout the upload.
struct InstanceUploadGenerator {
    const VertexDeclaration* declaration{};
    void* context{};
    bool (*write_record)(void*, const InstanceRenderEntry&, std::uint8_t* output,
        std::size_t available_bytes, std::string&){};
};

struct InstanceUploadCategory {
    std::uint32_t instance_count{}; // Native group+0C/+10.
    InstanceRenderEntry* output_entry{}; // Native group+14/+18; caller owns it.
    InstanceUploadModel* generated_model{}; // Native group+1C/+20.
    std::vector<InstanceRenderEntry*> source_entries; // Native +24/+30 lists.
};
struct InstanceUploadGroup {
    InstanceUploadGenerator* generator{}; // Native [[group+0]+0C].
    std::array<InstanceUploadCategory, 2> categories;
};

// Owns pointer storage only. The native queue +0C/+10/+14 is a borrowed entry
// array, count, capacity. clear() is explicit host frame lifecycle, not a port
// of an unreviewed native reset method. It retains allocated capacity.
class InstanceRenderQueue {
public:
    const std::vector<InstanceRenderEntry*>& entries() const { return entries_; }
    std::uint32_t capacity() const { return capacity_; }
    void clear() noexcept { entries_.clear(); }
private:
    friend bool append_instance_render_entry_00b51cb0(InstanceRenderQueue&,
        InstanceRenderEntry&, std::string&);
    std::vector<InstanceRenderEntry*> entries_;
    std::uint32_t capacity_{};
};

struct InstanceUploadContext {
    std::vector<InstanceUploadGroup*> groups; // Native context+38/+3C.
    const void* scene_binding{}; // Actual getter [[context+4]+1C], may be null.
    CameraTransform* camera{}; // Actual camera [[context+28]+8].
    // Native pointer array beginning at context+0C. Category1 selects index1;
    // category0 selects section.material_queue_index, including index1.
    std::vector<InstanceRenderQueue*> queues;
};
struct InstanceUploadStats {
    std::size_t groups_visited{}, categories_uploaded{}, records_written{}, entries_queued{};
};

// Original ECX section, stack count, RET4: unconditional +1C DWORD store.
void set_generated_section_instance_count_00b85590(GeneratedInstanceSection&,
    std::uint32_t count) noexcept;

// Original ECX entry, eight stack words, RET20h. Positive depth_override skips
// the camera/sphere getter; zero, negative and unordered use view-space sphere
// center Z +section.depth_bias. x87 arithmetic/store order is retained on Win32.
// Earlier field stores survive a failed model callback; sort_key is untouched.
bool initialize_instance_render_entry_00b51a20(InstanceRenderEntry&,
    float leading_value, GeneratedInstanceSection&, GeneratedInstanceGeometry&,
    InstanceUploadModel&, CameraTransform&, float visibility, float depth_override,
    std::uint32_t flags, std::string& error);

// Original ECX batch, stack borrowed entry pointer, RET4. Growth is max(256,
// 2*capacity), then pointer append. C++ allocation failures propagate; signed
// overflow/malformed native storage is rejected. No entry retain or copying.
bool append_instance_render_entry_00b51cb0(InstanceRenderQueue&,
    InstanceRenderEntry&, std::string& error);

// Original ECX context, RET, no stack args. Groups in list order; category0 then
// category1. Empty counts skip everything. Source counts must match, pointers
// and generator stride must be valid/stable, and callbacks cannot mutate lists.
// Category1 uses the recovered material/depth predicate: <=32 entries preserve
// native insertion order for ties; larger lists require distinct finite keys
// (the native partition/heapsort tie permutation remains outside this port).
// Each category attaches(scene,false), sorts, locks stream1(count,0,false),
// dispatches generator writes/stride advances, unlocks, sets section count,
// initializes its retained output entry (0,visibility1/.5,depth0,flags555h), and
// appends its actual pointer to the selected queue. Failure is not atomic:
// previous categories remain queued, mapped streams are unlocked, and already
// written bytes/callback state remain. Native ignores lock/generator failures.
HRESULT upload_instance_groups_00b1e990_fragment(D3D9StateCache&,
    InstanceUploadContext&, InstanceUploadStats&, std::string& error);
}
