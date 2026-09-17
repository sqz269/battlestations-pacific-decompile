#include "bsp/native_online_notification_dispatch.hpp"

#include "bsp/native_string_pool_storage.hpp"
#include "bsp/native_wide_string.hpp"
#include "bsp/xlive_library.hpp"

#include <windows.h>
#include <process.h>
#include <cstring>
#include <stdexcept>

namespace bsp {
namespace {
static_assert(sizeof(void*) == 4);
static_assert(sizeof(NativeString) == 8);
template<class T> T read(const NativeOnlineManagerStorage& manager, std::size_t offset) {
    T result;
    std::memcpy(&result, reinterpret_cast<const std::byte*>(&manager) + offset, sizeof(T));
    return result;
}
template<class T> void write(NativeOnlineManagerStorage& manager, std::size_t offset, T value) {
    std::memcpy(reinterpret_cast<std::byte*>(&manager) + offset, &value, sizeof(T));
}
template<class T> T import(void* module, unsigned ordinal) {
    const auto address = GetProcAddress(static_cast<HMODULE>(module), MAKEINTRESOURCEA(ordinal));
    if (!address) throw std::runtime_error("Missing raw notification XLive ordinal");
    T result;
    static_assert(sizeof(result) == sizeof(address));
    std::memcpy(&result, &address, sizeof(result));
    return result;
}
void clear_header(NativeString& header) noexcept {
    const std::uint32_t zero[2]{};
    std::memcpy(&header, zero, sizeof(zero));
}
void return_captured(char* pointer, std::uint32_t bytes, NativeStringRawPoolContext& strings) {
    if (!pointer) return;
    auto* pool = native_string_pool_get_or_create_00419cc0(
        strings.actual_published_01090aa8, strings.actual_manager_publication_01090aa0);
    return_native_string_pool_00bd1510(pool, pointer, bytes,
        strings.actual_small_returns_disabled_01090aa4);
}
void current_client28(void* client) {
    void* table;
    std::memcpy(&table, client, sizeof(table));
    using Method = void (__thiscall*)(void*);
    Method method;
    std::memcpy(&method, static_cast<std::byte*>(table) + 0x28, sizeof(method));
    method(client);
}
void unwind(NativeOnlineNotificationContext& context) {
    auto& frame = context.operation;
    while (frame.cleanup_state >= 0) {
        const auto previous = frame.cleanup_state;
        frame.cleanup_state = previous == 2 ? 1 : -1;
        destroy_native_string_header_0041dd20(
            previous == 1 ? &frame.header_20 : &frame.header_28, context.strings);
    }
}
} // namespace

NativeOnlineNotificationRuntime::NativeOnlineNotificationRuntime(const XLiveLibrary& library)
    : module_(library.module_handle()) {
    if (!module_) throw std::invalid_argument("Raw notifications require an already loaded XLive module");
}
void* NativeOnlineNotificationRuntime::create_listener(std::uint64_t areas) {
    return import<void* (__stdcall*)(std::uint64_t)>(module_, 5270)(areas);
}
bool NativeOnlineNotificationRuntime::get_next(void* listener, std::uint32_t filter,
    std::uint32_t& id, std::uint32_t& parameter) {
    return import<BOOL (__stdcall*)(void*, std::uint32_t, std::uint32_t*, std::uint32_t*)>(
        module_, 651)(listener, filter, &id, &parameter) != FALSE;
}
std::uint32_t NativeOnlineNotificationRuntime::accepted_invite(std::uint32_t user, void* output) {
    return import<std::uint32_t (__stdcall*)(std::uint32_t, void*)>(module_, 5315)(user, output);
}
std::int32_t NativeOnlineNotificationRuntime::update_system(const wchar_t* path) {
    return import<std::int32_t (__stdcall*)(const wchar_t*)>(module_, 5024)(path);
}
void NativeOnlineNotificationRuntime::sleep_milliseconds(std::uint32_t milliseconds) {
    Sleep(milliseconds);
}
[[noreturn]] void NativeOnlineNotificationRuntime::exit_process(int code) { _exit(code); }

void drain_native_online_notifications_00a40110(NativeOnlineNotificationContext& context) {
    auto& manager = context.manager;
    auto& frame = context.operation;
    if (frame.active) throw std::logic_error("Cannot replay an active raw notification frame");
    frame.active = true;
    frame.cleanup_state = -1;
    frame.cleanup_failed = false;
    ActualNativeStringPoolStorage storage(context.strings.actual_published_01090aa8,
        context.strings.actual_small_returns_disabled_01090aa4,
        context.strings.actual_manager_publication_01090aa0);
    try {
        if (read<std::uint32_t>(manager, 0x1c) == 0xffffffffu) {
            auto* listener = context.notifications.create_listener(0x2full);
            write(manager, 0x1c, listener);
        }
        poll_native_online_signin_00a3f3e0(manager, context.signin);
        while (context.notifications.get_next(read<void*>(manager, 0x1c), 0, frame.id, frame.parameter)) {
            switch (frame.id) {
            case 9:
                apply_native_online_notification9_fragment_00a401a9(
                    manager, frame.parameter, context.publications.hook_00f8abec);
                break;
            case 10:
                toggle_native_online_signin_00a3f440(manager, context.signin);
                break;
            case 11:
                if (const auto hook = context.publications.hook_00f8abf0) hook();
                break;
            case 14:
                refresh_native_online_profile_name_00a3e600(manager,
                    static_cast<std::uint8_t>(frame.parameter), context.profile);
                break;
            case 0x15: {
                clear_header(frame.header_20);
                frame.cleanup_state = 1;
                title_native_online_update_path_00a3ff20(frame.header_20, storage, context.updates);
                clear_header(frame.header_28);
                resize_native_string_header_0041dd40(&frame.header_28, context.strings, 0, true);
                const auto empty_length = frame.header_28.length();
                auto* const empty_pointer = frame.header_28.data();
                // CE3A0C is the native narrow empty string. The zero-length
                // resize above cannot call out or produce a nonnull buffer.
                if (empty_pointer) std::memcpy(empty_pointer, "", empty_length + 1u);
                auto* path = frame.header_20.data();
                bool differs = frame.header_20.length() != empty_length;
                if (!differs && frame.header_20.length() != 0)
                    differs = _stricmp(path, empty_pointer) != 0;
                return_captured(empty_pointer, empty_length + 1u, context.strings);
                if (!differs) {
                    frame.cleanup_state = -1;
                    if (path) return_captured(path, frame.header_20.length() + 1u, context.strings);
                    break;
                }
                frame.header_28.assign_0041e870(storage, "\\setup.exe");
                const auto suffix_length = frame.header_28.length();
                auto* const suffix = frame.header_28.data();
                frame.cleanup_state = 2;
                if (suffix_length != 0) {
                    const auto previous_length = frame.header_20.length();
                    resize_native_string_header_0041dd40(&frame.header_20,
                        context.strings, previous_length + suffix_length, true);
                    std::memmove(frame.header_20.data() + previous_length, suffix, suffix_length);
                    path = frame.header_20.data();
                }
                frame.cleanup_state = 1;
                return_captured(suffix, suffix_length + 1u, context.strings);
                launch_native_online_update_00a3e560("update.exe", path ? path : "", context.updates);
                context.notifications.sleep_milliseconds(200);
                context.notifications.exit_process(0);
            }
            case 0x16: {
                clear_header(frame.header_28);
                frame.cleanup_state = 0;
                system_native_online_update_path_00a3fde0(frame.header_28, storage, context.updates);
                const auto* path = frame.header_28.data();
                construct_native_wide_string_header_004c5e60(&frame.header_20, path ? path : "", storage);
                const auto* wide = reinterpret_cast<const wchar_t*>(frame.header_20.data());
                context.notifications.update_system(wide ? wide : L"");
                context.notifications.exit_process(0);
            }
            case 0x02000001:
                if (frame.parameter == 0x001510f0)
                    pump_native_online_achievements_00a3fa70(manager, 1,
                        context.achievements, context.memory, context.crt);
                else if (frame.parameter == 0x80151005u)
                    write(manager, 0x128, std::uint8_t{1});
                break;
            case 0x02000002:
                context.notifications.accepted_invite(frame.parameter, frame.invite_stack.data());
                // The +119/+11C probe has no surviving register result, but
                // retaining its reads avoids a synthetic state prerequisite.
                if (read<std::uint8_t>(manager, 0x119) != 0) {
                    const volatile auto unused = read<std::uint32_t>(manager, 0x11c);
                    (void)unused;
                }
                write(manager, 0x31, std::uint8_t{1});
                std::memcpy(reinterpret_cast<std::byte*>(&manager) + 0x32,
                    frame.invite_stack.data(), 0x54);
                break;
            case 0x02000007:
                write(manager, 0x30, std::uint8_t{1});
                break;
            case 0x04000002:
            case 0x04000003:
                if (auto* const client = context.publications.client_00f8a2fc) current_client28(client);
                break;
            default:
                break; // 004254B0 diagnostic calls are a verified bare RET.
            }
        }
        frame.active = false;
    } catch (...) {
        try { unwind(context); }
        catch (...) { frame.cleanup_failed = true; throw; }
        throw;
    }
}
} // namespace bsp
