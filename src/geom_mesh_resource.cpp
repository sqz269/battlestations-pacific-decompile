#include "bsp/geom_mesh_resource.hpp"

#include <new>
#include <stdexcept>
#include <utility>

namespace bsp {
namespace {

// The smallest wire footprint of each repeated record, used only to keep a
// corrupt count from reserving an absurd vector before the reads fail. The
// native parser reserves the raw count (00726D80, 00725B40, 007262A0) and has
// no such guard.
constexpr std::uint32_t kMinElementBytes = 8;    // u32 length + u32 index
constexpr std::uint32_t kMinVertexBytes = 12;    // three floats
constexpr std::uint32_t kMinEdgeBytes = 8;       // two integer reads
constexpr std::uint32_t kMinTriangleBytes = 28;  // seven integer reads
// 00E08138 index 14, the second kind 00727704 never merges.
constexpr int kMeshCategoryBullet = 14;

bool reserve_for_count(std::uint32_t count, std::uint32_t remaining,
    std::uint32_t min_bytes, const char* what, std::string& error) {
    if (count > remaining / min_bytes) {
        error = "GeomMesh ";
        error += what;
        error += " count exceeds the remaining payload.";
        return false;
    }
    return true;
}

bool read_u32_field(StructuredNode& node, std::uint32_t& value,
    const char* what, std::string& error) {
    if (node.read_u32(value)) return true;
    error = "Could not read GeomMesh ";
    error += what;
    error += '.';
    return false;
}

}  // namespace

bool parse_geom_mesh_resource_00727310(StructuredNode& node,
    GeomMeshResourcePayload& output, std::string& error) {
    error.clear();
    output = GeomMeshResourcePayload{};
    if (!node.ready()) {
        error = "GeomMesh node is not ready.";
        return false;
    }

    // 00727336 CALL 00BE9B20 / CMP EAX,4 / JNC. The native reads the budget off
    // the stream object at +64h; the host has only the node's own remaining
    // count, which is the same value for a chunk read directly under its
    // container. Below four bytes the native skips the node (00727342 through
    // 00BE9C40) and returns, leaving the mesh empty. That is not a dispatcher
    // failure, so it is reported through the payload flag.
    if (node.remaining() < 4) {
        output.payload_too_small = true;
        if (!node.skip_00be9c40()) {
            error = "Could not skip an undersized GeomMesh payload.";
            return false;
        }
        return true;
    }

    try {
        // 0072735C elementCount, then 00726D80 reserves the 2Ch-stride vector.
        std::uint32_t element_count = 0;
        if (!read_u32_field(node, element_count, "element count", error)) return false;
        // 00727377 CMP EDI,EBX / JLE: the count is compared as a signed value,
        // so a negative count skips the element loop entirely.
        if (static_cast<std::int32_t>(element_count) <= 0) element_count = 0;
        if (!reserve_for_count(element_count, node.remaining(), kMinElementBytes,
                "element", error))
            return false;
        output.authored_elements.reserve(element_count);
        for (std::uint32_t i = 0; i < element_count; ++i) {
            GeomMeshElement element;
            // 0072738A 00BEA010: u32 byte length then the raw characters.
            if (!node.read_string(element.name)) {
                error = "Could not read a GeomMesh element name.";
                return false;
            }
            // 0072738F..0072739E: a null character pointer is replaced by the
            // empty literal at 00E19BF4 before the lookup, which matches
            // nothing. A host std::string is never null, so an authored
            // zero-length name takes the same path and yields -1.
            element.kind = geom_mesh_element_kind_00727310(element.name.c_str());
            if (!read_u32_field(node, element.node_index, "element index", error))
                return false;
            output.authored_elements.push_back(std::move(element));
        }

        // 00727471 vertexCount, 00725B40 reserves mesh+18h, then three floats
        // per vertex into MSVC_Vector12_PushBack 004215D0.
        std::uint32_t vertex_count = 0;
        if (!read_u32_field(node, vertex_count, "vertex count", error)) return false;
        if (static_cast<std::int32_t>(vertex_count) <= 0) vertex_count = 0;
        if (!reserve_for_count(vertex_count, node.remaining(), kMinVertexBytes,
                "vertex", error))
            return false;
        output.vertices.resize(vertex_count);
        for (std::uint32_t i = 0; i < vertex_count; ++i) {
            for (float& component : output.vertices[i]) {
                if (!node.read_float(component)) {
                    error = "Could not read a GeomMesh vertex component.";
                    return false;
                }
            }
        }

        // 007274C4: a count, then two integer reads per entry that the native
        // writes into one scratch slot and drops.
        std::uint32_t edge_count = 0;
        if (!read_u32_field(node, edge_count, "edge count", error)) return false;
        if (static_cast<std::int32_t>(edge_count) <= 0) edge_count = 0;
        if (!reserve_for_count(edge_count, node.remaining(), kMinEdgeBytes, "edge",
                error))
            return false;
        output.edges.resize(edge_count);
        for (std::uint32_t i = 0; i < edge_count; ++i) {
            if (!read_u32_field(node, output.edges[i].a, "edge endpoint", error))
                return false;
            if (!read_u32_field(node, output.edges[i].b, "edge endpoint", error))
                return false;
        }

        // 007274EF triangleCount. 00727504 reserves each element's ordinal
        // vector with the integer quotient triangleCount / elementCount, and
        // 00727563 reserves mesh+28h with the full count.
        std::uint32_t triangle_count = 0;
        if (!read_u32_field(node, triangle_count, "triangle count", error))
            return false;
        // 00727568 TEST EDI,EDI / JLE guards the triangle loop with a signed
        // test, as the element and vertex loops are guarded.
        if (static_cast<std::int32_t>(triangle_count) <= 0) triangle_count = 0;
        if (!reserve_for_count(triangle_count, node.remaining(), kMinTriangleBytes,
                "triangle", error))
            return false;
        // 007274F6 CMP [ESP+88h],EDI / JLE skips this when elementCount <= 0;
        // 00727503 CDQ / IDIV is the integer quotient.
        if (element_count != 0) {
            const std::uint32_t share = triangle_count / element_count;
            for (GeomMeshElement& element : output.authored_elements)
                element.triangle_ordinals.reserve(share);
        }
        output.triangles.reserve(triangle_count);
        output.triangle_leading_words.reserve(triangle_count);
        output.triangle_edges.reserve(triangle_count);
        for (std::uint32_t i = 0; i < triangle_count; ++i) {
            // 007275D9: the element this triangle belongs to. 007275E6..
            // 00727612 bounds-check it against the element vector and
            // 00BF6713 throws when it is out of range; the host rejects.
            std::uint32_t element_index = 0;
            if (!read_u32_field(node, element_index, "triangle element index", error))
                return false;
            if (element_index >= output.authored_elements.size()) {
                error = "GeomMesh triangle names an element outside the list.";
                return false;
            }
            // 00727672 CALL 00BE9B20 / CMP EAX,7 / JC: one further integer read
            // only while at least seven bytes remain. The value is discarded.
            std::uint32_t leading = 0;
            if (node.remaining() >= 7) {
                if (!read_u32_field(node, leading, "triangle leading word", error))
                    return false;
            }
            std::uint32_t corner[3] = {0, 0, 0};
            for (std::uint32_t& value : corner) {
                if (!read_u32_field(node, value, "triangle corner", error))
                    return false;
            }
            std::array<std::uint32_t, 3> trailing{};
            for (std::uint32_t& value : trailing) {
                if (!read_u32_field(node, value, "triangle trailing word", error))
                    return false;
            }
            // 00727646: the ordinal stored into the element's list is the low
            // word of the triangle loop counter, which is this record's index
            // in mesh+28h.
            output.authored_elements[element_index].triangle_ordinals.push_back(
                static_cast<std::uint16_t>(i));
            GeomMeshTriangle triangle;
            // 007276D4/007276DC/007276E5 store the low words only.
            triangle.v0 = static_cast<std::uint16_t>(corner[0]);
            triangle.v1 = static_cast<std::uint16_t>(corner[1]);
            triangle.v2 = static_cast<std::uint16_t>(corner[2]);
            output.triangles.push_back(triangle);
            output.triangle_leading_words.push_back(leading);
            output.triangle_edges.push_back(trailing);
        }

        output.elements = output.authored_elements;
        merge_same_kind_elements_00727704(output.elements);
        return true;
    } catch (const std::bad_alloc&) {
        error = "Could not allocate the decoded GeomMesh payload.";
        return false;
    } catch (const std::length_error&) {
        error = "Decoded GeomMesh payload exceeds host container capacity.";
        return false;
    }
}

void merge_same_kind_elements_00727704(std::vector<GeomMeshElement>& elements) {
    // 00727708's forward walk. The index is not advanced after an erase
    // (00727A55 SUB EBX,1 against 00727A67 ADD EBX,1), so the slot is
    // re-examined with the element that moved into it.
    for (std::size_t i = 0; i < elements.size();) {
        const int kind = elements[i].kind;
        // 0072776C and 0072779D: `fizika` and `bullet` are never merged.
        if (kind == kMeshCategoryFizika || kind == kMeshCategoryBullet
            || i == 0) {
            ++i;
            continue;
        }
        std::size_t target = i;
        for (std::size_t j = 0; j < i; ++j) {
            // 00727810 compares element+4h, the remapped kind.
            if (elements[j].kind == kind) {
                target = j;
                break;
            }
        }
        if (target == i) {
            ++i;
            continue;
        }
        // 00727840..0072797D append the ordinals one at a time; 00727A51
        // then drops the merged element off the end of the vector.
        std::vector<std::uint16_t>& into = elements[target].triangle_ordinals;
        const std::vector<std::uint16_t>& from = elements[i].triangle_ordinals;
        into.insert(into.end(), from.begin(), from.end());
        elements.erase(elements.begin() + static_cast<std::ptrdiff_t>(i));
    }
}

GeomMeshStructuredResourceParser::GeomMeshStructuredResourceParser(
    std::vector<GeomMeshResourcePayload>& sink) noexcept
    : sink_(&sink) {}

std::string GeomMeshStructuredResourceParser::type_name() const {
    // 007258F0 pushes 00CFDBCC, whose bytes are "GeomMesh".
    return "GeomMesh";
}

bool GeomMeshStructuredResourceParser::decode(StructuredNode& node,
    StructuredResourcePayload& output, std::string& error) {
    // `output` is deliberately untouched: the registry's variant has no
    // GeomMesh alternative and this packet does not edit that header. See the
    // class comment in the header.
    (void)output;
    last_payload_ = GeomMeshResourcePayload{};
    if (!parse_geom_mesh_resource_00727310(node, last_payload_, error)) return false;
    ++decoded_count_;
    if (sink_ != nullptr) sink_->push_back(last_payload_);
    return true;
}

}  // namespace bsp
