#include "bsp/win32_event.hpp"

namespace bsp {
Win32Event::Win32Event(bool manual_reset)
    : handle_(CreateEventA(nullptr, manual_reset, FALSE, nullptr)) {}
Win32Event::~Win32Event() { if (handle_) CloseHandle(handle_); }
BOOL Win32Event::signal_00bd1910() { return SetEvent(handle_); }
DWORD Win32Event::wait_00bd17c0() { return WaitForSingleObject(handle_, INFINITE); }
BOOL Win32Event::reset_00bd1960() { return ResetEvent(handle_); }
}
