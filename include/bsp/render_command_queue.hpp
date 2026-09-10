#pragma once
#include "bsp/instance_grouping.hpp"
#include <atomic>
#include <memory_resource>

namespace bsp {
struct RenderCommand;

// Host intrusive ownership, not a native vtable/layout overlay. A subclass owns
// the actual camera/scene/target/batch object and implements its terminal release.
// The callback can observe/reenter the queue; it must not throw. New references
// start at one, as the native context constructor does.
class RenderCommandReference {
public:
    std::atomic<std::int32_t> reference_count{1};
    virtual void release_zero_references() noexcept = 0;
    virtual ~RenderCommandReference() = default;
    RenderCommandReference() = default;
    RenderCommandReference(const RenderCommandReference&) = delete;
    RenderCommandReference& operator=(const RenderCommandReference&) = delete;
};
void retain_render_command_reference(RenderCommandReference&) noexcept;
void release_render_command_reference(RenderCommandReference&) noexcept;
// Publish new pointer, retain it, release old pointer. Identity does nothing.
void assign_render_command_reference(RenderCommandReference*&,
    RenderCommandReference*) noexcept;

// Real typed queue storage. Releasing its final reference destroys the pointer
// array but does not destroy borrowed InstanceRenderEntry objects.
class RenderCommandBatch final : public RenderCommandReference {
public:
    std::uint32_t preparation_mode{}; // native batch+08
    InstanceRenderQueue entries; // native batch+0C/count+10/capacity+14
    void release_zero_references() noexcept override;
};

class RenderCommandContext final : public RenderCommandReference {
public:
    RenderCommandReference* camera{}; // native+08, actual camera owner/subclass
    RenderCommandReference* scene{}; // +0C, actual scene/scene-settings owner
    RenderCommand* command{}; // +10, borrowed; does not retain the command
    RenderCommandReference* target{}; // +14, actual target owner/subclass
    void release_zero_references() noexcept override;
};
// Native ECX context; RET. Release then clear fields+08,+0C,+14, reloading each
// after previous callbacks. The backpointer is not cleared by the native body.
void destroy_render_command_context_00b1d120(RenderCommandContext&) noexcept;

// The allocator must outlive its allocated bytes. This uses an actual host
// memory_resource pool and returns exactly length+1 bytes with alignment1; it
// does not claim the native global size-class pool's physical allocation ABI.
class RenderCommandDiagnosticString {
public:
    ~RenderCommandDiagnosticString();
    RenderCommandDiagnosticString() = default;
    RenderCommandDiagnosticString(const RenderCommandDiagnosticString&) = delete;
    RenderCommandDiagnosticString& operator=(const RenderCommandDiagnosticString&) = delete;
    void assign(const char* text, std::uint32_t length, std::pmr::memory_resource&);
    void return_storage() noexcept;
    const char* data() const noexcept { return data_; }
    std::uint32_t size() const noexcept { return length_; }
private:
    char* data_{};
    std::uint32_t length_{};
    std::pmr::memory_resource* pool_{};
};

struct RenderNodeRootList { CameraTransform* first{}; }; // native root+0C
// Native ECX root, stack node, RET4. Relinks neighbours/root head only: does not
// clear node sibling/root fields or release a reference.
void unlink_render_root_node_00b72220(RenderNodeRootList&, CameraTransform&) noexcept;

// Required actual model virtual+18 operation, resolved to00B6F310 for generated
// model vtable00D62DE8. It releases point-light links/children and the model owner;
// it is not replaceable with a no-op. The caller supplies the real typed model
// owner implementation; this interface does not invent its native layout.
class RenderCommandModelLifetime {
public:
    virtual ~RenderCommandModelLifetime() = default;
    virtual CameraTransform& transform() noexcept = 0;
    virtual void release_model_virtual18_00b6f310() noexcept = 0;
};
// Native ECX model; tail jump virtual+18. Unlinks shared CameraTransform parent,
// previous/next siblings and parent head/count, or root registration; only then
// invokes the required real model operation. No separate hierarchy is stored.
void unlink_and_release_render_model_00b6dfa0(RenderCommandModelLifetime&) noexcept;

// Resolves actual model identities in OwnedInstanceGroup, not a do-everything
// command/group destructor callback. Every live generated model needs a binding.
class RenderCommandModelLifetimes {
public:
    virtual ~RenderCommandModelLifetimes() = default;
    virtual RenderCommandModelLifetime& for_model(InstanceUploadModel&) noexcept = 0;
};

// Additional host ownership for the native group's diagnostic string. Group
// identity is stable until its command is disposed. Missing metadata means the
// original diagnostic string is empty; it never suppresses model destruction.
struct RenderCommandGroupDiagnostic {
    OwnedInstanceGroup* group{};
    std::unique_ptr<RenderCommandDiagnosticString> text;
};

struct RenderCommand {
    explicit RenderCommand(RenderCommandModelLifetimes& models) noexcept : model_lifetimes(models) {}
    RenderCommand(const RenderCommand&) = delete;
    RenderCommand& operator=(const RenderCommand&) = delete;
    RenderCommandReference* scene{}; // native command+04
    std::array<RenderCommandBatch*, 2> batches{}; // +0C,+10; required before execute/dispose
    RenderCommandDiagnosticString diagnostic; // +14 length,+18 storage
    RenderCommandContext* context{}; // +28, one owned reference, nullable
    InstanceGroupingState grouping; // native +2C by-binding and+38 ordered groups
    std::vector<RenderCommandGroupDiagnostic> group_diagnostics;
    RenderCommandModelLifetimes& model_lifetimes;
};

// Typed successful construction path of00B1EDC0's ownership operations. The
// caller supplies two actual acquired batch objects with one reference each;
// this consumes those references. Command must be fresh with context/batches
// null. Camera/scene/target retain independently; command also retains scene.
// Pool/job-service acquisition itself remains outside this function.
void initialize_render_command_context_00b1edc0_fragment(RenderCommand&,
    RenderCommandReference* command_scene, RenderCommandReference* camera,
    RenderCommandReference* context_scene, RenderCommandReference* target,
    RenderCommandBatch& batch0, RenderCommandBatch& batch1);

// Native00B1D760: release binding, model0 then model1, free borrowed source
// pointer arrays in reverse order, return diagnostic storage. The group is
// freed separately by its command owner. Native EH and allocator corruption
// paths are excluded; callbacks must not resize/reorder by_binding during
// destruction or inspect a shared_ptr while its terminal deleter is running.
void destroy_render_command_group_00b1d760(OwnedInstanceGroup&,
    RenderCommandModelLifetimes&, RenderCommandDiagnosticString*) noexcept;
// Native ECX command; RET. Releases two batch refs (required), disposes groups,
// releases/clears scene then context, frees ordered-group then by-binding arrays,
// and returns diagnostic storage. Does not free the command itself.
void destroy_render_command_00b1ddd0(RenderCommand&) noexcept;

// Native data/count/capacity are three DWORDs. This typed owner uses the same
// requested capacity and count rules, with new[] allocation and overflow checks.
// It owns pointer storage only; deleting/shrinking it never deletes commands.
class RenderCommandPointerArray {
public:
    RenderCommand*& operator[](std::size_t index) noexcept { return data_[index]; }
    RenderCommand* const& operator[](std::size_t index) const noexcept { return data_[index]; }
    std::uint32_t size() const noexcept { return count_; }
    std::uint32_t capacity() const noexcept { return capacity_; }
private:
    friend void reserve_render_command_pointers_00b1c6c0(RenderCommandPointerArray&, std::int32_t);
    friend void resize_render_command_pointers_00b1cc80(RenderCommandPointerArray&, std::int32_t);
    std::unique_ptr<RenderCommand*[]> data_;
    std::uint32_t count_{}, capacity_{};
};
void reserve_render_command_pointers_00b1c6c0(RenderCommandPointerArray&, std::int32_t requested);
void resize_render_command_pointers_00b1cc80(RenderCommandPointerArray&, std::int32_t count);

struct RenderCommandQueue {
    RenderCommandPointerArray commands; // native+14/+18/+1C
    std::uint32_t control{}; // +20; zero disposes executed commands and clears list
    RenderCommandContext* current_context{}; // +30; owns one reference if nonnull
};
class RenderCommandExecutor {
public:
    virtual ~RenderCommandExecutor() = default;
    virtual void execute(RenderCommand&) noexcept = 0; // actual command virtual+00
};

// Native ECX queue; RET. List/count/current-context/control are reloaded at the
// observed callback boundaries. Clear occurs AFTER current-context release,
// even if its terminal callback assigned a replacement. Execute must not throw.
// Reentry/mutation must preserve each currently addressed list slot; the native
// function also cannot safely consume a freed/reordered active command. Commands
// disposed in control0 must be heap allocations compatible with delete. A mode
// switch after disposal can retain stale pointers just as native; owner must
// ensure the resulting list is valid before another execution.
void execute_render_command_queue_00b1ebe0(RenderCommandQueue&, RenderCommandExecutor&) noexcept;
}
