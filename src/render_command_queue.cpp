#include "bsp/render_command_queue.hpp"
#include <cstring>
#include <limits>
#include <stdexcept>

namespace bsp {
namespace {
template<class Reference>
void release_then_clear(Reference*& field) noexcept {
    Reference* old = field;
    if (old) {
        release_render_command_reference(*old);
        field = nullptr; // deliberately after the terminal callback
    }
}
RenderCommandDiagnosticString* diagnostic_for(RenderCommand& command,
    const OwnedInstanceGroup& group) noexcept {
    for (auto& diagnostic : command.group_diagnostics)
        if (diagnostic.group == &group) return diagnostic.text.get();
    return nullptr;
}
}

void retain_render_command_reference(RenderCommandReference& reference) noexcept {
    reference.reference_count.fetch_add(1, std::memory_order_seq_cst);
}
void release_render_command_reference(RenderCommandReference& reference) noexcept {
    if (reference.reference_count.fetch_sub(1, std::memory_order_seq_cst) == 1)
        reference.release_zero_references();
}
void assign_render_command_reference(RenderCommandReference*& field,
    RenderCommandReference* incoming) noexcept {
    RenderCommandReference* old = field;
    if (old != incoming) {
        field = incoming;
        if (incoming) retain_render_command_reference(*incoming);
        if (old) release_render_command_reference(*old);
    }
}

void RenderCommandBatch::release_zero_references() noexcept { delete this; }
void destroy_render_command_context_00b1d120(RenderCommandContext& context) noexcept {
    release_then_clear(context.camera);
    release_then_clear(context.scene);
    release_then_clear(context.target);
}
void RenderCommandContext::release_zero_references() noexcept {
    destroy_render_command_context_00b1d120(*this);
    delete this;
}

RenderCommandDiagnosticString::~RenderCommandDiagnosticString() { return_storage(); }
void RenderCommandDiagnosticString::assign(const char* text, std::uint32_t length,
    std::pmr::memory_resource& pool) {
    if (length == (std::numeric_limits<std::uint32_t>::max)() || (!text && length))
        throw std::invalid_argument("Invalid render diagnostic string extent");
    auto* replacement = static_cast<char*>(pool.allocate(static_cast<std::size_t>(length) + 1, 1));
    if (length) std::memcpy(replacement, text, length);
    replacement[length] = 0;
    return_storage();
    data_ = replacement;
    length_ = length;
    pool_ = &pool;
}
void RenderCommandDiagnosticString::return_storage() noexcept {
    if (data_) {
        pool_->deallocate(data_, static_cast<std::size_t>(length_) + 1, 1);
        data_ = nullptr;
        length_ = 0;
        pool_ = nullptr;
    }
}

void unlink_render_root_node_00b72220(RenderNodeRootList& root,
    CameraTransform& node) noexcept {
    if (node.next_sibling) node.next_sibling->previous_sibling = node.previous_sibling;
    if (node.previous_sibling) node.previous_sibling->next_sibling = node.next_sibling;
    else root.first = node.next_sibling;
}
void unlink_and_release_render_model_00b6dfa0(RenderCommandModelLifetime& lifetime) noexcept {
    CameraTransform& node = lifetime.transform();
    CameraTransform* parent = node.parent;
    if (parent) {
        node.parent = nullptr;
        if (node.previous_sibling) node.previous_sibling->next_sibling = node.next_sibling;
        if (node.next_sibling) node.next_sibling->previous_sibling = node.previous_sibling;
        if (parent->first_child == &node) parent->first_child = parent->first_child->next_sibling;
        --parent->child_count;
        node.parent = nullptr;
    } else if (node.root_list) {
        unlink_render_root_node_00b72220(*node.root_list, node);
        node.root_list = nullptr;
    }
    lifetime.release_model_virtual18_00b6f310();
}

void initialize_render_command_context_00b1edc0_fragment(RenderCommand& command,
    RenderCommandReference* command_scene, RenderCommandReference* camera,
    RenderCommandReference* context_scene, RenderCommandReference* target,
    RenderCommandBatch& batch0, RenderCommandBatch& batch1) {
    if (command.context || command.batches[0] || command.batches[1])
        throw std::invalid_argument("Render command context initialization requires fresh storage");
    command.context = new RenderCommandContext;
    assign_render_command_reference(command.scene, command_scene);
    assign_render_command_reference(command.context->camera, camera);
    assign_render_command_reference(command.context->scene, context_scene);
    assign_render_command_reference(command.context->target, target);
    command.context->command = &command;
    command.batches[0] = &batch0;
    command.batches[1] = &batch1;
}

void destroy_render_command_group_00b1d760(OwnedInstanceGroup& group,
    RenderCommandModelLifetimes& lifetimes, RenderCommandDiagnosticString* diagnostic) noexcept {
    group.binding.reset();
    group.upload.generator = nullptr;
    for (std::size_t category_index = 0; category_index != 2; ++category_index) {
        auto& category = group.upload.categories[category_index];
        if (InstanceUploadModel* model = category.generated_model) {
            unlink_and_release_render_model_00b6dfa0(lifetimes.for_model(*model));
            category.generated_model = nullptr;
            // Inline typed adapters outlive the actual model release operation.
            // Drop their host owner/view fields before the group's geometry owner.
            model->context_owner.reset();
            model->context = nullptr;
            model->geometry = nullptr;
            model->attach_scene = nullptr;
            model->world_sphere_center = nullptr;
        }
        group.geometries[category_index].reset();
    }
    // 00BF7C6E destroys two12-byte array objects from the last to the first.
    for (std::size_t i = 2; i != 0; --i)
        std::vector<InstanceRenderEntry*>().swap(group.upload.categories[i - 1].source_entries);
    if (diagnostic) diagnostic->return_storage();
}

void destroy_render_command_00b1ddd0(RenderCommand& command) noexcept {
    // Native has no null guard or field clear for these two required references.
    release_render_command_reference(*command.batches[0]);
    release_render_command_reference(*command.batches[1]);
    std::size_t index = 0;
    while (index != command.grouping.by_binding.size()) {
        auto& cell = command.grouping.by_binding[index];
        if (cell) {
            destroy_render_command_group_00b1d760(*cell, command.model_lifetimes,
                diagnostic_for(command, *cell));
            cell.reset(); // native free followed by clearing the original slot
        }
        ++index;
    }
    release_then_clear(command.scene);
    release_then_clear(command.context);
    std::vector<InstanceUploadGroup*>().swap(command.grouping.ordered_groups);
    std::vector<std::unique_ptr<OwnedInstanceGroup>>().swap(command.grouping.by_binding);
    std::vector<RenderCommandGroupDiagnostic>().swap(command.group_diagnostics);
    command.diagnostic.return_storage();
}

void reserve_render_command_pointers_00b1c6c0(RenderCommandPointerArray& array,
    std::int32_t requested) {
    if (requested < 1) requested = 1;
    const auto capacity = static_cast<std::uint32_t>(requested);
    if (capacity <= array.capacity_) return;
    if (capacity > (std::numeric_limits<std::uint32_t>::max)() / sizeof(RenderCommand*))
        throw std::length_error("Render command pointer byte extent would overflow");
    std::unique_ptr<RenderCommand*[]> replacement(new RenderCommand*[capacity]);
    for (std::uint32_t i = 0; i < array.count_; ++i) replacement[i] = array.data_[i];
    array.data_.reset();
    array.data_ = std::move(replacement);
    array.capacity_ = capacity;
}
void resize_render_command_pointers_00b1cc80(RenderCommandPointerArray& array,
    std::int32_t count) {
    if (count < 0) throw std::invalid_argument("Negative render command count");
    const auto requested = static_cast<std::uint32_t>(count);
    if (requested > array.capacity_) reserve_render_command_pointers_00b1c6c0(array, count);
    for (std::uint32_t i = array.count_; i < requested; ++i) array.data_[i] = nullptr;
    while (array.count_ > requested) --array.count_;
    array.count_ = requested;
}

void execute_render_command_queue_00b1ebe0(RenderCommandQueue& queue,
    RenderCommandExecutor& executor) noexcept {
    std::uint32_t index = 0;
    while (index < queue.commands.size()) {
        RenderCommandContext* incoming = queue.commands[index]->context;
        RenderCommandContext* old = queue.current_context;
        if (old != incoming) {
            queue.current_context = incoming;
            if (incoming) retain_render_command_reference(*incoming);
            if (old) release_render_command_reference(*old);
        }
        // Terminal old-context release can replace/reallocate the command list.
        executor.execute(*queue.commands[index]);
        // Execute can replace current_context; release the CURRENT reference.
        release_then_clear(queue.current_context);
        if (queue.control == 0) {
            RenderCommand* command = queue.commands[index];
            if (command) {
                destroy_render_command_00b1ddd0(*command);
                delete command;
            }
        }
        ++index;
    }
    if (queue.control == 0) resize_render_command_pointers_00b1cc80(queue.commands, 0);
}
}
