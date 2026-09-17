#include "bsp/native_online_storage.hpp"

#include <Windows.h>

#include <cstring>
#include <stdexcept>
#include <string>
#include <type_traits>

namespace bsp {
namespace {
constexpr std::uint32_t pending = 0x3e5;
static_assert(sizeof(void*) == 4 && sizeof(wchar_t) == 2);
static_assert(sizeof(NativeOnlineStorageSdk::Download) == 4);
static_assert(sizeof(NativeOnlineStorageSdk::Upload) == 4);
static_assert(sizeof(NativeOnlineStorageSdk::Progress) == 4);
static_assert(sizeof(NativeOnlineStorageSdk::Result) == 4);
static_assert(sizeof(NativeOnlineStorageSdk::Error) == 4);

std::byte* base(NativeOnlineManagerStorage& manager) {
    return reinterpret_cast<std::byte*>(&manager);
}
std::uint32_t read32(NativeOnlineManagerStorage& manager, std::size_t offset) {
    std::uint32_t value;
    std::memcpy(&value, base(manager) + offset, sizeof(value));
    return value;
}
void write32(NativeOnlineManagerStorage& manager, std::size_t offset, std::uint32_t value) {
    std::memcpy(base(manager) + offset, &value, sizeof(value));
}
void* field(NativeOnlineManagerStorage& manager, std::size_t offset) {
    return base(manager) + offset;
}
const wchar_t* path(NativeOnlineManagerStorage& manager) {
    // A3ED10's SDK path output starts at +154, spans exactly 200h bytes.
    return reinterpret_cast<const wchar_t*>(base(manager) + 0x154);
}
void* buffer(NativeOnlineManagerStorage& manager) {
    return reinterpret_cast<void*>(static_cast<std::uintptr_t>(read32(manager, 0x14c)));
}
void set_buffer(NativeOnlineManagerStorage& manager, void* pointer) {
    write32(manager, 0x14c, static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(pointer)));
}
bool online(NativeOnlineManagerStorage& manager) {
    const auto index = read32(manager, 0x3b4); // one cached index at entry
    return read32(manager, 0x8c + static_cast<std::size_t>(index) * 4) == 2;
}
void replace_buffer(NativeOnlineManagerStorage& manager,
    const NativeOnlineStorageMemory& memory, std::size_t length) {
    if (void* old = buffer(manager)) {
        memory.release(old);
        set_buffer(manager, nullptr);
    }
    write32(manager, 0x150, static_cast<std::uint32_t>(length));
    set_buffer(manager, memory.allocate(length));
}
void clear_overlap_head(NativeOnlineManagerStorage& manager) {
    // The game clears five DWORDs, +130..+140, leaving +144..+14B intact.
    std::memset(field(manager, 0x130), 0, 0x14);
}
template<class Function>
Function ordinal(HMODULE module, std::uint16_t number) {
    const FARPROC entry = GetProcAddress(module, MAKEINTRESOURCEA(number));
    if (!entry) throw std::runtime_error("Missing XLive ordinal " + std::to_string(number));
    static_assert(sizeof(entry) == sizeof(Function));
    Function function;
    std::memcpy(&function, &entry, sizeof(function));
    return function;
}
} // namespace

NativeOnlineStorageSdk resolve_native_online_storage_sdk(void* loaded_module) {
    if (!loaded_module) throw std::invalid_argument("XLive module is not loaded");
    const auto module = static_cast<HMODULE>(loaded_module);
    return {
        ordinal<NativeOnlineStorageSdk::Download>(module, 5345),
        ordinal<NativeOnlineStorageSdk::Upload>(module, 5305),
        ordinal<NativeOnlineStorageSdk::Progress>(module, 5307),
        ordinal<NativeOnlineStorageSdk::Progress>(module, 5304),
        ordinal<NativeOnlineStorageSdk::Result>(module, 1083),
        ordinal<NativeOnlineStorageSdk::Error>(module, 1082)
    };
}

