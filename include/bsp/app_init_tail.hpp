#pragma once
// The tail of cSkeletonAppMidway::Init 0073d410 that bsp_game.exe still records
// as unimplemented. See docs/APP_INIT_TAIL.md.
// Addresses: 0073c3b0, 00736b60, 00736c30, 00be0660, 00736dd0, 00736ea0,
//            00b80a50, 00af06a0, 0073fa70, 0073e9a0, 00740410, 00740840.
//
// Every descriptive name here is a hypothesis, not a recovered symbol. The
// pure rules are reconstructed from the listing; each host method stands for
// exactly one native call site, named in its comment.

#include "bsp/app_bootstrap.hpp"
#include "bsp/gui_lua_reader.hpp"
#include "bsp/world_effects_startup.hpp"

#include <cstdint>
#include <string>
#include <vector>

namespace bsp {

// ===========================================================================
// A. Phase 2 hardware probe 0073c3b0
// ===========================================================================
//
// The four stored values are read first (0073c407..0073c443) and the routine
// returns at once unless all four are present, which
// probe_hardware_0073c3b0 in src/app_bootstrap.cpp already models. What
// follows is the part that routine delegates to its host: the comparisons at
// 0073c493..0073c7a4, the message at 0073c7aa..0073c827 and the registry
// write-back at 0073c843..0073c89e.

// The machine as the detection object at [ESP+0Cc430h] reports it. The object
// is constructed at 0073c467 from the language 00992830 returns and is torn
// down inline at 0073c8a3.
struct HardwareSnapshot {
    std::uint32_t gpu_device_id{0}; // 0098c870
    std::string sound_device;       // 0098d3d0, a char* into the object
    std::uint32_t cpu_speed{0};     // 0098c7f0
    std::uint32_t mem_size{0};      // 0098c800
};

// Which stored value disagrees with the machine. The enumerator value is the
// index the native passes to the message table 00996060, so the order below is
// also the order the fragments are concatenated in.
enum class HardwareCheck : std::int32_t {
    GpuDevice = 2,   // 0073c49f, CMP EAX,GPUDeviceID
    SoundDevice = 3, // 0073c5cb, 00449af0 BSP_NativeString_NotEqualCaseInsensitive
    CpuSpeed = 4,    // 0073c6fc, CMP EAX,CPUSpeed
    MemSize = 5,     // 0073c763, CMP EAX,MemSize
};

// 0073c811 and 0073c7c8: the caption and the trailing line of the message box.
inline constexpr std::int32_t kHardwareProbeCaptionMessage = 6;
inline constexpr std::int32_t kHardwareProbeTrailerMessage = 7;

// One localized fragment. The GPU and sound checks substitute text; the CPU and
// memory checks push the two raw integers, so the localized format string owns
// their spelling. Both pairs are ordered stored-value first, machine second.
struct HardwareProbeChange {
    HardwareCheck check{HardwareCheck::GpuDevice};
    std::string stored_text;
    std::string current_text;
    std::uint32_t stored_value{0};
    std::uint32_t current_value{0};
    bool numeric{false};

    std::int32_t message_index() const noexcept
    {
        return static_cast<std::int32_t>(check);
    }
};

// The GPU device ids are turned into names by 0098c890 before the comparison
// result is formatted (0073c4b4 for the stored id, 0073c4cb for the machine's).
// Both lookups happen even when the ids agree only in the sense that the
// comparison ran first; the native skips them on a match.
struct HardwareProbeInputs {
    HardwareProfile stored;
    HardwareSnapshot current;
    std::string stored_gpu_name;
    std::string current_gpu_name;
};

// The decision, as a pure rule. Empty means the machine matches the stored
// profile, no message box is shown and nothing is written back.
// The sound comparison is case-insensitive because 00449af0 is; the other
// three are plain 32-bit equality.
std::vector<HardwareProbeChange> hardware_probe_changes_0073c3b0(
    const HardwareProbeInputs& inputs);

// 0073c7ff: vswprintf of L"%s%s%s%s\n%s" over the four fragment buffers in the
// fixed order GPU, SoundDevice, CpuSpeed, MemSize, then the trailing line.
// A check that did not fire contributes an empty buffer, so the newline is
// always present and the fragments are NOT separated from each other.
std::string hardware_probe_message_0073c3b0(
    const std::vector<HardwareProbeChange>& changes,
    const std::vector<std::string>& fragment_text,
    const std::string& trailer_text);

struct HardwareProbeTailHost {
    virtual ~HardwareProbeTailHost() = default;

