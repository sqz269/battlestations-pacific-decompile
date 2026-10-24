// cc13-pool15: segment-9 routines rebuilt from the exported listing (bsp.py show <addr>
// --asm, read instruction by instruction on 2026-10-24, x87 stack tracked by hand).
// See include/bsp/pool_pool15_cc13.hpp for the host contract. Names are hypotheses.
// Build-tested only: no caller in src, not fixture- or game-validated.

#include "bsp/pool_pool15_cc13.hpp"

namespace bsp::pool15_cc13 {
namespace {

std::uint8_t* at(void* p, std::uint32_t offset) { return static_cast<std::uint8_t*>(p) + offset; }
template <class T> T& field(void* p, std::uint32_t offset) { return *reinterpret_cast<T*>(at(p, offset)); }

// World fields: the map box corners (WorldMapBounds in include/bsp/world_map_bounds.hpp).
constexpr std::uint32_t kNorthWestX = 0x711C;
constexpr std::uint32_t kNorthWestZ = 0x7124;
constexpr std::uint32_t kSouthEastX = 0x7128;
constexpr std::uint32_t kSouthEastZ = 0x7130;
// The four border-zone std::lists {+0 ?, +4 head node, +8 size} at world+7134h+edge*0Ch
// (LEA [EBX+EBX*2+1C4Dh]*4). Nodes are {+0 next, +4 prev, +8 record*}.
void* border_list(void* world, std::uint32_t edge) { return at(world, 0x7134 + edge * 0xC); }

}  // namespace

// ---------------------------------------------------------------------------
// 004C71C0 BSP_World_LayoutBorderZoneRecords (230 insns)
// ABI: __fastcall(ECX = world), no stack arguments; plain RET, or a tail JMP to
// 0049F260 with ECX = [world+21D0h] when that is set (004C7492..004C74A7). No EH frame.
// Callers: 004D5BD0 BSP_Game_ApplyMapBounds (tail), 004CA760, 004CA860, 004D62C0,
// 004CA930. Record layout (44h): +0 length, +8 edge, +0Ch index, +10h..+3Ch corners
// A, B, C, D (x, y, z; y never written), +40h running offset.
// For each edge 0..3 and each record in list order: edge, index and offset are stored;
// the last record (index == size - 1, size re-read at each test) gets length =
// 004B6C40(edge) - offset. Edges 0/2 (z0 = NW.z / SE.z, s = +1 / -1 from 00D7A24C /
// 00D7A260) lay x along the edge from NW.x and put z outside it; edges 1/3 (x0 = SE.x /
// NW.x) lay z downward from NW.z and put x outside. Constants: margin 50.0 (00CE3938),
// end extension 1000.0 (00CE47A0), depth 950.0 (00CE7640), all exact in float.
// The offset advances by the record's (possibly rewritten) length (FADD [ESI], 004C7452).
// x87: every sum is written below in the listing's operand order. The callers run on
// the main thread at mission setup, whose x87 precision control is 24-bit (PC 007Fh,
// docs/CC13_FPU_PRECISION.md), so each operation rounds to float: modelled as float
// arithmetic per operation. Uncertainty: under a 53-bit control the chained sums such
// as (NW.x + offset) + margin would round once instead of twice. The live host's typed
// model (world_map_bounds.cpp) takes these sums in double, which matches the 53-bit
// reading, not this one.
// The STL checked-iterator tests at 004C721A and 004C745C compare the current node with
// the head after the loop has already established they differ, so their
// _invalid_parameter_noinfo calls cannot run; they are not modelled.
// ---------------------------------------------------------------------------
void world_layout_border_zone_records_004c71c0(void* world, Pool15Host& host) {
    const float margin = 50.0f;
    const float extension = 1000.0f;
    const float depth = 950.0f;
    for (std::int32_t edge = 0; edge < 4; ++edge) {
        void* list = border_list(world, static_cast<std::uint32_t>(edge));
        void* head = field<void*>(list, 4);
        auto is_last = [&](std::int32_t index) { return index == field<std::int32_t>(list, 8) - 1; };
        float offset = 0.0f;
        std::int32_t index = 0;
        for (void* node = field<void*>(head, 0); node != head; node = field<void*>(node, 0), ++index) {
            void* r = field<void*>(node, 8);
            field<std::int32_t>(r, 0x8) = edge;
            field<std::int32_t>(r, 0xC) = index;
            field<float>(r, 0x40) = offset;
            if (is_last(index)) field<float>(r, 0x0) = host.game_map_box_span_004b6c40(world, edge) - offset;
            if (edge == 0 || edge == 2) {
                const float z0 = field<float>(world, edge == 0 ? kNorthWestZ : kSouthEastZ);
                const float s = edge == 0 ? 1.0f : -1.0f;
                // 004C72BC..004C7305: x along the edge.
                field<float>(r, 0x10) = (field<float>(world, kNorthWestX) + offset) + margin;
                field<float>(r, 0x1C) = (field<float>(world, kNorthWestX) + offset) + margin;
                const float length = field<float>(r, 0x0);
                field<float>(r, 0x28) = ((field<float>(world, kNorthWestX) + offset) + length) - margin;
                field<float>(r, 0x34) = (length + (field<float>(world, kNorthWestX) + offset)) - margin;
                if (index == 0) field<float>(r, 0x10) = field<float>(r, 0x10) - extension;
                if (is_last(index)) field<float>(r, 0x34) = field<float>(r, 0x34) + extension;
                // 004C7333..004C7368: z outside the edge.
                const float outer = z0 + depth * s;
                field<float>(r, 0x18) = outer;
                const float inner = z0 + s * margin;
                field<float>(r, 0x24) = inner;
                field<float>(r, 0x30) = inner;
                field<float>(r, 0x3C) = outer;
            } else {
                const float x0 = field<float>(world, edge == 1 ? kSouthEastX : kNorthWestX);
                const float s = edge == 1 ? 1.0f : -1.0f;
                // 004C73B7..004C73EC: x outside the edge.
                const float outer = x0 + depth * s;
                field<float>(r, 0x10) = outer;
                const float inner = x0 + s * margin;
                field<float>(r, 0x1C) = inner;
                field<float>(r, 0x28) = inner;
                field<float>(r, 0x34) = outer;
                // 004C73EF..004C7432: z along the edge, downward from NW.z.
                field<float>(r, 0x18) = (field<float>(world, kNorthWestZ) - offset) - margin;
                field<float>(r, 0x24) = (field<float>(world, kNorthWestZ) - offset) - margin;
                const float length = field<float>(r, 0x0);
                field<float>(r, 0x30) = ((field<float>(world, kNorthWestZ) - offset) - length) + margin;
                field<float>(r, 0x3C) = ((field<float>(world, kNorthWestZ) - offset) - length) + margin;
                if (index == 0) field<float>(r, 0x18) = field<float>(r, 0x18) + extension;
                if (is_last(index)) field<float>(r, 0x3C) = field<float>(r, 0x3C) - extension;  // FSUBR
            }
            offset = offset + field<float>(r, 0x0);
        }
    }
    if (void* traffic = field<void*>(world, 0x21D0); traffic != nullptr) {
        host.traffic_refresh_current_layers_if_set_0049f260(traffic);
    }
}

// ---------------------------------------------------------------------------
// 004C74C0 BSP_World_BorderZoneAtPoint (204 insns)
// ABI: __thiscall(ECX = world; const float point[3]*, iterator* out) RET 8; no EH
// frame; EAX is not a result. Callers 00489C40, 004CA860 (x2), 004D62C0.
// Outside distances (one float subtraction each, so precision control is irrelevant):
// d0 = p.z - NW.z, d2 = SE.z - p.z, d1 = p.x - SE.x, d3 = NW.x - p.x. The edge is the
// first of 0, 2, 1, 3 whose distance is > 0 and greater than its two neighbours'
// (d0 vs d1, d3; d2 vs d1, d3; d1 vs d0, d2; d3 vs d0, d2). Every test is a COMISS or
// FCOMI followed by JBE, so NaN fails it. When none holds (a point inside the map) the
// image loads EBX from the first argument's stack slot (004C75B6), i.e. the point
// POINTER becomes the edge; kept literally, which indexes far out of the world.
// out = {list, first}; an empty list trips _invalid_parameter_noinfo (004C75E1). The
// remaining distance is p.x - NW.x on edges 0/2 and NW.z - p.z otherwise; each pass
// subtracts the current record's length (FSTP float) and stops when 0 > remaining
// (FLDZ/FCOMIP/JA, NaN continues) or when the look-ahead node is the head; otherwise
// out.node advances. The checked-iterator tests are kept in listing order.
// ---------------------------------------------------------------------------
void world_border_zone_at_point_004c74c0(void* world, const float* point, void* out_iterator,
    Pool15Host& host) {
    const float d0 = point[2] - field<float>(world, kNorthWestZ);
    const float d2 = field<float>(world, kSouthEastZ) - point[2];
    const float d1 = point[0] - field<float>(world, kSouthEastX);
    const float d3 = field<float>(world, kNorthWestX) - point[0];
    std::uint32_t edge;
    if (d0 > 0.0f && d0 > d1 && d0 > d3) {
        edge = 0;
    } else if (d2 > 0.0f && d2 > d1 && d2 > d3) {
        edge = 2;
    } else if (d1 > 0.0f && d1 > d0 && d1 > d2) {
        edge = 1;
    } else if (d3 > 0.0f && d3 > d0 && d3 > d2) {
        edge = 3;
    } else {
        edge = static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(point));
    }
    void* list = border_list(world, edge);
    void* first = field<void*>(field<void*>(list, 4), 0);
    void*& out_list = field<void*>(out_iterator, 0);
    void*& out_node = field<void*>(out_iterator, 4);
    out_list = list;
    out_node = first;
    if (first == field<void*>(list, 4)) host.invalid_parameter_noinfo_00bf6713();
    void* next = field<void*>(first, 0);
    float remaining = (edge == 0 || edge == 2) ? point[0] - field<float>(world, kNorthWestX)
                                              : field<float>(world, kNorthWestZ) - point[2];
    for (;;) {
        if (out_list == nullptr) host.invalid_parameter_noinfo_00bf6713();
        if (out_node == field<void*>(out_list, 4)) host.invalid_parameter_noinfo_00bf6713();
        remaining = remaining - field<float>(field<void*>(out_node, 8), 0);
        if (0.0f > remaining) return;
        if (next == field<void*>(list, 4)) return;
        if (out_list == nullptr) host.invalid_parameter_noinfo_00bf6713();
        if (out_node == field<void*>(out_list, 4)) host.invalid_parameter_noinfo_00bf6713();
        out_node = field<void*>(out_node, 0);
        if (next == field<void*>(list, 4)) host.invalid_parameter_noinfo_00bf6713();
        next = field<void*>(next, 0);
    }
}

