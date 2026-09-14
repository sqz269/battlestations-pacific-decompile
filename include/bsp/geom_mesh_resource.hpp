#pragma once
#include "bsp/part_damage_reachability.hpp"
#include "bsp/structured_reader.hpp"
#include "bsp/structured_resource_registry.hpp"
#include <array>
#include <cstdint>
#include <string>
#include <vector>

// The `GeomMesh` resource payload: the authored collision geometry whose
// elements carry the (kind, index) pair the ship hit record copies into its
// +30h/+34h. docs/GEOM_MESH_RESOURCE.md carries the evidence.
//
// Producer: 00727310 BSP_GeomMeshResourceParser_ParsePayload,
// __thiscall(mesh /*ECX*/; StructuredNodeHandle* node), RET 4 at 00727357,
// reached only from the parse slot 00727A90 (00727AD7). The registration
// singleton is 00716FE0 with type-name getter 007258F0, which pushes the
// literal at 00CFDBCC -- the bytes "GeomMesh".
//
// Every name here is a hypothesis, not a recovered symbol. The payload is
// owning decoded wire data; it is not the native 50h-byte resource item, its
// reference-counted base, or any binary-compatible layout.

namespace bsp {

// One entry of the 2Ch-stride vector at mesh+8h. 00727260's 0x2E8BA2E9 /
// SAR EDX,3 magic at 00727277/0072727E is division by 0x2C, and the temporary
// assembled on 00727310's frame spans [ESP+4Ch]..[ESP+74h] = 2Ch bytes.
//
//   element+00h  vtable 00CFDBC8 (slot 0 = 00725880)     007273BF
//   element+04h  kind                                  007273DB
//   element+08h  node index                            007273DF
//   element+0Ch  owning mesh                           007273E3
//   element+10h  MSVC vector<u16> (proxy, begin +14h, end +18h, cap +1Ch)
//   element+20h  heap pointer, zeroed here, freed by the temporary's teardown
//   element+24h  heap pointer, likewise (00727410/00727421 through 00BF6989)
//   element+28h  zeroed here, not freed by the teardown
//
// This host record projects +04h, +08h and the +10h vector only. +20h, +24h
// and +28h are left to their producers: 00727310 only zeroes them.
struct GeomMeshElement {
    // The counted string read at 0072738A. No NUL on the wire.
    std::string name;
    // element+4h. 007149D0's table index with the 7 -> 9 remap; -1 when the
    // authored name is not in the 00E08138 table.
    int kind{};
    // element+8h. The raw u32 read at 007273AC, immediately after the name.
    std::uint32_t node_index{};
    // element+10h's vector. Each entry is the ordinal of a triangle the wire
    // assigned to this element (00727646 stores BX, the low word of the
    // triangle loop counter at [ESP+88h]).
    std::vector<std::uint16_t> triangle_ordinals;
};

// The 6-byte record appended to mesh+28h. 00726670 appends a zeroed one and
// 007276D4/007276DC/007276E5 fill it with the low words of three u32 reads.
struct GeomMeshTriangle {
    std::uint16_t v0{};
    std::uint16_t v1{};
    std::uint16_t v2{};
};

// One entry of the block at 007274C4: two integer reads through 00BE9A80 that
// 00727310 stores into one scratch slot and discards. Host-retained because
// every value in this installation's data is a vertex index and every pair is
// a distinct undirected pair, which is what an edge list looks like; the
// parser proves only that the pair is consumed.
struct GeomMeshEdge {
    std::uint32_t a{};
    std::uint32_t b{};
};

struct GeomMeshResourcePayload {
    // The element list in authored order, before the 00727704 merge pass.
    std::vector<GeomMeshElement> authored_elements;
    // The surviving elements after 00727704..00727A6D. This is the vector at
    // mesh+8h that docs/HIT_HULL_SEGMENT.md's 00723D60 walks with stride 2Ch.
    std::vector<GeomMeshElement> elements;
    // mesh+18h. Three floats per entry (00BE99D0 x3 at 00727492/0072749D/
    // 007274A8), appended by MSVC_Vector12_PushBack 004215D0.
    std::vector<std::array<float, 3>> vertices;
    // The 007274C4 block. Consumed and discarded by the native parser.
    std::vector<GeomMeshEdge> edges;
    // mesh+28h, in wire order. Index i of this vector is the ordinal the
    // element lists refer to.
    std::vector<GeomMeshTriangle> triangles;
    // Per triangle, the fourth read at 0072767E. The native discards it. It is
    // zero for every triangle of every chunk read in this installation.
    std::vector<std::uint32_t> triangle_leading_words;
    // Per triangle, the three trailing reads at 007276AE/007276BA/007276C6.
    // The native discards them. In this installation's data each one indexes
    // `edges` and names an edge of its own triangle.
    std::vector<std::array<std::uint32_t, 3>> triangle_edges;
    // 00727336: the parser tests the remaining payload against 4 and, when it
    // is smaller, skips the node and returns without touching the mesh. That
    // is not an error for the enclosing dispatcher, so it is reported here
    // rather than through the return value.
    bool payload_too_small{};
};

// 00727310. The node must be the attached leaf; the parser leaves it attached
// with its payload consumed, exactly as the native does, except on the
// 00727336 bail where it is skipped and detached.
//
// The native performs no bounds or short-read checks: it reads through
// 00BE9A00/00BEA010/00BE99D0/00BE9A80 whatever the stream returns. The host
// returns false with `error` set when a read fails or a count would exceed the
// node's remaining payload; that rejection is a host addition, not native
// behaviour. Output is not rolled back and consumed input is not restored.
bool parse_geom_mesh_resource_00727310(StructuredNode& node,
    GeomMeshResourcePayload& output, std::string& error);

// 00727704..00727A6D, the pass 00727310 runs after the triangle decode.
// Walking forward, an element whose kind is neither 0Dh (`fizika`) nor 0Eh
// (`bullet`) and whose kind already appeared at a lower index has its triangle
// ordinals appended to that earlier element (00727934 / 0072796B) and is then
// erased (00727A51 ADD [ESI+8],-2Ch). `fizika` and `bullet` elements are
// exempt at 0072776C and 0072779D, which is why a hull keeps one element per
// authored `fizika_NN` node and therefore one distinct +8h per segment.
void merge_same_kind_elements_00727704(std::vector<GeomMeshElement>& elements);

// The registry's published extension point. type_name() is the native getter
// 007258F0's literal.
//
// The base's decode() writes into StructuredResourcePayload, a closed variant
// of MeshResourcePayload / NoteResourcePayload / GroupParamsResourcePayload
// owned by include/bsp/structured_resource_registry.hpp. A GeomMesh payload is
// none of those and this packet does not edit that header, so decode() leaves
// its `output` argument untouched and delivers the decoded payload here
// instead: to the sink vector when one was supplied, and always through
// last_payload(). A caller that registers this parser with
// StructuredResourceRegistry::dispatch_items_00b7e970 therefore gets a
// DecodedStructuredResource whose type_name is "GeomMesh" but whose payload
// holds the variant's default alternative, which carries no GeomMesh data.
// Read the sink, not that record. Adding a fourth alternative to the variant
// is the coordination change that would remove this seam.
class GeomMeshStructuredResourceParser final : public StructuredResourceParser {
public:
    GeomMeshStructuredResourceParser() noexcept = default;
    // The sink is borrowed and must outlive the parser. Decoded payloads are
    // appended in decode order.
    explicit GeomMeshStructuredResourceParser(
        std::vector<GeomMeshResourcePayload>& sink) noexcept;

    std::string type_name() const override;
    bool decode(StructuredNode& node, StructuredResourcePayload& output,
        std::string& error) override;

    // The most recent decode's payload, successful or not. Empty before the
    // first decode.
    const GeomMeshResourcePayload& last_payload() const noexcept {
        return last_payload_;
    }
    std::size_t decoded_count() const noexcept { return decoded_count_; }

private:
    std::vector<GeomMeshResourcePayload>* sink_{};
    GeomMeshResourcePayload last_payload_;
    std::size_t decoded_count_{};
};

}  // namespace bsp
