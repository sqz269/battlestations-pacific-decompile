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
    if (!assets.preload_startup_scripts(completed, error)) {
        std::printf("Startup script preload: completed=%zu error=%s\n", completed, error.c_str());
        return false;
    }
    const auto observed = assets.opens();
    if (completed != 5 || observed.size() != 5 || assets.cache_entries() != 5
        || assets.physical_opens() != 5 || assets.cached_opens() != 0) return false;
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
