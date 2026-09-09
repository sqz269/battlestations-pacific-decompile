#include "bsp/memory_stream.hpp"
#include <algorithm>
#include <cstring>
#include <limits>
#include <new>
#include <utility>

namespace bsp {
struct MemoryStreamBacking {
    explicit MemoryStreamBacking(std::uint32_t count)
        : bytes(new std::uint8_t[count]), length(count) {}
    std::unique_ptr<std::uint8_t[]> bytes;
    std::uint32_t length;
    std::uint32_t initialized{};
};

MemoryStream MemoryStream::clone_reset_00bef6d0() const noexcept {
    MemoryStream clone;
    clone.backing_ = backing_;
    return clone;
}

std::int64_t MemoryStream::size_00bef600() const noexcept {
    return backing_ ? static_cast<std::int32_t>(backing_->length) : 0;
}

std::int64_t MemoryStream::position_00bef580() const noexcept {
    return backing_ ? static_cast<std::int32_t>(cursor_) : 0;
}

const std::uint8_t* MemoryStream::data_00bef610() const noexcept {
    return backing_ ? backing_->bytes.get() : nullptr;
}

std::uint32_t MemoryStream::initialized_size() const noexcept {
    return backing_ ? backing_->initialized : 0;
}

bool MemoryStream::fully_initialized() const noexcept {
    return backing_ && backing_->initialized == backing_->length;
}

bool MemoryStream::seek_00bef540(std::int64_t offset,
    std::uint32_t origin) noexcept {
    if (!backing_) return false;
    const std::uint32_t base = origin == 0 ? 0 :
        (origin == 1 ? cursor_ : backing_->length);
    const std::uint32_t next = base + static_cast<std::uint32_t>(offset);
    if (next > backing_->length) return false;
    cursor_ = next;
    return true;
}

bool MemoryStream::read_00bef590(void* destination, std::uint32_t requested,
    std::uint32_t* actual) noexcept {
    if (!backing_) return false;
    const std::uint32_t count = (std::min)(requested, backing_->length - cursor_);
    if (count != 0) {
        if (!destination || cursor_ > backing_->initialized ||
            count > backing_->initialized - cursor_) return false;
        const std::uint8_t* source = backing_->bytes.get() + cursor_;
        if (count == 4) {
            std::uint32_t word;
            std::memcpy(&word, source, sizeof(word));
            std::memcpy(destination, &word, sizeof(word));
        } else if (count == 2) {
            std::uint16_t word;
            std::memcpy(&word, source, sizeof(word));
            std::memcpy(destination, &word, sizeof(word));
        } else {
            std::memcpy(destination, source, count);
        }
    }
    cursor_ += count;
    if (actual) *actual = count;
    return true;
}

bool memory_stream_from_physical_00bef750_fragment(PhysicalFile& source,
    MemoryStream& output, DWORD& error) noexcept {
    if (!source.valid_00bf5020()) {
        error = ERROR_INVALID_HANDLE;
        return false;
    }
    if (!source.seek_00bf4f20(0, FILE_BEGIN, error)) return false;
    const std::uint64_t length = source.size_00bf4f90();
    if (length == 0) {
        error = ERROR_INVALID_DATA; // Native zero-length allocator path is unported.
        return false;
    }
    if (length >
        static_cast<std::uint64_t>((std::numeric_limits<std::int32_t>::max)())) {
        error = ERROR_FILE_TOO_LARGE; // Native truncated/negative size path is unported.
        return false;
    }
    MemoryStream converted;
    try {
        converted.backing_ = std::make_shared<MemoryStreamBacking>(
            static_cast<std::uint32_t>(length));
    } catch (const std::bad_alloc&) {
        error = ERROR_NOT_ENOUGH_MEMORY;
        return false;
    }
    const bool success = source.read_00bf5030(converted.backing_->bytes.get(),
        converted.backing_->length, converted.backing_->initialized, error);
    output = std::move(converted);
    return success;
}
}
