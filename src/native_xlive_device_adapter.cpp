#include "bsp/native_xlive_device_adapter.hpp"
#include "bsp/xlive_library.hpp"

#include <cstring>
#include <stdexcept>
#include <string>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native XLive device forwarding requires MSVC Win32.
#endif

namespace bsp {
namespace {
template<class... Args>
std::uint32_t call_ordinal(void* module, std::uint16_t ordinal, Args... args) {
    const auto address = GetProcAddress(static_cast<HMODULE>(module), MAKEINTRESOURCEA(ordinal));
    if (!address)
        throw std::runtime_error("Missing XLive device ordinal " + std::to_string(ordinal));
    using Function = std::uint32_t (__stdcall*)(Args...);
    Function function;
    static_assert(sizeof(function) == sizeof(address));
    std::memcpy(&function, &address, sizeof(function));
    return function(args...);
}
} // namespace

NativeXLiveDeviceAdapter::NativeXLiveDeviceAdapter(const XLiveLibrary& library)
    : module_(library.module_handle()) {
    if (!module_)
        throw std::invalid_argument("XLive device adapter requires a live library module");
}

std::uint32_t NativeXLiveDeviceAdapter::on_create_device_00c2f1c0(
    void* device, void* presentation_parameters) const {
    return call_ordinal(module_, 5005, device, presentation_parameters);
}

std::uint32_t NativeXLiveDeviceAdapter::on_destroy_device_00c2f1c6() const {
    return call_ordinal(module_, 5006);
}
} // namespace bsp
