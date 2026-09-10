#include "bsp/structured_model.hpp"
#include "bsp/structured_resource.hpp"
#include <cstring>
#include <new>
#include <stdexcept>
#include <utility>

namespace bsp {
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
                output.hierarchy.push_back(std::move(item));
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
                if (!registry.dispatch_items_00b7e970(*child,
                        output.resources, error)) return false;
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