void download_native_online_storage_00a3ed60(NativeOnlineManagerStorage& manager,
    const NativeOnlineStorageSdk& sdk, const NativeOnlineStorageMemory& memory) {
    if (!online(manager)) return;
    const auto state = read32(manager, 0x12c);
    if (state == 1 || state == 9 || state == 4) {
        replace_buffer(manager, memory, 0x100);
        std::memset(buffer(manager), 0, read32(manager, 0x150));
        clear_overlap_head(manager);
        std::memset(field(manager, 0x370), 0, 0x14); // five result DWORDs
        write32(manager, 0x12c, 2);
    }
    if (read32(manager, 0x12c) == 2) {
        const auto result = sdk.download(read32(manager, 0x11c), path(manager),
            read32(manager, 0x150), buffer(manager), 0x14,
            field(manager, 0x370), field(manager, 0x130));
        write32(manager, 0x354, result);
        write32(manager, 0x12c, result == pending ? 3 : 5);
    }
    if (read32(manager, 0x12c) != 3) return;
    if (read32(manager, 0x130) == pending) {
        // PUSH ECX at entry also initializes the original local output cell.
        auto progress = static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(&manager));
        sdk.download_progress(field(manager, 0x130), &progress, nullptr, nullptr);
        return;
    }
    if (sdk.overlapped_result(field(manager, 0x130), nullptr, 1) == 0) {
        write32(manager, 0x12c, 4);
        if (read32(manager, 0x370) == 9) {
            std::uint32_t first, second;
            std::memcpy(&first, buffer(manager), 4);
            std::memcpy(&second, static_cast<const std::byte*>(buffer(manager)) + 4, 4);
            write32(manager, 0x358, first);
            write32(manager, 0x35c, second);
        }
    } else {
        write32(manager, 0x12c, 5);
        const auto error = sdk.overlapped_error(field(manager, 0x130));
        write32(manager, 0x354, error);
        write32(manager, 0x12c, error == 0x8015c004u ? 10 : 5);
    }
}

void upload_native_online_storage_00a3ef20(NativeOnlineManagerStorage& manager,
    const NativeOnlineStorageSdk& sdk, const NativeOnlineStorageMemory& memory) {
    if (!online(manager)) return;
    const auto state = read32(manager, 0x12c);
    if (state == 1 || state == 9 || state == 4 || state == 10 || state == 5) {
        replace_buffer(manager, memory, 9);
        const auto first = read32(manager, 0x358);
        const auto second = read32(manager, 0x35c);
        std::memcpy(buffer(manager), &first, 4);
        std::memcpy(static_cast<std::byte*>(buffer(manager)) + 4, &second, 4);
        static_cast<std::byte*>(buffer(manager))[read32(manager, 0x150) - 1] = std::byte{0};
        clear_overlap_head(manager);
        // 4254B0 is an empty diagnostic body; the game's three log calls in
        // this preparation/submission path have no storage-visible effect.
        write32(manager, 0x12c, 6);
    }
    if (read32(manager, 0x12c) == 6) {
        const auto result = sdk.upload(read32(manager, 0x11c), path(manager),
            read32(manager, 0x150), buffer(manager), field(manager, 0x130));
        write32(manager, 0x354, result);
        write32(manager, 0x12c, result == pending ? 7 : 8);
    }
    if (read32(manager, 0x12c) != 7) return;
    if (read32(manager, 0x130) == pending) {
        auto progress = static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(&manager));
        sdk.upload_progress(field(manager, 0x130), &progress, nullptr, nullptr);
        return;
    }
    if (sdk.overlapped_result(field(manager, 0x130), nullptr, 1) == 0) {
        write32(manager, 0x12c, 9);
    } else {
        write32(manager, 0x12c, 8);
        write32(manager, 0x354, sdk.overlapped_error(field(manager, 0x130)));
    }
}
} // namespace bsp
