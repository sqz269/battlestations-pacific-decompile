#include "bsp/archive_compression.hpp"
#include <array>
#include <cstring>
#include <limits>
#include <stdexcept>
#include <utility>
#include <zlib.h>

namespace bsp {
namespace {
void require_scratch(const ArchiveCompressionState& state) {
    if (state.buffer_50c.size() != kArchiveCompressionBlockSize
        || state.used_520 > kArchiveCompressionBlockSize)
        throw std::logic_error("Archive compression scratch buffer is not initialized");
}
void release_bytes(std::vector<std::uint8_t>& bytes) {
    std::vector<std::uint8_t>{}.swap(bytes);
}
}
void flush_archive_block_00bd48f0(ArchiveCompressionState& state) {
    require_scratch(state);
    //00d62c70 is double bitsBF50624DE0000000. x87 FISTP uses truncate mode.
    const auto correction = static_cast<std::int32_t>(
        static_cast<double>(state.used_520) * -0.0010000000474974513);
    const auto capacity = static_cast<std::uint32_t>(
        static_cast<std::int32_t>(state.used_520) - correction + 12);
    std::vector<std::uint8_t> temporary(capacity);
    uLongf produced = capacity;
    const int result = compress(temporary.data(), &produced,
        state.buffer_50c.data(), state.used_520);
    // Native ignores this return value. Allocation/codec failure cannot produce
    // a valid archive; the host reports it rather than emitting unspecified data.
    if (result != Z_OK || produced > capacity)
        throw std::runtime_error("Archive zlib compression failed");
    ArchiveCompressedBlock block;
    block.bytes.assign(temporary.begin(), temporary.begin() + produced);
    release_bytes(temporary); // native frees temporary before appending the pair
    state.blocks_510.push_back(std::move(block)); //00bd4860 value-pair append
    state.used_520 = 0;
}
void write_compressed_archive_00bd49b0(ArchiveCompressionState& state,
    std::string_view bytes) {
    require_scratch(state);
    const auto remaining = kArchiveCompressionBlockSize - state.used_520;
    if (bytes.size() > remaining + kArchiveCompressionBlockSize)
        throw std::length_error("Native archive append remainder exceeds64KB");
    if (bytes.size() < remaining) {
        if (!bytes.empty()) std::memcpy(state.buffer_50c.data() + state.used_520,
            bytes.data(), bytes.size());
        state.used_520 += static_cast<std::uint32_t>(bytes.size());
        return;
    }
    if (remaining) std::memcpy(state.buffer_50c.data() + state.used_520,
        bytes.data(), remaining);
    state.used_520 += static_cast<std::uint32_t>(remaining);
    flush_archive_block_00bd48f0(state);
    const auto tail = bytes.size() - remaining;
    if (tail) std::memcpy(state.buffer_50c.data() + state.used_520,
        bytes.data() + remaining, tail);
    state.used_520 += static_cast<std::uint32_t>(tail);
}
void finalize_compressed_archive_00bd4a70(ArchiveCompressionState& state) {
    require_scratch(state);
    if (state.used_520) flush_archive_block_00bd48f0(state);
    release_bytes(state.buffer_50c);
    std::size_t total = 0;
    for (const auto& block : state.blocks_510) {
        if (block.bytes.size() < 2 || block.bytes.size() > 0xffffffffu - 4
            || total > 0xffffffffu - 4 - block.bytes.size())
            throw std::length_error("Native archive framing length is invalid");
        total += 4 + block.bytes.size();
    }
    state.buffer_50c.resize(total);
    state.used_520 = 0;
    constexpr std::array<std::uint8_t, 6> mask{0xc7, 4, 0x0f, 0x48, 0xfe, 0x4c};
    for (auto& block : state.blocks_510) {
        const auto length = static_cast<std::uint32_t>(block.bytes.size());
        auto* header = state.buffer_50c.data() + state.used_520;
        for (unsigned i = 0; i < 4; ++i)
            header[i] = static_cast<std::uint8_t>(length >> (i * 8));
        state.used_520 += 4;
        std::memcpy(state.buffer_50c.data() + state.used_520, block.bytes.data(), length);
        state.used_520 += length;
        for (std::size_t i = 0; i < mask.size(); ++i) header[i] ^= mask[i];
        release_bytes(block.bytes);
    }
    state.blocks_510.clear();
    state.mode_524 = 2;
}
void ArchiveCompressedSink::write(std::string_view bytes) {
    write_compressed_archive_00bd49b0(state_, bytes);
}
} // namespace bsp
