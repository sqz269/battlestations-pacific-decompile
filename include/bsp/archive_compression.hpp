#pragma once

#include "bsp/archive_text_writer.hpp"
#include <cstddef>
#include <cstdint>
#include <string_view>
#include <vector>

namespace bsp {

inline constexpr std::size_t kArchiveCompressionBlockSize = 0x10000;
struct ArchiveCompressedBlock {
    std::vector<std::uint8_t> bytes; // native pair: allocated pointer, DWORD length
};

// Host storage for the manager fields, not their original layout. During mode1
// buffer_50c is a 64KB scratch allocation; after00bd4a70 it is the framed file.
// The storage backend owns native begin/reset transitions and field+508h.
struct ArchiveCompressionState {
    std::vector<std::uint8_t> buffer_50c;
    std::vector<ArchiveCompressedBlock> blocks_510;
    std::uint32_t used_520{};
    std::int32_t mode_524{};
};

// Original ECX=manager, RET; checked assembly includes the tail after the
// incorrectly nonreturning _free at00bd4974. Uses linked stock zlib1.2.1 compress.
void flush_archive_block_00bd48f0(ArchiveCompressionState&);
// Original ECX=manager, NativeString* on stack, RET4. At most one flush per call;
// inputs whose remainder exceeds64KB would overflow native storage and are
// rejected by this host projection. Empty spans remain meaningful when full.
void write_compressed_archive_00bd49b0(ArchiveCompressionState&, std::string_view);
// Original ECX=manager, RET00bd4be4. Each block has a little-endian length then
// zlib bytes; XOR first six bytes with C7 04 0F 48 FE 4C. Frees scratch/blocks,
// sets final used byte count and mode2. Empty pending input produces no block.
void finalize_compressed_archive_00bd4a70(ArchiveCompressionState&);

class ArchiveCompressedSink final : public ArchiveByteSink {
public:
    explicit ArchiveCompressedSink(ArchiveCompressionState& state) noexcept : state_(state) {}
    void write(std::string_view bytes) override;
private:
    ArchiveCompressionState& state_;
};

} // namespace bsp
