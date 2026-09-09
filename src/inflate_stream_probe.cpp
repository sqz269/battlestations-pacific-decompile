// One explicit raw stored-DEFLATE fixture exercises the buffered stream adapter.
// Fixture framing is independent of zlib's compressor and of the game archives.
#include "bsp/inflate_stream.hpp"
#include <algorithm>
#include <cstdio>
#include <cstring>
#include <vector>

namespace {
class ShortReadSource final : public bsp::InflateSource {
public:
    std::vector<unsigned char> bytes;
    std::uint32_t cursor{}, reads{}, seeks{};
    bool seek_absolute(std::uint32_t offset) noexcept override {
        ++seeks;
        if (offset > bytes.size()) return false;
        cursor = offset;
        return true;
    }
    bool read(void* destination, std::uint32_t requested,
        std::uint32_t& actual) noexcept override {
        ++reads;
        actual = (std::min)({requested, 1021u,
            static_cast<std::uint32_t>(bytes.size()) - cursor});
        if (actual) std::memcpy(destination, bytes.data() + cursor, actual);
        cursor += actual;
        return true;
    }
};
}

bool probe_inflate_stream() {
    using Status = bsp::InflateStreamStatus;
    constexpr std::uint32_t decoded_size = 150123;
    constexpr std::uint32_t prefix_size = 13;
    std::vector<unsigned char> plain(decoded_size);
    std::uint32_t state = 0x651ab093;
    for (auto& byte : plain) {
        state ^= state << 13; state ^= state >> 17; state ^= state << 5;
        byte = static_cast<unsigned char>(state);
    }
    auto source = std::make_shared<ShortReadSource>();
    source->bytes.assign(prefix_size, 0xcc);
    for (std::uint32_t offset = 0; offset < decoded_size;) {
        const auto count = (std::min)(65535u, decoded_size - offset);
        const auto complement = count ^ 0xffffu;
        // Stored blocks start on byte boundaries: BFINAL, then LEN/NLEN LE.
        source->bytes.push_back(offset + count == decoded_size ? 1 : 0);
        source->bytes.push_back(static_cast<unsigned char>(count));
        source->bytes.push_back(static_cast<unsigned char>(count >> 8));
        source->bytes.push_back(static_cast<unsigned char>(complement));
        source->bytes.push_back(static_cast<unsigned char>(complement >> 8));
        source->bytes.insert(source->bytes.end(), plain.begin() + offset,
            plain.begin() + offset + count);
        offset += count;
    }
    const auto compressed_size = static_cast<std::uint32_t>(source->bytes.size()) - prefix_size;
    std::weak_ptr<ShortReadSource> retained = source;
    bool checked = true;
    std::uint32_t reads = 0;
    {
        bsp::InflateStream stream;
        checked = stream.open_00bbc1d0_fragment(source,
            {prefix_size, compressed_size, decoded_size}) == Status::ok;
        source.reset();
        std::vector<unsigned char> output(70000);
        const auto read_equal = [&](std::uint32_t offset, std::uint32_t requested,
            std::uint32_t expected_actual, Status expected_status) {
            output.assign(requested, 0xcc);
            std::uint32_t actual = 0xffffffffu;
            return stream.read_00bbc140(output.data(), requested, &actual) == expected_status
                && actual == expected_actual
                && std::equal(output.begin(), output.begin() + actual, plain.begin() + offset)
                && stream.position_00bbbe50() == offset + actual;
        };
        checked = checked && !retained.expired() && stream.size_00bbbdd0() == decoded_size
            && read_equal(0, 70000, 70000, Status::ok)
            && stream.seek_00bbc060(-123, 1) == Status::ok
            && read_equal(69877, 333, 333, Status::ok)
            && stream.seek_00bbc060(135000, 0) == Status::ok
            && stream.seek_00bbc060(0, 0) == Status::unsupported_backward_seek
            && stream.position_00bbbe50() == 135000
            && stream.seek_00bbc060(0x100000000LL + 135010, 0) == Status::ok
            && read_equal(135010, 16000, decoded_size - 135010, Status::end_of_stream)
            && stream.seek_00bbc060(7, 9) == Status::end_of_stream
            && stream.position_00bbbe50() == decoded_size;
        const auto owner = retained.lock();
        reads = owner ? owner->reads : 0;
        checked = checked && owner && owner->seeks == 1 && reads > 140;
    }
    checked = checked && retained.expired();
    std::printf("Raw inflater stream: decoded=%u compressed=%u short_source_reads=%u buffered_read_seek_EOF_and_retention=%d\n",
        decoded_size, compressed_size, reads, checked);
    return checked;
}
