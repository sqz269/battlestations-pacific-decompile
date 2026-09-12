// Offline stand-in for the ordinals bsp_game.exe resolves out of xlive.dll.
//
// This is NOT a reconstruction of Games for Windows Live and NOT a
// reimplementation of the installed xlive.dll.  It is a harness component: an
// ordinal-compatible Win32 DLL whose only job is to let the reconstructed host
// executable reach the frame loop without a live Live service, in the state the
// executable was in before the real DLL was wired up (commit 2c11966a).
//
// Contract of every export here:
//   * no side effect outside the caller-visible output this file documents,
//   * no allocation, no thread, no socket, no file I/O except the optional
//     call-count dump named by %BSP_XLIVE_STUB_LOG%,
//   * the answer an absent Live service gives: the message pump translates
//     nothing, the notification queue is empty, nobody is signed in, no title
//     update exists, the sockets layer succeeds having done no work.
//
// Ordinals, decorated names and stack cleanup are pinned by xlive_stub.def and
// must agree with the __stdcall signatures the host's four loaders declare:
//   src/xlive_library.cpp, src/xlive_startup_adapter.cpp,
//   src/xlive_sdk_adapter.cpp, src/native_xlive_device_adapter.cpp and
//   src/xlive_application_callbacks.cpp (ordinal 5277).
// A wrong parameter count here corrupts the caller's stack silently, so each
// function below states the byte count its RET imm16 must use.

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <winsock2.h>
#include <windows.h>

#if !defined(_M_IX86)
#error The original XLive ABI is Win32 x86; this stand-in must match it.
#endif

