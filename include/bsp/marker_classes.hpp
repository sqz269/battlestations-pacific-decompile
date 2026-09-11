#pragma once

#include "bsp/native_string.hpp"
#include "bsp/panel_sequence_types.hpp"

#include <cstdint>
#include <map>

struct lua_State;
namespace bsp {

// Native20h descriptor, with the native vtable address retained as identity.
// Resource+14 owns one intrusive reference returned by the resource manager.
// NativeString fields have no implicit cleanup. Do not copy/move live owners.
struct MarkerClassDescriptor {
    std::uint32_t native_vtable_00;
    NativeString name_04;
    NativeString mesh_0c;
    void* resource_14;
    std::int32_t animation_18;
    std::uint8_t synchronized_1c;
};

// Semantic standard-map projection of fixed native registry00E19974.
// count_08 is the native exposed count, independent of std::map::size during
// teardown. Existing key/value cells retain identity on duplicate insertion.
using MarkerClassMap = std::map<NativeString, MarkerClassDescriptor*, PanelSequenceNameLess>;
struct MarkerClassRegistry {
    MarkerClassMap entries;
    std::uint32_t count_08{};
};

struct MarkerClassHost {
    virtual ~MarkerClassHost() = default;
    // Resolve CURRENT[00E188A8]+1A0C, an embedded Lua owner, once at loader
    // entry. Return its already-open state. This is NOT mission_lua+1A08.
    virtual lua_State& current_game_lua_1a0c() = 0;
    virtual void* current_game_resource_factory_007175d0() = 0;
    virtual void* current_resource_manager_004c1400() = 0;
    // Individual unreconstructed resource operation. Receives the captured
    // factory, current manager and exact mutable temporary NativeString.
    // Return null or one already-retained actual intrusive resource (refs+4).
    virtual void* load_and_cache_resource_00b80720(void* manager,
        NativeString& name, void* factory) = 0;
    // Invoked after a concrete InterlockedDecrement(resource+4) reaches zero.
    // Dispatch the resource's CURRENT virtual+0, ECX=resource, no stack args.
    virtual void resource_zero_references(void*) = 0;
};
struct MarkerClassContext {
    MarkerClassHost& host;
    MarkerClassRegistry& registry_00e19974;
    NativeStringStorage& strings;
    const bool& crt_sse2_conversion;
};

//007188A0 ECX=mutable name, RET/EAX=resource. Captures factory before manager.
void* load_marker_resource_007188a0(NativeString&, MarkerClassHost&);
//006D8FC0 ECX=fresh20h descriptor; name*/mesh*/Anim/sync-byte stack; RET10.
MarkerClassDescriptor& construct_marker_class_006d8fc0(MarkerClassDescriptor&,
    const NativeString& name, const NativeString& mesh, std::int32_t animation,
    std::uint8_t synchronized, MarkerClassContext&);
//006D8BC0 ECX=descriptor, RET; resource then mesh then name. String headers
// remain unchanged; resource nulling follows its zero-reference callback.
void destroy_marker_class_006d8bc0(MarkerClassDescriptor&, MarkerClassContext&);
//006D90E0 ECX=descriptor; flags stack; RET4/EAX=original pointer (possibly freed).
MarkerClassDescriptor* scalar_delete_marker_class_006d90e0(MarkerClassDescriptor*,
    std::uint32_t flags, MarkerClassContext&);
//006D8460 ECX=registry; name* stack; RET4/EAX=node. Standard-map lower_bound.
MarkerClassMap::iterator lower_bound_marker_class_006d8460(MarkerClassRegistry&,
    const NativeString&);
//006DBC10 same input ABI; EAX=pointer cell node+14, RET4. Miss inserts null,
// independently copying the key; existing descriptor is neither freed nor read.
MarkerClassDescriptor*& lookup_marker_class_006dbc10(MarkerClassRegistry&,
    const NativeString&, NativeStringStorage&);
//006DBEB0 no native arguments, RET. Borrow current globals, read MarkerClasses,
// allocate/construct each descriptor then overwrite its registry pointer cell.
// Duplicate overwrite does NOT destroy the prior descriptor. No Lua open/close
// or script execution occurs here. Parent wires its current game owner getter.
void load_marker_classes_006dbeb0(MarkerClassContext&);
//006DB2F0 no native arguments, RET. Delete currently registered descriptors in
// ascending key order, null each captured cell AFTER deletion, then release
// registry keys in reverse order and set count0. Overwritten pointers are not
// retained by native code and cannot be recovered by this cleanup.
void clear_marker_classes_006db2f0(MarkerClassContext&);

} // namespace bsp
