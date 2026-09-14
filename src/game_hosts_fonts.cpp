#include "bsp/game_hosts_fonts.hpp"
#include "bsp/game_hosts_vfs.hpp"
#include "bsp/font_registry_startup.hpp"
#include "bsp/fingerprint_payload.hpp"
#include "bsp/gui_startup.hpp"
#include "bsp/vfs_candidates.hpp"
#include <cstring>
#include <stdexcept>

namespace bsp::game {
namespace {
class FontImageImports {
public:
    FontImageImports() {
        module_ = LoadLibraryExW(L"d3dx9_40.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
        if (!module_) throw std::runtime_error("Cannot load font D3DX9 image imports");
        FARPROC address = GetProcAddress(module_, "D3DXGetImageInfoFromFileInMemory");
        static_assert(sizeof(read_info) == sizeof(address));
        std::memcpy(&read_info, &address, sizeof(read_info));
        address = GetProcAddress(module_, "D3DXCreateTextureFromFileInMemoryEx");
        static_assert(sizeof(create) == sizeof(address));
        std::memcpy(&create, &address, sizeof(create));
        if (!read_info || !create) {
            FreeLibrary(module_);
            module_ = nullptr;
            throw std::runtime_error("Font D3DX9 image export missing");
        }
    }
    ~FontImageImports() { if (module_) FreeLibrary(module_); }
    FontImageImports(const FontImageImports&) = delete;
    FontImageImports& operator=(const FontImageImports&) = delete;
    ReadImageInfoFromMemory read_info{};
    CreateTextureFromMemory create{};
private:
    HMODULE module_{};
};
}

struct GameFontHost::Impl {
    GameHostLog& log;
    GameVfsHost& vfs;
    GameScriptHost& scripts;
    IDirect3DDevice9& device;
    std::uint32_t mip_reduction;
    FontImageImports imports;
    FontRegistryStartup fonts;
    FingerprintPayload fingerprint;
    std::size_t opens{};
    bool initialized{};

    Impl(GameHostLog& log_in, GameVfsHost& vfs_in, GameScriptHost& scripts_in,
        IDirect3DDevice9& device_in, std::uint32_t mip)
        : log(log_in), vfs(vfs_in), scripts(scripts_in), device(device_in), mip_reduction(mip) {}

    bool read(const std::string& requested, std::shared_ptr<MemoryStream>& stream,
        std::string& error) {
        auto* manager = vfs.ready() ? &vfs.context() : nullptr;
        if (!manager) { error = "Font VFS manager missing"; return false; }
        auto& mounts = *manager;
        std::string resolved = requested;
        if (!resolve_existing_resource_00bdf4c0_fragment(mounts,
                vfs.search_registrations(), resolved)) {
            error = "Font VFS cannot resolve " + requested;
            return false;
        }
        auto opened = open_resource_memory_00bdf310_fragment(mounts, resolved, 2);
        if (!opened.provider_opened || !opened.stream || !opened.stream->fully_initialized()) {
            error = "Font VFS cannot fully read " + resolved + ": " + opened.error;
            return false;
        }
        stream = std::move(opened.stream);
        ++opens;
        log.notef("font resource %s -> %s bytes=%lld", requested.c_str(),
            resolved.c_str(), static_cast<long long>(stream->size_00bef600()));
        return true;
    }
};

GameFontHost::GameFontHost(GameHostLog& log, GameVfsHost& vfs, GameScriptHost& scripts,
    IDirect3DDevice9& device, std::uint32_t mip_reduction)
    : impl_(std::make_unique<Impl>(log, vfs, scripts, device, mip_reduction)) {}
GameFontHost::~GameFontHost() = default;

void GameFontHost::initialize(const std::string& language_font_path) {
    load_descriptors_00ac3910(std::string(kFontRootPrefix), std::string(kFontDescriptorPath),
        language_font_path);
    preload_fingerprint_payload_00be9760();
}

void GameFontHost::load_descriptors_00ac3910(const std::string& root,
    const std::string& descriptor, const std::string& language_font_path) {
    auto& host = *impl_;
    if (host.initialized) throw std::logic_error("Application font startup already completed");
    const FontStreamResolver streams = [&host](const std::string& name,
        std::shared_ptr<MemoryStream>& stream, std::string& error) {
        return host.read(name, stream, error);
    };
    const FontRegistryResourceLoader load = [&host, &streams](const FontDescriptor& descriptor,
        const std::string& root, const std::string& language_path,
        std::unique_ptr<FontResources>& result, std::string& error) {
        return load_font_resources_from_streams_00ad4c30_fragment(host.device,
            host.imports.read_info, host.imports.create, streams, descriptor, root,
            language_path, host.mip_reduction, result, error);
    };
    std::string error;
    if (!host.fonts.load_lua_descriptors_00ac3910(host.scripts.files(), host.scripts.globals(),
            load, root, descriptor, language_font_path, error))
        throw std::runtime_error("Font registry startup: " + error);
    host.log.implemented("Phase 7 font registry resources", "00ac3910");
    host.log.notef("fonts owned=%zu resource_opens=%zu mip_reduction=%u",
        host.fonts.fonts().size(), host.opens, host.mip_reduction);
    host.initialized = true;
}

void GameFontHost::preload_fingerprint_payload_00be9760() {
    auto& host = *impl_;
    //0073bc0d..bc17 forces the payload preload, discarding its data pointer.
    //This is application RAII, not the original global singleton allocator ABI.
    std::shared_ptr<MemoryStream> payload;
    std::string error;
    if (!host.read("fonts/arial19.dat", payload, error))
        throw std::runtime_error("Fingerprint preload: " + error);
    host.fingerprint.load_00be9760(*payload);
    host.log.implemented("Phase 7 fingerprint payload preload", "00be9760");
    host.log.notef("fingerprint_defined_bytes=%zu decoded=%d", host.fingerprint.defined_size(),
        host.fingerprint.decoded() ? 1 : 0);
}

FontRegistryStartup& GameFontHost::registry() noexcept { return impl_->fonts; }
const FingerprintPayload& GameFontHost::fingerprint() const noexcept { return impl_->fingerprint; }
std::size_t GameFontHost::resource_opens() const noexcept { return impl_->opens; }
}
