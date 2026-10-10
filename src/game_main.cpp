// bsp_game.exe, milestones 1 through 2c: the reconstructed startup spine running as a
// Win32 process, with the phase-2 virtual file system and hardware probe, the phase-5
// settings load, the phase-6 locale tables and parser registrations, the phase-7 fonts
// and GUI startup, the phase-9 decal definitions, the title bring-up 004c9a70 and the
// front-end screen registry all doing real work.
//
// Entry sequence 008f81f0 (docs/WINMAIN_STARTUP.md) drives the whole run. Everything this
// file adds on top of run_win_main is process plumbing: the option parsing for --frames,
// --log, --game-root and --vfs-probe, the console attachment that lets a windowed process
// report to the terminal that started it, and the exit code. Evidence:
// docs/GAME_EXECUTABLE.md.
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <dbghelp.h>
#include <shellapi.h>
#include <wincrypt.h>

#include <cfloat>
#include <cstdint>
#include <cstdlib>
#include <array>
#include <cstdio>
#include <cstring>
#include <cwchar>
#include <filesystem>
#include <fstream>
#include <memory>
#include <string>
#include <exception>
#include <stdexcept>
#include <utility>
#include <vector>

#include "bsp/game_hosts.hpp"
#include "bsp/game_hosts_singletons.hpp"
#include "bsp/game_hosts_vfs.hpp"
#include "bsp/game_native_data_bootstrap.hpp"
#include "bsp/game_native_mutable_crt_data.hpp"
#include "bsp/game_native_particle_pools.hpp"
#include "bsp/game_native_weak_pool.hpp"
#include "bsp/game_native_resource_pools.hpp"
#include "bsp/game_native_surface_pool.hpp"
#include "bsp/game_native_texture_pool.hpp"
#include "bsp/game_native_graphics_pools.hpp"
#include "bsp/game_native_material_process.hpp"
#include "bsp/game_native_shader_process.hpp"
#include "bsp/game_native_hardware_layout_tree.hpp"
#include "bsp/game_native_lua_globals.hpp"
#include "bsp/game_native_dyn_process.hpp"
#include "bsp/game_native_physical_pool.hpp"
#include "bsp/game_native_vfs_application.hpp"
#include "bsp/game_native_vfs_runtime.hpp"
#include "bsp/native_physical_failure_entries.hpp"
#include "bsp/native_string.hpp"
#include "bsp/native_singleton_vector_leaves.hpp"
#include "bsp/native_vfs_owner_services.hpp"
#include "bsp/native_vfs_runtime_bindings.hpp"
#include "bsp/winmain_startup.hpp"

namespace bsp::game {
// Harness only (packet cc9_tooling_1), defined beside the frame-jitter hook in
// src/game_hosts_mission.cpp and declared here, their only caller, so no shared header changes.
long harness_mission_frame() noexcept;
void arm_harness_crash_test(long mission_frame) noexcept;
}  // namespace bsp::game

namespace {

// A WIN32-subsystem process has no console of its own. When it was started from a shell,
// borrow that console so the run log is visible where the command was typed.
void attach_parent_console() {
    if (!AttachConsole(ATTACH_PARENT_PROCESS)) return;
    FILE* stream = nullptr;
    freopen_s(&stream, "CONOUT$", "w", stdout);
    freopen_s(&stream, "CONOUT$", "w", stderr);
}

// These explicit spans are the union of the original read-only 64-KB bands
// consumed by the raw VFS entry path and the composed particle graph. The mapper
// verifies the entire original PE before it admits any table/literal read.
constexpr std::array<bsp::game::GameNativeDataSpan, 7> native_data_spans{{
    {0x00ce2000u, 1}, {0x00cf0000u, 1}, {0x00d00000u, 1},
    {0x00d10000u, 1}, {0x00d50000u, 1}, {0x00d60000u, 1},
    {0x00d70000u, 1}
}};
constexpr char handoff_prefix[] = "--bsp-native-data-handoff=";

// ---- Harness crash record (packet cc9_tooling_1). Harness only: nothing here is read by the
// frame or the simulation. An access violation used to leave no trace in the run log; the
// top-level filter writes one line (code, faulting address, module + offset, access, thread,
// mission frame), tries a minidump next to the log, and ends the process with the exception
// code. The reconstructed CRT failure paths (src/native_crt_*_failure.cpp) clear the filter on
// purpose, as the image's CRT does, so a /GS or invalid-parameter failure still bypasses it.
bsp::game::GameHostLog* g_crash_log = nullptr;
char g_crash_dump_path[MAX_PATH]{};

LONG WINAPI harness_crash_record(EXCEPTION_POINTERS* info) {
    const EXCEPTION_RECORD* const record = info != nullptr ? info->ExceptionRecord : nullptr;
    const unsigned long code = record != nullptr ? record->ExceptionCode : 0;
    const auto address = record != nullptr
        ? reinterpret_cast<std::uintptr_t>(record->ExceptionAddress) : 0;
    char module_path[MAX_PATH] = "(no module)";
    std::uintptr_t base = 0;
    HMODULE module = nullptr;
    if (address != 0 && GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS
            | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
            reinterpret_cast<LPCSTR>(address), &module) && module != nullptr) {
        base = reinterpret_cast<std::uintptr_t>(module);
        GetModuleFileNameA(module, module_path, MAX_PATH);
    }
    const char* module_name = std::strrchr(module_path, '\\');
    module_name = module_name != nullptr ? module_name + 1 : module_path;
    char access[64] = "";
    if (code == EXCEPTION_ACCESS_VIOLATION && record->NumberParameters >= 2) {
        const ULONG_PTR kind = record->ExceptionInformation[0];
        std::snprintf(access, sizeof(access), " %s %08lx",
            kind == 0 ? "reading" : kind == 1 ? "writing" : "executing",
            static_cast<unsigned long>(record->ExceptionInformation[1]));
    }
    char frame[48] = "none (not in a mission frame yet)";
    const long mission_frame = bsp::game::harness_mission_frame();
    if (mission_frame >= 0) std::snprintf(frame, sizeof(frame), "%ld", mission_frame);
    char line[768];
    std::snprintf(line, sizeof(line), "harness crash: exception %08lx at %08lx%s module %s+%lx "
        "(base %08lx) thread %lu mission_frame %s", code, static_cast<unsigned long>(address),
        access, module_name, static_cast<unsigned long>(address - base),
        static_cast<unsigned long>(base), GetCurrentThreadId(), frame);
    if (g_crash_log != nullptr) g_crash_log->note(line);
    else std::fprintf(stderr, "%s\n", line);

