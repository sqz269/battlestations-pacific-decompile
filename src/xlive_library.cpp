#include "bsp/xlive_library.hpp"

#include <cstring>
#include <filesystem>
#include <stdexcept>

namespace bsp {
struct XLiveLibrary::Impl {
    HMODULE module{};
    explicit Impl(const std::wstring& path) {
        static_assert(sizeof(void*) == 4, "The original XLive ABI is Win32.");
        if (!std::filesystem::path(path).is_absolute())
            throw std::invalid_argument("XLive DLL path must be absolute");
        module = LoadLibraryExW(path.c_str(), nullptr, LOAD_WITH_ALTERED_SEARCH_PATH);
        if (!module) throw std::runtime_error("Cannot load XLive DLL: Win32 error " +
            std::to_string(GetLastError()));
    }
    ~Impl() { if (module) FreeLibrary(module); }

    template<class Result, class... Args>
    Result call(std::uint16_t ordinal, Args... args) {
        const auto address = GetProcAddress(module, MAKEINTRESOURCEA(ordinal));
        if (!address) throw std::runtime_error("Missing XLive ordinal " +
            std::to_string(ordinal));
        using Function = Result (__stdcall*)(Args...);
        Function function;
        static_assert(sizeof(function) == sizeof(address));
        std::memcpy(&function, &address, sizeof(function));
        return function(args...);
    }
};

XLiveLibrary::XLiveLibrary(const std::wstring& path)
    : impl_(std::make_unique<Impl>(path)) {}
XLiveLibrary::~XLiveLibrary() = default;
void* XLiveLibrary::module_handle() const noexcept { return impl_->module; }
bool XLiveLibrary::pretranslate(MSG& message) {
    return impl_->call<BOOL>(5030, &message) != FALSE;
}
void* XLiveLibrary::notify_create_listener(std::uint64_t areas) {
    return impl_->call<void*>(5270, areas);
}
bool XLiveLibrary::notify_get_next(void* listener, std::uint32_t filter,
    std::uint32_t& id, std::uint32_t& parameter) {
    return impl_->call<BOOL>(651, listener, filter, &id, &parameter) != FALSE;
}
XLiveAcceptedInvite XLiveLibrary::invite_get_accepted_info(std::uint32_t user) {
    XLiveAcceptedInvite payload;
    const auto result = impl_->call<std::uint32_t>(5315, user, payload.data());
    if (result != 0) throw std::runtime_error(
        "XInviteGetAcceptedInfo did not provide a valid payload: " + std::to_string(result));
    return payload;
}
std::int32_t XLiveLibrary::update_system(const wchar_t* path) {
    return impl_->call<std::int32_t>(5024, path);
}
} // namespace bsp
