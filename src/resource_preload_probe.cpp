// One installed-source scenario through the recovered startup policy and cache.
#include "asset_stream_probe.hpp"
#include <algorithm>
#include <cstring>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iterator>

bool probe_startup_script_preloads(AssetStreamProbe& assets,
    const std::filesystem::path& game_root) {
    std::size_t completed = 0;
    std::string error;
    const auto& scans = assets.package_scans();
    std::printf("Installed startup package scans: passes=2 first_enumerated=%d first_entries=%zu second_enumerated=%d second_entries=%zu\n",
        scans[0].enumerated, scans[0].entries.size(), scans[1].enumerated, scans[1].entries.size());
    if (!assets.preload_startup_scripts(completed, error)) {
        std::printf("Startup script preload: completed=%zu error=%s\n", completed, error.c_str());
        return false;
    }
    const auto observed = assets.opens();
    if (completed != 5 || observed.size() != 5 || assets.cache_entries() != 5
        || assets.physical_opens() != 5 || assets.cached_opens() != 0) return false;
    std::vector<std::string> names, cached_names;
    for (const auto& request : observed) cached_names.push_back(request.name);
    std::sort(cached_names.begin(), cached_names.end());
    if (!assets.enumerate("scripts/datatables", "lua", 0, names, error)
        || names.size() < cached_names.size()
        || !std::equal(cached_names.begin(), cached_names.end(), names.begin())) return false;
    // The cache contributes its five sorted names first. Physical enumeration
    // adds the other loose scripts; duplicates across providers appear once.
    bool enumeration_checked = names.size() > cached_names.size();
    for (const auto& name : cached_names)
        enumeration_checked = enumeration_checked && std::count(names.begin(), names.end(), name) == 1;
    std::printf("Installed provider enumeration: names=%zu cache_first=%zu physical_additions=%zu first_spelling_and_dedup=%d\n",
        names.size(), cached_names.size(), names.size() - cached_names.size(), enumeration_checked);
    if (!enumeration_checked) return false;
    std::uint64_t total_bytes = 0;
    for (const auto& request : observed) {
        if (request.flags != 0x32 || request.cache) return false;
        // Compare mounted cached bytes with an independent file read. The
        // observed request order/flags also appear in the saved probe output.
        std::ifstream disk(game_root / request.name, std::ios::binary);
        if (!disk) return false;
        const std::vector<char> reference{std::istreambuf_iterator<char>(disk), {}};
        if (disk.bad()) return false;
        std::shared_ptr<bsp::MemoryStream> cached;
        if (!assets.read(request.name, cached, error, nullptr, request.flags)
            || cached->position_00bef580() != 0
            || cached->size_00bef600() != reference.size()
            || (!reference.empty() && std::memcmp(cached->data_00bef610(),
                reference.data(), reference.size()) != 0)) return false;
        total_bytes += reference.size();
        std::printf("Startup cached script: name=%s flags=0x%x bytes=%zu disk_bytes_equal=1\n",
            request.name.c_str(), request.flags, reference.size());
    }
    const bool checked = assets.physical_opens() == 5 && assets.cached_opens() == 5
        && assets.cache_entries() == 5 && assets.opens().size() == 10
        && std::all_of(assets.opens().begin() + 5, assets.opens().end(),
            [](const AssetStreamProbe::OpenObservation& opened) {
                return opened.cache && opened.flags == 0x32;
            });
    std::printf("Native startup script policy: completed=%zu bytes=%llu physical_mode50_opens=%zu cached_mode50_opens=%zu checked=%d\n",
        completed, static_cast<unsigned long long>(total_bytes),
        assets.physical_opens(), assets.cached_opens(), checked);
    return checked;
}
