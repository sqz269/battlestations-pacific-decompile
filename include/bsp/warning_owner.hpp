#pragma once

#include "bsp/gui_lua_runtime.hpp"
#include "bsp/lua_script_runtime.hpp"
#include "bsp/lua_state_owner.hpp"
#include "bsp/mission_events.hpp"
#include "bsp/native_hardware_layout_tree.hpp"
#include "bsp/native_string.hpp"
#include "bsp/random_threads.hpp"
#include "bsp/warning_messages.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <list>
#include <optional>

namespace bsp {
inline constexpr std::uint32_t kWarningOwnerNativeAllocationSize = 0x1b0;
inline constexpr std::uint32_t kWarningOwnerNativeVtable = 0x00d1b968;

// Actual native headers for the separate trees whose payload operations are
// outside this packet. These are authoritative storage, not mirrors of the
// canonical message/escape tables or the canonical pending/applied queues.
struct WarningOpaqueTreeHeader {
    std::uint32_t allocator_00;
    void* head_04;
    std::uint32_t count_08;
};
static_assert(sizeof(WarningOpaqueTreeHeader) == 0x0c);
static_assert(offsetof(WarningOpaqueTreeHeader, head_04) == 4);

// Explicit allocation contents for the words untouched by0098A020.
// Logical flags reuse WarningManagerState's existing bool projection; native
// padding and allocator words have separate uninterpreted storage below.
struct WarningOwnerAllocationWords {
    std::array<std::uint32_t, 8> clock_words_04_20;
    std::uint32_t unconsumed_34;
    std::uint32_t suppression_word_d0;
    std::uint32_t channel_flags_104;
    std::uint32_t air_raid_deadline_174;
    std::uint32_t air_raid_active_178;
    std::uint32_t collision_deadline_17c;
    std::uint32_t collision_active_180;
    std::uint32_t periodic_accumulator_198;
    // Native header offsets:28,D4,E0,EC,F8,108,124,138,144,150,15C,168,184,1A4.
    std::array<std::uint32_t, 14> allocator_words;
};

// Canonical C++ projection. sizeof(WarningOwner) is not native1B0h; the parent
// allocates host sizeof, calls the recovered constructor, publishes game+21E0,
// then initializes. Shell initialization is not a second native constructor.
struct WarningOwner {
    explicit WarningOwner(const WarningOwnerAllocationWords&) noexcept;
    WarningOwner(const WarningOwner&) = delete;
    WarningOwner& operator=(const WarningOwner&) = delete;
    std::uint32_t native_vtable_00;
    std::array<float, 8> clocks_04_20;
    TrackedCriticalSection* critical_section_24{};
    std::optional<std::list<std::uint32_t>> roster_28;
    std::uint32_t unconsumed_34;
    std::array<NativeString, 19> strings_38; // exactly0x13 elements, ends at nativeD0
    WarningManagerState state; // D0,E0,EC,174,178,17C,180,198,19C
    std::array<std::byte, 3> unconsumed_d1_d3;
    WarningOpaqueTreeHeader tree_d4;
    WarningOpaqueTreeHeader channels_f8;
    std::uint32_t channel_flags_104;
    std::optional<WarningMessageTables> tables_108; //108,114,11C,130
    WarningOpaqueTreeHeader tree_144;
    WarningOpaqueTreeHeader tree_150;
    WarningOpaqueTreeHeader tree_15c;
    WarningOpaqueTreeHeader tree_168;
    WarningOpaqueTreeHeader tree_184;
    void* environment_us_horn_190{};
    void* environment_air_raid_194{};
    WarningOpaqueTreeHeader tree_1a4;
    // Unconsumed native allocator words for the six canonical projected
    // containers:28,E0,EC,108,124,138. No fabricated native shadow sentinels.
    std::array<std::uint32_t, 6> canonical_allocator_words;
};

using WarningRosterIterator = NativeHardwareLayoutTreeIterator;
struct WarningOwnerServices {
    virtual ~WarningOwnerServices() = default;
    // Existing source-tree library boundaries. Both inputs are actual checked
    // owner/node words; outer sentinel byte29D, inner sentinel byte15. The
    // implementation must use its actual container/iterator, not invent data.
    virtual void increment_outer_00581290(WarningRosterIterator&) = 0;
    virtual void increment_inner_0057f510(WarningRosterIterator&) = 0;
    // Native00871BA0 writes out and returns&out; caller later releases that
    // temporary reference. Resource allocation/lookup remains external.
    virtual void acquire_effect_00871ba0(void*& out, const NativeString&, int flags) = 0;
    // Fresh actual effect vtable+0 dispatch after InterlockedDecrement reaches0.
    virtual void effect_zero_references(void*) = 0;
    // Exact hook targets; receive the singleton freshly read by the thunk.
    virtual void call_00985c50(WarningOwner&, std::uint32_t ecx_argument) = 0;
    virtual void call_00985250(WarningOwner&, std::uint32_t ecx_argument,
        std::uint32_t edx_argument) = 0;
};
struct WarningOwnerContext {
    NativeStringStorage& strings;
    WarningOwnerServices& services;
    WarningOwner*& singleton_f8a0c4;
    void*& world_e188a8; // actual world; tree pointer+6B4 is reloaded each outer comparison
    void*& hook_owner_f8bbcc; // actual owner; native hook DWORD at+230
    std::uint32_t& hook_f8bf4c;
    const SingletonLifetimeCallbacks& invalid_parameters;
};

//0098A020 ECX=fresh owner; EAX=this; RET0098A295. Real constructor, not a
//compiler-generated array helper. Exact native node sizes/flags are recorded.
WarningOwner& construct_warning_owner_0098a020(WarningOwner&);
//00973F20 ECX=owner; RET00974065. Clear destination, traverse actual nested
//source trees and append each inner node+C DWORD, preserving live global reads.
void rebuild_warning_roster_00973f20(WarningOwner&, WarningOwnerContext&);
//009870A0 ECX=owner; RET00987584. Full normal init sequence, including retained
//Lua scopes, effect reference order, hook publication and all scalar resets.
void initialize_warning_owner_009870a0(WarningOwner&, LuaStateOwnerEnvironment,
    LuaScriptRuntime&, WarningOwnerContext&);
// Installed native callback identities00987080/90. New host interfaces, not
//drop-in x86 thunks; a dispatcher binds these routines to those identities.
void warning_hook_00987080(WarningOwnerContext&, std::uint32_t ecx_argument);
void warning_hook_00987090(WarningOwnerContext&, std::uint32_t ecx_argument,
    std::uint32_t edx_argument);

// Host cleanup convenience, not native0098A2A0. Only the empty opaque trees
//created by this packet can be discarded here; populated-tree destruction is
//an explicit external boundary. Call before destroying the C++ shell.
void release_warning_owner_host_storage(WarningOwner&, WarningOwnerContext&);
} // namespace bsp
