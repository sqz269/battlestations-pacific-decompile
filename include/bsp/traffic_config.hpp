#pragma once

#include "bsp/lua_script_runtime.hpp"
#include "bsp/lua_state_owner.hpp"
#include "bsp/native_string.hpp"
#include "bsp/panel_sequence_types.hpp"
#include "bsp/random_threads.hpp"

#include <array>
#include <cstdint>
#include <map>
#include <optional>

namespace bsp {
// Actual native 0Ch header. The four runtime payload types remain unknown.
struct TrafficRuntimeTreeHeader {
    std::uint32_t allocator_word;
    void* head;
    std::uint32_t count;
};
static_assert(sizeof(TrafficRuntimeTreeHeader) == 0x0c);
struct TrafficGroupListNode {
    TrafficGroupListNode* next;
    TrafficGroupListNode* previous;
    void* payload; // sentinel payload is untouched and never read here
};
struct TrafficGroup {
    std::uint32_t native_vtable_00;
    std::uint32_t word_04;
    std::uint32_t word_08;
    std::uint32_t allocator_word_0c;
    TrafficGroupListNode* head_10;
    std::uint32_t count_14;
    float value_18;
    float value_1c;
};
static_assert(sizeof(TrafficGroup) == 0x20);
struct TrafficGroupAllocationWords { std::uint32_t allocator_word_0c; };
struct TrafficConfigAllocationWords {
    std::uint32_t group_pointer_word_08;
    std::uint32_t word_0c;
    std::array<std::uint32_t, 6> allocator_words;
};
using TrafficStringMap = std::map<NativeString, NativeString, PanelSequenceNameLess>;

// Canonical semantic projection of the native58h owner at game+21D0. The first
// four headers/sentinels are actual native storage, not shadow std::maps. The
// two string maps use the existing standard-container semantic convention;
// their native node/iterator layout and allocation timing are not reproduced.
struct TrafficConfig {
    explicit TrafficConfig(const TrafficConfigAllocationWords&) noexcept;
    std::uint32_t native_vtable_00{};
    TrackedCriticalSection* section_04{};
    TrafficGroup* group_08;
    std::uint32_t word_0c;
    TrafficRuntimeTreeHeader runtime_tree_10;
    TrafficRuntimeTreeHeader runtime_tree_1c;
    TrafficRuntimeTreeHeader runtime_tree_28;
    TrafficRuntimeTreeHeader runtime_tree_34;
    std::uint32_t soldiers_allocator_word_40;
    std::optional<TrafficStringMap> soldiers_40;
    std::uint32_t vehicles_allocator_word_4c;
    std::optional<TrafficStringMap> vehicles_4c;
};
struct TrafficConfigContext {
    NativeStringStorage& strings;
    const TrafficGroupAllocationWords& group_allocation_words;
};

//004A43C0 ECX=fresh owner, EAX=this, RET. Initializes six containers in order,
// then creates the concrete critical section. Leaves+8,+C,allocator words alone.
TrafficConfig& construct_traffic_config_004a43c0(TrafficConfig&);
//0049C9C0 ECX=map, name* stack, RET4, EAX=borrowed node+14h string. An absent
// value starts as an empty native string; existing node/key identity survives.
NativeString& lookup_traffic_pair_0049c9c0(TrafficStringMap&, const NativeString&,
    NativeStringStorage&);
//0049D690 ECX=owner, RET. Publishes a freshly constructed20h group at+8 and
// zeroes+C before opening Lua. Repeated loads overwrite+8 without deleting the
// old group. PartyPairs.Soldiers and PartyPairs.Vehicles update existing maps.
void load_traffic_config_0049d690(TrafficConfig&, LuaStateOwnerEnvironment,
    LuaScriptRuntime&, TrafficConfigContext&);
// Host cleanup conveniences, NOT reconstructed native destructor interfaces.
// They require an empty canonical group and empty untouched runtime trees.
// Populated runtime containers/unknown group vtables are rejected before any
// owner cleanup. A constructor-only shell must have a valid/null group pointer
// supplied in its allocation words, or call this only after successful load.
void release_empty_traffic_group(TrafficGroup*);
void release_traffic_config_startup_storage(TrafficConfig&, NativeStringStorage&);
} // namespace bsp
