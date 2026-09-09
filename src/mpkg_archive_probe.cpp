// One synthetic MPKG archive scenario. Framing and payload oracles are independent
// of MpkgArchive; see docs/MPKG_FIXTURE.md for generator identity and boundaries.
#include "bsp/mpkg_archive.hpp"
#include <algorithm>
#include <array>
#include <cstdio>
#include <cstring>
#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace {
using Bytes = std::vector<std::uint8_t>;
constexpr std::uint32_t prefix_size = 29;
constexpr std::uint32_t large_size = 0x40000 + 37;
constexpr std::string_view phrase =
    "MPKG independent raw-DEFLATE fixture: directory offsets and stream ownership.\n";
// Python 3.13.11; zlib compile 1.2.13/runtime 1.3.1; level9, raw -15,
// memLevel8, default strategy, phrase repeated2048. First block BTYPE=2.
// Compressed SHA256:707aac721ff2608596dc6553c4ed96ae9d3bbc2a93a889508164bb02e7fdd514.
constexpr char dynamic_hex[] =
    "edccb10dc2300000b09d2bf2001cc0864461804a0c3c50d1546420ad9220e07bb80379f1e8fe723a8694c7b8c41fb985"
    "32bcd6fbee70de5dbb30a5777b96b80d632af1d6e6f209f334d5d86a18f2186a2b717884f99563a9f7b46c56bdcd66b3"
    "d96c369bcd66b3d96c369bcd66b3d96c369bcd66b3d96c369bcd66b3d96c369bcd66b3d96c369bcd66b3d96c369bcd66"
    "b3d96c369bcd66b3d96c369bcd66b3d96c369bcd66b3d96c369bcd66b3d96c369bcd66b3d96c369bcd66b3d96c369bcd"
    "66b3d96c369bcd66b3d96c369bcd66b3d96c369bcd66b3d96c369bcd66b3d96c369bcd66b3d96c369bcd66b3d96c369b"
    "cd66b3d96c369bcd66b3d96c369bcd66b3d96c369bcd66b3d96c369bcd66b3d96c369bcd66b3d96c369bcd66b3d96c36"
    "9bcd66b3d96c369bcd66b3d96c369bcd66b3d96c369bcd66b3d96c369bcd66b3d96c369bcd66b3d96c369bcd66b3d96c"
    "369bcd66b3d96c369bcd66b3d96c369bcd66b3d96c369bcd66b3d96c369bcd66b3d96c369bcd66b3d96c369bcd66b3d9"
    "6c369bcd66b3d96c369bcd66b3d96c369bcd66b3d96c369bcd66b3d96c369bcd66b3d96c369bcd66b3d96c369bcd66b3"
    "d96c369bcd66b3d96c369bcd66b3d96c369bcd66b3d96c369bcd66b3d96c369bcd66b3d96c369bcd66b3d96c369bcd66"
    "b3d96c369bcd66b3d96c369bcd66b3d96c369bcd66b3d96c369bcd66b3d96c369bcd66b3d96c369bcd66b3d96c369bcd"
    "66b3d96c369bcd66b3d96c369bcd66b3d96c369bcd66b3d96c369bcd66b3d96c369bcd66b3d96c369bcd66b3d96c369b"
    "cd66b3d96c369bcd66b3d96c369bcd66b3d96c369bcd66b3d96c369bcd66b3d96c369bcd66b3d96c369bcd66b3d96c36"
    "9bcd66b3d96c369bcdf6d7db17";
// Verified native537-byte key00e144f0; copied from the static audit, not the
// archive implementation. SHA256:26abd3e2995fdbacecb6f352b5c0eb20ff20c9a7ada6312ba619c9d4c6fa6183.
constexpr char key_hex[] =
    "6c8b6f18b5a723dd6adaa19e74e607bcac106da1467c64756014972f763dcc0119b4c627ab2667eb035e7b6b0f49168e"
    "fd95d6187a6bd3650b655ce2d04c07d0c449325b4cb0461f1441d117f2cc1c09c4eaec7c75ef5a1a319e3b6f792e1712"
    "b52582cfbf5dc5308e7b07f9dc7445cfd9f4c7af60d335f0d5fcb0ea345e3d417f0fd5d562f7485e920c0f846d656a01"
    "23539c136d83bf23605565ce67fa62284fee2168d8276937d55bd5c5c76b2d6b36d77a70975a99fee8ea3ac740a90721"
    "4f368c210be30bc227d18dfc1a957674c7c42aafbedceab9e06fcdb9b6c1bc44c9cabb53ab74e49fc451637785b8099f"
    "bef8f6096d9aeb12b38895dde06d7ddc2455c140c0767d776529ce21c5b025a43002e02b20173062f90cc8729aa729fd"
    "f2eecbb97438bb3e30e9f3574fab738d35d278583f5d6e2b22c3f65041c012fdb58e6448bb5f93fa5efdaa15db836d0a"
    "6a3ea0fd7f40744353aada47bf5b31d7e07b330cb5b2097d642f6c20cf4c0b6090a2ece3c601e0f8a0154367e4e363d7"
    "e5ca931360f4df99f038dac070632d2f1ab02f8dc9b632c0c454335dfe80e6d15f2bd798930d6c8907b5bbb7493744cd"
    "7d2699a90e5de35a4c1644948012bdbb10850bca0bd50163a6ee829f241c339e0d7180ac447843fdba1598e0cad016a9"
    "1d2b90cb1550422fe809fad4f599c29135465ac12a6a5fa94f5ebc48a8ec98797993c875f366a20833975402fd7dea8d"
    "f71b25053cdb114e51";

