#pragma once
// Reconstruction of the entry sequence at 008f81f0 (BSP_WinMain) and the language
// resolution at 008f7db0. Descriptive names are hypotheses, not recovered symbols.
//
// The native routine is __stdcall(HINSTANCE, HINSTANCE, LPSTR, int) with RET 10h at both
// exits, and it never reads any of the four arguments. Every operating-system, COM and
// application step it performs is still unreconstructed, so those are injected through
// StartupHost instead of being invented here.

#include <cstddef>
#include <string>

namespace bsp {

// Language identifiers exactly as spelled in the binary's string table.
// "english" 00d15b14, "french" 00ce432c, "italian" 00ce4324, "german" 00ce4308,
// "spanish" 00ce431c.
extern const char kStartupLanguageEnglish[];
extern const char kStartupLanguageFrench[];
extern const char kStartupLanguageItalian[];
extern const char kStartupLanguageGerman[];
extern const char kStartupLanguageSpanish[];

// Single-instance mutex name at 00d16a64, used by 008f8301.
extern const char kSingleInstanceMutexName[];

// Appended to the current directory before the Game Explorer access check (00d16a78).
extern const char kGdfBinarySuffix[];

// Value pushed to BSP_Application_Initialize at 008f841e (00ce8168).
extern const char kApplicationInitializeMode[];

// Inputs of the language resolution at 008f7db0. The options file lives under
// SHGetSpecialFolderPathA(CSIDL_PERSONAL, fCreate=TRUE) joined with kOptionsDirectory and
// kOptionsFileName; the token key is 00d15f1c. The registry fallback is only consulted when
// the file cannot be opened, and only accepts a REG_DWORD.
extern const char kOptionsDirectory[];       // 00d15af0
extern const char kOptionsFileName[];        // 00d15ae0
extern const char kOptionsLanguageKey[];     // 00d15f1c
extern const char kRegistryKeyPath[];        // 00d15e24, under HKEY_LOCAL_MACHINE
extern const char kRegistryLanguageValue[];  // 00d15e18

// GetCurrentDirectoryA is called with this buffer size at 008f8254.
inline constexpr unsigned long kCurrentDirectoryCapacity = 0xbfeUL;

// Second argument of BSP_Application_Initialize, pushed as literal zero at 008f8423.
inline constexpr int kApplicationInitializeFlags = 0;

// Registry LCID to language mapping, switch at 008f7f9c. Any other value is english.
// 0x407 german, 0x40a spanish, 0x40c french, 0x410 italian.
const char* startup_language_from_registry_lcid(unsigned long lcid);

// Token scan of options.txt, loop at 008f8010..008f808b. Tokens are compared with the
// null-safe case-insensitive compare at 00438e10 against "Language" (00d15f1c); the first
// match consumes the following token as the value. An empty token ends the scan, matching
// the native break at 008f8044. Returns false when no assignment was found, in which case
// the caller keeps the value the string already held.
bool startup_language_from_options_tokens(const char* const* tokens, std::size_t count,
                                          std::string& language);

// Text and caption of the "already running" message box (008f832b..008f83c5).
struct StartupMessage {
    const wchar_t* text;
    const wchar_t* caption;
};

// Bounded 008f832b..008f83bf selection, before MessageBoxW. actual_language_header is
// the same live eight-byte header initialized by 008f7db0: length +0, data +4.
// Each comparison calls canonical 00425850 with that unchanged header address and a
// fixed nonnull language literal, in native order. No copy, allocation, release or
// null-header guard. The caller retains the owner through the message box; the native
// inline release at 008f83cb..008f83e9 follows it. This is not the WinMain ABI.
StartupMessage startup_already_running_message_from_native_header_008f832b(
    const void* actual_language_header) noexcept;

// C-string projection used by the existing std::string StartupHost. For these five
// nonnull, nonempty candidates it selects the same message as native 00425850,
// including null data and any recorded length. It does not preserve native-header
// identity, allocation/lifetime, or the complete general-purpose 00425850 contract.
StartupMessage startup_already_running_message(const char* language);

// Widening performed by 004c5e60: every byte is zero-extended into a UTF-16 unit. This is
// not MultiByteToWideChar, so a current directory outside ASCII reaches the Game Explorer
// as its raw ANSI bytes.
std::wstring startup_widen_path(const char* ansi_path);

// Arguments the CRT at 00bfd0dd passes in. Recorded for completeness; 008f81f0 reads none
// of them.
struct WinMainArguments {
    void* instance = nullptr;
    void* previous_instance = nullptr;
    const char* command_line = nullptr;
    int show_command = 0;
};

// Result of CreateMutexA at 008f8301 plus the GetLastError probe at 008f8311.
struct SingleInstanceMutex {
    void* handle = nullptr;
    bool already_exists = false;  // GetLastError() == ERROR_ALREADY_EXISTS (0xb7)
};

// Every step of 008f81f0 that is not yet reconstructed. There are no default
// implementations: this is an integration contract, not a set of game stubs.
struct StartupHost {
    StartupHost() = default;
    virtual ~StartupHost() = default;
    StartupHost(const StartupHost&) = delete;
    StartupHost& operator=(const StartupHost&) = delete;

