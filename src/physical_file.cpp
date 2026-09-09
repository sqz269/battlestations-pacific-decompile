#include "bsp/physical_file.hpp"
#include <cstring>

namespace bsp {
PhysicalFile::~PhysicalFile() {
    DWORD error{};
    close_00bf5090_fragment(error);
}

bool PhysicalFile::open_read_only_00bf52a0_fragment(const char* path,
    DWORD& error) noexcept {
    if (valid_00bf5020()) {
        error = ERROR_ALREADY_EXISTS; // Host ownership guard: native overwrites.
        return false;
    }
    handle_ = CreateFileA(path ? path : "", GENERIC_READ, FILE_SHARE_READ,
        nullptr, OPEN_EXISTING, 0, nullptr);
    bool success = valid_00bf5020();
    error = success ? ERROR_SUCCESS : GetLastError();
    if (success) {
        LARGE_INTEGER size;
        std::memcpy(&size, &size_, sizeof(size));
        success = GetFileSizeEx(handle_, &size) != FALSE;
        error = success ? ERROR_SUCCESS : GetLastError();
        std::memcpy(&size_, &size, sizeof(size_));
    }
    position_ = 0;
    return success;
}

bool PhysicalFile::valid_00bf5020() const noexcept {
    return handle_ != INVALID_HANDLE_VALUE;
}

std::uint64_t PhysicalFile::size_00bf4f90() const noexcept {
    return size_;
}

bool PhysicalFile::seek_00bf4f20(std::int64_t distance, DWORD origin,
    DWORD& error) noexcept {
    LARGE_INTEGER native_distance;
    native_distance.QuadPart = distance;
    LARGE_INTEGER position;
    std::memcpy(&position, &position_, sizeof(position));
    const bool success = SetFilePointerEx(handle_, native_distance,
        &position, origin) != FALSE;
    error = success ? ERROR_SUCCESS : GetLastError();
    std::memcpy(&position_, &position, sizeof(position_));
    return success;
}

bool PhysicalFile::read_00bf5030(void* destination, std::uint32_t requested,
    std::uint32_t& actual, DWORD& error) noexcept {
    DWORD transferred{};
    const bool success = ReadFile(handle_, destination, requested,
        &transferred, nullptr) != FALSE;
    error = success ? ERROR_SUCCESS : GetLastError();
    position_ += transferred; // Defined unsigned wrap matches ADD/ADC.
    actual = transferred;
    return success;
}

bool PhysicalFile::close_00bf5090_fragment(DWORD& error) noexcept {
    if (!valid_00bf5020()) {
        error = ERROR_SUCCESS;
        return true;
    }
    const bool success = CloseHandle(handle_) != FALSE;
    error = success ? ERROR_SUCCESS : GetLastError();
    handle_ = INVALID_HANDLE_VALUE;
    return success;
}
}