namespace {

// Call counts, indexed by a dense slot rather than by ordinal.  Written with
// interlocked increments because the host's pump and its loader threads may
// both reach the same export.
enum Slot {
    kWsaStartup, kWsaCleanup, kSocketNtohs, kNetSetSystemLinkPort,
    kNotifyGetNext, kGetOverlappedExtendedError, kGetOverlappedResult,
    kLiveOnCreateDevice, kLiveOnDestroyDevice, kLiveGetUpdateInformation,
    kLiveUpdateSystem, kLivePreTranslateMessage, kShowSigninUi, kUserGetXuid,
    kUserGetSigninState, kUserGetName, kUserCheckPrivilege, kShowMessageBoxUi,
    kUserGetSigninInfo, kNotifyCreateListener, kUserSetContext,
    kUserWriteAchievements, kLiveInitializeEx, kStorageUploadProgress,
    kStorageUploadFromMemory, kStorageDownloadProgress, kOnlineStartup,
    kInviteGetAcceptedInfo, kStorageBuildServerPath, kStorageDownloadToMemory,
    kSlotCount
};

const struct { const char* name; int ordinal; } kSlots[kSlotCount] = {
    { "XWSAStartup", 1 },
    { "XWSACleanup", 2 },
    { "XSocketNTOHS", 38 },
    { "XNetSetSystemLinkPort", 84 },
    { "XNotifyGetNext", 651 },
    { "XGetOverlappedExtendedError", 1082 },
    { "XGetOverlappedResult", 1083 },
    { "XLiveOnCreateDevice", 5005 },
    { "XLiveOnDestroyDevice", 5006 },
    { "XLiveGetUpdateInformation", 5022 },
    { "XLiveUpdateSystem", 5024 },
    { "XLivePreTranslateMessage", 5030 },
    { "XShowSigninUI", 5260 },
    { "XUserGetXUID", 5261 },
    { "XUserGetSigninState", 5262 },
    { "XUserGetName", 5263 },
    { "XUserCheckPrivilege", 5265 },
    { "XShowMessageBoxUI", 5266 },
    { "XUserGetSigninInfo", 5267 },
    { "XNotifyCreateListener", 5270 },
    { "XUserSetContext", 5277 },
    { "XUserWriteAchievements", 5278 },
    { "XLiveInitializeEx", 5297 },
    { "XStorageUploadFromMemoryGetProgress", 5304 },
    { "XStorageUploadFromMemory", 5305 },
    { "XStorageDownloadToMemoryGetProgress", 5307 },
    { "XOnlineStartup", 5310 },
    { "XInviteGetAcceptedInfo", 5315 },
    { "XStorageBuildServerPath", 5344 },
    { "XStorageDownloadToMemory", 5345 },
};

volatile LONG g_calls[kSlotCount];

void note(Slot slot) { InterlockedIncrement(&g_calls[slot]); }

// The listener identity handed back by XNotifyCreateListener.  The host rejects
// both NULL and (HANDLE)-1 at 00a40fd9/00a40fe0 before it treats the listener as
// valid, so neither of those may be returned.  Nothing here dereferences it.
char g_notify_listener_identity;

void append_decimal(char* out, int& at, unsigned long value) {
    char digits[16];
    int count = 0;
    do { digits[count++] = static_cast<char>('0' + (value % 10u)); value /= 10u; }
    while (value != 0 && count < 16);
    while (count > 0) out[at++] = digits[--count];
}

// Writes "<ordinal> <name> <count>" for every export that was reached, to the
// path in %BSP_XLIVE_STUB_LOG%.  Plain Win32 only: this runs under the loader
// lock at DLL_PROCESS_DETACH, so no CRT, no allocation and no LoadLibrary.
void dump_call_counts() {
    char path[MAX_PATH];
    const DWORD length = GetEnvironmentVariableA("BSP_XLIVE_STUB_LOG", path, MAX_PATH);
    if (length == 0 || length >= MAX_PATH) return;
    const HANDLE file = CreateFileA(path, GENERIC_WRITE, FILE_SHARE_READ, NULL,
        CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (file == INVALID_HANDLE_VALUE) return;
    char line[160];
    for (int slot = 0; slot < kSlotCount; ++slot) {
        const LONG count = g_calls[slot];
        if (count == 0) continue;
        int at = 0;
        append_decimal(line, at, static_cast<unsigned long>(kSlots[slot].ordinal));
        line[at++] = ' ';
        for (const char* c = kSlots[slot].name; *c != '\0'; ++c) line[at++] = *c;
        line[at++] = ' ';
        append_decimal(line, at, static_cast<unsigned long>(count));
        line[at++] = '\r';
        line[at++] = '\n';
        DWORD written = 0;
        WriteFile(file, line, static_cast<DWORD>(at), &written, NULL);
    }
    CloseHandle(file);
}

} // namespace

extern "C" {

// ---- src/xlive_startup_adapter.cpp ------------------------------------------

// RET 8.  The host checks the negotiated version byte by byte at
// 00a40f77-00a40f82 and calls XWSACleanup unless it is exactly 2.2, and
// XLiveStartupAdapter::x_wsa_startup throws XLiveWsaStartupOutputError on any
// nonzero result rather than fabricating WSADATA.  So this succeeds and echoes
// the requested version into a fully zeroed 0x190-byte WSADATA.
int __stdcall XWSAStartup(WORD requested_version, WSADATA* data) {
    note(kWsaStartup);
    if (data == NULL) return WSAEFAULT;
    ZeroMemory(data, sizeof(WSADATA));
    data->wVersion = requested_version;
    data->wHighVersion = requested_version;
    data->iMaxSockets = 0;
    data->iMaxUdpDg = 0;
    data->lpVendorInfo = NULL;
    return 0;
}

// RET 0.  Result discarded by the host.
int __stdcall XWSACleanup(void) { note(kWsaCleanup); return 0; }

// RET 4.  A pure byte swap with no service behind it; the host feeds the result
// straight to XNetSetSystemLinkPort, so returning the argument unswapped would
// be a different value, not a neutral one.  This is the one export that keeps
// the original's arithmetic.
u_short __stdcall XSocketNTOHS(u_short value) {
    note(kSocketNtohs);
    return static_cast<u_short>((value >> 8) | (value << 8));
}

// RET 4.  No link port exists offline; the host discards the result.
int __stdcall XNetSetSystemLinkPort(u_short port) {
    note(kNetSetSystemLinkPort);
    (void)port;
    return 0;
}

// RET 0.  Result discarded by the host at 00a40f63.
HRESULT __stdcall XOnlineStartup(void) { note(kOnlineStartup); return S_OK; }

// RET 8.  The host stores the HRESULT and branches on nothing; the original has
// no path for a missing xlive.dll because it is a plain import.
HRESULT __stdcall XLiveInitializeEx(const void* info, DWORD version) {
    note(kLiveInitializeEx);
    (void)info;
    (void)version;
    return S_OK;
}

// ---- src/xlive_library.cpp ---------------------------------------------------

// RET 4.  FALSE means "not handled", which is what leaves the host's own
// TranslateMessage/DispatchMessage pair in charge of every message.
BOOL __stdcall XLivePreTranslateMessage(const MSG* message) {
    note(kLivePreTranslateMessage);
    (void)message;
    return FALSE;
}

// RET 8.  One ULONGLONG argument (two stack slots).  Returns a stable non-NULL,
// non-(-1) identity so the host's validity test succeeds and its notification
// drain runs; the queue behind it is permanently empty.
HANDLE __stdcall XNotifyCreateListener(ULONGLONG areas) {
    note(kNotifyCreateListener);
    (void)areas;
    return &g_notify_listener_identity;
}

// RET 16.  FALSE means "queue empty".  The outputs are left untouched, which is
// what the original does when it returns FALSE.
BOOL __stdcall XNotifyGetNext(HANDLE listener, DWORD filter, DWORD* id, ULONG_PTR* parameter) {
    note(kNotifyGetNext);
    (void)listener;
    (void)filter;
    (void)id;
    (void)parameter;
    return FALSE;
}

// RET 8.  Unreachable while XNotifyGetNext reports an empty queue: the host only
// asks for invite details after a notification announces one.  ERROR_NOT_FOUND
// is the honest answer and the payload is left untouched; note that
// XLiveLibrary::invite_get_accepted_info throws on any nonzero result, so if a
// future caller reaches this it gets an explicit error, not a fabricated invite.
DWORD __stdcall XInviteGetAcceptedInfo(DWORD user, void* payload) {
    note(kInviteGetAcceptedInfo);
    (void)user;
    (void)payload;
    return ERROR_NOT_FOUND;
}

// RET 4.  title_update_path_00a3ff20 treats a negative HRESULT as "no title
// update", which is the state of an offline title.  The information block is
// left as the caller prepared it.
HRESULT __stdcall XLiveGetUpdateInformation(void* information) {
    note(kLiveGetUpdateInformation);
    (void)information;
    return E_FAIL;
}

// RET 4.  No update package exists, so nothing is launched.
HRESULT __stdcall XLiveUpdateSystem(const wchar_t* path) {
    note(kLiveUpdateSystem);
    (void)path;
    return E_FAIL;
}

// ---- src/native_xlive_device_adapter.cpp -------------------------------------

// RET 8.  The original hands the D3D9 device to the Live overlay; with no
// overlay there is nothing to attach and the renderer's device is untouched.
HRESULT __stdcall XLiveOnCreateDevice(void* device, void* presentation_parameters) {
    note(kLiveOnCreateDevice);
    (void)device;
    (void)presentation_parameters;
    return S_OK;
}

// RET 0.
HRESULT __stdcall XLiveOnDestroyDevice(void) { note(kLiveOnDestroyDevice); return S_OK; }

// ---- src/xlive_application_callbacks.cpp -------------------------------------

// RET 12.  Returns nothing; the native callbacks at 00735510/00735520 discard
// any result.  No context is stored because there is no session to store it in.
void __stdcall XUserSetContext(DWORD user_index, DWORD context_id, DWORD value) {
    note(kUserSetContext);
    (void)user_index;
    (void)context_id;
    (void)value;
}

// ---- src/xlive_sdk_adapter.cpp -----------------------------------------------
// Every export below answers for a process with no signed-in user.  The host
// reads an output only after a zero result, so a nonzero result leaves each
// caller's buffer exactly as the caller prepared it.

// RET 4.  0 = eXUserSigninState_NotSignedIn.  This is a state, not an error
// code: refresh_cached_local_user_00a3ebd0 compares it against 0 and 2.
DWORD __stdcall XUserGetSigninState(DWORD user_index) {
    note(kUserGetSigninState);
    (void)user_index;
    return 0;
}

// RET 12.  XLiveSdkAdapter::user_get_name returns early on a nonzero result and
// the caller substitutes an empty name.
DWORD __stdcall XUserGetName(DWORD user_index, char* name, DWORD capacity) {
    note(kUserGetName);
    (void)user_index;
    (void)name;
    (void)capacity;
    return ERROR_NO_SUCH_USER;
}

// RET 8.  The caller substitutes XUID 0 on a nonzero result.
DWORD __stdcall XUserGetXUID(DWORD user_index, ULONGLONG* xuid) {
    note(kUserGetXuid);
    (void)user_index;
    (void)xuid;
    return ERROR_NO_SUCH_USER;
}

// RET 12.  The caller substitutes privilege 0 on a nonzero result.
DWORD __stdcall XUserCheckPrivilege(DWORD user_index, DWORD privilege, BOOL* result) {
    note(kUserCheckPrivilege);
    (void)user_index;
    (void)privilege;
    (void)result;
    return ERROR_NO_SUCH_USER;
}

// RET 12.  x_user_get_signin_info_flags copies byte 8 of the 0x28-byte block
// only when the result is zero, so the block is not written here.
DWORD __stdcall XUserGetSigninInfo(DWORD user, DWORD flags, void* info) {
    note(kUserGetSigninInfo);
    (void)user;
    (void)flags;
    (void)info;
    return ERROR_NO_SUCH_USER;
}

// RET 8.  There is no sign-in overlay to raise.  A nonzero result makes
// xlive_system_pump.cpp:224 return instead of waiting for a UI that never
// appears, so this must not claim success.
DWORD __stdcall XShowSigninUI(DWORD users, DWORD flags) {
    note(kShowSigninUi);
    (void)users;
    (void)flags;
    return ERROR_NOT_SUPPORTED;
}

// RET 36.  Nine stack slots.  No overlay, so no choice is produced.
DWORD __stdcall XShowMessageBoxUI(DWORD user, const wchar_t* title, const wchar_t* text,
    DWORD button_count, const wchar_t* const* buttons, DWORD focus, DWORD flags,
    DWORD* choice, void* overlapped) {
    note(kShowMessageBoxUi);
    (void)user; (void)title; (void)text; (void)button_count; (void)buttons;
    (void)focus; (void)flags; (void)choice; (void)overlapped;
    return ERROR_NOT_SUPPORTED;
}

// RET 12.  No achievement store offline.
DWORD __stdcall XUserWriteAchievements(DWORD count, const void* achievements, void* overlapped) {
    note(kUserWriteAchievements);
    (void)count;
    (void)achievements;
    (void)overlapped;
    return ERROR_NO_SUCH_USER;
}

// RET 28.  Seven stack slots.  No server path exists; path_bytes is untouched.
DWORD __stdcall XStorageBuildServerPath(DWORD user, DWORD facility, const void* item_info,
    DWORD item_info_bytes, const wchar_t* item, wchar_t* path, DWORD* path_bytes) {
    note(kStorageBuildServerPath);
    (void)user; (void)facility; (void)item_info; (void)item_info_bytes;
    (void)item; (void)path; (void)path_bytes;
    return ERROR_NO_SUCH_USER;
}

// RET 28.  Seven stack slots.  Nothing is downloaded and no overlapped
// operation is started, so no completion is ever signalled.
DWORD __stdcall XStorageDownloadToMemory(DWORD user, const wchar_t* path, DWORD bytes,
    void* buffer, DWORD result_bytes, void* results, void* overlapped) {
    note(kStorageDownloadToMemory);
    (void)user; (void)path; (void)bytes; (void)buffer; (void)result_bytes;
    (void)results; (void)overlapped;
    return ERROR_NO_SUCH_USER;
}

// RET 20.  Five stack slots.
DWORD __stdcall XStorageUploadFromMemory(DWORD user, const wchar_t* path, DWORD bytes,
    const void* buffer, void* overlapped) {
    note(kStorageUploadFromMemory);
    (void)user; (void)path; (void)bytes; (void)buffer; (void)overlapped;
    return ERROR_NO_SUCH_USER;
}

// RET 16.  No transfer is in flight to report progress for.
DWORD __stdcall XStorageDownloadToMemoryGetProgress(void* overlapped, DWORD* progress,
    DWORD* total, DWORD* transferred) {
    note(kStorageDownloadProgress);
    (void)overlapped; (void)progress; (void)total; (void)transferred;
    return ERROR_IO_PENDING;
}

// RET 16.
DWORD __stdcall XStorageUploadFromMemoryGetProgress(void* overlapped, DWORD* progress,
    DWORD* total, DWORD* transferred) {
    note(kStorageUploadProgress);
    (void)overlapped; (void)progress; (void)total; (void)transferred;
    return ERROR_IO_PENDING;
}

// RET 12.  No overlapped operation was ever started here.
DWORD __stdcall XGetOverlappedResult(void* overlapped, DWORD* result, BOOL wait) {
    note(kGetOverlappedResult);
    (void)overlapped;
    (void)result;
    (void)wait;
    return ERROR_IO_INCOMPLETE;
}

// RET 4.
DWORD __stdcall XGetOverlappedExtendedError(void* overlapped) {
    note(kGetOverlappedExtendedError);
    (void)overlapped;
    return ERROR_IO_INCOMPLETE;
}

BOOL WINAPI DllMain(HINSTANCE instance, DWORD reason, LPVOID reserved) {
    (void)reserved;
    if (reason == DLL_PROCESS_ATTACH) {
        // No thread notifications: this stand-in starts no thread and needs none.
        DisableThreadLibraryCalls(instance);
    } else if (reason == DLL_PROCESS_DETACH) {
        dump_call_counts();
    }
    return TRUE;
}

} // extern "C"
