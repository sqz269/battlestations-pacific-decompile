// One synthetic MPKG archive scenario. Framing and payload oracles are independent
// of MpkgArchive; see docs/MPKG_FIXTURE.md for generator identity and boundaries.
#include "bsp/mpkg_archive.hpp"
#include "bsp/mpkg_provider.hpp"
#include "bsp/mounted_streams.hpp"
#include "bsp/package_scan.hpp"
#include "bsp/vfs_provider_manager.hpp"
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
    std::vector<FixtureEntry> entries;
    std::uint32_t central_offset{}, central_size{}, eocd_offset{}, padding{};
};
Fixture make_fixture(std::vector<FixtureEntry> entries) {
    Fixture f;
    f.entries = std::move(entries);
    Bytes decoded(prefix_size, 0x3d);
    for (auto& entry : f.entries) {
        if (!entry.method) entry.decoded_size = static_cast<std::uint32_t>(entry.payload.size());
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
    const auto entry_count = static_cast<std::uint16_t>(f.entries.size());
    word(decoded, f.eocd_offset + 8, entry_count);
    word(decoded, f.eocd_offset + 10, entry_count);
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
Fixture make_fixture() {
    std::vector<FixtureEntry> entries(4);
    entries[0] = {"Case.TXT", "different-local-name", 0, 5, 1, 2,
        ascii("first small decoded entry\n")};
    entries[1] = {"cASE.tXT", "duplicate", 0, 0, 0, 0,
        ascii("duplicate must not win\n")};
    entries[2] = {"Packed.TXT", "raw", 8, 3, 0, 1, from_hex(dynamic_hex),
        static_cast<std::uint32_t>(phrase.size() * 2048)};
    entries[3] = {"Large.BIN", "original-source", 0, 2, 0, 0, Bytes(large_size)};
    for (std::uint32_t i = 0; i < large_size; ++i)
        entries[3].payload[i] = static_cast<std::uint8_t>((i * 17u + 31u) & 255u);
    return make_fixture(std::move(entries));
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

// Same synthetic framing, now with real nested encoded bytes and the three
// startup factories. No physical fixture file or installed asset is created.
bool package_scan_fixture(std::string& error) {
    const auto marker_bytes = ascii("opened through the second freshly mounted package\n");
    const auto inner = make_fixture({{"marker.txt", "marker", 0, 0, 0, 0, marker_bytes}});
    const auto outer = make_fixture({{"./inner.mpkg", "nested", 0, 0, 0, 0, inner.encoded}});
    if (inner.encoded.size() > 0x40000) return false; // Must take decoded small-copy route.
    auto factories = std::make_shared<bsp::VfsProviderFactories>();
    std::shared_ptr<bsp::MemoryStream> marker;
    bool checked = true;
    unsigned queries{}, lookups{}, requests{};
    bool arguments = true, publication = true;
    std::array<bsp::PackageScanPass, 2> reports;
    {
        bsp::VfsProviderManager manager(factories);
        bsp::VfsProviderManager peer(factories);
        std::shared_ptr<bsp::VfsProviderIdentity> store_identity, peer_identity, physical_identity;
        checked = manager.mount_system_path_00be1890_fragment("filestore", ".", 0, 0, 3,
            store_identity, error) == bsp::VfsProviderCreateStatus::created
            && peer.mount_system_path_00be1890_fragment("FILESTORE", ".", 0, 0, 7,
                peer_identity, error) == bsp::VfsProviderCreateStatus::created
            && store_identity == peer_identity && store_identity->system_name.empty()
            && store_identity->kind == bsp::VfsProviderKind::file_store && store_identity->device_id == 7
            && manager.find_system_name_00bdb120("") == store_identity
            && !manager.find_system_name_00bdb120("filestore")
            && manager.file_store_00be80b0_fragment() == peer.file_store_00be80b0_fragment();
        // The physical factory's audited trailing-backslash gate creates a
        // provider without checking the filesystem. Keep it in the peer only.
        checked = checked && peer.mount_system_path_00be1890_fragment("synthetic-root\\", "physical",
            4, 1, 8, physical_identity, error) == bsp::VfsProviderCreateStatus::created
            && physical_identity->kind == bsp::VfsProviderKind::physical_directory
            && physical_identity->system_name == "synthetic-root\\" && physical_identity->device_id == 8
            && peer.registrations().front().native_ownership_flag == 1;
        if (!checked) return false;
        auto store = manager.file_store_00be80b0_fragment();
        std::vector<std::shared_ptr<bsp::MemoryStream>> stored_sources;
        const auto put = [&](const char* name, const Bytes& bytes) {
            auto source = std::make_shared<bsp::MemoryStream>();
            DWORD copy_error{};
            if (!bsp::memory_stream_from_bytes_00befa40_fragment(bytes.data(),
                static_cast<std::uint32_t>(bytes.size()), *source, copy_error)
                || !source->seek_00bef540(7, 0)
                || store->add_file_00be7760(name, source) != bsp::FileStoreInsertResult::inserted)
                return false;
            stored_sources.push_back(std::move(source));
            return true;
        };
        // FileStore preserves './'. Its native tree ordering puts bad before
        // patch2 and '.mpkg' last; the last name fails factory's length>5 gate.
        if (!put("./patch2.mpkg", outer.encoded) || !put("./bad.mpkg", ascii("not an MPKG archive\n"))
            || !put(".mpkg", outer.encoded)) return false;
        bsp::PackageScanCallbacks callbacks;
        callbacks.enumerate = [&](const std::string& directory, const std::string& extension,
            std::uint32_t flags, std::vector<std::string>& names, std::string& diagnostic) {
            ++queries;
            arguments = arguments && directory == "." && extension == "mpkg" && flags == 0;
            return bsp::enumerate_resources_00bdd990_fragment(manager.context(), directory,
                extension, flags, names, diagnostic);
        };
        callbacks.already_mounted = [&](const std::string& name) {
            ++lookups;
            return static_cast<bool>(manager.find_system_name_00bdb120(name));
        };
        callbacks.mount = [&](const std::string& name, const std::string& prefix,
            std::int32_t priority, std::uint8_t ownership, std::int32_t device,
            std::string& diagnostic) {
            ++requests;
            arguments = arguments && prefix == "." && priority == 1000 && ownership == 0 && device == -1;
            const auto count = manager.registrations().size();
            auto provider = store_identity; // Decline/failure must preserve the caller's output.
            const auto result = manager.mount_system_path_00be1890_fragment(name, prefix,
                priority, ownership, device, provider, diagnostic);
            if (result == bsp::VfsProviderCreateStatus::created) {
                publication = publication && manager.registrations().size() == count + 1
                    && provider && provider->kind == bsp::VfsProviderKind::mpkg
                    && provider->system_name == name && provider->device_id == -1;
                return bsp::PackageMountStatus::mounted;
            }
            publication = publication && manager.registrations().size() == count && provider == store_identity;
            return result == bsp::VfsProviderCreateStatus::declined
                ? bsp::PackageMountStatus::declined : bsp::PackageMountStatus::failed;
        };
        const bool scan_success = bsp::startup_scan_packages_0073d881_fragment(callbacks, reports);
        const auto entry_is = [](const bsp::PackageScanEntry& entry, const char* name,
            bsp::PackageScanDisposition disposition) {
            return entry.system_name == name && entry.priority == 1000 && entry.disposition == disposition;
        };
        using Disposition = bsp::PackageScanDisposition;
        checked = !scan_success && queries == 2 && lookups == 7 && requests == 6
            && arguments && publication && reports[0].enumerated && reports[1].enumerated
            && reports[0].entries.size() == 3 && reports[1].entries.size() == 4
            && entry_is(reports[0].entries[0], "./bad.mpkg", Disposition::failed)
            && entry_is(reports[0].entries[1], "./patch2.mpkg", Disposition::mounted)
            && entry_is(reports[0].entries[2], ".mpkg", Disposition::declined)
            && entry_is(reports[1].entries[0], "./inner.mpkg", Disposition::mounted)
            && entry_is(reports[1].entries[1], "./bad.mpkg", Disposition::failed)
            && entry_is(reports[1].entries[2], "./patch2.mpkg", Disposition::already_mounted)
            && entry_is(reports[1].entries[3], ".mpkg", Disposition::declined)
            && !reports[0].error.empty() && !reports[1].error.empty()
            && manager.registrations().size() == 3 && store_identity->device_id == 7;
        const auto mounted_outer = manager.find_system_name_00bdb120("./PATCH2.MPKG");
        const auto mounted_inner = manager.find_system_name_00bdb120("./inner.mpkg");
        checked = checked && mounted_outer && mounted_outer->system_name == "./patch2.mpkg"
            && mounted_inner && mounted_inner->system_name == "./inner.mpkg"
            && !manager.find_system_name_00bdb120("inner.mpkg");
        for (const auto& source : stored_sources) checked = checked && source->position_00bef580() == 7;
        auto opened = bsp::open_resource_memory_00bdf310_fragment(manager.context(), "marker.txt", 2);
        checked = checked && opened.provider_opened && opened.stream;
        marker = std::move(opened.stream);
        if (!opened.error.empty()) error = opened.error;
        const auto outcome_name = [](Disposition disposition) {
            switch (disposition) {
            case Disposition::already_mounted: return "already_mounted";
            case Disposition::mounted: return "mounted";
            case Disposition::declined: return "declined";
            case Disposition::failed: return "failed";
            default: return "unsupported";
            }
        };
        for (std::size_t pass = 0; pass < reports.size(); ++pass)
            for (const auto& entry : reports[pass].entries)
                std::printf("Package scan entry: pass=%zu name=%s prefix=. priority=%d ownership=0 device=-1 outcome=%s error=%s\n",
                    pass + 1, entry.system_name.c_str(), entry.priority,
                    outcome_name(entry.disposition), entry.error.c_str());
    }
    checked = checked && marker && matches_owned_output(*marker, marker_bytes);
    std::printf("Synthetic package startup: outer_bytes=%zu inner_bytes=%zu scans=%u lookups=%u mount_requests=%u first_entries=%zu second_entries=%zu shared_factory_and_system_identity=%d fresh_nested_scan_and_failure_continuation=%d error=%s\n",
        outer.encoded.size(), inner.encoded.size(), queries, lookups, requests,
        reports[0].entries.size(), reports[1].entries.size(), checked, checked, error.c_str());
    return checked;
}

// Extend this same archive fixture through real mounted-provider selection.
// The changed source and recursive alias distinguish a live VFS callback from
// a captured physical source or copied mount snapshot.
bool mounted_fixture(const Fixture& fixture, std::string& error) {
    auto context = std::make_shared<bsp::VfsMountContext>();
    auto store = std::make_shared<bsp::FileStore>();
    const auto put = [&](const char* name, const Bytes& bytes) {
        auto memory = std::make_shared<bsp::MemoryStream>();
        DWORD failure{};
        return bsp::memory_stream_from_bytes_00befa40_fragment(bytes.data(),
            static_cast<std::uint32_t>(bytes.size()), *memory, failure)
            && memory->seek_00bef540(7, 0)
            && store->add_file_00be7760(name, memory) == bsp::FileStoreInsertResult::inserted;
    };
    auto changed = fixture.encoded;
    const auto offset = fixture.entries[3].data_offset;
    std::fill(changed.begin() + offset, changed.begin() + offset + large_size, 0xa5);
    if (!put("fixture.mpkg", fixture.encoded) || !put("changed.mpkg", changed)) return false;
    std::vector<std::string> sentinel_names;
    if (!store->enumerate_00be6480("fixture", std::string(13, '?'), 0, sentinel_names, error)
        || sentinel_names != std::vector<std::string>{"fixture.mpkg"}) return false;
    context->mounts.push_back(bsp::bind_file_store_fragment("", store));
    std::weak_ptr<bsp::VfsMountContext> current = context;
    unsigned source_calls{};
    bool source_arguments = true;
    bsp::MpkgLogicalOpen open = [current, &source_calls, &source_arguments]
        (const std::string& name, std::uint32_t flags) {
        ++source_calls;
        source_arguments = source_arguments && name == "Fixture.MpKg" && flags == 2;
        auto live = current.lock();
        return live ? bsp::open_resource_memory_00bdf310_fragment(*live, name, flags)
            : bsp::VfsMemoryOpen{false, {}, "Current VFS expired."};
    };
    std::shared_ptr<bsp::MpkgArchive> archive;
    bool checked = bsp::create_mpkg_archive_00bb9d90_fragment("Fixture.MpKg", open,
        archive, error) == bsp::MpkgCreateStatus::loaded;
    if (!checked) return false;
    auto mounted = bsp::bind_mpkg_archive_fragment("packages", archive);
    std::uint32_t provider_flags = UINT32_MAX;
    const auto enumerate = mounted.enumerate;
    mounted.enumerate = [enumerate, &provider_flags](const std::string& directory,
        const std::string& extension, std::uint32_t flags,
        std::vector<std::string>& output, std::string& diagnostic) {
        provider_flags = flags;
        return enumerate(directory, extension, flags, output, diagnostic);
    };
    context->mounts.insert(context->mounts.begin(), std::move(mounted));
    std::vector<std::string> names{"pACKED.tXT"};
    // Enumeration does not normalize or alias; whole names keep first spelling.
    context->aliases = {{"packages/P", "ignored"}};
    checked = bsp::enumerate_resources_00bdd990_fragment(*context, "packages/P", "TXT", 1, names, error)
        && names == std::vector<std::string>{"pACKED.tXT"};
    names.clear();
    checked = checked && bsp::enumerate_resources_00bdd990_fragment(*context,
        "packages/P", "TXT", 0x101, names, error)
        && names == std::vector<std::string>{"Packed.TXT"} && provider_flags == 1;
    names.clear();
    checked = checked && bsp::enumerate_resources_00bdd990_fragment(*context,
        "packages/C", "TXT", 1, names, error) && names == std::vector<std::string>{"Case.TXT"};
    names.clear();
    checked = checked && archive->enumerate_00bb97b0_fragment("P", "TXT", 0x100, names, error)
        && names.empty(); // Low flag byte zero rejects a name with no slash.
    context->aliases = {{"fixture.mpkg", "changed.mpkg"}};
    auto opened = bsp::open_resource_memory_00bdf310_fragment(*context, "packages/Large.BIN", 0x32);
    checked = checked && opened.provider_opened && opened.stream
        && matches_owned_output(*opened.stream, Bytes(large_size, 0xa5));
    if (!opened.error.empty()) error = opened.error;
    // Source aliases back to this archive's large entry: guard must return a
    // diagnostic and unwind, then a subsequent legitimate open must succeed.
    context->aliases = {{"fixture.mpkg", "packages/large.bin"}};
    auto recursive = bsp::open_resource_memory_00bdf310_fragment(*context, "packages/large.bin", 2);
    const bool recursion_rejected = recursive.provider_opened && !recursive.stream
        && recursive.error.find("recursive") != std::string::npos;
    checked = checked && recursion_rejected;
    context->aliases.clear();
    auto original = bsp::open_resource_memory_00bdf310_fragment(*context, "packages/large.bin", 2);
    checked = checked && original.provider_opened && original.stream
        && matches_owned_output(*original.stream, Bytes(fixture.encoded.begin() + offset,
            fixture.encoded.begin() + offset + large_size))
        && source_calls == 4 && source_arguments;
    context.reset();
    auto expired = bsp::bind_mpkg_archive_fragment("", archive).open_read_only("large.bin", 2);
    checked = checked && expired.provider_opened && !expired.stream
        && expired.error == "Current VFS expired.";
    std::printf("Mounted MPKG: source_calls=%u live_alias_reopen_and_cursor=%d enumeration_alias_flag_and_first_spelling=%d recursive_source_rejected=%d expired_owner_error=%d error=%s\n",
        source_calls, checked, checked, recursion_rejected,
        expired.error == "Current VFS expired.", error.c_str());
    return checked;
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
        if (!package_scan_fixture(error)) return false;
        if (!mounted_fixture(fixture, error)) return false;
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
                [original, trace, &reopened_lifetime](std::string&) -> std::shared_ptr<bsp::InflateSource> {
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