    // The minidump is optional: dbghelp is loaded here so the build gains no import.
    using WriteDump = BOOL(WINAPI*)(HANDLE, DWORD, HANDLE, MINIDUMP_TYPE,
        PMINIDUMP_EXCEPTION_INFORMATION, PMINIDUMP_USER_STREAM_INFORMATION,
        PMINIDUMP_CALLBACK_INFORMATION);
    if (g_crash_dump_path[0] != '\0') {
        const char* outcome = "dbghelp.dll unavailable";
        if (HMODULE dbghelp = LoadLibraryA("dbghelp.dll")) {
            const auto write = reinterpret_cast<WriteDump>(
                GetProcAddress(dbghelp, "MiniDumpWriteDump"));
            const HANDLE file = CreateFileA(g_crash_dump_path, GENERIC_WRITE, 0, nullptr,
                CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
            if (write != nullptr && file != INVALID_HANDLE_VALUE) {
                MINIDUMP_EXCEPTION_INFORMATION exception{GetCurrentThreadId(), info, FALSE};
                outcome = write(GetCurrentProcess(), GetCurrentProcessId(), file,
                    MiniDumpNormal, info != nullptr ? &exception : nullptr, nullptr, nullptr)
                    ? "written" : "MiniDumpWriteDump failed";
            } else {
                outcome = "cannot create the file";
            }
            if (file != INVALID_HANDLE_VALUE) CloseHandle(file);
        }
        std::snprintf(line, sizeof(line), "harness crash: minidump %s: %s", g_crash_dump_path,
            outcome);
        if (g_crash_log != nullptr) g_crash_log->note(line);
    }
    return EXCEPTION_EXECUTE_HANDLER;  // no WER dialog; the process exits with `code`
}

// Remove `--crash-test [N]` (hidden, harness only) before the public parser sees argv. The
// bootstrap child receives the same command line and strips it the same way. Returns the
// mission frame to fault before, or -1.
long take_crash_test_option(std::vector<char*>& argv) {
    long frame = -1;
    for (auto it = argv.begin(); it != argv.end();) {
        if (std::strcmp(*it, "--crash-test") != 0) { ++it; continue; }
        it = argv.erase(it);
        frame = 10;
        if (it != argv.end() && **it != '\0'
                && std::strspn(*it, "0123456789") == std::strlen(*it)) {
            frame = std::strtol(*it, nullptr, 10);
            it = argv.erase(it);
        }
    }
    return frame;
}

// The bootstrap appends this token last. Exclude it before the public parser
// sees argv, including when a public option is missing its required value.
bool is_handoff_child() {
    if (__argc < 2) return false;
    const char* const token = __argv[__argc - 1];
    const std::size_t prefix_size = sizeof(handoff_prefix) - 1;
    if (std::strncmp(token, handoff_prefix, prefix_size) != 0 ||
        std::strlen(token + prefix_size) != 8) return false;
    unsigned long value = 0;
    for (const char* digit = token + prefix_size; *digit; ++digit) {
        const unsigned nibble = (*digit >= '0' && *digit <= '9') ? *digit - '0' :
            (*digit >= 'A' && *digit <= 'F') ? *digit - 'A' + 10 :
            (*digit >= 'a' && *digit <= 'f') ? *digit - 'a' + 10 : 16;
        if (nibble == 16) return false;
        value = (value << 4) | nibble;
    }
    DWORD flags = 0;
    return GetHandleInformation(reinterpret_cast<HANDLE>(value), &flags) &&
        (flags & HANDLE_FLAG_INHERIT);
}

// Windows command-line quoting: double backslashes before quotes and before
// the closing quote. Rebuild the public argument vector without changing its
// token boundaries, then let the bootstrap append its private handle token.
std::wstring quote_argument(const std::wstring& argument) {
    std::wstring quoted(1, L'"');
    std::size_t slashes = 0;
    for (const wchar_t ch : argument) {
        if (ch == L'\\') { ++slashes; continue; }
        if (ch == L'"') {
            quoted.append(slashes * 2 + 1, L'\\');
            quoted.push_back(ch);
        } else {
            quoted.append(slashes, L'\\');
            quoted.push_back(ch);
        }
        slashes = 0;
    }
    quoted.append(slashes * 2, L'\\');
    quoted.push_back(L'"');
    return quoted;
}

std::wstring child_arguments() {
    int count = 0;
    LPWSTR* const arguments = CommandLineToArgvW(GetCommandLineW(), &count);
    if (!arguments) throw std::runtime_error("Cannot read bsp_game command line");
    std::wstring result;
    try {
        for (int index = 1; index < count; ++index) {
            if (std::wcsncmp(arguments[index], L"--bsp-native-data-handoff=", 26) == 0)
                throw std::invalid_argument("The --bsp-native-data-handoff= prefix is reserved for the child bootstrap");
            if (!result.empty()) result.push_back(L' ');
            result += quote_argument(arguments[index]);
        }
    } catch (...) { LocalFree(arguments); throw; }
    LocalFree(arguments);
    return result;
}

std::filesystem::path own_executable() {
    std::wstring path(MAX_PATH, L'\0');
    for (;;) {
        const DWORD size = GetModuleFileNameW(nullptr, path.data(),
            static_cast<DWORD>(path.size()));
        if (!size) throw std::runtime_error("Cannot resolve bsp_game executable");
        if (size < path.size()) { path.resize(size); return path; }
        if (path.size() > 32768) throw std::runtime_error("bsp_game executable path is too long");
        path.resize(path.size() * 2);
    }
}

std::filesystem::path original_executable(const bsp::game::GameExecutableOptions& options) {
    const auto root = options.game_root.empty() ? std::filesystem::current_path()
        : std::filesystem::absolute(std::filesystem::path(options.game_root));
    const auto image = root / "battlestationspacific.exe";
    if (!std::filesystem::is_regular_file(image))
        throw std::runtime_error("Supported original battlestationspacific.exe is absent from game root: "
            + image.string());
    return image;
}

void check_original_identity(const std::filesystem::path& image) {
    constexpr std::uintmax_t supported_size = 12223752;
    constexpr std::array<BYTE, 32> supported_sha256{{
        0xb6, 0x82, 0xa8, 0x2c, 0x52, 0xf8, 0x1f, 0x95,
        0x7b, 0x2c, 0x70, 0x22, 0x20, 0x77, 0x30, 0x5a,
        0x93, 0x3f, 0x72, 0x48, 0x16, 0x86, 0xc8, 0x88,
        0x43, 0x07, 0x7f, 0x71, 0x4b, 0x95, 0x6d, 0xd6
    }};
    if (std::filesystem::file_size(image) != supported_size)
        throw std::runtime_error("Original executable has an unsupported size: " +
            image.string());
    std::ifstream file(image, std::ios::binary);
    if (!file) throw std::runtime_error("Cannot read original executable: " + image.string());
    struct HashContext {
        HCRYPTPROV provider = 0;
        HCRYPTHASH hash = 0;
        ~HashContext() {
            if (hash) CryptDestroyHash(hash);
            if (provider) CryptReleaseContext(provider, 0);
        }
    } context;
    if (!CryptAcquireContextW(&context.provider, nullptr, nullptr, PROV_RSA_AES,
            CRYPT_VERIFYCONTEXT) ||
        !CryptCreateHash(context.provider, CALG_SHA_256, 0, 0, &context.hash))
        throw std::runtime_error("Cannot initialize original executable SHA-256 check");
    std::array<char, 65536> bytes{};
    for (;;) {
        file.read(bytes.data(), bytes.size());
        const std::streamsize count = file.gcount();
        if (count > 0 && !CryptHashData(context.hash,
                reinterpret_cast<const BYTE*>(bytes.data()), static_cast<DWORD>(count), 0))
            throw std::runtime_error("Cannot hash original executable");
        if (count < static_cast<std::streamsize>(bytes.size())) {
            if (!file.eof()) throw std::runtime_error("Cannot read entire original executable");
            break;
        }
    }
    std::array<BYTE, 32> digest{};
    DWORD size = static_cast<DWORD>(digest.size());
    if (!CryptGetHashParam(context.hash, HP_HASHVAL, digest.data(), &size, 0) ||
        size != digest.size())
        throw std::runtime_error("Cannot finish original executable SHA-256 check");
    if (digest != supported_sha256)
        throw std::runtime_error("Original executable SHA-256 is unsupported: " +
            image.string());
}

int run_bootstrap_parent(const bsp::game::GameExecutableOptions& options) {
    check_original_identity(original_executable(options));
    const auto child_path = own_executable();
    bsp::game::GameNativeDataBootstrapChild child(child_path, child_arguments(),
        native_data_spans.data(), native_data_spans.size(),
        bsp::game::GameNativeDataPlan::CanonicalCrtV2);
    child.reserve_and_resume();
    child.wait_for_mapping(30000);
    const HANDLE process = static_cast<HANDLE>(child.process_handle());
    if (WaitForSingleObject(process, INFINITE) != WAIT_OBJECT_0)
        throw std::runtime_error("Cannot wait for native-data child exit");
    DWORD exit_code = 0;
    if (!GetExitCodeProcess(process, &exit_code))
        throw std::runtime_error("Cannot read native-data child exit code");
    return static_cast<int>(exit_code);
}

// One Source-owner check, using the same concrete owners as ordinary startup.
// The heap-owned application and host are outside the try block: an interrupted
// native initialization/drain must reach the retention handler before either
// owner is destroyed. Comparison buffers never become native owners.
int qualify_vfs_failure_owner(bsp::game::GameHostLog& log,
    bsp::game::GameNativeReadOnlyData& data, const std::filesystem::path& original_image) {
    std::unique_ptr<bsp::game::GameSingletonHost> host;
    std::unique_ptr<bsp::game::GameNativeVfsApplication> application;
    const char* phase = "construct";
    try {
        const auto require = [](bool condition, const char* reason) {
            if (!condition) throw std::runtime_error(reason);
        };
        const auto word = [](const void* owner, std::size_t offset) {
            std::uint32_t value;
            std::memcpy(&value, static_cast<const unsigned char*>(owner) + offset, sizeof value);
            return value;
        };
        const auto record = [&](const char* stage, const char* kind,
            const void* bytes, std::size_t count) {
            constexpr char digits[] = "0123456789abcdef";
            const auto* source = static_cast<const unsigned char*>(bytes);
            for (std::size_t offset = 0; offset < count; offset += 64) {
                const auto chunk = (count - offset < 64) ? count - offset : 64;
                std::string hex(chunk * 2, '0');
                for (std::size_t index = 0; index < chunk; ++index) {
                    const auto value = source[offset + index];
                    hex[index * 2] = digits[value >> 4];
                    hex[index * 2 + 1] = digits[value & 15];
                }
                log.notef("vfs_failure_owner record phase=%s kind=%s offset=%zu hex=%s",
                    stage, kind, offset, hex.c_str());
            }
        };
        const auto unimplemented_before = log.unimplemented_count();
        host = std::make_unique<bsp::game::GameSingletonHost>(log);
        application = std::make_unique<bsp::game::GameNativeVfsApplication>(
            log, *host, data, original_image);
        phase = "initialize_core";
        log.note("vfs_failure_owner initialize_core begin");
        application->initialize_core();
        const auto pool_status = application->physical_pool_registration_status();
        require(pool_status.has_value(), "physical pool initialization did not return");
        log.notef("vfs_failure_owner initialize_core completed physical_pool_atexit=%d",
            *pool_status);

        phase = "capture_owner";
        auto& runtime = application->runtime();
        auto services = application->borrow_raw_services();
        auto& publication = application->owners().vfs_publication_0109ceec();
        void* const manager = runtime.actual_manager();
        void* const singleton = host->manager_publication_01090aa0();
        require(manager && singleton && publication == manager &&
            services.actual_vfs_publication_0109ceec == manager,
            "application, runtime and service owner publications disagree");
        require(word(manager, 0) == 0x00d68d04 && word(manager, 0x90) == 0x00530620 &&
            word(manager, 0x8c) == 0x00735b30 && word(manager, 0x38) == 3,
            "genuine VFS core profile, callbacks or factory count differ");
        const auto begin = word(singleton, 4);
        const auto end = word(singleton, 8);
        const auto capacity = word(singleton, 0x0c);
        require(begin && end >= begin && capacity >= end && (end - begin) % 4 == 0,
            "actual singleton registration bounds are invalid");
        const auto slot_count = bsp::count_native_singleton_slots_00bcf910(singleton, nullptr);
        require(slot_count && slot_count == (end - begin) / 4,
            "actual singleton registration count differs");
        const auto* slots = reinterpret_cast<const void*>(begin);
        bool registered = false;
        for (std::uint32_t index = 0; index < slot_count; ++index)
            registered = registered || word(slots, index * 4u) ==
                reinterpret_cast<std::uintptr_t>(manager);
        require(registered, "actual A0 owner is not registered in the shared manager");

        std::array<unsigned char, 0xa0> original{};
        std::array<unsigned char, 0x14> singleton_record{};
        std::vector<unsigned char> registrations(end - begin);
        std::memcpy(original.data(), manager, original.size());
        std::memcpy(singleton_record.data(), singleton, singleton_record.size());
        std::memcpy(registrations.data(), slots, registrations.size());
        log.notef("vfs_failure_owner owner=%p singleton=%p vfs_cell=%p singleton_cell=%p "
            "slots=%08X count=%u factories=3", manager, singleton,
            const_cast<void*>(static_cast<const volatile void*>(&publication)),
            const_cast<void*>(static_cast<const volatile void*>(&host->manager_publication_01090aa0())),
            static_cast<unsigned>(begin), static_cast<unsigned>(slot_count));
        record("original", "a0", original.data(), original.size());
        record("original", "singleton", singleton_record.data(), singleton_record.size());
        record("original", "registrations", registrations.data(), registrations.size());
        const auto unchanged = [&](const char* stage, const auto& expected) {
            require(publication == manager && services.actual_vfs_publication_0109ceec == manager &&
                host->manager_publication_01090aa0() == singleton,
                "owner publication changed during the borrowed use");
            require(std::memcmp(manager, expected.data(), expected.size()) == 0,
                "VFS owner record changed outside the expected publication");
            require(std::memcmp(singleton, singleton_record.data(), singleton_record.size()) == 0,
                "singleton registration header changed");
            require(std::memcmp(slots, registrations.data(), registrations.size()) == 0,
                "singleton registrations changed");
            record(stage, "a0", manager, expected.size());
            record(stage, "singleton", singleton, singleton_record.size());
            record(stage, "registrations", slots, registrations.size());
            log.notef("vfs_failure_owner phase=%s records_and_publications_equal=1", stage);
        };

        phase = "original_finite_dispatch";
        services.bindings.open_failure_entry(word(manager, 0x90), manager);
        unchanged(phase, original);

        phase = "qualify_loaded_code";
        const auto module = GetModuleHandleW(nullptr);
        require(module != nullptr, "rebuilt game module is unavailable");
        const auto module_base = reinterpret_cast<std::uintptr_t>(module);
        const auto* dos = reinterpret_cast<const IMAGE_DOS_HEADER*>(module);
        require(dos->e_magic == IMAGE_DOS_SIGNATURE && dos->e_lfanew > 0 && dos->e_lfanew < 0x1000,
            "rebuilt game DOS header is invalid");
        const auto* pe = reinterpret_cast<const IMAGE_NT_HEADERS32*>(module_base + dos->e_lfanew);
        require(pe->Signature == IMAGE_NT_SIGNATURE && pe->FileHeader.Machine == IMAGE_FILE_MACHINE_I386 &&
            pe->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR32_MAGIC,
            "rebuilt game is not the expected Win32 module");
        log.notef("vfs_failure_owner module=%s base=%08X preferred=%08X image_bytes=%u timestamp=%08X",
            own_executable().string().c_str(), static_cast<unsigned>(module_base),
            static_cast<unsigned>(pe->OptionalHeader.ImageBase),
            static_cast<unsigned>(pe->OptionalHeader.SizeOfImage),
            static_cast<unsigned>(pe->FileHeader.TimeDateStamp));
        const auto qualify = [&](const char* symbol, std::uintptr_t address, const auto& expected) {
            HMODULE actual_module = nullptr;
            require(GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
                GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, reinterpret_cast<LPCSTR>(address),
                &actual_module) && actual_module == module,
                "named raw entry is outside the rebuilt game module");
            require(address >= module_base && address - module_base < pe->OptionalHeader.SizeOfImage &&
                expected.size() <= pe->OptionalHeader.SizeOfImage - (address - module_base),
                "named raw entry exceeds the rebuilt module");
            MEMORY_BASIC_INFORMATION page{};
            require(VirtualQuery(reinterpret_cast<const void*>(address), &page, sizeof page) == sizeof page &&
                page.State == MEM_COMMIT && page.AllocationBase == module && !(page.Protect & PAGE_GUARD) &&
                ((page.Protect & 0xff) == PAGE_EXECUTE_READ ||
                 (page.Protect & 0xff) == PAGE_EXECUTE_READWRITE ||
                 (page.Protect & 0xff) == PAGE_EXECUTE_WRITECOPY) &&
                address - reinterpret_cast<std::uintptr_t>(page.BaseAddress) <= page.RegionSize &&
                expected.size() <= page.RegionSize - (address - reinterpret_cast<std::uintptr_t>(page.BaseAddress)),
                "named raw entry is not in a readable executable module region");
            const auto* code = reinterpret_cast<const volatile unsigned char*>(address);
            for (std::size_t index = 0; index < expected.size(); ++index)
                require(code[index] == expected[index], "named raw entry whole bytes differ");
            log.notef("vfs_failure_owner code=%s address=%08X rva=%08X bytes=%zu protect=%08X",
                symbol, static_cast<unsigned>(address), static_cast<unsigned>(address - module_base),
                expected.size(), static_cast<unsigned>(page.Protect));
            record("loaded_code", symbol, reinterpret_cast<const void*>(address), expected.size());
        };
        constexpr std::array<unsigned char, 1> c3{{0xc3}};
        constexpr std::array<unsigned char, 11> notifier{{0x8b,0x81,0x90,0,0,0,0xff,0xd0,0xc2,4,0}};
        const auto target = reinterpret_cast<std::uintptr_t>(&bsp::raw_ignore_native_vfs_mount_failure_00530620);
        qualify("raw_ignore_native_vfs_mount_failure_00530620", target, c3);
        qualify("raw_notify_native_vfs_request_failure_00bd9e30",
            reinterpret_cast<std::uintptr_t>(&bsp::raw_notify_native_vfs_request_failure_00bd9e30), notifier);

        phase = "publish";
        require(application->publish_and_borrow_raw_failure_manager() == manager,
            "application publisher returned another owner");
        auto published = original;
        const auto target_word = static_cast<std::uint32_t>(target);
        std::memcpy(published.data() + 0x90, &target_word, sizeof target_word);
        unchanged(phase, published);
        phase = "repeat_publish";
        require(application->publish_and_borrow_raw_failure_manager() == manager,
            "repeated application publisher returned another owner");
        unchanged(phase, published);
        phase = "published_finite_dispatch";
        services.bindings.open_failure_entry(word(manager, 0x90), manager);
        unchanged(phase, published);

        phase = "raw_notifier";
        constexpr std::uint32_t forwarded_edx = 0x13579bdf;
        constexpr std::uint32_t discarded_word = 0x2468ace0;
        log.notef("vfs_failure_owner notifier begin forwarded_edx=%08X discarded_word=%08X",
            static_cast<unsigned>(forwarded_edx), static_cast<unsigned>(discarded_word));
        bsp::raw_notify_native_vfs_request_failure_00bd9e30(manager, forwarded_edx, discarded_word);
        unchanged(phase, published);
        log.note("vfs_failure_owner notifier returned calls=1");

        // End all borrowed record use before this call. Only still-live owner
        // publication cells and the guarded application API are inspected later.
        phase = "shared_drain";
        log.note("vfs_failure_owner shared_drain begin borrowed_uses_finished=1");
        host->shutdown();
        require(publication == nullptr && host->manager_publication_01090aa0() == nullptr,
            "shared drain left an owner publication live");
        phase = "retired_guard";
        bool rejected = false;
        try { (void)application->publish_and_borrow_raw_failure_manager(); }
        catch (const std::logic_error& error) {
            rejected = std::strcmp(error.what(),
                "Native VFS core is not available for a failure-manager borrow") == 0;
        }
        require(rejected, "retired application did not reject before owner access");
        require(log.unimplemented_count() == unimplemented_before,
            "diagnostic reached an unimplemented host service");
        log.note("vfs_failure_owner shared_drain completed vfs_null=1 singleton_null=1 retired_rejected=1");
        phase = "destroy_drained_owners";
        application.reset();
        host.reset();
        log.note("vfs_failure_owner PASS owners_destroyed=1 normal_CRT_exit=1");
        return 0;
    } catch (const std::exception& error) {
        log.notef("vfs_failure_owner FAIL phase=%s reason=%s retention=_Exit", phase, error.what());
    } catch (...) {
        log.notef("vfs_failure_owner FAIL phase=%s reason=unknown retention=_Exit", phase);
    }
    log.close();
    std::fflush(stdout);
    std::fflush(stderr);
    std::_Exit(3);
}