    virtual std::uint32_t current_gpu_device_id() = 0;         // 0098c870
    virtual std::string gpu_device_name(std::uint32_t id) = 0; // 0098c890
    virtual std::string current_sound_device() = 0;            // 0098d3d0
    virtual std::uint32_t current_cpu_speed() = 0;             // 0098c7f0
    virtual std::uint32_t current_memory_size() = 0;           // 0098c800

    // 00996060 then 00735450: index the flat table at 00d1e918 as
    // [language + index * 6] and vswprintf it with the two arguments. A
    // fragment with no arguments passes both empty.
    virtual std::string localized_message(std::int32_t index,
        const std::string& first, const std::string& second) = 0;

    // MessageBoxW(NULL, text, caption, MB_YESNO | MB_ICONEXCLAMATION) at
    // 0073c827; true only for IDYES (6).
    virtual bool ask_write_default_options(const std::string& text,
        const std::string& caption) = 0;

    // 0098f430, __thiscall on the detection object: fopen(path, "wb") and write
    // the generated defaults. Reached only on IDYES.
    virtual void write_default_options_file(const std::string& path) = 0;

    // 0098d570 -> 0098d470 RegSetValueExA, four bytes.
    virtual void write_hardware_dword(const std::string& value_name,
        std::uint32_t value) = 0;
    // 0098d590 -> 0098d470 RegSetValueExA, strlen + 1 bytes.
    virtual void write_hardware_string(const std::string& value_name,
        const std::string& value) = 0;
};

// The sequence at 0073c456..0073c8a3, given a complete stored profile. The
// write-back at 0073c843 runs whenever anything changed, on both the Yes and
// the No branch: declining the options rewrite still makes the new hardware
// the stored profile, so the warning appears exactly once per change.
HardwareProbeResult run_hardware_probe_tail_0073c3b0(const HardwareProfile& stored,
    HardwareProbeTailHost& host);

// ===========================================================================
// B. The provider factory tail, 0073d94f-0073d98d
// ===========================================================================

// The three singletons in this tail share one shape: test the cached global,
// take the singleton-lifetime lock, test it again, allocate, construct,
// register a sub-object of the instance with the lifetime manager, publish.
struct LazySingletonRecord {
    std::uint32_t cache_global{0};        // the DAT_ that holds the instance
    std::uint32_t allocation_size{0};     // operator new argument
    std::uint32_t primary_vtable{0};      // [instance+00]
    std::uint32_t secondary_vtable{0};    // [instance+04] or [instance+08]
    std::uint32_t lifetime_subobject{0};  // offset registered with 00bd0c30
};

// 00736b60, __cdecl void*(void), RET. The mpkg factory 00736a90 is the same
// routine over DAT_010904f4 and the vtable pair 00cfea14 / 00cfea10; the only
// behavioural difference is slot 1, the Create.
inline constexpr LazySingletonRecord kMpakProviderFactory_00736b60{
    0x010904d4u, 8u, 0x00cfea20u, 0x00cfea1cu, 4u};
inline constexpr LazySingletonRecord kMpkgProviderFactory_00736a90{
    0x010904f4u, 8u, 0x00cfea14u, 0x00cfea10u, 4u};

// Slot 1 of each primary vtable. The mpak Create 00bb83a0 accepts a path only
// when it is longer than four characters and its last five bytes compare equal
// to ".mpak" without case; the mpkg Create is 00bb9d90.
inline constexpr const char kMpakArchiveExtension[] = ".mpak";

// 00be0660, __thiscall(ECX = the VFS provider manager, [esp+4] = factory),
// RET 4. A std::list push_back onto the list at manager+30h/+34h: no
// deduplication, no reference count, no ownership. The order is the call
// order, which is physical, FileStore, MPKG, then MPAK from this tail.
void vfs_register_provider_factory_00be0660(std::vector<const void*>& factories,
    const void* factory);

// 00736c30, __cdecl void*(void), RET: the same lazy shape over DAT_010904d8,
// 1Ch bytes, constructed by 00bb4fb0, lifetime sub-object at +08h.
inline constexpr LazySingletonRecord kPakArchiveRegistry_00736c30{
    0x010904d8u, 0x1cu, 0x00d64190u, 0x00d6418cu, 8u};

// The 1Ch registry 00bb4fb0 builds. +0Ch is copied into every PAK provider the
// factory creates (00bb8240 writes it to provider+18h).
struct PakArchiveRegistryState {
    std::int32_t enabled_04{1};          // +04
    std::int32_t provider_limit_0c{100}; // +0C, 0x64
    std::uint32_t list_10{0};            // +10
    std::uint32_t list_14{0};            // +14
    std::uint32_t list_18{0};            // +18
};

PakArchiveRegistryState pak_archive_registry_state_00bb4fb0() noexcept;

// ===========================================================================
// C. Phase 6, the two resource type parsers
// ===========================================================================

// Both parsers are 8-byte singletons with no data: a primary vtable at +00 and
// the lifetime sub-object vtable at +04. Slot +4 of the primary vtable returns
// the registration key as an owned string; slot +8 creates a resource item.
struct StructuredParserIdentity {
    const char* type_name;          // vtable slot +4
    std::uint32_t singleton_global; // the cached instance
    std::uint32_t primary_vtable;   // [instance+00]
    std::uint32_t create_item_slot; // vtable slot +8
};

inline constexpr StructuredParserIdentity kAnimationChannelsParser_00736dd0{
    "AnimationChannels", 0x01090298u, 0x00cfea38u, 0x00b8a910u};
inline constexpr StructuredParserIdentity kBoneParser_00736ea0{
    "Bone", 0x0109029cu, 0x00cfea48u, 0x00b8a990u};

struct ResourceParserRegistrationHost {
    virtual ~ResourceParserRegistrationHost() = default;
    // 004c1400: the resource manager singleton at 010901c4. Phase 6 fetches it
    // once per registration (0073db41 and 0073db55), not once for both.
    virtual void* resource_manager_004c1400() = 0;
    // 00736dd0 and 00736ea0.
    virtual void* parser_singleton(const StructuredParserIdentity& identity) = 0;
    // 00b80a50, __thiscall(ECX = manager, [esp+4] = parser), RET 4, bool in AL.
    // Appends into the parser map at manager+8 keyed by the name slot +4
    // returns; false means an equal name was already registered and the
    // incumbent parser was kept. Phase 6 discards both results.
    virtual bool register_type_parser_00b80a50(void* manager, void* parser) = 0;
};

// 0073db41..0073db69, in order.
void run_resource_parser_registration_0073db41(ResourceParserRegistrationHost& host);

// ===========================================================================
// D. The startup singleton publication shared by 0073fa70 and 00af06a0
// ===========================================================================
//
// Phase 8's foliage group manager 00af0b10 is already reconstructed in
// src/world_effects_startup.cpp. Its base constructor 00af06a0 is not, and the
// decal system's base constructor 0073fa70 runs the identical four steps, so
// one rule covers both.

struct StartupSingletonPublication {
    std::uint32_t instance_global{0}; // 00af06f6 / 0073fac6
    std::uint32_t base_vtable{0};     // 00af06c8 / 0073fa98
};

inline constexpr StartupSingletonPublication kFoliageGroupManagerBase_00af06a0{
    0x00f8c274u, 0x00d5d7ecu};
inline constexpr StartupSingletonPublication kDecalSystemBase_0073fa70{
    0x00e1aea0u, 0x00cff218u};

struct StartupSingletonPublicationHost {
    virtual ~StartupSingletonPublicationHost() = default;
    // 00415350 then the manager's critical section at +10h: enter through
    // [00ce2218] and bump the depth at +18h (00af06e7, 0073fab7).
    virtual void enter_lifetime_lock() = 0;
    // 00bd0c30, __thiscall(ECX = manager, [esp+4] = the published pointer),
    // RET 4. The native re-reads the global it has just written rather than
    // using the register (00af0713 reloads 00f8c274, 0073fad1 reloads 00e1aea0).
    virtual void register_lifetime(void* instance) = 0;
    // depth-- then [00ce2210] leave.
    virtual void leave_lifetime_lock() = 0;
    // The store to the publication global.
    virtual void publish(void* instance) = 0;
};

void publish_startup_singleton_00af06a0(StartupSingletonPublicationHost& host,
    void* instance);

// ===========================================================================
// E. Phase 9 decal definitions 00740840
// ===========================================================================

// The 1Ch decal system: the base constructor 0073fa70 publishes it at 00e1aea0
// and installs 00cff218; 00740872 then overwrites +00 with the derived vtable
// 00cff2a0 and zeroes +04h..+18h. +10h/+14h/+18h are the record vector.
inline constexpr std::uint32_t kDecalSystemVTable_00740840 = 0x00cff2a0u;

// The growth at 00740d1d: when count == capacity the native asks 0073e9a0 for
// capacity * 2, floored to 1. 0073e9a0 itself floors its argument to 1 and
// returns without reallocating when the capacity already covers the request.
std::int32_t decal_vector_growth_0073e9a0(std::int32_t capacity) noexcept;

// The seven Lua keys, in the order 00740840 reads them. The table key supplies
// the record name before any of them.
inline constexpr const char kDecalKeySize[] = "Size";               // 00cff278
inline constexpr const char kDecalKeyRadius[] = "Radius";           // 00ce5b54
inline constexpr const char kDecalKeyMaxnum[] = "Maxnum";           // 00cff270
inline constexpr const char kDecalKeyTexture[] = "Texture";         // 00ce5fec
inline constexpr const char kDecalKeyShader[] = "Shader";           // 00cff268
inline constexpr const char kDecalKeyLifeTime[] = "LifeTime";       // 00cff25c
inline constexpr const char kDecalKeyFadeOutTime[] = "FadeOutTime"; // 00cff250

// The table walk at 0074099a..00740d74, over an already-loaded Lua state. The
// caller supplies the state because the native builds its own owner at
// 007408a0, opens the libraries with mask 1 at 007408b6 and runs
// scripts/datatables/decals.lua through 00b69d40; those four calls are the
// host's, not part of the parse.
//
// Only entries whose KEY is a string become records (00b660a0 at 007409b9);
// the key is the record name. The record is allocated and constructed before
// the name is read, so an entry with a non-string key allocates nothing.
// Presence is never tested: a record whose table omits a key keeps the zero
// 00740410 left there for the floats and reads an empty texture or shader name.
std::vector<DecalDefinition> parse_decal_definitions_00740840(GuiLuaHost& lua,
    const GuiLuaRef& decals_table, const bool& crt_sse2_conversion);

// 0074092f..00740965 then the walk: look the global "Decals" up through the
// globals pseudo-index and parse it. An absent or non-table global yields no
// records, as the native's first 00b67080 does.
std::vector<DecalDefinition> load_decal_definitions_00740840(GuiLuaHost& lua,
    const bool& crt_sse2_conversion);

} // namespace bsp
