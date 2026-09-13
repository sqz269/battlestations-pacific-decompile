#pragma once

#include "bsp/native_unit_observer_endpoint.hpp"
#include "bsp/native_lua_objects.hpp"
#include "bsp/pose_refresh.hpp"

namespace bsp {

// Borrowed lvalues on the SAME existing canonical unit; no new native owner,
// endpoint, pose, name or cached-key snapshot. PoseRefreshView must reference
// that unit's actual74/C8/CC/10C fields, including parent resolution.
struct NativeUnitKilledBaseView {
    NativeUnitObserverAlias alias;
    void* volatile& owner_30;
    volatile std::uint32_t& type_c4;
    PoseRefreshView& pose;
    volatile std::uint32_t& name_length_154;
    const char* volatile& name_data_158;
    volatile std::uint16_t& network_id_174;
    volatile std::uint32_t& self_key_length_178;
};

// The actual holder returned by004C1570 supplies its own+4 lvalue. This view
// is not another8-byte lock owner and does not create a publication/domain.
struct NativeUnitKilledLockView {
    TrackedCriticalSection* const volatile& section_04;
};

class NativeUnitKilledBaseAccess {
public:
    virtual ~NativeUnitKilledBaseAccess() = default;
    // REQUIRED full getter004C1570 for actualF878FC: manager-lock double
    // check,8-byte owner construction4BD150, publication, second manager
    // registration, unlock, current publication. Caller then captures+4.
    virtual NativeUnitKilledLockView lock_owner_004c1570() = 0;
    // REQUIRED full927B40: actual[E188A8]+1A0C Lua owner, globals.thisTable,
    // then CURRENT unit NativeString178 key, temporary object cleanup. Builds
    // a fresh actual14h output at this address; ECXunit/outputstack/RET4.
    virtual void construct_self_00927b40(const NativeUnitObserverAlias&,
        NativeLuaObjectStorage& fresh_output) = 0;
    // REQUIRED native wrapper contracts over actual Lua stack objects.
    // B67530/B67580/B67400: checkstack(2), push key current length/data,
    // push lightuserdata/newtable/float32 converted to Lua number, settable
    // using CURRENT owner/index after callbacks. No private table/shadow state.
    virtual void set_lightuserdata_00b67530(NativeLuaObjectStorage&,
        const NativeString& key, void* value) = 0; // RET8
    virtual void set_new_table_00b67580(NativeLuaObjectStorage&,
        const NativeString& key) = 0; // RET4
    virtual void set_number_00b67400(NativeLuaObjectStorage&,
        const NativeString& key, float value) = 0; // RET8
    virtual void call_virtual_134(const NativeUnitObserverAlias&,
        std::uint32_t captured_profile) = 0; // same ECXunit, no arguments
};

struct NativeUnitKilledBaseContext {
    NativeStringRawPoolContext& strings;
    // Borrow the actual type-name pointer table and empty-name byte address.
    // Indexing is unchecked native32, with no invented bounds/null fallback.
    const char* const volatile* type_names_00e0cd80;
    const char* empty_name_00f89ac8;
    NativeUnitKilledBaseAccess& access;
};

// Complete normal00928C80..00928F44[709]. Native ECXunit, RET; source ABI
// differs. Key-length gate; captured section; actual self/LastPosition objects,
// pooled scratch strings; stale-pose refresh; captured y,x,z; number writes;
// reverse object cleanup/unlock; exact inert4254B0 argument loads; reload+30,
// current profile slot134 when nonnull; distinct exact inert923050 tail call.
// Native eight-state FH3/mapDDA508 cleanup is documented; source guards do
// completed-object cleanup only. Original FH3/SEH and FPU status unproved.
// No runtime binding, semantic-unit cast or default external provider.
void killed_native_unit_base_00928c80(
    NativeUnitKilledBaseView, NativeUnitKilledBaseContext&);

} // namespace bsp