Bytes from_hex(std::string_view text) {
    const auto digit = [](char c) { return c <= '9' ? c - '0' : c - 'a' + 10; };
    Bytes result;
    result.reserve(text.size() / 2);
    for (std::size_t i = 0; i < text.size(); i += 2)
        result.push_back(static_cast<std::uint8_t>(digit(text[i]) * 16 + digit(text[i + 1])));
    return result;
}
Bytes ascii(std::string_view value) { return Bytes(value.begin(), value.end()); }
void word(Bytes& bytes, std::size_t offset, std::uint16_t value) {
    bytes[offset] = static_cast<std::uint8_t>(value);
    bytes[offset + 1] = static_cast<std::uint8_t>(value >> 8);
}
void dword(Bytes& bytes, std::size_t offset, std::uint32_t value) {
    word(bytes, offset, static_cast<std::uint16_t>(value));
    word(bytes, offset + 2, static_cast<std::uint16_t>(value >> 16));
}
struct FixtureEntry {
    std::string central_name, local_name;
    std::uint16_t method{}, local_extra{}, central_extra{}, comment{};
    Bytes payload;
    std::uint32_t decoded_size{}, local_offset{}, data_offset{};
};
struct Fixture {
    Bytes encoded;
    std::array<FixtureEntry, 4> entries;
    std::uint32_t central_offset{}, central_size{}, eocd_offset{}, padding{};
};
Fixture make_fixture() {
    Fixture f;
    f.entries[0] = {"Case.TXT", "different-local-name", 0, 5, 1, 2,
        ascii("first small decoded entry\n")};
    f.entries[1] = {"cASE.tXT", "duplicate", 0, 0, 0, 0,
        ascii("duplicate must not win\n")};
    f.entries[2] = {"Packed.TXT", "raw", 8, 3, 0, 1, from_hex(dynamic_hex)};
    f.entries[3] = {"Large.BIN", "original-source", 0, 2, 0, 0, Bytes(large_size)};
    for (std::uint32_t i = 0; i < large_size; ++i)
        f.entries[3].payload[i] = static_cast<std::uint8_t>((i * 17u + 31u) & 255u);
    Bytes decoded(prefix_size, 0x3d);
    for (auto& entry : f.entries) {
        entry.decoded_size = entry.method ? static_cast<std::uint32_t>(phrase.size() * 2048)
            : static_cast<std::uint32_t>(entry.payload.size());
        entry.local_offset = static_cast<std::uint32_t>(decoded.size());
        const auto start = decoded.size();
        decoded.resize(start + 30, 0);
        dword(decoded, start, 0x04034b50); word(decoded, start + 4, 20);
        word(decoded, start + 8, entry.method);
        dword(decoded, start + 18, static_cast<std::uint32_t>(entry.payload.size()));
        dword(decoded, start + 22, entry.decoded_size);
        word(decoded, start + 26, static_cast<std::uint16_t>(entry.local_name.size()));
        word(decoded, start + 28, entry.local_extra);
        decoded.insert(decoded.end(), entry.local_name.begin(), entry.local_name.end());
        decoded.insert(decoded.end(), entry.local_extra, 0x6a);
        entry.data_offset = static_cast<std::uint32_t>(decoded.size());
        decoded.insert(decoded.end(), entry.payload.begin(), entry.payload.end());
    }
    f.central_offset = static_cast<std::uint32_t>(decoded.size());
    for (const auto& entry : f.entries) {
        const auto start = decoded.size();
        decoded.resize(start + 46, 0);
        dword(decoded, start, 0x02014b50); word(decoded, start + 4, 20);
        word(decoded, start + 6, 20); word(decoded, start + 10, entry.method);
        dword(decoded, start + 20, static_cast<std::uint32_t>(entry.payload.size()));
        dword(decoded, start + 24, entry.decoded_size);
        word(decoded, start + 28, static_cast<std::uint16_t>(entry.central_name.size()));
        word(decoded, start + 30, entry.central_extra);
        word(decoded, start + 32, entry.comment);
        dword(decoded, start + 42, entry.local_offset - prefix_size);
        decoded.insert(decoded.end(), entry.central_name.begin(), entry.central_name.end());
        decoded.insert(decoded.end(), entry.central_extra, 0x5a);
        decoded.insert(decoded.end(), entry.comment, 0x4a);
    }
    const auto directory_end = decoded.size();
    // Keep a29-byte prefix adjustment while seeking the directory at its direct
    // absolute offset. The size includes some padding, not all padding.
    decoded.insert(decoded.end(), prefix_size + 7, 0x6d);
    while ((decoded.size() + 22) % 753 != 17) decoded.push_back(0x6d);
    f.padding = static_cast<std::uint32_t>(decoded.size() - directory_end);
    f.eocd_offset = static_cast<std::uint32_t>(decoded.size());
    f.central_size = f.eocd_offset - f.central_offset - prefix_size;
    decoded.resize(decoded.size() + 22, 0);
    dword(decoded, f.eocd_offset, 0x06054b50);
    word(decoded, f.eocd_offset + 8, 4); word(decoded, f.eocd_offset + 10, 4);
    dword(decoded, f.eocd_offset + 12, f.central_size);
    dword(decoded, f.eocd_offset + 16, f.central_offset);
    const auto key = from_hex(key_hex);
    const auto length = static_cast<std::uint32_t>(decoded.size());
    f.encoded.resize(length);
    // Inverse of the audit's native transform: place each known decoded byte
    // at its encoded source position. Crucially, i % block_length is global.
    for (std::uint32_t i = 0; i < length; ++i) {
        const auto block = i / 753;
        const auto block_length = block == length / 753 ? length % 753 : 753;
        const auto source = block * 753 + block_length - 1 - i % block_length;
        f.encoded[source] = decoded[i] ^ key[source % key.size()];
    }
    return f;
}
struct ReopenTrace {
    std::uint32_t calls{}, seeks{}, reads{}, first_seek{}, source_size{};
    std::uint64_t requested{}, actual{};
    bool original_identity{true};
};
class OriginalEncodedSource final : public bsp::InflateSource {
public:
    OriginalEncodedSource(std::shared_ptr<const Bytes> bytes, std::shared_ptr<ReopenTrace> trace)
        : bytes_(std::move(bytes)), trace_(std::move(trace)) {}
    bool seek_absolute(std::uint32_t offset) noexcept override {
        if (trace_->seeks++ == 0) trace_->first_seek = offset;
        if (offset > bytes_->size()) return false;
        cursor_ = offset;
        return true;
    }
    bool read(void* destination, std::uint32_t requested, std::uint32_t& actual) noexcept override {
        ++trace_->reads;
        trace_->requested += requested;
        actual = (std::min)(requested, static_cast<std::uint32_t>(bytes_->size()) - cursor_);
        if (actual) std::memcpy(destination, bytes_->data() + cursor_, actual);
        cursor_ += actual;
        trace_->actual += actual;
        return true;
    }
    const Bytes* identity() const noexcept { return bytes_.get(); }
private:
    std::shared_ptr<const Bytes> bytes_;
    std::shared_ptr<ReopenTrace> trace_;
    std::uint32_t cursor_{};
};

