#include "bsp/structured_hierarchy.hpp"
#include "bsp/structured_resource.hpp"
#include <cstring>
#include <new>
#include <utility>

namespace bsp {
bool read_matrix_00b936e0(StructuredNode& node,
    std::array<float, 16>& matrix) noexcept {
    // Native4x4 loop writes contiguous slots without transpose or expansion.
    for (float& value : matrix) if (!node.read_float(value)) return false;
    return true;
}

bool read_sphere_00b932e0(StructuredNode& node,
    std::array<float, 4>& sphere) noexcept {
    for (float& value : sphere) if (!node.read_float(value)) return false;
    return true;
}

void sphere_to_box_00b7d160(const std::array<float, 4>& sphere,
    std::array<float, 6>& box) noexcept {
    // Stage the complete output before assignment, like wrapper00b7d220.
    // The audited leaf calculates maxima before minima. Use explicit x87
    // operations rather than changing this arithmetic to compiler-selected SSE.
    const float* source = sphere.data();
    std::array<float, 6> converted;
    float* destination = converted.data();
    __asm {
        mov eax, source
        mov edx, destination
        fld dword ptr [eax]
        fadd dword ptr [eax + 12]
        fstp dword ptr [edx + 12]
        fld dword ptr [eax + 4]
        fadd dword ptr [eax + 12]
        fstp dword ptr [edx + 16]
        fld dword ptr [eax + 8]
        fadd dword ptr [eax + 12]
        fstp dword ptr [edx + 20]
        fld dword ptr [eax]
        fsub dword ptr [eax + 12]
        fstp dword ptr [edx]
        fld dword ptr [eax + 4]
        fsub dword ptr [eax + 12]
        fstp dword ptr [edx + 4]
        fld dword ptr [eax + 8]
        fsub dword ptr [eax + 12]
        fstp dword ptr [edx + 8]
    }
    box = converted;
}

bool parse_hierarchy_item_00b7eb90(StructuredNode& item,
    HierarchyItem& output, std::string& error) {
    error.clear();
    if (!item.ready()) {
        error = "Hierarchy Item reader is not ready.";
        return false;
    }

    try {
        HierarchyItem parsed;
        while (item.has_remaining_00715bf0()) {
            auto child = item.read_child_00bea680();
            if (!child) {
                error = "Could not read Hierarchy Item child header.";
                return false;
            }

            const char* tag = child->tag().c_str();
            const char* field = "unknown";
            bool success;
            if (_stricmp(tag, "Parent") == 0) {
                field = "Parent";
                success = child->read_u32(parsed.parent);
            } else if (_stricmp(tag, "Resource") == 0) {
                field = "Resource";
                std::uint32_t value;
                success = child->read_u32(value);
                if (success) parsed.resources.push_back(value);
            } else if (_stricmp(tag, "Matrix") == 0) {
                field = "Matrix";
                std::array<float, 16> values;
                success = read_matrix_00b936e0(*child, values);
                if (success) parsed.matrix = values;
            } else if (_stricmp(tag, "Name") == 0) {
                field = "Name";
                success = child->read_string(parsed.name);
            } else if (_stricmp(tag, "Flags") == 0) {
                field = "Flags";
                success = child->read_u32(parsed.flags);
            } else if (_stricmp(tag, "BoundingSphere") == 0) {
                field = "BoundingSphere";
                success = read_sphere_00b932e0(*child, parsed.sphere);
                if (success) sphere_to_box_00b7d160(parsed.sphere, parsed.box);
            } else if (_stricmp(tag, "BoundingBox") == 0) {
                field = "BoundingBox";
                success = read_bounding_box_00b93310(*child, parsed.box);
            } else {
                success = child->skip_00be9c40();
            }
            if (!success) {
                error = "Could not read Hierarchy Item field: ";
                error += field;
                return false;
            }

            // This deliberately does not skip leftover bytes of recognized
            // fields. Native handle destruction debits the declared payload
            // but never advances over an unread tail. Unknown fields already
            // detached through their explicit skip above.
            if (!child->close()) {
                error = "Could not close Hierarchy Item field.";
                return false;
            }
        }
        if (!item.ready()) {
            error = "Hierarchy Item reader failed before completion.";
            return false;
        }
        output = std::move(parsed);
        return true;
    } catch (const std::bad_alloc&) {
        error = "Could not allocate Hierarchy Item values.";
        return false;
    }
}
}
