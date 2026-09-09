#pragma once
#include "bsp/platform_loop.hpp"

namespace bsp {
// Semantic event wrapper, not the native eight-byte polymorphic object layout.
// Owns its handle. Native calls ignore errors; this interface exposes results.
class Win32Event {
public:
    explicit Win32Event(bool manual_reset);
    ~Win32Event();
    Win32Event(const Win32Event&) = delete;
    Win32Event& operator=(const Win32Event&) = delete;
    bool valid() const { return handle_ != nullptr; }
    BOOL signal_00bd1910();
    DWORD wait_00bd17c0();
    BOOL reset_00bd1960();
    HANDLE native() const { return handle_; }
private:
    HANDLE handle_{};
};
}