int qualify_vfs_physical_read_owner(bsp::game::GameHostLog& log,
    bsp::game::GameNativeReadOnlyData& data, const std::filesystem::path& original_image) {
    std::unique_ptr<bsp::game::GameSingletonHost> host;
    std::unique_ptr<bsp::game::GameNativeVfsApplication> application;
    const char* phase = "construct";
    try {
        const auto require = [](bool condition, const char* reason) {
            if (!condition) throw std::runtime_error(reason);
        };
        const auto word = [](const void* owner, std::size_t offset) {
            std::uint32_t value;
            std::memcpy(&value, static_cast<const unsigned char*>(owner) + offset, sizeof value);
            return value;
        };
        const auto record = [&](const char* stage, const char* kind,
            const void* bytes, std::size_t count) {
            constexpr char digits[] = "0123456789abcdef";
            const auto* source = static_cast<const unsigned char*>(bytes);
            for (std::size_t offset = 0; offset < count; offset += 64) {
                const auto chunk = (count - offset < 64) ? count - offset : 64;
                std::string hex(chunk * 2, '0');
                for (std::size_t index = 0; index < chunk; ++index) {
                    const auto value = source[offset + index];
                    hex[index * 2] = digits[value >> 4];
                    hex[index * 2 + 1] = digits[value & 15];
                }
                log.notef("vfs_physical_read record phase=%s kind=%s address=%08X offset=%zu hex=%s",
                    stage, kind, static_cast<unsigned>(reinterpret_cast<std::uintptr_t>(bytes)),
                    offset, hex.c_str());
            }
        };
        const auto unimplemented_before = log.unimplemented_count();
        host = std::make_unique<bsp::game::GameSingletonHost>(log);
        application = std::make_unique<bsp::game::GameNativeVfsApplication>(
            log, *host, data, original_image);
        phase = "initialize_core";
        application->initialize_core();
        const auto pool_status = application->physical_pool_registration_status();
        require(pool_status.has_value() && *pool_status == 0,
            "actual physical provider pool did not register normal CRT cleanup");
        log.notef("vfs_physical_read initialized physical_pool_atexit=%d physical_process_bytes=%zu",
            *pool_status, sizeof(bsp::game::GameNativePhysicalPoolProcess));
        auto& runtime = application->runtime();
        auto services = application->borrow_raw_services();
        auto& owners = application->owners();
        auto& streams = owners.streams();
        auto& publication = owners.vfs_publication_0109ceec();
        void* const manager = runtime.actual_manager();
        require(manager && publication == manager && services.actual_vfs_publication_0109ceec == manager &&
            &streams.physical.manager_0109ceec == &publication &&
            &streams.pool_0109dc28 == &owners.stream_pool_publication_0109dc28(),
            "genuine application and physical context owner cells disagree");
        require(word(manager, 0) == 0x00d68d04 && word(manager, 0x90) == 0x00530620 &&
            word(manager, 0x8c) == 0x00735b30 && word(manager, 0x38) == 3,
            "genuine VFS core profile, callbacks or factory count differ");
        require(!owners.stream_pool_publication_0109dc28() && !owners.batch_lock_publication_0109dbbc(),
            "fresh read diagnostic already has a stream pool or batch lock");
        const auto graph = [&](const char* stage) {
            require(publication == manager && services.actual_vfs_publication_0109ceec == manager,
                "genuine VFS publication changed before the shared drain");
            void* const singleton = host->manager_publication_01090aa0();
            require(singleton != nullptr, "shared singleton publication disappeared");
            const auto begin = word(singleton, 4), end = word(singleton, 8), capacity = word(singleton, 0x0c);
            require(begin && end >= begin && capacity >= end && (end - begin) % 4 == 0,
                "actual singleton registration bounds are invalid");
            const auto count = bsp::count_native_singleton_slots_00bcf910(singleton, nullptr);
            require(count && count == (end - begin) / 4 && count < 256,
                "actual singleton registration count is invalid");
            bool has_manager = false, has_pool = false, has_lock = false;
            const auto* slots = reinterpret_cast<const void*>(begin);
            auto* pool = owners.stream_pool_publication_0109dc28();
            auto* lock = owners.batch_lock_publication_0109dbbc();
            for (std::uint32_t index = 0; index < count; ++index) {
                const auto value = word(slots, index * 4u);
                has_manager = has_manager || value == reinterpret_cast<std::uintptr_t>(manager);
                has_pool = has_pool || value == reinterpret_cast<std::uintptr_t>(pool);
                has_lock = has_lock || value == reinterpret_cast<std::uintptr_t>(lock);
            }
            require(has_manager && (!pool || has_pool) && (!lock || has_lock),
                "actual owner, stream pool or lock is absent from shared registration");
            record(stage, "a0", manager, 0xa0);
            record(stage, "singleton", singleton, 0x14);
            record(stage, "registrations", slots, end - begin);
            if (pool) {
                require(word(pool, 0) == 0x00d68ec0 && word(pool, 8) <= word(pool, 0x0c) &&
                    word(pool, 8) <= 1, "actual stream pool profile or bounds differ");
                record(stage, "stream_pool", pool, 0x10);
                if (word(pool, 8)) {
                    require(word(pool, 4) != 0, "nonempty stream pool has no pointer storage");
                    record(stage, "pool_slots", reinterpret_cast<const void*>(word(pool, 4)), word(pool, 8) * 4u);
                }
            }
            if (lock) record(stage, "batch_lock", lock, 8);
            log.notef("vfs_physical_read graph phase=%s manager=%p singleton=%p slots=%u pool=%p lock=%p",
                stage, manager, singleton, count, pool, lock);
        };
        graph("initialized");

        phase = "qualify_loaded_code";
        const auto module = GetModuleHandleW(nullptr);
        require(module != nullptr, "rebuilt game module is unavailable");
        const auto base = reinterpret_cast<std::uintptr_t>(module);
        const auto* dos = reinterpret_cast<const IMAGE_DOS_HEADER*>(module);
        require(dos->e_magic == IMAGE_DOS_SIGNATURE && dos->e_lfanew > 0 && dos->e_lfanew < 0x1000,
            "rebuilt game DOS header is invalid");
        const auto* pe = reinterpret_cast<const IMAGE_NT_HEADERS32*>(base + dos->e_lfanew);
        require(pe->Signature == IMAGE_NT_SIGNATURE && pe->FileHeader.Machine == IMAGE_FILE_MACHINE_I386 &&
            pe->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR32_MAGIC,
            "rebuilt game is not the expected Win32 module");
        const auto target = reinterpret_cast<std::uintptr_t>(&bsp::raw_ignore_native_vfs_mount_failure_00530620);
        HMODULE code_module = nullptr;
        MEMORY_BASIC_INFORMATION memory{};
        require(GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
            reinterpret_cast<LPCWSTR>(target), &code_module) && code_module == module &&
            target >= base && target - base < pe->OptionalHeader.SizeOfImage &&
            VirtualQuery(reinterpret_cast<const void*>(target), &memory, sizeof memory) == sizeof memory &&
            memory.State == MEM_COMMIT && memory.AllocationBase == module &&
            memory.Protect == PAGE_EXECUTE_READ && *reinterpret_cast<const volatile unsigned char*>(target) == 0xc3,
            "named Source C3 is outside the qualified loaded game image");
        log.notef("vfs_physical_read module=%s base=%08X preferred=%08X image_bytes=%08X timestamp=%08X "
            "c3=%08X c3_rva=%08X protection=%08X",
            own_executable().string().c_str(), static_cast<unsigned>(base), pe->OptionalHeader.ImageBase,
            pe->OptionalHeader.SizeOfImage, pe->FileHeader.TimeDateStamp, static_cast<unsigned>(target),
            static_cast<unsigned>(target - base), memory.Protect);
        record(phase, "source_c3", reinterpret_cast<const void*>(target), 1);
        // Complete unchanged provider extents are pinned by the static gate
        // before this diagnostic is run. Addresses come only from named Source
        // symbols; the retained live bytes are compared with that fresh PE.
        const auto code_record = [&](const char* kind, std::uintptr_t address, std::size_t size) {
            HMODULE owner = nullptr;
            MEMORY_BASIC_INFORMATION region{};
            require(GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                reinterpret_cast<LPCWSTR>(address), &owner) && owner == module &&
                address >= base && size <= pe->OptionalHeader.SizeOfImage &&
                address - base <= pe->OptionalHeader.SizeOfImage - size &&
                VirtualQuery(reinterpret_cast<const void*>(address), &region, sizeof region) == sizeof region &&
                region.State == MEM_COMMIT && region.AllocationBase == module &&
                region.Protect == PAGE_EXECUTE_READ &&
                address >= reinterpret_cast<std::uintptr_t>(region.BaseAddress) &&
                address - reinterpret_cast<std::uintptr_t>(region.BaseAddress) + size <= region.RegionSize,
                "named Source provider body is outside the retained loaded code image");
            log.notef("vfs_physical_read code kind=%s address=%08X rva=%08X bytes=%zu protection=%08X",
                kind, static_cast<unsigned>(address), static_cast<unsigned>(address - base), size, region.Protect);
            record(phase, kind, reinterpret_cast<const void*>(address), size);
        };
        code_record("code_open", reinterpret_cast<std::uintptr_t>(&bsp::open_native_physical_stream_00bf52a0), 480);
        code_record("code_read", reinterpret_cast<std::uintptr_t>(&bsp::read_native_physical_stream_00bf5030), 103);
        code_record("code_recycle", reinterpret_cast<std::uintptr_t>(&bsp::recycle_native_physical_stream_00bf55a0), 125);
        code_record("code_delete", reinterpret_cast<std::uintptr_t>(&bsp::delete_native_physical_stream_00bf5090), 62);
        code_record("code_pool_delete", reinterpret_cast<std::uintptr_t>(&bsp::delete_native_physical_stream_pool_00bf4370), 167);
        phase = "publish_callable_owner";
        require(application->publish_and_borrow_raw_failure_manager() == manager &&
            word(manager, 0x90) == target && word(manager, 0x8c) == 0x00735b30,
            "public application publisher did not retain the genuine callable owner");
        graph("published");

        phase = "mount_physical_directory";
        const auto canonical_image = std::filesystem::canonical(original_image);
        const std::string system = canonical_image.parent_path().string() + "\\";
        constexpr char logical_name[] = "cc12_physical_read/battlestationspacific.exe";
        require(canonical_image.filename() == "battlestationspacific.exe",
            "diagnostic input is not the supported installed executable");
        void* const provider = runtime.mount(system.c_str(), "cc12_physical_read", 0, 1, 0xffffffffu);
        require(provider && word(provider, 0) == 0x00d69168 && word(provider, 4) == 1 &&
            word(provider, 8) == system.size() && word(provider, 0x0c) &&
            std::memcmp(reinterpret_cast<const void*>(word(provider, 0x0c)), system.c_str(), system.size() + 1) == 0 &&
            (word(provider, 0x28) & 0xffu) == 0 && word(provider, 0x34) == 0,
            "actual physical directory producer or owned root differs");
        record("mounted", "provider", provider, 0x3c);
        record("mounted", "provider_root", reinterpret_cast<const void*>(word(provider, 0x0c)), system.size() + 1);
        log.notef("vfs_physical_read mounted provider=%p system=%s virtual=cc12_physical_read priority=0 flags=1 device=FFFFFFFF",
            provider, system.c_str());
        graph("mounted");

        phase = "open_actual_stream";
        bsp::NativeString name{};
        name.assign_0041e870(services.strings, logical_name);
        void* stream = services.bindings.open(word(manager, 0), manager, name, 2);
        require(stream && word(stream, 0) == 0x00d691b0 && word(stream, 4) == 1,
            "real physical open did not return its actual one-reference stream");
        const auto table = word(stream, 0);
        const auto handle = reinterpret_cast<HANDLE>(word(stream, 8));
        require(handle && handle != INVALID_HANDLE_VALUE && word(stream, 0x10) == 0 &&
            word(stream, 0x14) == 0 && word(stream, 0x18) == 12223752 && word(stream, 0x1c) == 0 &&
            word(reinterpret_cast<const void*>(table), 0x24) == 0x00bf5030,
            "actual opened HANDLE, read identity, position or size differs");
        void* const pool = owners.stream_pool_publication_0109dc28();
        require(pool && word(pool, 4) == 0 && word(pool, 8) == 0 && word(pool, 0x0c) == 0 &&
            owners.batch_lock_publication_0109dbbc(), "fresh real stream pool/lock differs after open");
        BY_HANDLE_FILE_INFORMATION info{};
        require(GetFileType(handle) == FILE_TYPE_DISK && GetFileInformationByHandle(handle, &info) &&
            !(info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && info.nFileSizeHigh == 0 &&
            info.nFileSizeLow == 12223752, "actual opened handle is not the expected regular file");
        std::wstring final_name(32768, L'\0');
        const auto final_length = GetFinalPathNameByHandleW(handle, final_name.data(),
            static_cast<DWORD>(final_name.size()), FILE_NAME_NORMALIZED | VOLUME_NAME_DOS);
        require(final_length && final_length < final_name.size(), "actual HANDLE final path query failed");
        final_name.resize(final_length);
        const std::wstring expected_name = L"\\\\?\\" + canonical_image.native();
        require(_wcsicmp(final_name.c_str(), expected_name.c_str()) == 0,
            "actual HANDLE does not name the retained installed input");
        log.notef("vfs_physical_read opened stream=%p handle=%p logical=%s flags=2 final=%s "
            "volume=%08X file_index=%08X%08X size=%08X%08X attributes=%08X write_time=%08X%08X",
            stream, handle, logical_name, std::filesystem::path(final_name).string().c_str(),
            info.dwVolumeSerialNumber, info.nFileIndexHigh, info.nFileIndexLow,
            info.nFileSizeHigh, info.nFileSizeLow, info.dwFileAttributes,
            info.ftLastWriteTime.dwHighDateTime, info.ftLastWriteTime.dwLowDateTime);
        std::array<unsigned char, 0x20> opened{};
        std::memcpy(opened.data(), stream, opened.size());
        record("opened", "stream", stream, opened.size());
        graph("opened");

        phase = "read_once";
        constexpr std::array<unsigned char, 64> expected{
            0x4d,0x5a,0x90,0,3,0,0,0,4,0,0,0,0xff,0xff,0,0,
            0xb8,0,0,0,0,0,0,0,0x40,0,0,0,0,0,0,0,
            0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
            0,0,0,0,0,0,0,0,0,0,0,0,0x20,1,0,0};
        std::array<unsigned char, 64> bytes{};
        std::uint32_t actual = 0;
        log.notef("vfs_physical_read read begin stream=%p requested=64", stream);
        services.bindings.read(table, stream, bytes.data(), static_cast<std::uint32_t>(bytes.size()), &actual);
        require(actual == bytes.size() && bytes == expected, "one real read returned unexpected count or bytes");
        auto after_read = opened;
        const std::uint32_t position = actual;
        std::memcpy(after_read.data() + 0x10, &position, sizeof position);
        require(std::memcmp(stream, after_read.data(), after_read.size()) == 0 &&
            publication == manager && word(manager, 0x90) == target &&
            owners.stream_pool_publication_0109dc28() == pool && word(pool, 8) == 0,
            "actual stream or owning context changed outside the read position");
        record("read", "bytes", bytes.data(), bytes.size());
        record("read", "stream", stream, after_read.size());
        graph("read");
        log.notef("vfs_physical_read read returned calls=1 actual=%u position=%08X%08X refs=%u",
            actual, word(stream, 0x14), word(stream, 0x10), word(stream, 4));

        phase = "complete_zero_reference_release";
        const auto stream_address = reinterpret_cast<std::uintptr_t>(stream);
        const auto remaining = InterlockedDecrement(reinterpret_cast<volatile LONG*>(
            static_cast<unsigned char*>(stream) + 4));
        require(remaining == 0, "actual caller release did not reach zero references");
        services.bindings.zero_reference(table, stream);
        stream = nullptr; // The live stream borrow ended; only the pool owns its dead backing.
        require(owners.stream_pool_publication_0109dc28() == pool && word(pool, 8) == 1 &&
            word(pool, 0x0c) == 1 && word(pool, 4), "actual zero-reference return did not populate its pool");
        const auto* cells = reinterpret_cast<const void*>(word(pool, 4));
        require(word(cells, 0) == stream_address, "actual pool did not reacquire the released backing");
        const auto* dead_backing = reinterpret_cast<const void*>(word(cells, 0));
        auto retired_bytes = after_read;
        const std::uint32_t retired_profile = 0x00ceb130, zero = 0, invalid_handle = 0xffffffffu;
        std::memcpy(retired_bytes.data(), &retired_profile, sizeof retired_profile);
        std::memcpy(retired_bytes.data() + 4, &zero, sizeof zero);
        std::memcpy(retired_bytes.data() + 8, &invalid_handle, sizeof invalid_handle);
        require(std::memcmp(dead_backing, retired_bytes.data(), retired_bytes.size()) == 0,
            "owner-reacquired dead pool backing differs after complete release");
        record("released", "dead_pool_backing", dead_backing, retired_bytes.size());
        graph("released");
        log.note("vfs_physical_read release returned refs=0 pool_count=1 pool_capacity=1 live_borrow_ended=1");
        name.release_to(services.strings);

        phase = "shared_drain";
        log.note("vfs_physical_read shared_drain begin");
        host->shutdown();
        require(!publication && !host->manager_publication_01090aa0() &&
            !owners.stream_pool_publication_0109dc28() && !owners.batch_lock_publication_0109dbbc(),
            "actual shared drain left a live publication");
        bool rejected = false;
        try { (void)application->publish_and_borrow_raw_failure_manager(); }
        catch (const std::logic_error& error) {
            rejected = std::strcmp(error.what(),
                "Native VFS core is not available for a failure-manager borrow") == 0;
        }
        require(rejected && log.unimplemented_count() == unimplemented_before,
            "retired rejection or implemented service boundary differs");
        log.note("vfs_physical_read shared_drain completed vfs_null=1 singleton_null=1 pool_null=1 lock_null=1 retired_rejected=1");
        phase = "destroy_drained_owners";
        application.reset();
        host.reset();
        log.note("vfs_physical_read PASS read_calls=1 owners_destroyed=1 normal_CRT_exit=1");
        return 0;
    } catch (const std::exception& error) {
        log.notef("vfs_physical_read FAIL phase=%s reason=%s retention=_Exit", phase, error.what());
    } catch (...) {
        log.notef("vfs_physical_read FAIL phase=%s reason=unknown retention=_Exit", phase);
    }
    log.close();
    std::fflush(stdout);
    std::fflush(stderr);
    std::_Exit(3);
}

