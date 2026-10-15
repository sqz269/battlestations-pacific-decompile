#pragma once

#include <cstddef>
#include <cstdint>

#include "bsp/cc13_avoid1.hpp"
#include "bsp/cc13_left55.hpp"
#include "bsp/cc13_x87c.hpp"

namespace bsp {

// The separate path list consumed here, not the manager's hull list or the
// TerrainGridLayer registry list. 0041C0B0 produces these three node words.
struct NativeZoneLeafPathListNode {
    NativeZoneLeafPathListNode* next;       // +00h
    NativeZoneLeafPathListNode* previous;   // +04h
    Cc13X87cContourPathNative* path;         // +08h
};
struct NativeZoneLeafPathList {
    std::uint32_t proxy;                    // +00h, untouched here
    NativeZoneLeafPathListNode* head;        // +04h
    std::uint32_t size;                     // +08h
};
static_assert(sizeof(NativeZoneLeafPathListNode) == 0x0C);
static_assert(sizeof(NativeZoneLeafPathList) == 0x0C);

// Only the touched parameter prefix is established; its enclosing producer is
// unknown. Reuse the 00880420 shore-mask prefix instead of giving its fields
// conflicting meanings. The input cell_size is saved once and reused for every owner.
struct NativeZoneLeafParams {
    cc13_left55::ShoreMaskParams shore;      // +00h..+13h
    float rounded_unit_length;              // +14h, produced here
    NativeZoneLeafPathList paths;            // +18h..+23h
};
static_assert(sizeof(cc13_left55::ShoreMaskParams) == 0x14);
static_assert(offsetof(NativeZoneLeafParams, rounded_unit_length) == 0x14);
static_assert(offsetof(NativeZoneLeafParams, paths) == 0x18);
static_assert(sizeof(NativeZoneLeafParams) == 0x24);

// Partial scene-chain view: the producer and word +00h are not established.
// The leaf reads [[[00E188A8]+19CCh]+34Ch] even with an explicit owner.
struct NativeZoneLeafSceneNode {
    std::uint32_t unread_00;
    NativeZoneLeafSceneNode* next;           // +04h
    Cc13X87cContourOwnerNative* owner;        // +08h
};
static_assert(sizeof(NativeZoneLeafSceneNode) == 0x0C);

// Mandatory dependencies, with no defaults or success substitutes. These are
// native call boundaries, not recovered C++ member declarations. A caller may
// provide compatible adapters over the existing reconstructions. In particular
// unit_length is called directly from x86 assembly so ST0 reaches FSTP32 without
// an intervening C++ arithmetic conversion. All callback backing and the two
// pointed-to actual globals must remain live; bindings/control storage must be
// separate from the native records. No validation or admission guards are added.
struct NativeZoneLeafBindings {
    void* const* actual_00e188a8;
    const volatile std::uint32_t* actual_0109eea4;

    // 004120D0 consumes the incoming manager in ECX, signed layer on stack.
    Cc13AvoidZoneNativeGroup* (__thiscall* group_for_layer_004120d0)(
        Cc13AvoidZoneManagerNative*, std::int32_t);
    void (__thiscall* remove_owner_0041d150)(Cc13AvoidZoneNativeGroup*, void*);
    float (__thiscall* unit_length_00880250)(Cc13X87cContourOwnerNative*);
    void* (__cdecl* allocate_00bf55be)(std::uint32_t);
    void (__cdecl* free_00bf6989)(void*);
    void (__thiscall* shore_mask_00880420)(Cc13X87cContourOwnerNative*,
        std::int8_t**, std::int32_t, const cc13_left55::ShoreMaskParams*);
    // Both callees overwrite ECX before reading it; the leaf's saved/forwarded
    // manager register is dead at these two boundaries. RET 0Ch / RET 8.
    Cc13X87cContourPathNative* (__stdcall* trace_00421ce0)(
        Cc13X87cContourOwnerNative*, std::int8_t* const*, std::int32_t);
    int (__cdecl* format_00412f80)(char*, const char*, ...);
    void* (__cdecl* pool_00419cc0)();
    char* (__thiscall* pool_allocate_00bd1120)(void*, std::uint32_t, std::uint32_t);
    void (__thiscall* pool_return_00bd1510)(void*, void*, std::uint32_t, std::uint32_t);
    void* (__cdecl* copy_00bf7680)(void*, const void*, std::uint32_t);
    void (__stdcall* simplify_0041ea30)(Cc13X87cContourPathNative*,
        const NativeZoneLeafParams*);
    // 0041C0B0 does not read ECX, but preserve the caller's list register too.
    NativeZoneLeafPathListNode* (__thiscall* buy_node_0041c0b0)(
        NativeZoneLeafPathList*, NativeZoneLeafPathListNode*,
        NativeZoneLeafPathListNode*, Cc13X87cContourPathNative* const*);
    void (__thiscall* grow_list_0041fdd0)(NativeZoneLeafPathList*, std::uint32_t);
};

// Complete bounded orchestration of 00423640..0042395F. Original ECX manager,
// stack requested_owner/params, RET 8; new Source API below is not that ABI.
// Names are hypotheses. There are no known callers and no gameplay-path claim.
// All records and callback-visible spans must be valid when the native schedule
// reaches them. The output list is already constructed. No local EH cleanup is
// added: a throwing dependency can leave columns, a path, or a node outstanding.
// CVTTSS2SI and the x87 ratio retain hardware exceptional/rounding behavior;
// BF7420 reads the supplied actual mutable mode word at every conversion.
// Evidence and remaining limits: docs/GOAL15_ZONE_LEAF.md.
void generate_native_zone_paths_00423640(Cc13AvoidZoneManagerNative& manager,
    Cc13X87cContourOwnerNative* requested_owner, NativeZoneLeafParams& params,
    const NativeZoneLeafBindings& bindings);

} // namespace bsp
