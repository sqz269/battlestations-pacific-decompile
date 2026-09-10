// Reconstruction of BSP_WinMain (008f81f0) and its language resolution helper (008f7db0).
// Control flow follows the disassembly, not the Ghidra pseudocode: the pseudocode aliases the
// COM interface slot with the language string object and misplaces the teardown returns.

#include "bsp/winmain_startup.hpp"

#include <cstring>
#include <string>

namespace bsp {
namespace {

// 00438e10: null-safe case-insensitive compare used by the options token scan.
int compare_insensitive_00438e10(const char* left, const char* right) {
    if (left == right) {
        return 0;
    }
    if (left == nullptr) {
        return -1;
    }
    if (right == nullptr) {
        return 1;
    }
    return _stricmp(left, right);
}

// 00425850: equality of a stored native string against a C string. A null data pointer only
// matches a null or empty candidate; otherwise the comparison is _stricmp.
bool language_equals_00425850(const char* stored, const char* candidate) {
    if (stored != nullptr) {
        if (candidate == nullptr) {
            return *stored == '\0';
        }
        return _stricmp(stored, candidate) == 0;
    }
    if (candidate == nullptr) {
        return true;
    }
    return *candidate == '\0';
}

// Message pairs at 00d16a10/00d16a00, 00d16990/00d1697c, 00d16920/00d16910,
// 00d168c0/00d168b0 and 00d16858. Non-ASCII characters are written as universal-character
// escapes so the source encoding cannot alter them. Both oddities below are in the image:
// the Italian text begins with a space, and the French text uses U+2019 as its apostrophe.
const wchar_t kTextEnglish[] = L"Battlestations: Pacific already running.";
const wchar_t kCaptionEnglish[] = L"Error";
const wchar_t kTextFrench[] =
    L"Battlestations: Pacific est d\u00e9j\u00e0 en cours d\u2019ex\u00e9cution.";
const wchar_t kCaptionFrench[] = L"Erreur";
const wchar_t kTextItalian[] = L" Battlestations: Pacific \u00e8 gi\u00e0 in esecuzione.";
const wchar_t kCaptionItalian[] = L"Errore";
const wchar_t kTextGerman[] = L"Battlestations: Pacific l\u00e4uft bereits.";
const wchar_t kCaptionGerman[] = L"Fehler";
const wchar_t kTextSpanish[] = L"Battlestations: Pacific ya est\u00e1 en marcha.";

}  // namespace

const char kStartupLanguageEnglish[] = "english";
const char kStartupLanguageFrench[] = "french";
const char kStartupLanguageItalian[] = "italian";
const char kStartupLanguageGerman[] = "german";
const char kStartupLanguageSpanish[] = "spanish";

const char kSingleInstanceMutexName[] = "MidwayThreadMutex";
const char kGdfBinarySuffix[] = "\\battlestationspacific.exe";
const char kApplicationInitializeMode[] = "cachedload";

const char kOptionsDirectory[] = "\\Battlestations-Pacific";
const char kOptionsFileName[] = "\\options.txt";
const char kOptionsLanguageKey[] = "Language";
const char kRegistryKeyPath[] = "SOFTWARE\\Eidos\\Battlestations Pacific";
const char kRegistryLanguageValue[] = "language";

const char* startup_language_from_registry_lcid(unsigned long lcid) {
    switch (lcid) {
        case 0x407UL:
            return kStartupLanguageGerman;
        case 0x40aUL:
            return kStartupLanguageSpanish;
        case 0x40cUL:
            return kStartupLanguageFrench;
        case 0x410UL:
            return kStartupLanguageItalian;
        default:
            return kStartupLanguageEnglish;
    }
}

bool startup_language_from_options_tokens(const char* const* tokens, std::size_t count,
                                          std::string& language) {
    if (tokens == nullptr) {
        return false;
    }
    for (std::size_t index = 0; index < count; ++index) {
        const char* token = tokens[index];
        // 008f8039..008f8044: an empty token ends the scan.
        if (token == nullptr || *token == '\0') {
            return false;
        }
        if (compare_insensitive_00438e10(token, kOptionsLanguageKey) != 0) {
            continue;
        }
        // 008f8070: advance once, then take the following token as the value.
        if (index + 1 >= count) {
            // The native code reads a token here unconditionally. What the unreconstructed
            // tokenizer yields at end of file is not established, so nothing is assigned.
            return false;
        }
        const char* value = tokens[index + 1];
        language.assign(value == nullptr ? "" : value);
        return true;
    }
    return false;
}

StartupMessage startup_already_running_message(const char* language) {
    // 008f8334: the english pair is loaded before the first comparison and survives every
    // non-match.
    StartupMessage message{kTextEnglish, kCaptionEnglish};
    if (language_equals_00425850(language, kStartupLanguageEnglish)) {
        return message;
    }
    if (language_equals_00425850(language, kStartupLanguageFrench)) {
        message.text = kTextFrench;
        message.caption = kCaptionFrench;
        return message;
    }
    if (language_equals_00425850(language, kStartupLanguageItalian)) {
        message.text = kTextItalian;
        message.caption = kCaptionItalian;
        return message;
    }
    if (language_equals_00425850(language, kStartupLanguageGerman)) {
        message.text = kTextGerman;
        message.caption = kCaptionGerman;
        return message;
    }
    if (language_equals_00425850(language, kStartupLanguageSpanish)) {
        // 008f83b5 falls into 008f83ba, which reloads the english caption, so spanish shares
        // the english caption in the original.
        message.text = kTextSpanish;
    }
    return message;
}

std::wstring startup_widen_path(const char* ansi_path) {
    std::wstring wide;
    if (ansi_path == nullptr) {
        return wide;
    }
    const std::size_t length = std::strlen(ansi_path);
    wide.reserve(length);
    for (std::size_t index = 0; index < length; ++index) {
        const unsigned char byte = static_cast<unsigned char>(ansi_path[index]);
        wide.push_back(static_cast<wchar_t>(byte));
    }
    return wide;
}

int run_win_main(const WinMainArguments& arguments, StartupHost& host) {
    // 008f81f0 never touches its incoming frame above the return address: the instance
    // handles, the command line and the show command are all unused.
    static_cast<void>(arguments);

    // 008f81f8..008f82cc. CoUninitialize runs even when either initializer failed, because
    // both failure branches jump straight to it.
    if (host.com_initialize() >= 0 && host.com_initialize_security() >= 0) {
        const bool have_game_explorer = host.game_explorer_create();
        if (have_game_explorer) {
            char directory[kCurrentDirectoryCapacity] = {};
            host.current_directory(directory, kCurrentDirectoryCapacity);

            // 008f825f..008f8288: the suffix is appended in place, then the whole path is
            // widened. The native buffer leaves only 0xc10 bytes for a 0xbfe byte directory
            // plus the 26 character suffix, so a maximal current directory would overrun it
            // by eight bytes; the reconstruction sizes the string instead of reproducing it.
            std::string gdf_path(directory);
            gdf_path += kGdfBinarySuffix;
            const std::wstring wide_path = startup_widen_path(gdf_path.c_str());

            // 008f828d: an empty widened path has no buffer, and the shared empty wide
            // string at 00f89988 is substituted. The suffix makes this unreachable in
            // practice; it is kept because the original tests it.
            const wchar_t* path_argument = wide_path.empty() ? L"" : wide_path.c_str();

            if (!host.game_explorer_verify_access(path_argument)) {
                // 008f82ee: parental controls denied the title. The native exit runs before
                // the interface is released and before CoUninitialize, and does not return.
                host.exit_process(0);
                return 0;
            }
            // 008f82b7: release the widened path.
        }
        if (have_game_explorer) {
            host.game_explorer_release();
        }
    }
    host.com_uninitialize();

    // 008f82d2..008f82f5. The per-thread slot is a one byte allocation; a failed allocation
    // silently skips registration and the matching teardown.
    host.random_threads_initialize();
    void* thread_slot = host.allocate_thread_slot();
    if (thread_slot != nullptr) {
        host.random_threads_register_current();
    }

    // 008f82f7..008f831c. The already-running path needs both a live handle and
    // ERROR_ALREADY_EXISTS; a failed CreateMutexA lets the game start.
    const SingleInstanceMutex mutex = host.create_single_instance_mutex(kSingleInstanceMutexName);
    if (mutex.handle != nullptr && mutex.already_exists) {
        // 008f8322..008f83f9. This exit closes nothing: the mutex handle stays open, the
        // thread slot is not released, and the random subsystem is not shut down.
        const std::string language = host.resolve_language();
        const StartupMessage message = startup_already_running_message(language.c_str());
        host.error_message_box(message.text, message.caption);
        return 0;
    }

    // 008f83fc..008f8496.
    host.set_thread_affinity_to_first_processor();
    host.publish_game_resource_factory();
    host.application_construct();
    host.application_initialize(kApplicationInitializeFlags, kApplicationInitializeMode);
    host.platform_run_loop_dispatch();
    host.application_shutdown();
    host.application_destruct();
    host.destroy_singleton_lifetime_manager();
    host.close_mutex(mutex.handle);
    if (thread_slot != nullptr) {
        host.random_threads_unregister_current();
        host.release_thread_slot(thread_slot);
    }
    host.random_threads_shutdown();
    return 0;
}

}  // namespace bsp
