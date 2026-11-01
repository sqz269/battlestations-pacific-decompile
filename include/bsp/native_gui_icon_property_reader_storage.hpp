#pragma once
#include "bsp/native_gui_widget_property_children.hpp"
#include "bsp/native_gui_icon_property_state_invocations.hpp"
#include "bsp/native_lua_reader_fields.hpp"
#include "bsp/object_handle_resolvers.hpp"

namespace bsp {
namespace game { class GameNativeReadOnlyData; }

// Cold borrows of the actual13 PE literals and mapped reader/Icon tables.
// data_at authenticates original recorded addresses; no strings are copied or
// manufactured. The actual read-only owner stays live through all calls.
class NativeGuiIconPropertyReaderLiterals final {
public:
    explicit NativeGuiIconPropertyReaderLiterals(const game::GameNativeReadOnlyData&);
    const char* key(std::uint32_t actual_literal_index) const;
private:
    const game::GameNativeReadOnlyData& data_;
    const char* const keys_[13];
};

// Two genuine arrays, not a fabricated four-word subarray reference. Logical
// Native S-18 key pair and S-10..-1 four homes overlap the reached outgoing
// 0C/10/14/04/A4 packets exactly. Unwritten bool payload high bytes persist.
struct NativeGuiIconPropertyReaderCallHomes {
    std::uint32_t key_pair_18[2];
    std::uint32_t actual_homes_10[4];
};
static_assert(offsetof(NativeGuiIconPropertyReaderCallHomes, actual_homes_10) == 8);
static_assert(sizeof(NativeGuiIconPropertyReaderCallHomes) == 24);

// Caller-retained ordinary backing, no implicit Native cleanup or home seeds.
// Logical S10 metadata;S14 name8;S1C size2;S24 pivot2;S2C UV4;S44 state.
// Native saved registers/exception links/return address are not replicated.
// Lookup/query/enter scratch is actual fresh/unbound Lua storage; each normal
// helper destroys it before reuse. Existing resolver/child-frame policies stay.
struct NativeGuiIconPropertyReaderStorageFrame {
    NativeGuiIconPropertyReaderStorageFrame() {}
    NativeGuiIconPropertyReaderStorageFrame(const NativeGuiIconPropertyReaderStorageFrame&) = delete;
    NativeGuiIconPropertyReaderStorageFrame& operator=(const NativeGuiIconPropertyReaderStorageFrame&) = delete;
    NativeGuiIconPropertyReaderStorageFrame(NativeGuiIconPropertyReaderStorageFrame&&) = delete;
    NativeGuiIconPropertyReaderStorageFrame& operator=(NativeGuiIconPropertyReaderStorageFrame&&) = delete;
    NativeGuiIconPropertyReaderCallHomes calls;
    std::uint32_t metadata_10, name_14[2], size_1c[2], pivot_24[2], uv_2c[4], state_44;
    std::uint32_t captured_profile, captured_target; // Source history only.
    NativeGuiWidgetPropertyScratch base;
    NativeGuiWidgetPropertyChildrenAcquired base_children;
    NativeLuaReaderFieldScratch field;
    NativeLuaObjectStorage query, enter;
    NativeLuaFieldResolverFrame resolver;
};

struct NativeGuiIconPropertyReaderStorageContext {
    NativeLuaReaderStorage& reader; // genuine actual object, not a raw cast
    NativeLuaFieldValueBindings& fields;
    NativeLuaFieldResolverAdapter& resolvers;
    NativeGuiWidgetPropertyInteractionContext& base_interaction;
    NativeGuiWidgetPropertyChildrenRawContext& base_children;
    NativeGuiIconPropertyStateInvocations& states;
    const NativeGuiIconPropertyReaderLiterals& literals;
    // Required initialized caller-owned PRIVATE Source ABI input. Every DWORD
    // bit pattern is admitted. No default/Native parameter snapshot/Original
    // stack identity or authority is asserted. Read CURRENT after prepare and
    // copy to the four inherited tag homes before37C3; their later native
    // lowBYTE writes/FULL DWORD reads remain unchanged.
    const volatile std::uint32_t (&state_private_preimages)[4];
};

// Whole AB3310 ordinary normal payload/call schedule,1319B/398instructions.
// Original ECX Icon138 + one mutable visitor DWORD HOME/RET4 becomes this new
// Source API. actual_visitor_home MUST be ordinary DWORD backing borrowed via
// a volatile view: raw field helpers may legally write the SAME object through
// their ordinary/memcpy paths. Capture its real reader identity once before
// AAA710. Context.reader is that SAME already-constructed actual reader.
// PartialDisplayType then overwrites this ORIGINAL home;3449 reloads it into
// Icon110. No replacement integer, seed0, pointer-to-reader cast or home restore.
// Unsupported/no-write field paths can preserve original visitor bits.
// This incoming-home object is separate from Frame and its ordinary subobjects;
// the declared outgoing-packet aliases do not alias private Source controls.
//
// Reached packet stores, current profile/target data loads, query/enter/pop and
// private output backing stay live. Direct calls use existing genuine actual
// reader/resolver suppliers; data words NEVER become callable pointers. Caller
// proves those current edges belong to the existing supplied reader class and
// the captured known IconA4 is genuine66 over this SAME canonical companion.
// Arbitrary OTHER visitor/A4 transport and production Page/type6 bindings stay
// outside this supplied Source domain, not replaced by defaults/profile gates.
// Source-only prepare runs after UVleave, before captureA4; it constructs real
// records, then CURRENT4DWORD preimages are copied, homes staged, real66 called.
// No capped loop/record pool or hidden callback/factory is substituted.
//
// State0 is stored BEFORE Texture's constructor field call. SAME name header
// is already LEN0/DATA0. Ordinary C++ catch covers that call and later body;
// actual tag0 field/store/resize order keeps header0/0 or current valid ownership
// on admitted C++ escape (get/allocate before publication; return getter before
// noexcept actual pool return; no escaping C++ call between DATA/LEN publication).
// Unpublished allocation and Lua/other provider debt remain independently live.
// Catch calls real41DD20 on SAME header then barethrows only after normal cleanup.
// No reader pop/rollback/replay/new terminate or Native Origin is inferred.
// Lua nonlocal transfers, hardware faults/unmaskedFP, arbitrary invalid/corrupt
// callback outputs and Original FH3/SEH/secondary identity remain unproved.
// Normal captureDATA precedes state-1 and CURRENTLEN+1/getter/return; this footer
// is OUTSIDE the catch. Never replay it on a later failure.
//
// Keep distinct Frame/reader/Lua tracking/storage/preimages/RO tables/services
// and actual Icon/vector/resource/canonical lifetime through nested calls and
// any escape. Source private controls/carriers are disjoint from provider-write
// extents; packet/payload aliases must satisfy the actual helpers' valid domains.
// Failed/partial/completed graph and external debt stay retained, even after
// name cleanup consumes a block. Existing helper normal-domain/secondary/SDK
// policies remain qualified; this adds no Native owner/refcount/NoThrow/FP reset,
// Original ABI/FLAGS/fault/runtime/activation or extra Original/vendor/helper/CG
// credit. Whole AAA710 still needs its genuine actual derived child providers.
void read_native_gui_icon_properties_storage_00ab3310(void* actual_icon138,
    volatile std::uint32_t& actual_visitor_home,
    NativeGuiIconPropertyReaderStorageFrame&, NativeGuiIconPropertyReaderStorageContext&);
} // namespace bsp
