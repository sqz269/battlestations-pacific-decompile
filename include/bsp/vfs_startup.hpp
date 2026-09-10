#pragma once
#include <cstdint>
#include <string>

#include "bsp/app_bootstrap.hpp"

namespace bsp {
// Projection of the VFS and resource-parser bring-up inside cSkeletonAppMidway
// Init (0073d410). Three non-contiguous spans are modelled here:
//   phase 2      0073d604..0073d899  provider manager, factories, mounts
//   factory tail 0073d94f..0073d98d  the .mpak factory and the PAK registry
//   phase 6      0073db41..0073db69  resource manager and its two parsers
// Renderer, input, settings and audio bring-up run between the tail and phase 6
// and is deliberately absent; see docs/APP_INIT_VFS_SINGLETONS.md.

// Source of the system-path argument of each 00be1890 call site.
enum class VfsStartupSystemPath {
    // GetCurrentDirectoryA(0FAh, buf) at 0073d697 then _strcat_s(buf, 100h, "\")
    // at 0073d6ac. The two sizes differ in the native code and are not a typo of
    // this projection.
    current_directory,
    // A literal pushed straight into the mount call.
    literal,
};

// One 00be1890 call site. Native ECX is the provider manager and the five stack
// arguments are in this order; the call is RET 14h and returns a provider or
// null. Ownership and device id are the raw bytes/words the native code pushes.
struct VfsStartupMount {
    VfsStartupSystemPath system_path;
    const char* system_path_literal; // null unless system_path == literal
    const char* virtual_path;
    std::int32_t priority;
    std::uint8_t ownership;
    std::int32_t device_id;
};

// The three startup mounts in native call order, at 0073d6f9, 0073d792 and
// 0073d829. Literals are 00ce3a70 ("."), 00cff208 ("persistent_data") and
// 00cff1fc ("filestore"). The FileStore mount is the one that pushes ownership
// 0; both physical mounts push 1.
inline constexpr VfsStartupMount kVfsStartupMounts[3] = {
    {VfsStartupSystemPath::current_directory, nullptr, ".", 0, 1, -1},
    {VfsStartupSystemPath::current_directory, nullptr, "persistent_data", 99, 1, -1},
    {VfsStartupSystemPath::literal, "filestore", ".", 300, 0, -1},
};

// Values stored into the provider manager at 0073d642 and 0073d652. Both
// targets are a single C3 byte followed by INT3 padding, so neither handler
// does anything; they are carried as addresses rather than modelled as calls.
inline constexpr std::uint32_t kManagerMountFailureHandler00530620 = 0x00530620u;
inline constexpr std::uint32_t kManagerSecondaryHandler00735b30 = 0x00735b30u;

// Native object addresses returned by the singleton getters, kept opaque. The
// projection sequences them and never dereferences them.
using VfsStartupObject = void*;

// Integration boundary. Each method is one native call site or one native store,
// listed in the order Init reaches it. There are no default implementations:
// nothing here stands in for unrecovered native behaviour.
struct VfsStartupHost {
    virtual ~VfsStartupHost() = default;

