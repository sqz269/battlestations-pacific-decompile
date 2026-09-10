#include "bsp/vfs_startup.hpp"

namespace bsp {
void run_vfs_startup_phase2(VfsStartupState& state, VfsStartupHost& host) {
    state.first_time_block_ran = !host.provider_manager_installed();
    if (state.first_time_block_ran) {
        host.probe_hardware_0073c3b0();
        host.construct_provider_manager_00beda60();
        host.install_manager_handlers(kManagerMountFailureHandler00530620,
            kManagerSecondaryHandler00735b30);

        // 0073d66d and 0073d680. Each getter runs before its registration, and
        // the manager pointer is re-read from DAT_0109ceec between them.
        host.register_provider_factory_00be0660(host.file_store_factory_004fc150());
        ++state.factories_registered;
        host.register_provider_factory_00be0660(host.mpkg_factory_00736a90());
        ++state.factories_registered;

        const std::string root = host.current_directory_with_separator();
        for (const VfsStartupMount& mount : kVfsStartupMounts) {
            std::string system_path;
            if (mount.system_path == VfsStartupSystemPath::current_directory) {
                system_path = root;
            } else {
                system_path = mount.system_path_literal;
            }
            host.mount_system_path_00be1890(system_path, mount.virtual_path,
                mount.priority, mount.ownership, mount.device_id);
            ++state.mounts_requested;
        }

        // 0073d881 and 0073d888, with the same ECX and no arguments. The scan
        // skips names an existing provider already covers, so the second pass
        // only picks up what the first one left.
        host.mount_packages_0073cb10();
        host.mount_packages_0073cb10();
    }
    // 0073d894 is the JNZ target of the gate, so it runs either way.
    host.register_resource_search_paths_00738360();
}

void run_vfs_startup_factory_tail(VfsStartupState& state, VfsStartupHost& host,
    bool cached_load) {
    host.register_provider_factory_00be0660(host.mpak_factory_00736b60());
    ++state.factories_registered;
    host.set_manager_pak_registry_00bd9230(host.pak_archive_registry_00736c30());
    host.set_manager_cached_load_00bd9f90(cached_load);
    host.create_pak_registry_lock_00bb40b0();
}

void run_vfs_startup_phase6(VfsStartupState& state, VfsStartupHost& host) {
    VfsStartupObject manager = host.resource_manager_004c1400();
    state.animation_channels_parser_registered = host.register_type_parser_00b80a50(
        manager, host.animation_channels_parser_00736dd0());

    manager = host.resource_manager_004c1400();
    state.bone_parser_registered =
        host.register_type_parser_00b80a50(manager, host.bone_parser_00736ea0());
}

void run_vfs_startup(VfsStartupState& state, VfsStartupHost& host,
    const CommandLineOptions& options) {
    run_vfs_startup_phase2(state, host);
    // 0073d94a parses the command line, so cachedload is only known here. It
    // could not have influenced any phase-2 mount.
    run_vfs_startup_factory_tail(state, host, options.cached_load);
    run_vfs_startup_phase6(state, host);
}
}
