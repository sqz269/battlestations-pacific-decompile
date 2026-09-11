#include "bsp/storage_backend.hpp"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <array>
#include <cstring>
#include <limits>
#include <stdexcept>
#include <utility>
extern "C" {
#include <lua.h>
#include <lauxlib.h>
#include <zlib.h>
}

namespace bsp {
namespace {
constexpr const char* kinds[]{"quick", "player", "game"};
const char* kind_name(std::uint32_t kind) {
    if (kind >= std::size(kinds))
        throw std::out_of_range("native storage kind table has three entries");
    return kinds[kind];
}
std::string_view c_prefix(std::string_view value) {
    return value.substr(0, value.find('\0'));
}
void free_bytes(std::vector<std::uint8_t>& value) noexcept {
    std::vector<std::uint8_t>().swap(value);
}
struct File {
    HANDLE handle{INVALID_HANDLE_VALUE};
    ~File() { if (handle != INVALID_HANDLE_VALUE) CloseHandle(handle); }
};
bool plain_archive(const std::vector<std::uint8_t>& bytes) {
    char prefix[5]{};
    std::memcpy(prefix, bytes.data(), 4);
    return _stricmp(prefix, "Opti") == 0 || _stricmp(prefix, "Play") == 0 ||
        _stricmp(prefix, "BSP_") == 0 || _stricmp(prefix, "Save") == 0;
}
bool decode_blocks(std::vector<std::uint8_t>& bytes) {
    constexpr std::uint8_t key[]{0xc7, 0x04, 0x0f, 0x48, 0xfe, 0x4c};
    std::vector<std::pair<std::size_t, std::uint32_t>> blocks;
    std::size_t cursor = 0;
    while (cursor < bytes.size()) {
        if (bytes.size() - cursor < std::size(key)) return false;
        for (std::size_t i = 0; i < std::size(key); ++i) bytes[cursor + i] ^= key[i];
        std::uint32_t length;
        std::memcpy(&length, bytes.data() + cursor, sizeof(length));
        // Native trusts the DWORD and may read outside the allocation. The
        // projection rejects that undefined domain before handing bytes to zlib.
        if (length > bytes.size() - cursor - 4) return false;
        blocks.emplace_back(cursor + 4, length);
        cursor += 4 + length;
    }
    if (blocks.size() > std::numeric_limits<std::uint32_t>::max() / 0x10000u)
        return false;
    std::vector<std::uint8_t> expanded(blocks.size() * 0x10000u);
    std::size_t used = 0;
    for (const auto& block : blocks) {
        uLongf length = 0x10000;
        if (uncompress(expanded.data() + used, &length,
            bytes.data() + block.first, block.second) != Z_OK) return false;
        used += length;
    }
    expanded.resize(used);
    bytes.swap(expanded);
    return true;
}
} // namespace

PcStorageBackend::PcStorageBackend(std::string root, NativeStringStorage& strings,
    PcStorageLuaHost& lua, ArchiveCompressionState& compression)
    : root_(std::move(root)), strings_(strings), lua_(lua), compression_(compression) {}
PcStorageBackend::~PcStorageBackend() {
    operation_.prompt_message_14.release_to(strings_);
}
void PcStorageBackend::free_and_clear_storage_buffer_30() noexcept {
    free_bytes(buffer_30_);
    buffer_present_30_ = false;
}
void PcStorageBackend::close_storage_archive_00b65e80() noexcept {
    lua_.close_storage_archive_00b65e80();
}
std::string PcStorageBackend::path(std::string_view name, const char* leaf) const {
    std::string result(c_prefix(root_));
    result += '\\';
    result += c_prefix(name);
    if (leaf) { result += '\\'; result += leaf; }
    if (result.size() >= 256)
        throw std::length_error("native storage path exceeds its 256-byte buffer");
    return result;
}
bool PcStorageBackend::file_exists(std::string_view name, const char* leaf) const {
    File file{CreateFileA(path(name, leaf).c_str(), GENERIC_READ, FILE_SHARE_READ,
        nullptr, OPEN_EXISTING, 0, nullptr)};
    return file.handle != INVALID_HANDLE_VALUE;
}
bool PcStorageBackend::storage_query_1c(std::string_view name, std::uint32_t kind) {
    // 00beb1e0 short circuits before indexing the kind table.
    return file_exists(name, "valid") && file_exists(name, kind_name(kind));
}
void PcStorageBackend::request_read_00bd3d70(std::string_view name, std::uint32_t kind) {
    read_name_24_.assign(name);
    read_kind_2c_ = kind;
    error_20_ = false;
    operation_code_04_ = 2;
}
void PcStorageBackend::request_write_00bd3dc0(std::string_view name) {
    write_name_500_.assign(name);
    error_20_ = false;
    operation_code_04_ = 3;
}
void PcStorageBackend::request_single_write_00bd3e10(std::string_view name, std::uint32_t kind) {
    write_name_500_.assign(name);
    write_kind_508_ = kind;
    error_20_ = false;
    operation_code_04_ = 4;
}
void PcStorageBackend::request_delete_00bd3e70(std::string_view name) {
    delete_name_528_.assign(name);
    error_20_ = false;
    operation_code_04_ = 5;
}
void PcStorageBackend::clear_marker(std::string_view name) {
    DeleteFileA(path(name, "valid").c_str());
    if (file_exists(name, "valid")) error_20_ = true;
}
void PcStorageBackend::stamp_marker(std::string_view name) {
    bool stamped = false;
    {
        File file{CreateFileA(path(name, "valid").c_str(), GENERIC_WRITE,
            FILE_SHARE_WRITE, nullptr, CREATE_ALWAYS, 0, nullptr)};
        if (file.handle != INVALID_HANDLE_VALUE) {
            SYSTEMTIME now;
            FILETIME stamp;
            GetSystemTime(&now);
            SystemTimeToFileTime(&now, &stamp);
            stamped = SetFileTime(file.handle, nullptr, nullptr, &stamp) != FALSE;
        }
    }
    if (!stamped || !file_exists(name, "valid")) error_20_ = true;
}
void PcStorageBackend::delete_slot() {
    error_20_ = false;
    DeleteFileA(path(delete_name_528_, "valid").c_str());
    for (const auto* kind : kinds) DeleteFileA(path(delete_name_528_, kind).c_str());
    RemoveDirectoryA(path(delete_name_528_).c_str());
    // Native judges deletion by the marker only; it ignores unrelated files.
    if (file_exists(delete_name_528_, "valid")) error_20_ = true;
}
void PcStorageBackend::open_read_archive_00bd3470() {
    lua_.open_storage_archive_00b6a020(1);
    auto* state = storage_lua_38();
    if (!state) throw std::logic_error("storage Lua owner returned no state");
    (void)lua_gettop(state);
    int status = luaL_loadbuffer(state, reinterpret_cast<const char*>(buffer_30_.data()),
        buffer_30_.size(), "DoBuffer");
    if (status == 0) status = lua_pcall(state, 0, LUA_MULTRET, 0);
    (void)lua_gettop(state); // 00b65ef0 retains results/error; it does not pop them.
    if (status != 0) {
        error_20_ = true;
        free_and_clear_storage_buffer_30();
        close_storage_archive_00b65e80();
    }
}
void PcStorageBackend::read_archive_00bd3ec0() {
    error_20_ = false;
    std::vector<std::uint8_t> bytes;
    {
        File file{CreateFileA(path(read_name_24_, kind_name(read_kind_2c_)).c_str(),
            GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, 0, nullptr)};
        LARGE_INTEGER size{};
        if (file.handle == INVALID_HANDLE_VALUE || !GetFileSizeEx(file.handle, &size) ||
            size.QuadPart < 0 || size.QuadPart > std::numeric_limits<DWORD>::max()) {
            error_20_ = true;
        } else {
            bytes.resize(static_cast<std::size_t>(size.QuadPart));
            DWORD received{};
            if (!ReadFile(file.handle, bytes.data(), static_cast<DWORD>(bytes.size()),
                &received, nullptr) || received != bytes.size()) error_20_ = true;
        }
    }
    if (bytes.size() < 6) error_20_ = true;
    if (!error_20_ && !plain_archive(bytes) && !decode_blocks(bytes)) error_20_ = true;
    if (error_20_) { free_and_clear_storage_buffer_30(); return; }
    buffer_30_ = std::move(bytes);
    buffer_present_30_ = true;
    open_read_archive_00bd3470();
}
void PcStorageBackend::begin_write_00bd34c0() {
    compression_.buffer_50c.resize(0x10000);
    compression_.used_520 = 0;
    compression_.mode_524 = 1;
    // Native begin leaves the block list untouched.
}
void PcStorageBackend::abandon_write_00bd3500() noexcept {
    free_bytes(compression_.buffer_50c);
    compression_.mode_524 = 0;
}
void PcStorageBackend::write_archive_00bd37b0() {
    error_20_ = false;
    if (compression_.used_520 > compression_.buffer_50c.size())
        throw std::logic_error("storage write buffer shorter than native byte count");
    {
        File file{CreateFileA(path(write_name_500_, kind_name(write_kind_508_)).c_str(),
            GENERIC_WRITE, FILE_SHARE_WRITE, nullptr, CREATE_ALWAYS, 0, nullptr)};
        DWORD written{};
        if (file.handle == INVALID_HANDLE_VALUE || !WriteFile(file.handle,
            compression_.buffer_50c.data(), compression_.used_520, &written, nullptr))
            error_20_ = true;
        // Native checks the API BOOL only, not the byte count.
    }
    if (!error_20_) abandon_write_00bd3500();
}
void PcStorageBackend::prompt_00bd4140(const char* key, bool yes_no, bool accept) {
    operation_.prompt_pending_0c = 1;
    operation_.prompt_observed_10 = 0;
    const auto size = static_cast<std::uint32_t>(std::strlen(key));
    operation_.prompt_message_14.resize_0041dd40(strings_, size, true);
    std::memcpy(operation_.prompt_message_14.data(), key, size);
    operation_.prompt_flag_0d = yes_no;
    operation_.prompt_flag_0e = yes_no;
    operation_.prompt_flag_0f = accept;
    operation_.response_1c = 0;
}
void PcStorageBackend::complete(std::int32_t state) noexcept {
    operation_.state_08 = state;
    operation_code_04_ = 0; // 00bd3590
}
void PcStorageBackend::update_00beb9b0() {
    if (operation_code_04_ == 1) {
        ready_21_ = true; // PC slot+08 is the recovered literal-true leaf00beaa70.
        complete(0);
    } else if (operation_code_04_ == 2) {
        read_archive_00bd3ec0();
        if (!error_20_) stamp_marker(read_name_24_);
        else prompt_00bd4140("globals.loadfailed", false, true);
        complete(error_20_ ? 1 : 0);
    } else if (operation_code_04_ == 3 || operation_code_04_ == 4) {
        const bool two_files = operation_code_04_ == 3;
        if (operation_.state_08 == 0 || operation_.state_08 == 1) {
            if (two_files) {
                CreateDirectoryA(path(write_name_500_).c_str(), nullptr);
                if (!error_20_) clear_marker(write_name_500_);
                if (error_20_) {
                    prompt_00bd4140("globals.savefailed", false, true);
                    complete(1);
                    return;
                }
                prompt_00bd4140("globals.saving_pc", false, false);
                write_kind_508_ = 0;
            } else {
                prompt_00bd4140("globals.saving_pc", false, false);
                clear_marker(write_name_500_); // Native deliberately continues on error.
            }
            begin_write_00bd34c0();
            operation_.state_08 = 2;
        } else if (operation_.state_08 == 2) {
            finalize_compressed_archive_00bd4a70(compression_);
            write_archive_00bd37b0();
            if (error_20_) {
                abandon_write_00bd3500();
                prompt_00bd4140("globals.savefailed", false, true);
                complete(1);
            } else if (two_files && write_kind_508_ == 0) {
                write_kind_508_ = 1;
                begin_write_00bd34c0();
            } else {
                stamp_marker(write_name_500_);
                complete(0); // Native ignores marker failure in the final state.
            }
        }
    } else if (operation_code_04_ == 5) {
        if (operation_.state_08 == 0 || operation_.state_08 == 1) {
            prompt_00bd4140("globals.deleteslot", true, false);
            operation_.state_08 = 3;
        } else if (operation_.state_08 == 3) {
            if (operation_.response_1c == 1) {
                delete_slot();
                complete(error_20_ ? 1 : 0);
            } else complete(1);
        }
    }
}

void PcStorageOperationHost::storage_update_virtual_44(StorageManagerOperation& operation) {
    if (&operation != &backend_.operation())
        throw std::logic_error("storage driver is bound to a different manager");
    backend_.update_00beb9b0();
}
FrontEndPromptScreen& PcStorageOperationHost::prompt_screen_00425d10() {
    return ui_.prompt_screen_00425d10();
}
FrontEndPromptHost& PcStorageOperationHost::prompt_host() { return ui_.prompt_host(); }
void PcStorageOperationHost::try_begin_render_frame_004c6c30() { ui_.try_begin_render_frame_004c6c30(); }
void PcStorageOperationHost::render_game_004ca440() { ui_.render_game_004ca440(); }
void PcStorageOperationHost::finish_render_frame_004ca1f0() { ui_.finish_render_frame_004ca1f0(); }

} // namespace bsp