// ---------------------------------------------------------------------------
// 004CACA0 BSP_NativeString_AppendInt (51 insns)
// ABI: __thiscall(ECX = string; int value) RET 4. Handler 00C655B8; state 0 while the
// temporary lives, -1 before its release. 004260B0 builds a temporary NativeString
// {length, data} from the value; when its length n is non-zero the target is resized
// to old + n (0041DD40, preserve 1) and the n digits are copied to data + old (no
// terminator copied); then the temporary's data, when set, goes back to the pool with
// size n + 1. Callers include 004CD450, 0074BEC0, 00781910, 0095C1C0, 00964790,
// 00AE84E0 (x8), 00BD7B30; the host's text writers append ints themselves.
// ---------------------------------------------------------------------------
void native_string_append_int_004caca0(void* string, std::int32_t value, Pool15Host& host) {
    struct { std::uint32_t length; void* data; } temporary;
    host.native_string_from_int_004260b0(&temporary, value);
    const std::uint32_t n = temporary.length;
    void* const data = temporary.data;
    if (n != 0) {
        const std::uint32_t old = field<std::uint32_t>(string, 0);
        host.native_string_resize_0041dd40(string, n + old, 1);
        host.memcpy_00bf7680(field<std::uint8_t*>(string, 4) + old, data, n);
    }
    if (data != nullptr) {
        void* pool = host.sized_pool_00419cc0();
        host.sized_pool_return_block_00bd1510(pool, data, n + 1, 1);
    }
}

}  // namespace bsp::pool15_cc13
