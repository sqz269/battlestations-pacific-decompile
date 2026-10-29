#include "bsp/bink_runtime_library.hpp"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

#include <cstring>
#include <filesystem>
#include <stdexcept>

namespace bsp {
namespace {

// A completed member owns the handle before any export lookup can throw.
// An incomplete Impl therefore releases only its own unescaped module load.
class BinkModule final {
public:
    explicit BinkModule(const std::wstring& path) {
        static_assert(sizeof(void*) == 4, "The installed Bink ABI is Win32.");
        const std::filesystem::path selected(path);
        if (path.find(L'\0') != std::wstring::npos || !selected.is_absolute() ||
            selected.extension().empty())
            throw std::invalid_argument("Bink DLL path must be absolute with an explicit extension and no embedded NUL");
        display_path_ = selected.u8string();
        // Select this exact filename, without LoadLibrary's implicit .dll.
        // Restrict dependency filesystem search to its directory and System32;
        // never change the process search path or retry a name.
        module_ = LoadLibraryExW(path.c_str(), nullptr,
            LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR | LOAD_LIBRARY_SEARCH_SYSTEM32);
        if (!module_) {
            const DWORD error = GetLastError();
            throw std::runtime_error("Cannot load Bink DLL '" + display_path_ +
                "': Win32 error " + std::to_string(error));
        }
    }

    ~BinkModule() { if (module_) FreeLibrary(module_); }
    BinkModule(const BinkModule&) = delete;
    BinkModule& operator=(const BinkModule&) = delete;
    BinkModule(BinkModule&&) = delete;
    BinkModule& operator=(BinkModule&&) = delete;

    template<class Function>
    Function require(const char* name) const {
        const FARPROC address = GetProcAddress(module_, name);
        if (!address) {
            const DWORD error = GetLastError();
            throw std::runtime_error(std::string("Missing Bink export ") + name +
                " from '" + display_path_ + "': Win32 error " + std::to_string(error));
        }
        Function function;
        static_assert(sizeof(function) == sizeof(address));
        std::memcpy(&function, &address, sizeof(function));
        return function;
    }

private:
    std::string display_path_;
    HMODULE module_{};
};

} // namespace

struct BinkRuntimeLibrary::Impl {
    // Destroyed last, after its stable borrowed table storage.
    BinkModule module;
    BinkImports imports{};
    BinkFactoryImports factory{};

    explicit Impl(const std::wstring& path) : module(path) {
        imports.open = module.require<decltype(imports.open)>("_BinkOpen@8");
        imports.close = module.require<decltype(imports.close)>("_BinkClose@4");
        imports.set_sound_track = module.require<decltype(imports.set_sound_track)>("_BinkSetSoundTrack@8");
        imports.set_volume = module.require<decltype(imports.set_volume)>("_BinkSetVolume@12");
        imports.do_frame = module.require<decltype(imports.do_frame)>("_BinkDoFrame@4");
        imports.copy_to_buffer = module.require<decltype(imports.copy_to_buffer)>("_BinkCopyToBuffer@28");
        imports.next_frame = module.require<decltype(imports.next_frame)>("_BinkNextFrame@4");
        imports.wait = module.require<decltype(imports.wait)>("_BinkWait@4");
        imports.get_realtime = module.require<decltype(imports.get_realtime)>("_BinkGetRealtime@12");
        imports.go_to = module.require<decltype(imports.go_to)>("_BinkGoto@12");
        imports.pause = module.require<decltype(imports.pause)>("_BinkPause@8");
        factory.set_sound_system = module.require<decltype(factory.set_sound_system)>("_BinkSetSoundSystem@8");
        factory.open_direct_sound = module.require<decltype(factory.open_direct_sound)>("_BinkOpenDirectSound@4");
    }
};

BinkRuntimeLibrary::BinkRuntimeLibrary(const std::wstring& path)
    : impl_(std::make_unique<Impl>(path)) {}
BinkRuntimeLibrary::~BinkRuntimeLibrary() = default;
const BinkImports& BinkRuntimeLibrary::imports() const noexcept { return impl_->imports; }
const BinkFactoryImports& BinkRuntimeLibrary::factory_imports() const noexcept { return impl_->factory; }

} // namespace bsp