void report_summary(bsp::game::GameHostLog& log, const bsp::game::GameRunSummary& summary) {
    log.notef("summary window_created=%d device_created=%d device_hr=0x%08lx "
        "back_buffer=%ux%u frames_presented=%llu presents_skipped=%llu loop_finished=%d exit_code=%d",
        summary.window_created ? 1 : 0, summary.device_created ? 1 : 0,
        static_cast<unsigned long>(summary.device_result), summary.back_buffer_width,
        summary.back_buffer_height, summary.frames_presented, summary.presents_skipped,
        summary.loop_finished ? 1 : 0, summary.exit_code);
    log.notef("summary vfs_ready=%d loose_mounts=%zu/%zu package_scans=%zu "
        "cachedload=%d probes=%zu/%zu", summary.vfs_ready ? 1 : 0, summary.mounts_created,
        summary.mounts_requested, summary.package_scans_completed,
        summary.cached_load ? 1 : 0, summary.probes_resolved, summary.probes_requested);
    log.notef("summary options_file=%d path=%s language=%s resolution=%dx%d fullscreen=%d "
        "vsync=%d antialias=%d", summary.options_file_present ? 1 : 0,
        summary.options_path.empty() ? "(none)" : summary.options_path.c_str(),
        summary.language.empty() ? "(none)" : summary.language.c_str(),
        summary.settings_width, summary.settings_height,
        summary.settings_fullscreen ? 1 : 0, summary.settings_vsync ? 1 : 0,
        summary.settings_antialias);
    log.notef("summary input_scripts_ready=%d devices=%zu input_names=%zu controller_names=%zu "
        "renderer_api_shared=%d locale_keys=%zu locale_files=%zu",
        summary.input_scripts_ready ? 1 : 0, summary.input_devices, summary.input_names,
        summary.controller_names, summary.renderer_api_shared ? 1 : 0,
        summary.locale_keys, summary.locale_files);
    log.notef("summary fonts_loaded=%zu font_resource_opens=%zu fingerprint_defined_bytes=%zu",
        summary.fonts_loaded, summary.font_resource_opens, summary.fingerprint_defined_bytes);
    log.notef("summary gui_resources=%zu pages=%zu/%zu widgets=%zu widgets_with_texture=%zu",
        summary.gui_resources_acquired, summary.gui_pages_loaded, summary.gui_pages_requested,
        summary.gui_widgets, summary.gui_widgets_with_texture);
    log.notef("summary bridge_open=%d atlas=%s atlas_items=%zu textures=%zu quads=%zu "
        "frames=%llu", summary.gui_bridge_open ? 1 : 0,
        summary.gui_bridge_atlas.empty() ? "(none)" : summary.gui_bridge_atlas.c_str(),
        summary.gui_bridge_atlas_items, summary.gui_bridge_textures, summary.gui_bridge_quads,
        summary.gui_bridge_frames);
    log.notef("summary init_tail hardware_probe=%d stored_values=%d factories=%zu "
        "parsers=%zu pak_registry=%d pak_lock=%d decals=%zu",
        summary.hardware_probe_ran ? 1 : 0, summary.hardware_profile_values,
        summary.provider_factories, summary.resource_parsers,
        summary.pak_registry ? 1 : 0, summary.pak_lock ? 1 : 0,
        summary.decal_definitions);
    log.notef("summary frontend title_init=%d press_start_slot=%d screens=%zu "
        "screen_pages=%zu pump_frames=%llu enters=%zu exits=%zu commits=%zu",
        summary.title_init_ran ? 1 : 0, summary.press_start_registered ? 0x5C : -1,
        summary.screens_registered, summary.screen_owned_pages, summary.pump_frames,
        summary.screen_enters, summary.screen_exits, summary.visibility_commits);
    log.notef("summary mainmenu press_start_frame=%ld injected=%d shell=%d manager=%d "
        "screen=%d step=%s state=%d", summary.press_start_frame,
        summary.press_start_injected ? 1 : 0, summary.shell_entered ? 1 : 0,
        summary.main_menu_manager_active ? 1 : 0, summary.published_screen_id,
        summary.path_step.empty() ? "PressStartPoll" : summary.path_step.c_str(),
        summary.final_game_state);
    log.notef("summary text bridge=%d widgets=%zu runs=%zu glyphs=%zu quads=%zu",
        summary.text_bridge_open ? 1 : 0, summary.text_widgets, summary.text_runs,
        summary.text_glyphs, summary.text_quads);
    if (!summary.menu_select.empty()) {
        log.notef("summary mission select=%s tree=%d groups=%zu missions=%zu selected=%s "
            "list_page=%d detail=%d requested=%d step=%s", summary.menu_select.c_str(),
            summary.mission_tree_loaded ? 1 : 0, summary.mission_tree_groups,
            summary.mission_tree_missions,
            summary.mission_selected_id.empty() ? "(none)"
                                                : summary.mission_selected_id.c_str(),
            summary.mission_list_page, summary.mission_detail_built ? 1 : 0,
            summary.mission_start_requested ? 1 : 0,
            summary.mission_step.empty() ? "Idle" : summary.mission_step.c_str());
        log.notef("summary mission scene record=%d path=%s entities=%zu classes=%zu "
            "load_steps=%zu stopped_at=%s", summary.mission_scene_record ? 1 : 0,
            summary.mission_scene_path.empty() ? "(none)"
                                               : summary.mission_scene_path.c_str(),
            summary.mission_scene_entities, summary.mission_scene_classes,
            summary.mission_load_host_steps,
            summary.mission_load_stopped_at.empty() ? "(nothing)"
                                                    : summary.mission_load_stopped_at.c_str());
        log.notef("summary mission load finished=%d concrete=%zu records=%zu state=0x%02X "
            "entered=%d script=%s", summary.mission_load_finished ? 1 : 0,
            summary.mission_load_concrete, summary.mission_load_records,
            summary.mission_game_state, summary.mission_entered ? 1 : 0,
            summary.mission_script_path.empty() ? "(none)"
                                                : summary.mission_script_path.c_str());
        log.notef("summary mission lua bindings=%zu natives=%zu calls=%llu",
            summary.mission_lua_bindings, summary.mission_lua_natives,
            summary.mission_lua_native_calls);
        log.notef("summary mission frames requested=%ld ran=%llu simulated=%llu exit=%s",
            summary.mission_frames_requested, summary.mission_frames_run,
            summary.mission_frames_simulated,
            summary.mission_exit_note.empty() ? "(not reached)"
                                              : summary.mission_exit_note.c_str());
        log.notef("summary mission exit injected=%d frames=%llu completed=%d",
            summary.mission_complete_injected ? 1 : 0, summary.mission_exit_frames,
            summary.mission_exit_completed ? 1 : 0);
    }
    if (!summary.screenshot_path.empty()) {
        log.notef("summary screenshot=%d frame=%ld path=%s",
            summary.screenshot_written ? 1 : 0, summary.screenshot_frame,
            summary.screenshot_path.c_str());
    }
    log.notef("host methods %zu concrete, %zu unimplemented",
        log.implemented_count(), log.unimplemented_count());
    for (const auto& record : log.records()) {
        log.notef("  %-56s %-20s %-13s calls=%llu", record.method.c_str(),
            record.native_address.c_str(), record.implemented ? "concrete" : "UNIMPLEMENTED",
            record.calls);
    }
}

}  // namespace