    // ---- phase 2, 0073d604..0073d899 ----
    // The first-time gate DAT_0109ceec, read at 0073d604. The pointer is stored
    // by singleton base constructor 00bda6f0 from inside 00beda60, not by Init.
    virtual bool provider_manager_installed() = 0;
    // 0073d610, inside the gate. Owned by app_init_bootstrap_options.
    virtual void probe_hardware_0073c3b0() = 0;
    // 0073d615..0073d637: malloc(0A0h), then 00beda60 with ECX = that block when
    // it is non-null. 00beda60 registers physical factory 00bed990 itself, so
    // that factory never appears in register_provider_factory_00be0660 below.
    virtual void construct_provider_manager_00beda60() = 0;
    // manager+90h then manager+8Ch. Native re-reads DAT_0109ceec without a null
    // check, so a failed allocation would store through a null manager here.
    virtual void install_manager_handlers(std::uint32_t handler_90h,
        std::uint32_t handler_8ch) = 0;
    // 004fc150, the FileStore factory singleton (external, app_shutdown packet).
    virtual VfsStartupObject file_store_factory_004fc150() = 0;
    // 00736a90, the .mpkg provider factory singleton in DAT_010904f4.
    virtual VfsStartupObject mpkg_factory_00736a90() = 0;
    // 00736b60, the .mpak provider factory singleton in DAT_010904d4.
    virtual VfsStartupObject mpak_factory_00736b60() = 0;
    // 00be0660: ECX manager, factory on the stack, RET 4. Appends without any
    // duplicate check, so a second Init pass appends the same factory again.
    virtual void register_provider_factory_00be0660(VfsStartupObject factory) = 0;
    // The shared system-path buffer built once at 0073d68d..0073d6b1.
    virtual std::string current_directory_with_separator() = 0;
    // 00be1890. The native return value is discarded at all three call sites.
    virtual void mount_system_path_00be1890(const std::string& system_path,
        const std::string& virtual_path, std::int32_t priority,
        std::uint8_t ownership, std::int32_t device_id) = 0;
    // 0073cb10, the "." / mpkg package scan. Init calls it twice consecutively.
    virtual void mount_packages_0073cb10() = 0;
    // 00738360, outside the gate: it runs on every Init pass.
    virtual void register_resource_search_paths_00738360() = 0;

    // ---- factory tail, 0073d94f..0073d98d, ungated ----
    // 00736c30, the 1Ch-byte PAK registry singleton in DAT_010904d8.
    virtual VfsStartupObject pak_archive_registry_00736c30() = 0;
    // 00bd9230: manager+88h = registry. RET 4 (external, platform packet).
    virtual void set_manager_pak_registry_00bd9230(VfsStartupObject registry) = 0;
    // 00bd9f90: manager+78h = the cachedload byte DAT_00e1ae76. RET 4.
    virtual void set_manager_cached_load_00bd9f90(bool cached_load) = 0;
    // 00bb40b0: DAT_010904e0 = a fresh critical section. This is the lock the
    // .mpak factory body 00bb83a0 takes, and it is created after that factory
    // has already been registered.
    virtual void create_pak_registry_lock_00bb40b0() = 0;

    // ---- phase 6, 0073db41..0073db69 ----
    // 004c1400, the resource manager singleton in 010901c4.
    virtual VfsStartupObject resource_manager_004c1400() = 0;
    // 00736dd0 / 00736ea0, the two 8-byte parser singletons Init adds to the six
    // the manager constructor already registered.
    virtual VfsStartupObject animation_channels_parser_00736dd0() = 0;
    virtual VfsStartupObject bone_parser_00736ea0() = 0;
    // 00b80a50: ECX manager, parser on the stack, AL result, RET 4. False means
    // an equivalent type name was already registered and kept its parser.
    virtual bool register_type_parser_00b80a50(VfsStartupObject manager,
        VfsStartupObject parser) = 0;
};

// What one pass through the sequence did. Counters describe this pass only.
struct VfsStartupState {
    // DAT_0109ceec was still null at 0073d604, so the whole phase-2 block ran.
    bool first_time_block_ran = false;
    // 00be0660 calls made by Init. The physical factory that 00beda60 registers
    // for itself is not counted here.
    int factories_registered = 0;
    // 00be1890 calls made. Native discards each result, so this is a request
    // count, not a count of providers that were created.
    int mounts_requested = 0;
    bool animation_channels_parser_registered = false; // 00b80a50 result
    bool bone_parser_registered = false;               // 00b80a50 result
};

// 0073d604..0073d899. The search-path registration at the end is the only step
// outside the first-time gate.
void run_vfs_startup_phase2(VfsStartupState& state, VfsStartupHost& host);

// 0073d94f..0073d98d. Ungated, and it runs after the package scan, so the .mpak
// factory cannot take part in startup package mounting.
void run_vfs_startup_factory_tail(VfsStartupState& state, VfsStartupHost& host,
    bool cached_load);

// 0073db41..0073db69. Init re-reads the manager singleton for each registration
// instead of caching it; that is reproduced here.
void run_vfs_startup_phase6(VfsStartupState& state, VfsStartupHost& host);

// The three spans in Init order. Everything Init does between them is absent.
void run_vfs_startup(VfsStartupState& state, VfsStartupHost& host,
    const CommandLineOptions& options);
}