    // 008f81f8 CoInitializeEx(nullptr, COINIT_MULTITHREADED|COINIT_SPEED_OVER_MEMORY=8),
    // 008f820a CoInitializeSecurity(..., RPC_C_AUTHN_LEVEL_DEFAULT=0, RPC_C_IMP_LEVEL_IMPERSONATE=3,
    // ...), 008f82cc CoUninitialize. Both initializers return an HRESULT and the sequence
    // continues only while it is non-negative.
    virtual long com_initialize() = 0;
    virtual long com_initialize_security() = 0;
    virtual void com_uninitialize() = 0;

    // 008f8254 GetCurrentDirectoryA into a caller-owned buffer.
    virtual void current_directory(char* buffer, unsigned long capacity) = 0;

    // 008f8245 CoCreateInstance(CLSID_GameExplorer {9a5ea990-3034-4d6f-9128-01f3c61022bc},
    // nullptr, CLSCTX_ALL, IID_IGameExplorer {e7b2fb72-d728-49b3-a5f2-18ebf5f1349e}, &p).
    // Zero the output pointer before the call. Return true for nonnegative
    // HRESULT; preserve the actual pointer separately for the release guard.
    virtual bool game_explorer_create() = 0;
    // 008f82aa, vtable slot +18h: IGameExplorer::VerifyAccess(path, &has_access).
    // Native leaves the BOOL uninitialized before the call and ignores HRESULT.
    // Return the actual post-call BOOL !=0; do not initialize it to permitted.
    virtual bool game_explorer_verify_access(const wchar_t* gdf_binary_path) = 0;
    // 008f82c4, vtable slot +08h: IUnknown::Release, guarded by actual pointer.
    // Called after the creation branch even for negative HRESULT; do not clear
    // the local afterward or add automatic release to the denied exit path.
    virtual void game_explorer_release() = 0;

    // 008f82f0, full-cleanup CRT exit(0), including registered on-exit callbacks.
    // Reached only when VerifyAccess reports no access. The native call does not
    // return, and it runs before the interface is released and before CoUninitialize.
    virtual void exit_process(int code) = 0;

    // 00bd2e20, 00bd2fe0, 00bd3050, 00bd30d0. The per-thread slot is a one byte allocation
    // (operator new at 00bf681b, released through 00bf65ac); registration is skipped when
    // the allocation fails, and neither register nor unregister takes an argument.
    virtual void random_threads_initialize() = 0;
    virtual void* allocate_thread_slot() = 0;
    virtual void random_threads_register_current() = 0;
    virtual void random_threads_unregister_current() = 0;
    virtual void release_thread_slot(void* slot) = 0;
    virtual void random_threads_shutdown() = 0;

    // 008f8301 CreateMutexA(nullptr, TRUE, name), 008f846e CloseHandle.
    virtual SingleInstanceMutex create_single_instance_mutex(const char* name) = 0;
    virtual void close_mutex(void* handle) = 0;

    // Projected value of 008f8326 -> 008f7db0. The original initializes the caller's
    // actual native header in ECX; this interface returns a separate std::string.
    // Empty preserves message selection for native null data, not its ownership.
    virtual std::string resolve_language() = 0;

    // 008f83c5 MessageBoxW(nullptr, text, caption, MB_ICONHAND).
    virtual void error_message_box(const wchar_t* text, const wchar_t* caption) = 0;

    // 008f83fc SetThreadAffinityMask(GetCurrentThread(), 1).
    virtual void set_thread_affinity_to_first_processor() = 0;

    // 008f840b BSP_GameResourceFactory_GetSingleton, result stored to 00f8d31c.
    virtual void publish_game_resource_factory() = 0;

    // 008f8419 / 008f8429 / 008f8432 / 008f843b / 008f8444, all on the same 0x1c byte
    // stack object: BSP_Application_Construct, BSP_Application_Initialize(flags, mode),
    // BSP_Platform_RunLoopDispatch, BSP_Application_Shutdown, BSP_Application_Destruct.
    virtual void application_construct() = 0;
    virtual void application_initialize(int flags, const char* mode) = 0;
    virtual void platform_run_loop_dispatch() = 0;
    virtual void application_shutdown() = 0;
    virtual void application_destruct() = 0;

    // 008f8449..008f8460: when the singleton manager pointer at 01090aa0 is set, destroy it
    // through BSP_SingletonLifetime_Destroy, free it, and clear the pointer.
    virtual void destroy_singleton_lifetime_manager() = 0;
};

// Full sequence of 008f81f0. Always returns 0: both native exits are XOR EAX,EAX / RET 10h.
int run_win_main(const WinMainArguments& arguments, StartupHost& host);

}  // namespace bsp
