#pragma once

#include "bsp/file_store_cc13.hpp"

#include <algorithm>
#include <array>
#include <cstring>
#include <functional>
#include <stdexcept>
#include <utility>

namespace bsp::game {

// Borrowed, forward-only view of the extent reported by an opened VFS stream.
// Buffering is application transport, not a recovered native layout/ABI. The
// native owner, reference and current-profile dispatch stay with the caller.
class VfsWholeStreamReader final : public cc13_files::ByteStream {
public:
    using Read = std::function<void(void*, std::uint32_t, std::uint32_t*)>;

    VfsWholeStreamReader(std::uint32_t extent, Read read)
        : remaining_(extent), read_(std::move(read)) {}

    std::size_t read(void* destination, std::size_t size) override {
        std::size_t copied = 0;
        while (copied < size) {
            if (next_ == buffered_) {
                if (remaining_ == 0) break;
                const auto requested = (std::min)(remaining_,
                    static_cast<std::uint32_t>(buffer_.size()));
                std::uint32_t actual = 0;
                read_(buffer_.data(), requested, &actual);
                if (actual == 0 || actual > requested)
                    throw std::runtime_error("Native VFS full-file read was short or invalid");
                remaining_ -= actual;
                next_ = 0;
                buffered_ = actual;
            }
            const auto count = (std::min)(size - copied,
                static_cast<std::size_t>(buffered_ - next_));
            std::memcpy(static_cast<char*>(destination) + copied,
                buffer_.data() + next_, count);
            next_ += static_cast<std::uint32_t>(count);
            copied += count;
            position_ += count;
        }
        return copied;
    }

    std::int64_t tell() override { return static_cast<std::int64_t>(position_); }
    // 00BE4770 only reads. These operations are outside this borrowed view's
    // contract, rather than silently mutating the retained source behind it.
    std::size_t write(const void*, std::size_t) override {
        throw std::logic_error("Whole-file VFS view is read-only");
    }
    void seek(std::int64_t) override {
        throw std::logic_error("Whole-file VFS view is forward-only");
    }

private:
    std::uint32_t remaining_;
    Read read_;
    // Host I/O batch size; contains only bytes returned by the actual stream.
    std::array<char, 4096> buffer_;
    std::uint32_t next_ = 0, buffered_ = 0;
    std::uint64_t position_ = 0;
};

} // namespace bsp::game
