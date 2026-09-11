#pragma once

#include <cstdint>

namespace bsp {
class XLiveLibrary;

// Bind the caller-selected, already loaded XLiveLibrary. It must outlive this
// adapter and the device operations. No DLL loading, initialization, device
// ownership or replacement SDK implementation is introduced here.
class NativeXLiveDeviceAdapter final {
public:
    explicit NativeXLiveDeviceAdapter(const XLiveLibrary&);

    // Original C2F1C0 -> IAT CE25D0 -> xlive.dll ordinal5005.
    // Win32 stdcall(device, presentation_parameters), two pointer words.
    // Pass the actual device and writable presentation structure without copies.
    // B29670 supplies renderer+1A10 and renderer+1A28, then ignores EAX.
    std::uint32_t on_create_device_00c2f1c0(void* actual_device,
        void* actual_presentation_parameters) const;

    // Original C2F1C6 -> IAT CE25D4 -> xlive.dll ordinal5006.
    // Win32 stdcall(), no arguments. Preserve the full returned EAX word.
    std::uint32_t on_destroy_device_00c2f1c6() const;

    // The caller retains both current F8ABE8 gates and native call ordering.
    // Missing exports throw a source binding error; no success is synthesized.
    // This is external SDK forwarding, not reconstructed XLive internals or
    // a replacement for the original import-thunk ABI/live IAT mutation.
private:
    void* const module_; // borrowed HMODULE
};
} // namespace bsp
