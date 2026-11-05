#include "bsp/structured_model.hpp"
#include "bsp/structured_resource.hpp"
#include "bsp/native_resource_hierarchy_append.hpp"
#include "bsp/native_resource_pointer_array.hpp"
#include "bsp/native_resource_root_dispatch.hpp"
#include <cstring>
#include <memory>
#include <new>
#include <stdexcept>
#include <utility>

namespace bsp {
StructuredResourceItems::~StructuredResourceItems() { release(); }

StructuredResourceItems::StructuredResourceItems(StructuredResourceItems&& other) noexcept
    : resource_fields_(std::exchange(other.resource_fields_, {})) {}

StructuredResourceItems& StructuredResourceItems::operator=(StructuredResourceItems&& other) noexcept {
    if (this != &other) {
        release();
        resource_fields_ = std::exchange(other.resource_fields_, {});
    }
    return *this;
}

DecodedStructuredResource** StructuredResourceItems::data() const noexcept {
    static_assert(sizeof(void*) == 4);
    DecodedStructuredResource** result;
    std::memcpy(&result, resource_fields_.data() + 0x10, sizeof result);
    return result;
}

std::size_t StructuredResourceItems::size() const noexcept {
    std::uint32_t count;
    std::memcpy(&count, resource_fields_.data() + 0x14, sizeof count);
    return count;
}

DecodedStructuredResource& StructuredResourceItems::operator[](std::size_t index) noexcept {
    return *data()[index];
}
const DecodedStructuredResource& StructuredResourceItems::operator[](std::size_t index) const noexcept {
    return *data()[index];
}

void StructuredResourceItems::append_native(DecodedStructuredResource&& item) {
    // bsp.py show 00B87AA0/00B872F0: the array stores the actual opaque
    // pointer without class/refcount operations, growing by sixteen slots.
    // Keep each decoded record stable while that backing array reallocates.
    auto retained = std::make_unique<DecodedStructuredResource>(std::move(item));
    append_native_resource_item_00b87aa0(resource_fields_.data(), retained.get());
    retained.release();
}

void StructuredResourceItems::release() noexcept {
    const auto count = size();
    auto* const records = data();
    for (std::size_t index = 0; index != count; ++index) delete records[index];
    // bsp.py show 00B87B20: resize this SAME triplet to zero before freeing
    // its backing. Source pointee destruction remains separate from raw slots.
    destroy_native_resource_item_pointers_00b87b20(resource_fields_.data() + 0x10);
    resource_fields_.fill(std::byte{});
}

StructuredHierarchyItems::~StructuredHierarchyItems() { release(); }

StructuredHierarchyItems::StructuredHierarchyItems(StructuredHierarchyItems&& other) noexcept
    : resource_fields_(std::exchange(other.resource_fields_, {})) {}

StructuredHierarchyItems& StructuredHierarchyItems::operator=(StructuredHierarchyItems&& other) noexcept {
    if (this != &other) {
        release();
        resource_fields_ = std::exchange(other.resource_fields_, {});
    }
    return *this;
}

HierarchyItem** StructuredHierarchyItems::data() const noexcept {
    static_assert(sizeof(void*) == 4);
    HierarchyItem** result;
    std::memcpy(&result, resource_fields_.data() + 0x1c, sizeof result);
    return result;
}

std::size_t StructuredHierarchyItems::size() const noexcept {
    std::uint32_t count;
    std::memcpy(&count, resource_fields_.data() + 0x20, sizeof count);
    return count;
}

HierarchyItem& StructuredHierarchyItems::operator[](std::size_t index) noexcept {
    return *data()[index];
}
const HierarchyItem& StructuredHierarchyItems::operator[](std::size_t index) const noexcept {
    return *data()[index];
}

void StructuredHierarchyItems::append_native(HierarchyItem&& item) {
    // bsp.py show 00B87AE0/00B87350: append the opaque record address, grow
    // the actual triplet by 16, shallow-copy pointers, then increment count.
    // Only this array owns encounter order; the retained C++ record is stable
    // across array growth and is released if reserve throws before admission.
    auto retained = std::make_unique<HierarchyItem>(std::move(item));
    append_native_hierarchy_item_00b87ae0(resource_fields_.data(),
        reinterpret_cast<std::uint32_t>(retained.get()));
    retained.release();
}

void StructuredHierarchyItems::release() noexcept {
    const auto count = size();
    auto* const records = data();
    for (std::size_t index = 0; index != count; ++index) delete records[index];
    // bsp.py show 00B87B40/00B87410: pointee ownership is separate from the
    // raw array. The existing destroy body resizes to zero then frees backing.
    destroy_native_hierarchy_item_pointers_00b87b40(resource_fields_.data() + 0x1c);
    resource_fields_.fill(std::byte{}); // retire stale fields after array lifetime
}

StructuredHierarchyItems::operator std::vector<HierarchyItem>() && {
    std::vector<HierarchyItem> result;
    result.reserve(size());
    for (std::size_t index = 0; index != size(); ++index)
        result.push_back(std::move((*this)[index]));
    release();
    return result;
}

void RendererRootHooks_00d5f0a8::begin_root() noexcept {
    // Complete native body00b1fe40: C3 (RET), followed by CC padding.
}

void RendererRootHooks_00d5f0a8::end_root() noexcept {
    // Complete native00b28570..00b28592 installs then immediately restores
    // FS:[0] and its stack frame. No calls or renderer state accesses occur.
    // This host normal-path projection does not synthesize native SEH state.
}

bool parse_hierarchy_00b7f100(StructuredNode& container,
    StructuredModel& output, std::string& error) {
    error.clear();
    if (!container.ready()) {
        error = "Hierarchy container is not ready.";
        return false;
    }
    try {
        while (container.has_remaining_00715bf0()) {
            auto child = container.read_child_00bea680();
            if (!child) {
                error = "Could not read Hierarchy child header.";
                return false;
            }
            if (_stricmp(child->tag().c_str(), "Item") == 0) {
                HierarchyItem item;
                if (!parse_hierarchy_item_00b7eb90(*child, item, error)) return false;
                // Native00b7eb90 appends before its caller releases the handle.
                // The dependent owner type preserves the original vector path
                // when chain563 is OFF. ON uses the native pointer-array body
                // for the real parsed records consumed by model/muzzle readers.
                [&](auto& output) {
                    output.hierarchy.append_native(std::move(item));
                }(output);
            } else {
                if (!child->skip_00be9c40()) {
                    error = "Could not skip unsupported Hierarchy payload.";
                    return false;
                }
                output.skipped_hierarchy_tags.push_back(child->tag());
            }
            if (!child->close()) {
                error = "Could not close Hierarchy child.";
                return false;
            }
        }
        if (!container.ready()) {
            error = "Hierarchy reader failed before completion.";
            return false;
        }
        return true;
    } catch (const std::bad_alloc&) {
        error = "Could not allocate retained Hierarchy records.";
        return false;
    } catch (const std::length_error&) {
        error = "Retained Hierarchy records exceed host container capacity.";
        return false;
    }
}

bool dispatch_root_00b7f430(StructuredNode& root,
    StructuredResourceRegistry& registry, StructuredModel& output,
    StructuredModelDispatchHooks& hooks, std::string& error) {
    error.clear();
    if (!root.ready()) {
        error = "Structured root is not ready.";
        return false;
    }
    hooks.begin_root();
    if (!root.read_control_00be9a40()) {
        error = "Could not read structured root control word.";
        return false;
    }
    try {
        while (root.has_remaining_00715bf0()) {
            auto child = root.read_child_00bea680();
            if (!child) {
                error = "Could not read structured root child header.";
                return false;
            }
            output.root_order.push_back(child->tag());
            const char* tag = child->tag().c_str();
            if (_stricmp(tag, "Resource") == 0) {
                const bool dispatched = [&](auto& output) {
                    {
                        // Preserve native append-before-child-close and partial
                        // output on failure; consumers use this same owner/order.
                        auto sink = StructuredResourceOutput::native_items(output.resources);
                        if (!registry.dispatch_items_00b7e970(*child, sink, error)) return false;
                    }
                    return true;
                }(output);
                if (!dispatched) return false;
            } else if (_stricmp(tag, "Hierarchy") == 0) {
                if (!parse_hierarchy_00b7f100(*child, output, error)) return false;
            } else if (_stricmp(tag, "BoundingBox") == 0) {
                std::array<float, 6> bounds;
                if (!read_bounding_box_00b93310(*child, bounds)) {
                    error = "Could not read structured root BoundingBox.";
                    return false;
                }
                //00b7f51c reloads manager+24, then performs six MOVSS stores.
                // The typed host call binds its destination for the traversal.
                std::memcpy(output.bounding_box.data(), bounds.data(), sizeof(bounds));
                output.has_bounding_box = true;
            } else if (!child->skip_00be9c40()) {
                error = "Could not skip unsupported structured root payload.";
                return false;
            }
            // Native release never seeks over an unread recognized payload.
            if (!child->close()) {
                error = "Could not close structured root child.";
                return false;
            }
        }
        if (!root.ready()) {
            error = "Structured root reader failed before completion.";
            return false;
        }
    } catch (const std::bad_alloc&) {
        error = "Could not allocate retained structured root records.";
        return false;
    } catch (const std::length_error&) {
        error = "Retained structured root records exceed host container capacity.";
        return false;
    }
    hooks.end_root();
    return true;
}
}