int WINAPI WinMain(HINSTANCE instance, HINSTANCE previous_instance, LPSTR command_line,
    int show_command) {
    attach_parent_console();

    const bool handoff_child = is_handoff_child();
    std::vector<char*> public_argv(__argv, __argv + __argc);
    const long crash_test_frame = take_crash_test_option(public_argv);
    bsp::game::GameExecutableOptions options;
    std::string error;
    if (!options.parse(static_cast<int>(public_argv.size()) - (handoff_child ? 1 : 0),
            public_argv.data(), error)) {
        std::fprintf(stderr, "bsp_game: %s\n", error.c_str());
        std::fprintf(stderr, "usage: bsp_game.exe [--frames N] [--log <path>]"
            " [--game-root <dir>] [--settings-personal-root <dir>] [--vfs-probe <virtual path>]"
            " [--press-start-frame N] [--menu-select <mission id>] [--mission-frames N]"
            " [--window-monitor <n|primary|smallest|largest>] [--window-origin X,Y]"
            " [--window-resolution <WxH|fit>]"
            " [--mission-complete-frame N]"
            " [--order throttle=<f>,rudder=<f> | --order <command>[:<entity>]"
            " | --order <command>=<x>,<z>]"
            " [--order-unit <name>] [--ai-drive <unit>=<throttle>,<rudder>]"
            " [--order-frame N] [--mission-frame-seconds S] [--frame-jitter <pct>[,<seed>]]"
            " [--present-interval <vsync|immediate|native>]"
            " [--trajectory-csv <path>]"
            " [--screenshot <path>] [--screenshot-frame N]"
            " [--screenshot-mission-frame N] [--hardware-probe-commit]"
            " [--qualify-vfs-failure-owner] [--qualify-vfs-physical-read-owner]\n");
        return 2;
    }

    if (!handoff_child) {
        try { return run_bootstrap_parent(options); }
        catch (const std::exception& failure) {
            std::fprintf(stderr, "bsp_game: native-data bootstrap failed: %s\n",
                failure.what());
            return 2;
        }
    }

    std::filesystem::path original_image;
    try { original_image = original_executable(options); }
    catch (const std::exception& failure) {
        std::fprintf(stderr, "bsp_game: native-data source failed: %s\n", failure.what());
        return 2;
    }

    bsp::game::GameHostLog log;
    if (!log.open(options.log_path)) {
        std::fprintf(stderr, "bsp_game: cannot write log %s\n", options.log_path.c_str());
        return 2;
    }
    // Harness crash record: installed once the log exists, removed before the log closes.
    // The dump path is made absolute now, before --game-root changes the current directory.
    struct CrashLogScope {
        ~CrashLogScope() { g_crash_log = nullptr; }
    } crash_log_scope;
    g_crash_log = &log;
    if (!options.log_path.empty()) {
        std::string dump = options.log_path;
        const std::size_t dot = dump.find_last_of('.');
        const std::size_t slash = dump.find_last_of("\\/");
        if (dot != std::string::npos && (slash == std::string::npos || dot > slash)) dump.resize(dot);
        dump += ".dmp";
        if (GetFullPathNameA(dump.c_str(), MAX_PATH, g_crash_dump_path, nullptr) >= MAX_PATH)
            g_crash_dump_path[0] = '\0';
    }
    SetUnhandledExceptionFilter(harness_crash_record);
    // Harness only (packet cc9_display_required), not the original executable's behaviour.
    // An idle display timeout turns the display off during unattended runs, and then
    // CreateDevice and Present fail with D3DERR_DEVICELOST (0x88760868). The run asks the
    // system to keep the display and the machine awake while it lasts, and clears the
    // request on every return path after this point.
    struct DisplayRequiredScope {
        EXECUTION_STATE previous{0};
        explicit DisplayRequiredScope(bsp::game::GameHostLog& run_log)
            : previous(SetThreadExecutionState(ES_CONTINUOUS | ES_DISPLAY_REQUIRED
                  | ES_SYSTEM_REQUIRED)) {
            run_log.notef("harness display required: SetThreadExecutionState(ES_CONTINUOUS | "
                "ES_DISPLAY_REQUIRED | ES_SYSTEM_REQUIRED) previous=0x%08lx%s",
                static_cast<unsigned long>(previous), previous == 0 ? " (call failed)" : "");
        }
        ~DisplayRequiredScope() { SetThreadExecutionState(ES_CONTINUOUS); }
    } display_required_scope(log);
    if (crash_test_frame > 0) {
        bsp::game::arm_harness_crash_test(crash_test_frame);
        log.notef("harness crash test armed: null write before mission frame %ld",
            crash_test_frame);
    }

    bsp::game::GameNativeReadOnlyData* native_data = nullptr;
    volatile std::uint32_t* native_crt_feature = nullptr;
    try {
        auto reservation = bsp::game::accept_native_data_handoff(
            native_data_spans.data(), native_data_spans.size(),
            bsp::game::GameNativeDataPlan::CanonicalCrtV2);
        const auto& owner = bsp::game::GameNativeCanonicalDataOwner::initialize(original_image,
            native_data_spans.data(), native_data_spans.size(), std::move(reservation));
        native_data = &owner.read_only_data();
        native_crt_feature = &owner.mutable_crt_data().feature_word();
        log.notef("native-data handoff mapped verified original image %s",
            original_image.string().c_str());
    } catch (const std::exception& failure) {
        std::fprintf(stderr, "bsp_game: native-data handoff failed: %s\n", failure.what());
        log.notef("native-data handoff failed: %s", failure.what());
        log.close();
        return 2;
    }

    if (options.qualify_vfs_physical_read_owner)
        return qualify_vfs_physical_read_owner(log, *native_data, original_image);
    if (options.qualify_vfs_failure_owner)
        return qualify_vfs_failure_owner(log, *native_data, original_image);

    try {
        auto& particle_pools = bsp::game::game_native_particle_pool_process();
        const int model_atexit = particle_pools.initialize_model_once_00cd7830();
        const int parameter_atexit = particle_pools.initialize_parameters_once_00cd78b0();
        log.notef("native particle pools initialized: model_atexit=%d parameter_atexit=%d "
            "storage=process_actual38h/actual38h", model_atexit, parameter_atexit);
        auto& material_process = bsp::game::game_native_material_process();
        const int material_atexit = material_process.initialize_material_once_00cd78d0();
        const int material_parameter_atexit = material_process.initialize_parameters_once_00cd78f0();
        log.notef("native material pools initialized: material_atexit=%d parameter_atexit=%d "
            "storage=process_actual38h/actual38h", material_atexit, material_parameter_atexit);
        auto& layout_tree = bsp::game::game_native_hardware_layout_tree_process();
        const int tree_atexit = layout_tree.initialize_once_00cd7960();
        log.notef("native hardware-layout tree initialized: atexit=%d storage=process_actual0ch",
            tree_atexit);
        auto& surface_pool = bsp::game::game_native_surface_pool_process();
        const int surface_atexit = surface_pool.initialize_once_00cd7b40();
        log.notef("native D3D9 surface pool initialized: atexit=%d storage=process_actual38h",
            surface_atexit);
        auto& texture_pool = bsp::game::game_native_texture_pool_process();
        const int texture_atexit = texture_pool.initialize_once_00cd7b60();
        log.notef("native texture2D pool initialized: atexit=%d storage=process_actual38h",
            texture_atexit);
        auto& graphics_pools = bsp::game::game_native_graphics_pool_process();
        using GraphicsPool = bsp::game::GameNativeGraphicsPool;
        // Original initializer table CE3518..CE353C, before CD7E40's mesh pool.
        constexpr std::array graphics_order{GraphicsPool::cube_texture,
            GraphicsPool::volume_texture, GraphicsPool::material_pass,
            GraphicsPool::vertex_declaration, GraphicsPool::layout_record,
            GraphicsPool::physical_index, GraphicsPool::physical_vertex,
            GraphicsPool::logical_vertex, GraphicsPool::logical_index,
            GraphicsPool::hardware_layout};
        for (const auto pool : graphics_order) {
            const int pool_atexit = graphics_pools.initialize_once(pool);
            log.notef("native graphics pool initialized: global=%08x atexit=%d "
                "storage=process_actual38h", static_cast<unsigned>(pool), pool_atexit);
        }
        auto& shader_process = bsp::game::game_native_shader_process();
        const int shader_states_atexit = shader_process.initialize_state_pool_once_00cd7cc0();
        log.notef("native shader state-list pool CRT CD7CC0: atexit=%d storage=process_actual38h",shader_states_atexit);
        auto& lua_globals = bsp::game::game_native_lua_globals_process();
        const int lua_region_atexit = lua_globals.initialize_once_00cd7ce0();
        log.notef("native Lua globals initialized: region_atexit=%d storage=process_actual0ch",
            lua_region_atexit);
        auto& resource_pools = bsp::game::game_native_resource_pool_process();
        // The plain-node/group positions are explicit Source composition;
        // they do not establish the original CRT-table order.
        const int plain_node_atexit = resource_pools.initialize_plain_node_once_00cd7d10();
        log.notef("native plain-node pool initialized: atexit=%d storage=process_actual38h",
            plain_node_atexit);
        const int camera_atexit = resource_pools.initialize_camera_once_00cd7dd0();
        log.notef("native camera pool CRT CD7DD0: atexit=%d", camera_atexit);
        const int mesh_atexit = resource_pools.initialize_mesh_once_00cd7e40();
        // Explicit Source composition before any model consumer. This does not
        // establish the original CRT-table ordering of CD7F00/CD7F20.
        const int resource_model_atexit = resource_pools.initialize_model_once_00cd7f00();
        const int model_base_atexit = resource_pools.initialize_model_base_once_00cd7f20();
        log.notef("native model pools initialized: model_atexit=%d model_base_atexit=%d "
            "storage=process_actual38h/actual38h", resource_model_atexit, model_base_atexit);
        const int section_atexit = resource_pools.initialize_section_once_00cd8250();
        log.notef("native geometry pools initialized: mesh_atexit=%d section_atexit=%d "
            "storage=process_actual38h/actual38h", mesh_atexit, section_atexit);
        const int hierarchy_atexit = resource_pools.initialize_hierarchy_once_00cd82d0();
        log.notef("native resource hierarchy pool initialized: atexit=%d storage=process_actual38h "
            "slots=actual88h", hierarchy_atexit);
        const int group_atexit = resource_pools.initialize_group_once_00cd8460();
        log.notef("native group pool initialized: atexit=%d storage=process_actual38h "
            "slots=actual18ch", group_atexit);
        // CD8A60 is at CE363C in the original initializer table, after CD82D0
        // and before CD9010's physical provider pool (started by VFS).
        auto& weak_pool = bsp::game::game_native_weak_pool_process();
        const int weak_atexit = weak_pool.initialize_once_00cd8a60();
        log.notef("native weak-handle pool initialized: atexit=%d storage=process_actual38h",
            weak_atexit);
        // CC8950 is the Dyn dispatcher initializer at original CRT table CE36B0.
        // Engine/profile/world allocation remains in the native game constructor.
        auto& dynamics = bsp::game::game_native_dyn_process(bsp::game::application_camera_axes_crt());
        const int dynamics_atexit = dynamics.initialize_once_00cc8950();
        log.notef("native Dyn dispatch CRT CC8950: atexit=%d owners=8 "
            "solver_tables=mode0/mode1", dynamics_atexit);
        const int convex_atexit = dynamics.initialize_convex_pool_once_00cc89c0(
            bsp::game::game_native_physical_pool_process().allocator_list_domain_00e188b4(),
            *native_crt_feature);
        log.notef("native Dyn convex pool CRT CC89C0: atexit=%d storage=process_actual38h",
            convex_atexit);
    } catch (const std::exception& failure) {
        std::fprintf(stderr, "bsp_game: native pool startup failed: %s\n", failure.what());
        log.notef("native pool startup failed: %s", failure.what());
        log.close();
        return 1;
    }

    // docs/X87_CONTROL_WORD.md establishes statically that the CRT startup sets
    // the x87 precision field to 53 bits (`__setdefaultprecision` asks for
    // _PC_53 under _MCW_PC at 00c0683c) and that no game code changes it again,
    // and that the device is created without D3DCREATE_FPU_PRESERVE so d3d9.dll
    // drops the field to 24 bits for the life of the device. This is the first
    // half of that read taken at run time: the mode before Direct3D exists. The
    // second is taken at the first fixed simulation step. Neither changes any
    // arithmetic; both are observations.
    const unsigned long precision_before_d3d = bsp::game::x87_precision_field();
    log.notef("x87 precision before Direct3D: %s (_controlfp_s & _MCW_PC = 0x%08lx)",
        bsp::game::x87_precision_name(precision_before_d3d), precision_before_d3d);
    log.notef("bsp_game milestone 2l, frames=%ld press_start_frame=%ld screenshot_frame=%ld "
        "screenshot_mission_frame=%ld menu_select=%s mission_frames=%ld "
        "mission_complete_frame=%ld order_frame=%ld order=throttle %.3f rudder %.3f "
        "mission_frame_seconds=%.4f trajectory_csv=%s log=%s",
        options.frame_limit, options.press_start_frame, options.screenshot_frame,
        options.screenshot_mission_frame,
        options.menu_select.empty() ? "(none)" : options.menu_select.c_str(),
        options.mission_frames, options.mission_complete_frame, options.order_frame,
        static_cast<double>(options.order_throttle),
        static_cast<double>(options.order_rudder),
        static_cast<double>(options.mission_frame_seconds),
        options.trajectory_csv.empty() ? "(none)" : options.trajectory_csv.c_str(),
        options.log_path.empty() ? "(stdout only)" : options.log_path.c_str());
    // Packet cc9_frame_delta_jitter: --frame-jitter or BSP_FRAME_JITTER; off by default.
    bsp::game::set_mission_frame_jitter(options.frame_jitter_percent, options.frame_jitter_seed);
    if (options.frame_jitter_percent > 0.0f) {
        log.notef("frame jitter %g%% seed %u", static_cast<double>(options.frame_jitter_percent),
            static_cast<unsigned>(options.frame_jitter_seed));
    } else {
        log.notef("frame jitter off");
    }
    // Packet cc9_tooling_present_interval: printed only when an override is in force, so a
    // run without it logs exactly what it did before.
    if (options.present_interval >= 0) {
        log.notef("present interval %s (harness override)",
            options.present_interval == 0 ? "vsync" : "immediate");
    }
    if (!options.ai_drive_unit.empty()) {
        log.notef("--ai-drive %s=%.3f,%.3f: a labelled diagnostic stand-in for the ship AI "
            "state step, engaged on --order-frame. It substitutes nothing after the two "
            "setters 009dbf90 / 009dffb0",
            options.ai_drive_unit.c_str(), static_cast<double>(options.ai_drive_throttle),
            static_cast<double>(options.ai_drive_rudder));
    }

    // The phase-2 mounts use GetCurrentDirectoryA at 0073d697, so pointing the run at an
    // installed game means setting the process current directory, not injecting a path. The
    // log is already open, so a relative --log path stays relative to the invoking shell.
    if (!options.game_root.empty()) {
        if (!SetCurrentDirectoryA(options.game_root.c_str())) {
            log.notef("cannot enter game root %s", options.game_root.c_str());
            log.close();
            return 2;
        }
        log.notef("game root %s", options.game_root.c_str());
    }

    std::unique_ptr<bsp::game::GameStartupHost> host;
    try { host = std::make_unique<bsp::game::GameStartupHost>(
        log, instance, options, native_data); }
    catch (const std::exception& failure) {
        std::fprintf(stderr, "bsp_game: startup host failed: %s\n", failure.what());
        log.notef("startup host failed: %s", failure.what());
        log.close();
        return 1;
    }

    // 008f81f0 reads none of its four arguments; they are recorded for completeness.
    bsp::WinMainArguments arguments;
    arguments.instance = instance;
    arguments.previous_instance = previous_instance;
    arguments.command_line = command_line;
    arguments.show_command = show_command;

    int result = 1;
    try {
        result = bsp::run_win_main(arguments, *host);
    } catch (const std::exception& error) {
        log.notef("startup failed: %s", error.what());
    }

    host->exit_if_native_vfs_interrupted();
    bsp::game::GameRunSummary summary = host->summary();
    summary.exit_code = result;
    // --frames counts loop ticks. A completed native frame can inhibit Present
    // during startup/reset; report that separately from a successful HRESULT.
    // A reached Present with a failed HRESULT counts in neither category.
    // Milestone 2g is the one exception: a mission that ended through the debrief path
    // finishes the run where the game does, which is before the frame count is reached.
    if (options.frame_limit > 0 && !summary.mission_exit_completed
        && summary.frames_presented + summary.presents_skipped
            < static_cast<unsigned long long>(options.frame_limit)) {
        summary.exit_code = 1;
    }
    // A --vfs-probe that did not read bytes fails the run, so a scripted check needs only the
    // exit code. The three probes the milestone always performs do not affect it.
    if (summary.exit_code == 0 && host->vfs() != nullptr) {
        for (const std::string& requested : options.vfs_probes) {
            for (const auto& probe : host->vfs()->probes()) {
                if (probe.requested == requested && !probe.opened) summary.exit_code = 3;
            }
        }
    }
    report_summary(log, summary);
    host.reset(); // The canonical RO/RW owner remains mapped through process teardown.
    log.close();
    return summary.exit_code;
}
