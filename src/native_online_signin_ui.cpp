#include "bsp/native_online_signin_ui.hpp"

#include "bsp/locale_text_lookup.hpp"
#include "bsp/native_online_storage_requests.hpp"
#include "bsp/native_string_pool_storage.hpp"
#include "bsp/native_wide_string.hpp"
#include "bsp/xlive_library.hpp"

#include <Windows.h>

#include <array>
#include <cstddef>
#include <cstring>
#include <cstdlib>
#include <limits>
#include <stdexcept>
#include <string>

namespace bsp {
namespace {
static_assert(sizeof(void*) == 4 && sizeof(wchar_t) == 2);
constexpr std::uint32_t pending = 0x3e5;

std::byte* bytes(NativeOnlineManagerStorage& manager) noexcept {
    return reinterpret_cast<std::byte*>(&manager);
}
template<class T> T read(const NativeOnlineManagerStorage& manager, std::size_t at) noexcept {
    T value;
    std::memcpy(&value, reinterpret_cast<const std::byte*>(&manager) + at, sizeof(value));
    return value;
}
template<class T> void write(NativeOnlineManagerStorage& manager, std::size_t at,
    T value) noexcept {
    std::memcpy(bytes(manager) + at, &value, sizeof(value));
}
void state(NativeOnlineManagerStorage& manager, std::uint32_t value) noexcept {
    // The native diagnostic 004254B0 is RET. The state store precedes it.
    write(manager, 0x3b0, value);
}
template<class... Args> std::uint32_t ordinal(void* module,
    std::uint16_t number, Args... args) {
    const FARPROC address = GetProcAddress(static_cast<HMODULE>(module),
        MAKEINTRESOURCEA(number));
    if (!address) throw std::runtime_error("Missing XLive ordinal " + std::to_string(number));
    using Function = std::uint32_t (__stdcall*)(Args...);
    Function function;
    static_assert(sizeof(function) == sizeof(address));
    std::memcpy(&function, &address, sizeof(function));
    return function(args...);
}

struct UiWideOutputs {
    explicit UiWideOutputs(NativeStringStorage& owner) : storage(owner) {}
    ~UiWideOutputs() noexcept {
        destroy_native_wide_string_header_00436430(&offline, storage);
        destroy_native_wide_string_header_00436430(&signin, storage);
        destroy_native_wide_string_header_00436430(&question, storage);
        destroy_native_wide_string_header_00436430(&title, storage);
    }
    NativeStringStorage& storage;
    NativeOnlineUiWide8 title{};
    NativeOnlineUiWide8 question{};
    NativeOnlineUiWide8 signin{};
    NativeOnlineUiWide8 offline{};
};

void localize(NativeOnlineSigninUiCalls& calls, NativeStringStorage& storage,
    NativeOnlineUiWide8& output, const char* literal) {
    NativeString key;
    key.assign_0041e870(storage, literal);
    try {
        calls.append_localization_00a9fad0(output, key, storage);
    } catch (...) {
        key.release_to(storage);
        throw;
    }
    key.release_to(storage); // 0041DD20, before constructing the next key.
}
const wchar_t* text_or_empty(const NativeOnlineUiWide8& wide) noexcept {
    // Native 00F8ABF8 is a shared zero wide character.
    static const wchar_t empty = L'\0';
    return wide.data ? wide.data : &empty;
}
} // namespace

NativeOnlineSigninUiCrt standard_native_online_signin_ui_crt() noexcept {
    return {&_invalid_parameter_noinfo, &::memmove_s};
}

NativeOnlineSigninUiRuntime::NativeOnlineSigninUiRuntime(
    const XLiveLibrary& library, const LocaleTextResolver& locale)
    : module_(library.module_handle()), locale_(locale) {
    if (!module_) throw std::invalid_argument("Native sign-in UI needs a live XLive module");
}

void NativeOnlineSigninUiRuntime::append_localization_00a9fad0(
    NativeOnlineUiWide8& output, const NativeString& key, NativeStringStorage& storage) {
    if ((key.length() && !key.data()) || (output.length && !output.data))
        throw std::invalid_argument("Invalid native localization header");
    // LocaleTextResolver is the existing current-table/Lua-context binding for
    // A9FAD0. The four native outputs are initially {0,null}; preserve append
    // semantics if a caller supplies a populated header.
    std::u16string value;
    if (output.length) value.assign(reinterpret_cast<const char16_t*>(output.data),
        reinterpret_cast<const char16_t*>(output.data) + output.length);
    const std::size_t before = value.size();
    locale_.append_key_list_00a9fad0(value, key.length()
        ? std::string(key.data(), key.length()) : std::string{});
    if (value.size() > (std::numeric_limits<std::uint32_t>::max)())
        throw std::length_error("Native localization output exceeds DWORD length");
    resize_native_wide_string_header_004c53e0(&output, storage,
        static_cast<std::uint32_t>(value.size()), true);
    if (value.size() != before) {
        auto* const current = output.data;
        std::memcpy(current + before, value.data() + before,
            (value.size() - before) * sizeof(char16_t));
    }
}

std::uint32_t NativeOnlineSigninUiRuntime::show_message_box_00a4d5c0(
    std::uint32_t user, const wchar_t* title, const wchar_t* text,
    std::uint32_t count, const wchar_t* const* buttons, std::uint32_t focus,
    std::uint32_t flags, std::uint32_t* choice, void* overlapped) {
    return ordinal(module_, 5266, user, title, text, count, buttons,
        focus, flags, choice, overlapped);
}
std::uint32_t NativeOnlineSigninUiRuntime::get_overlapped_result_00a4d42e(
    void* overlapped, std::uint32_t* result, std::uint32_t wait) {
    return ordinal(module_, 1083, overlapped, result, wait);
}
std::uint32_t NativeOnlineSigninUiRuntime::show_signin_00a4d5ba(
    std::uint32_t users, std::uint32_t flags) {
    return ordinal(module_, 5260, users, flags);
}
std::uint32_t NativeOnlineSigninUiRuntime::user_get_signin_info_00a4d560(
    std::uint32_t user, std::uint32_t flags, void* output) {
    return ordinal(module_, 5267, user, flags, output);
}
void NativeOnlineSigninUiRuntime::build_storage_path_00a3ed10(
    NativeOnlineManagerStorage& manager) {
    build_native_online_storage_path_00a3ed10(manager,
        resolve_native_online_storage_build_path(module_));
}

void reset_native_online_ui_slots_00a3e700(NativeOnlineManagerStorage& manager) noexcept {
    write(manager, 0x3b0, std::uint32_t{0});
    write(manager, 0x3b4, std::uint32_t{1});
    write(manager, 0x3b8, std::uint32_t{1});
}

void reset_native_online_signin_state_00a40020(
    NativeOnlineManagerStorage& manager, NativeOnlineSigninCalls& calls,
    NativeOnlineSigninUiCrt crt) {
    const auto callback = read<std::uint32_t>(manager, 0x20);
    if (callback != 0 && read<std::uint32_t>(manager, 0x8c) != 0)
        calls.call_callback20(callback, 0); // ECX=0; reread manager after call.
    if (read<std::uint8_t>(manager, 0x3bd) == 0) {
        write(manager, 0x119, std::uint8_t{0});
        write(manager, 0x11c, std::uint32_t{1});
        write(manager, 0x11a, std::uint8_t{0});
    }
    write(manager, 0x2f, std::uint8_t{0});
    write(manager, 0x2d, std::uint8_t{0});
    write(manager, 0x2e, std::uint8_t{0});
    write(manager, 0x120, std::uint8_t{0});
    write(manager, 0x124, std::uint32_t{0xffffffff});
    write(manager, 0x28, std::uint32_t{0});
    write(manager, 0x2c, std::uint8_t{0});
    write(manager, 0x35c, std::uint32_t{0});
    write(manager, 0x358, std::uint32_t{0});
    const auto end = read<std::uint32_t>(manager, 0x368);
    if (read<std::uint32_t>(manager, 0x364) > end)
        crt.invalid_parameter_noinfo();
    const auto begin = read<std::uint32_t>(manager, 0x364);
    if (begin > read<std::uint32_t>(manager, 0x368))
        crt.invalid_parameter_noinfo();
    if (begin != end) {
        // The CRT handlers can return after mutating the queue. Native keeps
        // captured EDI=end and EBP=begin, then SARs the wrapped DWORD delta.
        const auto current_end = read<std::uint32_t>(manager, 0x368);
        const auto delta = current_end - end;
        const auto words = (delta >> 2) | ((delta & 0x80000000u) ? 0xc0000000u : 0u);
        const auto bytes_to_move = words * 4u;
        const auto new_end = begin + bytes_to_move; // captured before memmove_s
        if (words != 0 && (words & 0x80000000u) == 0) {
            const auto* source = reinterpret_cast<const void*>(end);
            auto* destination = reinterpret_cast<void*>(begin);
            (void)crt.memmove_s(destination, bytes_to_move, source, bytes_to_move);
        }
        write(manager, 0x368, new_end);
    }
    state(manager, 0);
    write(manager, 0x3b4, std::uint32_t{1});
    write(manager, 0x3b8, std::uint32_t{1});
}

void pump_native_online_signin_ui_00a40510(NativeOnlineSigninUiContext& context) {
    auto& manager = context.manager;
    auto& calls = context.calls;
    switch (read<std::uint32_t>(manager, 0x3b0)) {
    case 1: {
        if (read<std::uint8_t>(manager, 0x2c) != 0) {
            reset_native_online_ui_slots_00a3e700(manager);
            return;
        }
        ActualNativeStringPoolStorage storage(context.strings.actual_published_01090aa8,
            context.strings.actual_small_returns_disabled_01090aa4,
            context.strings.actual_manager_publication_01090aa0);
        UiWideOutputs output(storage);
        localize(calls, storage, output.title, "FE_xbox.xsm_SignIn_Title");
        localize(calls, storage, output.question, "FE_xbox.xsm_SignIn_Question");
        localize(calls, storage, output.signin, "FE_xbox.xsm_signin_signinuser_pc");
        localize(calls, storage, output.offline,
            "FE_xbox.xsm_signin_continuewithoutsigningin_pc");
        // Seven DWORDs, including the two reserved trailing ones. The choice
        // at +3DC and following +3E0/+3E4 are not cleared by this sequence.
        for (std::size_t offset = 0x3c0; offset != 0x3dc; offset += 4)
            write(manager, offset, std::uint32_t{0});
        const wchar_t* buttons[2] = {
            text_or_empty(output.signin), text_or_empty(output.offline)};
        const wchar_t* title = text_or_empty(output.title);
        const wchar_t* question = text_or_empty(output.question);
        if (read<std::uint8_t>(manager, 0x3e8) == 0) {
            const auto result = calls.show_message_box_00a4d5c0(
                read<std::uint32_t>(manager, 0x3b4), title, question, 2, buttons,
                0, 1, reinterpret_cast<std::uint32_t*>(bytes(manager) + 0x3dc),
                bytes(manager) + 0x3c0);
            if (result == pending) state(manager, 2);
        }
        return; // four native wide destructors run even after pending.
    }
    case 2:
        if (read<std::uint8_t>(manager, 0x2c) != 0) {
            if (read<std::uint32_t>(manager, 0x3c0) != pending)
                reset_native_online_ui_slots_00a3e700(manager);
            return;
        }
        if (read<std::uint32_t>(manager, 0x3c0) == pending) return;
        if (calls.get_overlapped_result_00a4d42e(
                bytes(manager) + 0x3c0, nullptr, 1) != 0) {
            reset_native_online_signin_state_00a40020(manager, context.signin_calls, context.crt);
            return;
        }
        if (read<std::uint32_t>(manager, 0x3dc) == 0) {
            state(manager, 3);
        } else if (read<std::uint32_t>(manager, 0x3dc) == 1) {
            write(manager, 0x3b8, read<std::uint32_t>(manager, 0x3b4));
            write(manager, 0x119, std::uint8_t{0});
            write(manager, 0x11a, std::uint8_t{0});
            write(manager, 0x11c, std::uint32_t{1});
            write(manager, 0x120, std::uint8_t{0});
            write(manager, 0x124, std::uint32_t{0xffffffff});
            state(manager, 5);
        }
        return;
    case 3:
        if (read<std::uint8_t>(manager, 0x2c) == 0) {
            if (read<std::uint8_t>(manager, 0x3e8) != 0) return;
            if (calls.show_signin_00a4d5ba(1, 0) != 0) return;
        }
        reset_native_online_signin_state_00a40020(manager, context.signin_calls, context.crt);
        return;
    case 4:
        if (read<std::uint8_t>(manager, 0x2c) != 0) {
            reset_native_online_ui_slots_00a3e700(manager);
            return;
        }
        write(manager, 0x3b8, read<std::uint32_t>(manager, 0x3b4));
        write(manager, 0x3e4, std::uint32_t{0});
        write(manager, 0x3bc, std::uint8_t{1});
        write(manager, 0x119, std::uint8_t{1});
        write(manager, 0x11c, read<std::uint32_t>(manager, 0x3b4));
        write(manager, 0x120, std::uint8_t{1});
        write(manager, 0x124, std::uint32_t{0});
        state(manager, 5);
        return;
    case 5: {
        if (read<std::uint8_t>(manager, 0x2c) != 0) {
            reset_native_online_ui_slots_00a3e700(manager);
            return;
        }
        if (read<std::uint8_t>(manager, 0x119) != 0) {
            auto info = context.signin_info_preimage; // exact caller-provided stack bytes
            const auto result = calls.user_get_signin_info_00a4d560(
                read<std::uint32_t>(manager, 0x11c), 1, info.data());
            if (result == 0) {
                write(manager, 0x11a, static_cast<std::uint8_t>(
                    std::to_integer<std::uint8_t>(info[8]) & 1u));
                const auto user = read<std::uint32_t>(manager, 0x11c);
                if (read<std::uint32_t>(manager, 0x8c +
                        static_cast<std::uint32_t>(user * 4u)) == 2)
                    calls.build_storage_path_00a3ed10(manager);
                state(manager, 6);
                return;
            }
        }
        write(manager, 0x11a, std::uint8_t{0});
        state(manager, 6);
        return;
    }
    case 6:
        if (read<std::uint8_t>(manager, 0x2c) != 0) {
            reset_native_online_ui_slots_00a3e700(manager);
            return;
        }
        write(manager, 0x3bd, std::uint8_t{0});
        write(manager, 0x28, std::uint32_t{2});
        state(manager, 7);
        return;
    default:
        return;
    }
}
} // namespace bsp