bool matches_owned_output(bsp::MemoryStream& stream, const Bytes& expected) {
    if (!stream.fully_initialized() || stream.size_00bef600() != expected.size()
        || stream.position_00bef580() != 0) return false;
    Bytes observed(expected.size() + 7, 0xcc);
    std::uint32_t actual = 0xffffffffu;
    return stream.read_00bef590(observed.data(), static_cast<std::uint32_t>(observed.size()), &actual)
        && actual == expected.size() && stream.position_00bef580() == expected.size()
        && std::equal(expected.begin(), expected.end(), observed.begin())
        && std::all_of(observed.begin() + actual, observed.end(), [](auto byte) { return byte == 0xcc; });
}
}

bool probe_mpkg_archive() {
    std::array<bsp::MemoryStream, 3> output;
    std::array<Bytes, 3> expected;
    std::array<std::uint32_t, 3> expected_offsets{};
    auto trace = std::make_shared<ReopenTrace>();
    std::weak_ptr<const Bytes> original_lifetime;
    std::weak_ptr<OriginalEncodedSource> reopened_lifetime;
    std::string error;
    bool checked = true;
    std::size_t observed_entries{};
    std::uint32_t opened_routes{};
    std::uint32_t fixture_size{}, directory{}, central_size{}, eocd{}, padding{}, partial_phase{};
    {
        auto fixture = make_fixture();
        fixture_size = static_cast<std::uint32_t>(fixture.encoded.size());
        directory = fixture.central_offset; central_size = fixture.central_size;
        eocd = fixture.eocd_offset; padding = fixture.padding;
        partial_phase = (fixture_size - 17) % 17;
        expected[0] = ascii("first small decoded entry\n");
        expected[1].reserve(phrase.size() * 2048);
        for (unsigned i = 0; i < 2048; ++i)
            expected[1].insert(expected[1].end(), phrase.begin(), phrase.end());
        const auto large_offset = fixture.entries[3].data_offset;
        expected[2].assign(fixture.encoded.begin() + large_offset,
            fixture.encoded.begin() + large_offset + large_size);
        expected_offsets = {fixture.entries[0].data_offset,
            fixture.entries[2].data_offset, large_offset};
        checked = fixture_size % 753 == 17 && partial_phase != 0
            && eocd - directory - central_size == prefix_size
            && fixture.entries[0].local_name.size() != fixture.entries[0].central_name.size()
            && fixture.entries[0].local_extra != fixture.entries[0].central_extra
            && (fixture.entries[2].payload[0] & 7u) == 5u
            && expected[1].size() > 0x10000 && expected[2].size() > 0x40000
            && expected[2] != fixture.entries[3].payload;
        auto original = std::make_shared<const Bytes>(std::move(fixture.encoded));
        original_lifetime = original;
        trace->source_size = static_cast<std::uint32_t>(original->size());
        bsp::MemoryStream encoded;
        DWORD copy_error{};
        bsp::MpkgArchive archive;
        checked = checked && bsp::memory_stream_from_bytes_00befa40_fragment(original->data(),
            static_cast<std::uint32_t>(original->size()), encoded, copy_error)
            && encoded.seek_00bef540(7, 0)
            && archive.load_00bb9920_fragment(encoded,
                [original, trace, &reopened_lifetime]() -> std::shared_ptr<bsp::InflateSource> {
                    ++trace->calls;
                    auto source = std::make_shared<OriginalEncodedSource>(original, trace);
                    trace->original_identity = trace->original_identity && source->identity() == original.get();
                    reopened_lifetime = source;
                    return source;
                }, error)
            && encoded.position_00bef580() == 7 && archive.entry_count() == 4
            && archive.contains_00bb8e00("cAsE.TxT");
        observed_entries = archive.entry_count();
        constexpr std::array<std::string_view, 3> names{"cAsE.TxT", "pACKED.txt", "lARGE.bin"};
        constexpr std::array<bsp::MpkgEntryRoute, 3> routes{
            bsp::MpkgEntryRoute::copy_from_decoded,
            bsp::MpkgEntryRoute::raw_inflate_from_decoded,
            bsp::MpkgEntryRoute::reopen_original_range};
        for (std::size_t i = 0; checked && i < output.size(); ++i) {
            bsp::MpkgEntryReadPlan plan;
            checked = archive.plan_entry_00bb8d60_fragment(names[i], 2, plan, error)
                && plan.route == routes[i] && plan.source_offset == expected_offsets[i]
                && plan.decoded_size == expected[i].size()
                && plan.compressed_size == (i == 1 ? 637u : expected[i].size())
                && plan.method == (i == 1 ? 8 : 0)
                && archive.open_entry_00bb8d60_fragment(names[i], 2, output[i], error);
            if (checked) ++opened_routes;
        }
        original.reset();
    } // Archive, its decoded backing, original encoded owner and source are gone.
    checked = checked && original_lifetime.expired() && reopened_lifetime.expired()
        && trace->calls == 1 && trace->seeks == 1 && trace->first_seek == expected_offsets[2]
        && trace->reads > 0 && trace->requested == large_size && trace->actual == large_size
        && trace->source_size == fixture_size && trace->original_identity;
    for (std::size_t i = 0; checked && i < output.size(); ++i)
        checked = matches_owned_output(output[i], expected[i]);
    std::printf("Synthetic MPKG: bytes=%u entries=%zu directory=%u central_size=%u eocd=%u padding=%u prefix=%u final_block=17 phase=%u routes_opened=%u dynamic=637/%zu reopen=%u seeks=%u reads=%u requested=%llu actual=%llu encoded_source_and_owned_outputs=%d error=%s\n",
        fixture_size, observed_entries, directory, central_size, eocd, padding, prefix_size, partial_phase, opened_routes,
        expected[1].size(), trace->calls, trace->seeks, trace->reads,
        static_cast<unsigned long long>(trace->requested), static_cast<unsigned long long>(trace->actual),
        checked, error.c_str());
    return checked;
}
