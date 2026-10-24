// cc13 pool23: referenced game rows from triage T5. Build only; nothing in a host path calls these.
// Descriptive names are hypotheses. See include/bsp/pool_pool23_cc13.hpp for the bound callees.

#include "bsp/pool_pool23_cc13.hpp"

#include <cmath>
#include <cstring>

namespace bsp {
namespace {

template <class T>
T load(const std::byte* base, std::size_t offset) {
    T value;
    std::memcpy(&value, base + offset, sizeof(T));
    return value;
}

template <class T>
void store(std::byte* base, std::size_t offset, T value) {
    std::memcpy(base + offset, &value, sizeof(T));
}

std::byte* load_ptr(const std::byte* base, std::size_t offset) {
    return load<std::byte*>(base, offset);
}

// 0047BA40 per-axis step. The main thread runs the x87 at 24-bit precision control, so the
// FDIV float result and the FILD/FMUL product both round to a float mantissa.
std::int32_t cell_index(float relative, float cell, Pool23Host& host) {
    const float quotient = relative / cell;
    return host.ftol_00bf7420(quotient);
}

float cell_start(std::int32_t index, float cell) {
    return static_cast<float>(static_cast<double>(index) * static_cast<double>(cell));
}

}  // namespace

// 00451B90 BSP_PanelSequenceMap_BreakDialog
// Evidence: sole caller 008B07C0 BSP_LuaBinding_BreakDialog (cc13-triT5 triage). Listing
// 00451B90..00451C7F (84 insns): find(name) in the map at +1Ch (0044BA00) with the result checked
// (container nonnull and == the map, else 00BF6713). When the node is not the map end (+20h):
// if name equals the current dialog +28h case-insensitively (00435C40), the record from 00451920
// gets byte +38h = 1; otherwise a second find feeds erase(where) 00451020. Every path then releases
// the by-value string (data nonnull): 00419CC0 then 00BD1510(data, length + 1, 1).
// ABI: __thiscall(ECX = owner, NativeString by value at [ESP+4]), RET 8.
// EH: frame 00C60658, state 0 covers the by-value string (unwind 0041DD20); the body stores -1 just
// before its own release. No call between can be shown to throw here, so the unwind is not modelled.
// Uncertainty: the map record type behind 00451920 is not recovered; +38h is a "break requested" byte.
void panel_sequence_map_break_dialog_00451b90(std::byte* owner, Pool23NativeString name, Pool23Host& host) {
    std::byte* const map = owner + 0x1C;
    std::byte* const end = load_ptr(owner, 0x20);
    Pool23CheckedIterator found{};
    const Pool23CheckedIterator* where = host.panel_map_find_0044ba00(map, &found, &name);
    if (where->container == nullptr || where->container != map) {
        host.invalid_parameter_00bf6713();
    }
    if (where->node != end) {
        if (host.native_string_equals_insensitive_00435c40(&name, owner + 0x28)) {
            std::byte* record = host.panel_map_lookup_or_insert_00451920(map, &name);
            store<std::uint8_t>(record, 0x38, 1);
        } else {
            const Pool23CheckedIterator* again = host.panel_map_find_0044ba00(map, &found, &name);
            Pool23CheckedIterator erased{};
            host.panel_map_erase_00451020(map, &erased, again->container, again->node);
        }
    }
    if (name.data != nullptr) {
        void* pool = host.sized_storage_pool_00419cc0();
        host.sized_storage_return_block_00bd1510(pool, name.data, name.length + 1, 1);
    }
}

// 0047BA40 BSP_QuadTree_CellRangeForBounds
// Evidence: listing 0047BA40..0047BB3F (89 insns). Relative coordinates are each spilled as float:
// x0 = box[0] - [+58h], x1 = box[3] - [+58h] (FSUBP ST2 = ST2 - ST0, then FSUBR [box+0Ch]),
// z0 = box[2] - [+60h], z1 = box[5] - [+60h]. For each, i = _ftol(v / [+40h]) is stored, then
// p = i * [+40h]. Minimum outputs: FXCH; FCOMIP v, p; JBE skips, so i + 1 when v > p (unordered
// keeps i). Maximum outputs: FCOMIP p, v; JC skips, so i - 1 when p >= v (unordered keeps i).
// ABI: __thiscall(ECX = tree, box*, x0*, x1*, z0*, z1*), RET 14h; no return value.
// Uncertainty: the main-thread 24-bit precision is modelled; a different control word would change
// only the rounding of the quotient and product.
void quad_tree_cell_range_for_bounds_0047ba40(const std::byte* tree, const float* box, std::int32_t* x0,
                                              std::int32_t* x1, std::int32_t* z0, std::int32_t* z1,
                                              Pool23Host& host) {
    const float origin_x = load<float>(tree, 0x58);
    const float rel_x0 = box[0] - origin_x;
    const float rel_x1 = box[3] - origin_x;
    const float origin_z = load<float>(tree, 0x60);
    const float rel_z0 = box[2] - origin_z;
    const float rel_z1 = box[5] - origin_z;

    std::int32_t i = cell_index(rel_x0, load<float>(tree, 0x40), host);
    *x0 = i;
    if (rel_x0 > cell_start(i, load<float>(tree, 0x40))) {
        *x0 = i + 1;
    }
    i = cell_index(rel_x1, load<float>(tree, 0x40), host);
    *x1 = i;
    if (cell_start(i, load<float>(tree, 0x40)) >= rel_x1) {
        *x1 = i - 1;
    }
    i = cell_index(rel_z0, load<float>(tree, 0x40), host);
    *z0 = i;
    if (rel_z0 > cell_start(i, load<float>(tree, 0x40))) {
        *z0 = i + 1;
    }
    i = cell_index(rel_z1, load<float>(tree, 0x40), host);
    *z1 = i;
    if (cell_start(i, load<float>(tree, 0x40)) >= rel_z1) {
        *z1 = i - 1;
    }
}

// 004FAAC0 BSP_TacticalLibraryEntries_FindIndexById
// Evidence: listing 004FAAC0..004FAB30 (49 insns). EDI counts while EDI < 004F92E0(vector)
// (unsigned JC, re-read each pass). Each pass checks first ([+4]) nonnull and EDI < signed
// (last - first) / 6Ch (magic 4BDA12F7 >> 5, unsigned JC), else 00BF6713, then compares the dword
// at first + EDI*6Ch + 48h with the key; a match returns EDI, the end returns -1 (OR EAX,-1).
// ABI: __stdcall(vector* [ESP+4], key [ESP+8]), RET 8; ECX is overwritten before use.
std::int32_t tactical_library_entries_find_index_004faac0(const std::byte* vector, std::int32_t key,
                                                          Pool23Host& host) {
    std::uint32_t index = 0;
    if (host.library_entry_vector_size_004f92e0(vector) == 0) {
        return -1;
    }
    do {
        const std::byte* first = load_ptr(vector, 0x4);
        if (first == nullptr ||
            index >= static_cast<std::uint32_t>((load_ptr(vector, 0x8) - first) / 0x6C)) {
            host.invalid_parameter_00bf6713();
        }
        first = load_ptr(vector, 0x4);
        if (load<std::int32_t>(first, index * 0x6C + 0x48) == key) {
            return static_cast<std::int32_t>(index);
        }
        ++index;
    } while (index < host.library_entry_vector_size_004f92e0(vector));
    return -1;
}

// 006407A0 BSP_InGameHudMarkers_AcquireMarkerWidgets
// Evidence: listing 006407A0..0064085A (62 insns). Pointer vector at +4Ch (first +50h, last +54h);
// when the used count [+7Ch] equals its size a new 84h block is allocated (00BF681B), constructed by
// 0063C110 with [+8Ch] and pushed (006401E0). Then [+7Ch] is checked against the size (first
// nonnull, unsigned JC, else 00BF6713), elem = first[[+7Ch]], elem byte +4 = flag, [+7Ch] += 1.
// ABI: __thiscall(ECX = markers, byte flag [ESP+4]), RET 4, EAX = elem.
// EH: frame 00C7A29B, state 0 spans the 0063C110 call; its unwind 00C7A290 frees the block
// (00BF65AC). The host constructor reports failure by throwing, so the state is not modelled here.
// Uncertainty: the existing hosts (hud_markers_acquire_record_006407a0 / hud_markers_sink_006407a0)
// carry this address; this body is a separate unwired reconstruction.
std::byte* hud_markers_acquire_widgets_006407a0(std::byte* markers, std::uint8_t flag, Pool23Host& host) {
    std::byte* const vector = markers + 0x4C;
    auto size_of = [&]() -> std::uint32_t {
        const std::byte* first = load_ptr(markers, 0x50);
        if (first == nullptr) {
            return 0;
        }
        return static_cast<std::uint32_t>((load_ptr(markers, 0x54) - first) >> 2);
    };
    if (load<std::uint32_t>(markers, 0x7C) == size_of()) {
        void* block = host.operator_new_00bf681b(0x84);
        std::byte* widgets = nullptr;
        if (block != nullptr) {
            widgets = host.hud_unit_marker_widgets_construct_0063c110(block, load<std::uint32_t>(markers, 0x8C));
        }
        host.marker_vector_push_back_006401e0(vector, &widgets);
    }
    const std::uint32_t used = load<std::uint32_t>(markers, 0x7C);
    const std::byte* first = load_ptr(markers, 0x50);
    if (first == nullptr || used >= size_of()) {
        host.invalid_parameter_00bf6713();
    }
    std::byte* const elem = load<std::byte*>(load_ptr(markers, 0x50), used * 4);
    store<std::uint8_t>(elem, 0x4, flag);
    store<std::uint32_t>(markers, 0x7C, load<std::uint32_t>(markers, 0x7C) + 1);
    return elem;
}

// 006F3270 BSP_CommandBuilding_OnHealthChanged
// Evidence: CommandBuilding vtable 00CFB028 slot (12 insns). XORPS/COMISS 0 against [+370h]; JB
// (0 < health, or unordered) tail-jumps 00958A30. Otherwise a side [+54h] of 2 returns; any other
// side stores [+7B0h] = [+54h] and [+7B4h] = [+2D8h] and tail-jumps 006F2940 (send neutralize).
// The decompiler dropped the two stores; they are in the listing.
// ABI: __fastcall(ECX = building), plain RET; the tail jumps keep ECX.
void command_building_on_health_changed_006f3270(std::byte* building, Pool23Host& host) {
    if (!(load<float>(building, 0x370) <= 0.0f)) {
        host.unit_react_to_health_change_00958a30(building);
        return;
    }
    const std::uint32_t side = load<std::uint32_t>(building, 0x54);
    if (side == 2) {
        return;
    }
    store<std::uint32_t>(building, 0x7B0, side);
    store<std::uint32_t>(building, 0x7B4, load<std::uint32_t>(building, 0x2D8));
    host.command_building_send_neutralize_006f2940(building);
}

// 007030D0 BSP_MessageRefMap_FindKeyByValue
// Evidence: listing 007030D0..00703162 (56 insns). Iterator {container = owner+4, node =
// head->left} with head = [owner+8] re-read each pass. Each pass checks the container (nonnull and
// == owner+4). At the head the loop ends and returns begin()->key: head->left + 0Ch, checked
// against head. Otherwise the node is checked against head, node + 14h (the mapped value address)
// is compared with the argument, and a match returns node + 0Ch; else 00701FC0 advances.
// ABI: __thiscall(ECX = owner, value* [ESP+4]), RET 4, EAX = key address.
// Uncertainty: the not-found return of the first key is what the listing does; callers may never
// reach it.
std::byte* message_ref_map_find_key_by_value_007030d0(std::byte* owner, const void* value, Pool23Host& host) {
    std::byte* const map = owner + 0x4;
    Pool23CheckedIterator it{map, load_ptr(load_ptr(owner, 0x8), 0x0)};
    for (;;) {
        std::byte* const head = load_ptr(map, 0x4);
        if (it.container == nullptr || it.container != map) {
            host.invalid_parameter_00bf6713();
        }
        if (it.node == head) {
            std::byte* const leftmost = load_ptr(load_ptr(map, 0x4), 0x0);
            if (leftmost == load_ptr(map, 0x4)) {
                host.invalid_parameter_00bf6713();
            }
            return leftmost + 0xC;
        }
        if (it.container == nullptr) {
            host.invalid_parameter_00bf6713();
        }
        if (it.node == load_ptr(it.container, 0x4)) {
            host.invalid_parameter_00bf6713();
        }
        if (it.node + 0x14 == static_cast<const std::byte*>(value)) {
            if (it.node == load_ptr(it.container, 0x4)) {
                host.invalid_parameter_00bf6713();
            }
            return it.node + 0xC;
        }
        host.message_map_iterator_increment_00701fc0(&it);
    }
}

// 00723840 BSP_UnitPartCollision_WalkAllSegments
// Evidence: listing 00723840..007238D3 (63 insns). n = first [+0Ch] ? signed (last [+10h] - first)
// / 2Ch : 0 (magic 2E8BA2E9 >> 3), computed once. For i in [0, n): checks first nonnull and i <
// the size recomputed (unsigned JC), else 00BF6713; calls 00722CA0(ECX = [+0Ch] + i*2Ch)(arg, i % 7)
// (magic 24924925).
// ABI: __thiscall(ECX = part, arg [ESP+4]), RET 4. EAX is the last 00722CA0 result; with no pass it
// is 0 when first is null, else the low dword of (last - first) * 2E8BA2E9 left by the IMUL.
// Uncertainty: callers are not known to read EAX; the no-pass value is kept for fidelity.
std::uint32_t unit_part_collision_walk_all_segments_00723840(std::byte* part, std::uint32_t arg,
                                                             Pool23Host& host) {
    std::uint32_t eax = 0;
    std::int32_t count = 0;
    const std::byte* first = load_ptr(part, 0xC);
    if (first != nullptr) {
        const std::int32_t bytes = static_cast<std::int32_t>(load_ptr(part, 0x10) - first);
        count = bytes / 0x2C;
        eax = static_cast<std::uint32_t>(static_cast<std::int64_t>(bytes) * 0x2E8BA2E9LL);
    }
    for (std::int32_t i = 0; i < count; ++i) {
        const std::byte* now = load_ptr(part, 0xC);
        if (now == nullptr ||
            static_cast<std::uint32_t>(i) >=
                static_cast<std::uint32_t>(static_cast<std::int32_t>(load_ptr(part, 0x10) - now) / 0x2C)) {
            host.invalid_parameter_00bf6713();
        }
        eax = host.unit_part_segment_walk_00722ca0(load_ptr(part, 0xC) + i * 0x2C, arg,
                                                   static_cast<std::uint32_t>(i) % 7);
    }
    return eax;
}

// 006AC490 BSP_BuildingPad_SetOccupant
// Evidence: listing 006AC490..006AC4C6 (20 insns). entry = pad + 1E4h; when [entry+14h] differs
// from the new unit, an old unit is unregistered with 006952A0(ECX = old, EDX = entry), the field is
// written, and a new unit is registered with 00694A60(ECX = unit, EDX = entry).
// ABI: __thiscall(ECX = pad, unit [ESP+4]), RET 4.
// Uncertainty: the live host BuildingPadModel::set_occupant_006ac490 (src/building_pads.cpp) also
// carries this address; this body is not wired in.
void building_pad_set_occupant_006ac490(std::byte* pad, std::byte* unit, Pool23Host& host) {
    std::byte* const entry = pad + 0x1E4;
    std::byte* const old = load_ptr(entry, 0x14);
    if (old == unit) {
        return;
    }
    if (old != nullptr) {
        host.observer_unregister_pair_006952a0(old, entry);
    }
    store<std::byte*>(entry, 0x14, unit);
    if (unit != nullptr) {
        host.observer_register_pair_00694a60(unit, entry);
    }
}

namespace {

// One option table of 00626C50: names (8-byte NativeStrings), owner ids (dwords) and default
// strings (8-byte), each a vector {first, last} at the given globals.
struct OptionTableGlobals {
    std::uint32_t names;     // first; last at +4
    std::uint32_t owners;    // first; last at +4
    std::uint32_t defaults;  // first; last at +4
    std::uint32_t bits;      // vector<bool> object, 0 for the first table (bound helpers instead)
    std::uint32_t map_offset;
};

std::byte* global_ptr(Pool23Host& host, std::uint32_t address) {
    return load_ptr(host.global_address(address), 0);
}

// first nonnull and index < (last - first) >> shift (SAR, then an unsigned compare).
bool vector_index_ok(Pool23Host& host, std::uint32_t vector, std::uint32_t index, int shift) {
    const std::byte* first = global_ptr(host, vector);
    if (first == nullptr) {
        return false;
    }
    const std::int32_t bytes = static_cast<std::int32_t>(global_ptr(host, vector + 4) - first);
    return index < static_cast<std::uint32_t>(bytes >> shift);
}

void apply_option_table(const OptionTableGlobals& table, std::byte* record, std::uint32_t key,
                        Pool23Host& host) {
    std::byte* const map = record + table.map_offset;
    for (std::uint32_t i = 0; vector_index_ok(host, table.names, i, 3); ++i) {
        if (!vector_index_ok(host, table.owners, i, 2)) {
            host.invalid_parameter_00bf6713();
        }
        if (load<std::uint32_t>(global_ptr(host, table.owners), i * 4) != key) {
            continue;
        }
        if (!vector_index_ok(host, table.defaults, i, 3)) {
            host.invalid_parameter_00bf6713();
        }
        std::byte* const fallback = global_ptr(host, table.defaults) + i * 8;
        if (!vector_index_ok(host, table.names, i, 3)) {
            host.invalid_parameter_00bf6713();
        }
        // NativeString assignment: self-check, resize, then copy the resized length.
        std::byte* const value = host.option_map_subscript_00625f50(map, global_ptr(host, table.names) + i * 8);
        if (value != fallback) {
            host.native_string_resize_0041dd40(value, load<std::uint32_t>(fallback, 0), 1);
            if (load<std::uint32_t>(fallback, 0) != 0) {
                host.memcpy_00bf7680(load_ptr(value, 4), load_ptr(fallback, 4), load<std::uint32_t>(value, 0));
            }
        }

        std::uint32_t flag = 0;
        std::byte* name = nullptr;
        if (table.bits == 0) {
            // 00626D66..00626DB3: the names entry is taken before the bound bit read.
            if (!vector_index_ok(host, table.names, i, 3)) {
                host.invalid_parameter_00bf6713();
            }
            name = global_ptr(host, table.names) + i * 8;
            Pool23BitIterator at{};
            Pool23BitIterator* it = host.bit_vector_iterator_at_0048f930(host.global_address(0x00E196FC), &at, i);
            flag = host.bit_iterator_deref_0048f230(it) ? 1u : 0u;
        } else {
            // 00626EB8..00626F82: begin() of the vector<bool>, += i, then the checked dereference.
            std::byte* const bits = host.global_address(table.bits);
            std::uint32_t* const words = reinterpret_cast<std::uint32_t*>(load_ptr(bits, 0x8));
            if (reinterpret_cast<std::byte*>(words) > load_ptr(bits, 0xC)) {
                host.invalid_parameter_00bf6713();
            }
            Pool23BitIterator it{bits, words, 0};
            host.bit_iterator_advance_0048d3b0(&it, i);
            if (it.container == nullptr) {
                host.invalid_parameter_00bf6713();
                host.invalid_parameter_00bf6713();
                host.invalid_parameter_00bf6713();
            } else if (it.word == nullptr) {
                host.invalid_parameter_00bf6713();
            }
            const std::byte* const first_word = load_ptr(it.container, 0x8);
            if (first_word > load_ptr(it.container, 0xC)) {
                host.invalid_parameter_00bf6713();
            }
            const std::int32_t word_bytes =
                static_cast<std::int32_t>(reinterpret_cast<const std::byte*>(it.word) - first_word);
            const std::uint32_t bit_index = (static_cast<std::uint32_t>(word_bytes >> 2) << 5) + it.offset;
            if (bit_index >= load<std::uint32_t>(it.container, 0)) {
                host.invalid_parameter_00bf6713();
            }
            flag = (*it.word & (1u << (it.offset & 31))) != 0 ? 1u : 0u;
            if (!vector_index_ok(host, table.names, i, 3)) {
                host.invalid_parameter_00bf6713();
            }
            name = global_ptr(host, table.names) + i * 8;
        }
        store<std::uint32_t>(host.option_map_subscript_00625f50(map, name), 0x8, flag);
    }
}

}  // namespace

// 00626C50 BSP_LuaOptionTables_ApplyDefaultStrings
// Evidence: listing 00626C50..00627655 (749 insns, export assembly.txt). rec = 004B4750(ECX =
// [00E188A8]); key = [rec+250h]. Three option tables are walked in turn: names 00E196D0/E0/F0,
// owner ids 00E1973C/4C/5C, default strings 00E1976C/7C/8C. For every index whose owner equals
// key, map[name] (00625F50) is assigned the default string and map[name]+8 the table's flag bit:
// table 1 through the bound vector<bool> 00E196FC helpers 0048F930/0048F230, tables 2 and 3 by an
// inlined checked read of vector<bool> 00E19710 / 00E19724 after 0048D3B0. Every vector access is
// _SECURE_SCL checked (00BF6713). The maps are rec+204h/210h/21Ch when [rec+1E4h] != 0 and
// rec+228h/234h/240h otherwise; a normalized diff of 00626C90..00627169 against 00627170..00627649
// shows the two halves differ only in those six offsets, and tables 2 and 3 differ only in globals.
// ABI: void(void), plain RET. Callers reach it through host virtuals objectives_refresh_00626c50 /
// objective_lists_reset_00626c50 (implemented elsewhere; not replaced).
// Uncertainty: "option tables / default strings / flag" are readings of the shapes; the null-container
// path after 0048D3B0 dereferences null exactly as the image does (it never returns there in practice).
void lua_option_tables_apply_default_strings_00626c50(Pool23Host& host) {
    std::byte* const record = host.mission_scoring_get_commit_record_004b4750(global_ptr(host, 0x00E188A8));
    const std::uint32_t key = load<std::uint32_t>(record, 0x250);
    const std::uint32_t base = load<std::uint32_t>(record, 0x1E4) != 0 ? 0x204u : 0x228u;
    const OptionTableGlobals tables[3] = {
        {0x00E196D0, 0x00E1973C, 0x00E1976C, 0, base},
        {0x00E196E0, 0x00E1974C, 0x00E1977C, 0x00E19710, base + 0xC},
        {0x00E196F0, 0x00E1975C, 0x00E1978C, 0x00E19724, base + 0x18},
    };
    for (const OptionTableGlobals& table : tables) {
        apply_option_table(table, record, key, host);
    }
}

// 006AC5D0 BSP_ShipAi_LandPadApproachPoint, as one body
// Evidence: listing 006AC5D0..006ACB35 (370 insns, export assembly.txt), stack offsets tracked from
// the aligned frame (B = ESP after PUSH EDI). The facing (006AC260) is read first. When [pad+208h]
// differs from the zone group the landing line is rebuilt: [+208h] = group; the pose is refreshed
// (00414DB0) when byte [+0C8h] is 0; origin [+20Ch..214h] = position [+0FCh..104h]; astern [+218h]
// = 1000.0 (00CE3804); run [+21Ch] = 800.0 (00CE3950); group = 004120D0(004218E0(), id), and when
// nonzero: refresh again, a ray along the facing (+100000.0 00CF81F0 from outside any zone through
// 0041B4E0, -100000.0 00CF81F8 inside one through 00416DD0); a hit moves the origin to hit - facing;
// [+20Ch] = x, [+210h] = +0.0 (XORPS), [+214h] = z; a back-cast sets [+218h] = |origin - hit|
// (00414C60); a cast from 5.0 (00D7A370) ahead to [+21Ch] ahead sets [+21Ch] = |origin - hit| *
// 0.89999997615814209 (00D7A390). The per-call arm: along = dz*fz + dx*fx, across =
// |dz*fx + (-0.0 - fz)*dx| (00D7A208, SSE). Outside the corridor (across > hw, along > hw or
// -along > [+21Ch], each ordered) the point is d = ([+218h] > hw ? hw : [+218h]) behind the origin;
// inside it offset = 00419010(0.5, 1, 1, 0, across/hw) * hw - along, raised to -[+218h], ahead of
// the origin. y is [+210h] +/- 0.0 (double 00D7A258).
// x87: the three FMUL ST1 sites (006AC6CC, 006AC727, 006AC86A) are DC C9, st(1) = st(1) * st(0),
// checked with tools/x87_dc_form_scan.py. 24-bit precision control is modelled as float arithmetic.
// ABI: __thiscall(ECX = pad, out*, from*, zone group id, float half_width), RET 10h.
// Host overlap: the live path uses refresh_pad_line_006ac5d0 (src/building_pads.cpp) plus
// ship_ai_land_pad_approach_point_006ac5d0 (src/ship_ai_follow_land.cpp) through the host virtual
// pad_approach_point_006ac5d0; this body is not wired in. Against those it adds the two 00414DB0
// pose refreshes, the y output, the fourth 00416DD0 argument and the NaN order of the hold-off min.
// Uncertainty: the hit buffers are uninitialised in the image and zeroed here; the fourth 00416DD0
// argument's role is unknown (it points at the far-point temporaries).
void ship_ai_land_pad_approach_point_whole_006ac5d0(std::byte* pad, float* out, const float* from,
                                                    std::uint32_t zone_group, float half_width,
                                                    Pool23Host& host) {
    float facing_buffer[3] = {};
    const float* facing = host.entity_forward_xz_normalized_006ac260(pad, facing_buffer);
    const float fx = facing[0];
    const float fz = facing[2];

    if (load<std::uint32_t>(pad, 0x208) != zone_group) {
        store<std::uint32_t>(pad, 0x208, zone_group);
        if (load<std::uint8_t>(pad, 0xC8) == 0) {
            host.entity_pose_refresh_world_00414db0(pad);
        }
        store<float>(pad, 0x20C, load<float>(pad, 0xFC));
        store<float>(pad, 0x210, load<float>(pad, 0x100));
        store<float>(pad, 0x214, load<float>(pad, 0x104));
        store<float>(pad, 0x218, 1000.0f);
        store<float>(pad, 0x21C, 800.0f);
        std::byte* const group = host.avoid_zone_manager_group_for_layer_004120d0(
            host.avoid_zone_manager_get_singleton_004218e0(), zone_group);
        if (group != nullptr) {
            if (load<std::uint8_t>(pad, 0xC8) == 0) {
                host.entity_pose_refresh_world_00414db0(pad);
            }
            float point[2] = {load<float>(pad, 0xFC), load<float>(pad, 0x104)};
            float hit[2] = {0.0f, 0.0f};
            bool struck = false;
            std::byte* const zone = host.avoid_zone_group_zone_containing_004178f0(group, point);
            if (zone != nullptr) {
                float scaled[2] = {fx * -100000.0f, -100000.0f * fz};
                const float far_point[2] = {scaled[0] + point[0], scaled[1] + point[1]};
                struck = host.avoid_zone_segment_hit_00416dd0(zone, point, far_point, hit, scaled);
            } else {
                const float scaled[2] = {fx * 100000.0f, 100000.0f * fz};
                const float far_point[2] = {scaled[0] + point[0], scaled[1] + point[1]};
                struck = host.avoid_zone_group_segment_hit_0041b4e0(group, point, far_point, hit);
            }
            if (struck) {
                point[0] = hit[0] - fx;
                point[1] = hit[1] - fz;
            }
            store<float>(pad, 0x20C, point[0]);
            store<float>(pad, 0x210, 0.0f);
            store<float>(pad, 0x214, point[1]);

            const float astern = load<float>(pad, 0x218);
            const float back[2] = {point[0] - astern * fx, point[1] - astern * fz};
            float back_hit[2] = {0.0f, 0.0f};
            if (host.avoid_zone_group_segment_hit_0041b4e0(group, point, back, back_hit)) {
                const float gap[2] = {point[0] - back_hit[0], point[1] - back_hit[1]};
                store<float>(pad, 0x218, host.vector2_length_00414c60(gap));
            }

            const float start[2] = {fx * 5.0f + point[0], 5.0f * fz + point[1]};
            const float run = load<float>(pad, 0x21C);
            const float end[2] = {point[0] + fx * run, point[1] + fz * run};
            float run_hit[2] = {0.0f, 0.0f};
            if (host.avoid_zone_group_segment_hit_0041b4e0(group, start, end, run_hit)) {
                const float gap[2] = {point[0] - run_hit[0], point[1] - run_hit[1]};
                store<float>(pad, 0x21C, host.vector2_length_00414c60(gap) * 0.89999997615814209f);
            }
        }
    }

    const float dx = load<float>(pad, 0x20C) - from[0];
    const float negative_fz = -0.0f - fz;
    const float dz = load<float>(pad, 0x214) - from[2];
    const float along = dz * fz + dx * fx;
    const float across = std::fabs(dz * fx + negative_fz * dx);

    if (across > half_width || along > half_width || -along > load<float>(pad, 0x21C)) {
        const float astern = load<float>(pad, 0x218);
        const float back = astern > half_width ? half_width : astern;
        out[0] = load<float>(pad, 0x20C) - fx * back;
        out[1] = load<float>(pad, 0x210) - 0.0f;
        out[2] = load<float>(pad, 0x214) - back * fz;
        return;
    }
    const float blend = host.math_interpolate_clamped_00419010(0.5f, 1.0f, 1.0f, 0.0f, across / half_width);
    float offset = blend * half_width - along;
    const float floor = -load<float>(pad, 0x218);
    if (floor > offset) {
        offset = floor;
    }
    out[0] = load<float>(pad, 0x20C) + fx * offset;
    out[1] = load<float>(pad, 0x210) + 0.0f;
    out[2] = load<float>(pad, 0x214) + offset * fz;
}

}  // namespace bsp
