#pragma once

#include "bsp/gui_lua_runtime.hpp"
#include "bsp/gui_texture.hpp"
#include "bsp/lua_script_runtime.hpp"
#include "bsp/lua_state_owner.hpp"
#include "bsp/panel_sequence_types.hpp"

#include <array>
#include <map>
#include <optional>
#include <vector>

namespace bsp {

struct PowerupNativeContainer {
    std::uint32_t allocator_00;
    void* head_04;
    std::uint32_t count_08;
};
struct PowerupNativeVector {
    void* begin_00;
    void* end_04;
    void* capacity_08;
};
// Actual1C4h constructor storage. Runtime nodes are left as native allocations;
// configuration map payloads use the separate standard-container projections.
struct PowerupOwnerNativeStorage {
    std::uint32_t vtable_00;
    PowerupNativeVector base_vector_04;
    std::uint32_t flag_word_10;
    PowerupNativeContainer descriptors_14;
    std::array<PowerupNativeContainer, 8> lists_20;
    std::array<PowerupNativeContainer, 16> lists_80;
    PowerupNativeVector vector_140;
    std::array<PowerupNativeContainer, 8> maps_14c;
    PowerupNativeContainer map_1ac;
    PowerupNativeContainer random_names_1b8;
};
struct PowerupOwnerAllocationWords {
    std::uint32_t flag_word_10; // constructor clears only its low byte
    //14;20..74 stepC;80..134 stepC;14C..1A0 stepC;1AC;1B8.
    std::array<std::uint32_t, 35> allocator_words;
};

// Actual38h faction record. Constructor leaves10..20 untouched.
struct PowerupFactionConfig {
    NativeString name_00;
    NativeString description_08;
    std::uint32_t word_10;
    std::uint32_t word_14;
    std::int32_t unit_class_index_18;
    std::int32_t equipment_index_1c;
    std::uint32_t word_20;
    void* texture_24;
    std::array<float, 4> uv_28;
};
// ActualE4h descriptor. Strings own pooled buffers; texture pointers each own
// one returned intrusive reference. Loading over a texture does NOT release
// the previous pointer. Native copy constructors also do NOT retain textures.
struct PowerupClassConfig {
    NativeString name_00;
    std::int32_t group_08;
    std::array<PowerupFactionConfig, 2> factions_0c;
    float duration_7c;
    float cooldown_80;
    std::int32_t target_type_84;
    std::int32_t target_filter_88;
    std::array<float, 16> multipliers_8c;
    std::uint8_t random_cc;
    void* texture_d0;
    std::array<float, 4> uv_d4;
};
struct PowerupDescriptorAllocationWords {
    //08; each faction+10..20;7C..CC. Includes untouched paddingCD..CF.
    std::array<std::uint32_t, 32> scalar_words;
};
using PowerupClassMap = std::map<NativeString, PowerupClassConfig, PanelSequenceNameLess>;
using PowerupNameVectorMap = std::map<NativeString, std::vector<NativeString>, PanelSequenceNameLess>;
struct PowerupConfigOwner {
    explicit PowerupConfigOwner(const PowerupOwnerAllocationWords&) noexcept;
    PowerupConfigOwner(const PowerupConfigOwner&) = delete;
    PowerupConfigOwner& operator=(const PowerupConfigOwner&) = delete;
    PowerupOwnerNativeStorage native;
    std::optional<PowerupClassMap> descriptors_14;
    std::optional<PowerupNameVectorMap> random_names_1b8;
};

struct PowerupConfigHost {
    virtual ~PowerupConfigHost() = default;
    // Execute actual004C12B0 and project its current texture/atlas services.
    // Called only when a nonempty picture string reaches texture resolution.
    virtual const GuiTextureCallbacks& current_gui_texture_callbacks_004c12b0() = 0;
    // Current game+[18CC + game.18EC*4] -> selected player+28, not a cached race.
    virtual const std::int32_t& current_selected_player_faction_28() = 0;
    // After real InterlockedDecrement(texture+4)==0, current virtual0,
    // ECX=texture, no stack arguments. No default result or fake free.
    virtual void texture_zero_references(void*) = 0;
};
struct PowerupConfigContext {
    NativeStringStorage& strings;
    PowerupConfigHost& host;
    const bool& crt_sse2_conversion;
    const PowerupDescriptorAllocationWords& descriptor_scratch;
    // Native008E6850 never initializes its local float2 passed at008E6A23.
    // Captured bits are inputs, copied once immediately before that call.
    const std::array<std::uint32_t, 2>& faction_texture_size_words;
};

// Native allocation helpers: no consumed inputs, EAX=node, RET. Payload and
// padding stay allocator bytes; only links/color/isnil are initialized.
void* allocate_powerup_class_node_008e5c50(); //FCh, colorF8/isnilF9
void* allocate_powerup_runtime_map_node_008e5ca0(); //2Ch, color28/isnil29
void* allocate_name_vector_map_node_008e5cf0(); //28h, color24/isnil25; shared warnings
void* allocate_powerup_list_head_008e5960(); //24h, links0/4=self
void* allocate_powerup_large_list_head_008e5980(); //30h, links0/4=self
PowerupNativeContainer& construct_powerup_list_008e65a0(PowerupNativeContainer&);
PowerupNativeContainer& construct_powerup_large_list_008e65c0(PowerupNativeContainer&);
PowerupNativeContainer& construct_powerup_runtime_map_008ea750(PowerupNativeContainer&);

//008EDC60 ECX=fresh1C4h owner, EAX=this, RET008EDD9E. Publishes00F88C30
// internally after ALL container construction. Caller004DC6A0 publishes again.
PowerupConfigOwner& construct_powerup_config_008edc60(PowerupConfigOwner&,
    PowerupConfigOwner*& publication_00f88c30);
void prepare_powerup_descriptor_allocation(PowerupClassConfig&,
    const PowerupDescriptorAllocationWords&) noexcept;
//008E6130/6720 constructors: ECX=this, EAX=this, RET; no scalar zero defaults.
PowerupFactionConfig& construct_powerup_faction_008e6130(PowerupFactionConfig&);
PowerupClassConfig& construct_powerup_class_008e6720(PowerupClassConfig&);
//008E7120/79D0 copies: ECX=fresh destination, source* stack, RET4. Texture
// pointers are shallow copies with NO retain; do not infer shared ownership.
PowerupFactionConfig& copy_powerup_faction_008e7120(PowerupFactionConfig&,
    const PowerupFactionConfig&, NativeStringStorage&);
PowerupClassConfig& copy_powerup_class_008e79d0(PowerupClassConfig&,
    const PowerupClassConfig&, NativeStringStorage&);
//008E6170/67B0: ECX=this, RET. Textures then strings; no object deallocation.
void destroy_powerup_faction_008e6170(PowerupFactionConfig&, PowerupConfigContext&);
void destroy_powerup_class_008e67b0(PowerupClassConfig&, PowerupConfigContext&);

PowerupClassMap::iterator lower_bound_powerup_class_008e5b10(PowerupClassMap&,
    const NativeString&);
PowerupNameVectorMap::iterator lower_bound_powerup_names_008e5bb0(PowerupNameVectorMap&,
    const NativeString&);
// Registry lookups preserve existing mapped-cell identity. Misses own independent
// NativeString keys and default payload copies; standard containers replace STL.
PowerupClassConfig& lookup_powerup_class_008eb4a0(PowerupConfigOwner&,
    const NativeString&, PowerupConfigContext&);
std::vector<NativeString>& lookup_powerup_names_008ec460(PowerupConfigOwner&,
    const NativeString&, NativeStringStorage&);
// Descriptor readers: ECX=record; native reader* stack; RET4. New explicit
// borrowed Lua reference replaces the native reader's current stack element.
void load_powerup_faction_008e6850(PowerupFactionConfig&, GuiLua51Host&,
    GuiLuaRef table, PowerupConfigContext&);
void load_powerup_class_008e7350(PowerupClassConfig&, GuiLua51Host&,
    GuiLuaRef table, PowerupConfigContext&);
//008ECEC0 ECX=owner, RET008ED4AE. Fresh Lua owner, mask4, actual script/runtime
// and overrides. Does not clear prior configuration or random-name lists.
void load_powerup_config_008ecec0(PowerupConfigOwner&, LuaStateOwnerEnvironment,
    LuaScriptRuntime&, PowerupConfigContext&);

// Host cleanup of this configuration projection and constructor allocations.
// Requires ALL runtime vectors/lists/maps still empty. Not008EDDA0 or the
// full game's runtime owner destructor; does not clear publication or free this.
void release_powerup_configuration_storage(PowerupConfigOwner&, PowerupConfigContext&);

} // namespace bsp
