#include "bsp/mpkg_archive.hpp"
#include "bsp/resource_enumeration.hpp"
#include <algorithm>
#include <array>
#include <cstring>
#include <limits>
#include <iterator>
#include <utility>
#include <vector>

namespace bsp {
namespace {
// Original PE00e144f0,537 bytes; SHA256
//26abd3e2995fdbacecb6f352b5c0eb20ff20c9a7ada6312ba619c9d4c6fa6183.
constexpr std::array<std::uint8_t, 537> transform_key = {
    0x6c, 0x8b, 0x6f, 0x18, 0xb5, 0xa7, 0x23, 0xdd, 0x6a, 0xda, 0xa1, 0x9e, 0x74, 0xe6, 0x07, 0xbc,
    0xac, 0x10, 0x6d, 0xa1, 0x46, 0x7c, 0x64, 0x75, 0x60, 0x14, 0x97, 0x2f, 0x76, 0x3d, 0xcc, 0x01,
    0x19, 0xb4, 0xc6, 0x27, 0xab, 0x26, 0x67, 0xeb, 0x03, 0x5e, 0x7b, 0x6b, 0x0f, 0x49, 0x16, 0x8e,
    0xfd, 0x95, 0xd6, 0x18, 0x7a, 0x6b, 0xd3, 0x65, 0x0b, 0x65, 0x5c, 0xe2, 0xd0, 0x4c, 0x07, 0xd0,
    0xc4, 0x49, 0x32, 0x5b, 0x4c, 0xb0, 0x46, 0x1f, 0x14, 0x41, 0xd1, 0x17, 0xf2, 0xcc, 0x1c, 0x09,
    0xc4, 0xea, 0xec, 0x7c, 0x75, 0xef, 0x5a, 0x1a, 0x31, 0x9e, 0x3b, 0x6f, 0x79, 0x2e, 0x17, 0x12,
    0xb5, 0x25, 0x82, 0xcf, 0xbf, 0x5d, 0xc5, 0x30, 0x8e, 0x7b, 0x07, 0xf9, 0xdc, 0x74, 0x45, 0xcf,
    0xd9, 0xf4, 0xc7, 0xaf, 0x60, 0xd3, 0x35, 0xf0, 0xd5, 0xfc, 0xb0, 0xea, 0x34, 0x5e, 0x3d, 0x41,
    0x7f, 0x0f, 0xd5, 0xd5, 0x62, 0xf7, 0x48, 0x5e, 0x92, 0x0c, 0x0f, 0x84, 0x6d, 0x65, 0x6a, 0x01,
    0x23, 0x53, 0x9c, 0x13, 0x6d, 0x83, 0xbf, 0x23, 0x60, 0x55, 0x65, 0xce, 0x67, 0xfa, 0x62, 0x28,
    0x4f, 0xee, 0x21, 0x68, 0xd8, 0x27, 0x69, 0x37, 0xd5, 0x5b, 0xd5, 0xc5, 0xc7, 0x6b, 0x2d, 0x6b,
    0x36, 0xd7, 0x7a, 0x70, 0x97, 0x5a, 0x99, 0xfe, 0xe8, 0xea, 0x3a, 0xc7, 0x40, 0xa9, 0x07, 0x21,
    0x4f, 0x36, 0x8c, 0x21, 0x0b, 0xe3, 0x0b, 0xc2, 0x27, 0xd1, 0x8d, 0xfc, 0x1a, 0x95, 0x76, 0x74,
    0xc7, 0xc4, 0x2a, 0xaf, 0xbe, 0xdc, 0xea, 0xb9, 0xe0, 0x6f, 0xcd, 0xb9, 0xb6, 0xc1, 0xbc, 0x44,
    0xc9, 0xca, 0xbb, 0x53, 0xab, 0x74, 0xe4, 0x9f, 0xc4, 0x51, 0x63, 0x77, 0x85, 0xb8, 0x09, 0x9f,
    0xbe, 0xf8, 0xf6, 0x09, 0x6d, 0x9a, 0xeb, 0x12, 0xb3, 0x88, 0x95, 0xdd, 0xe0, 0x6d, 0x7d, 0xdc,
    0x24, 0x55, 0xc1, 0x40, 0xc0, 0x76, 0x7d, 0x77, 0x65, 0x29, 0xce, 0x21, 0xc5, 0xb0, 0x25, 0xa4,
    0x30, 0x02, 0xe0, 0x2b, 0x20, 0x17, 0x30, 0x62, 0xf9, 0x0c, 0xc8, 0x72, 0x9a, 0xa7, 0x29, 0xfd,
    0xf2, 0xee, 0xcb, 0xb9, 0x74, 0x38, 0xbb, 0x3e, 0x30, 0xe9, 0xf3, 0x57, 0x4f, 0xab, 0x73, 0x8d,
    0x35, 0xd2, 0x78, 0x58, 0x3f, 0x5d, 0x6e, 0x2b, 0x22, 0xc3, 0xf6, 0x50, 0x41, 0xc0, 0x12, 0xfd,
    0xb5, 0x8e, 0x64, 0x48, 0xbb, 0x5f, 0x93, 0xfa, 0x5e, 0xfd, 0xaa, 0x15, 0xdb, 0x83, 0x6d, 0x0a,
    0x6a, 0x3e, 0xa0, 0xfd, 0x7f, 0x40, 0x74, 0x43, 0x53, 0xaa, 0xda, 0x47, 0xbf, 0x5b, 0x31, 0xd7,
    0xe0, 0x7b, 0x33, 0x0c, 0xb5, 0xb2, 0x09, 0x7d, 0x64, 0x2f, 0x6c, 0x20, 0xcf, 0x4c, 0x0b, 0x60,
    0x90, 0xa2, 0xec, 0xe3, 0xc6, 0x01, 0xe0, 0xf8, 0xa0, 0x15, 0x43, 0x67, 0xe4, 0xe3, 0x63, 0xd7,
    0xe5, 0xca, 0x93, 0x13, 0x60, 0xf4, 0xdf, 0x99, 0xf0, 0x38, 0xda, 0xc0, 0x70, 0x63, 0x2d, 0x2f,
    0x1a, 0xb0, 0x2f, 0x8d, 0xc9, 0xb6, 0x32, 0xc0, 0xc4, 0x54, 0x33, 0x5d, 0xfe, 0x80, 0xe6, 0xd1,
    0x5f, 0x2b, 0xd7, 0x98, 0x93, 0x0d, 0x6c, 0x89, 0x07, 0xb5, 0xbb, 0xb7, 0x49, 0x37, 0x44, 0xcd,
    0x7d, 0x26, 0x99, 0xa9, 0x0e, 0x5d, 0xe3, 0x5a, 0x4c, 0x16, 0x44, 0x94, 0x80, 0x12, 0xbd, 0xbb,
    0x10, 0x85, 0x0b, 0xca, 0x0b, 0xd5, 0x01, 0x63, 0xa6, 0xee, 0x82, 0x9f, 0x24, 0x1c, 0x33, 0x9e,
    0x0d, 0x71, 0x80, 0xac, 0x44, 0x78, 0x43, 0xfd, 0xba, 0x15, 0x98, 0xe0, 0xca, 0xd0, 0x16, 0xa9,
    0x1d, 0x2b, 0x90, 0xcb, 0x15, 0x50, 0x42, 0x2f, 0xe8, 0x09, 0xfa, 0xd4, 0xf5, 0x99, 0xc2, 0x91,
    0x35, 0x46, 0x5a, 0xc1, 0x2a, 0x6a, 0x5f, 0xa9, 0x4f, 0x5e, 0xbc, 0x48, 0xa8, 0xec, 0x98, 0x79,
    0x79, 0x93, 0xc8, 0x75, 0xf3, 0x66, 0xa2, 0x08, 0x33, 0x97, 0x54, 0x02, 0xfd, 0x7d, 0xea, 0x8d,
    0xf7, 0x1b, 0x25, 0x05, 0x3c, 0xdb, 0x11, 0x4e, 0x51,
};

std::uint16_t u16(const std::uint8_t* bytes) noexcept {
    return static_cast<std::uint16_t>(static_cast<std::uint16_t>(bytes[0]) |
        (static_cast<std::uint16_t>(bytes[1]) << 8));
}

std::uint32_t u32(const std::uint8_t* bytes) noexcept {
    return static_cast<std::uint32_t>(bytes[0]) |
        (static_cast<std::uint32_t>(bytes[1]) << 8) |
        (static_cast<std::uint32_t>(bytes[2]) << 16) |
        (static_cast<std::uint32_t>(bytes[3]) << 24);
}

bool extent(std::size_t offset, std::size_t count, std::size_t length) noexcept {
    return offset <= length && count <= length - offset;
}

class DecodedSource final : public InflateSource {
public:
    explicit DecodedSource(const MemoryStream& archive) noexcept
        : cursor_(archive.clone_reset_00bef6d0()) {}
    bool seek_absolute(std::uint32_t offset) noexcept override {
        return cursor_.seek_00bef540(offset, 0);
    }
    bool read(void* destination, std::uint32_t requested,
        std::uint32_t& actual) noexcept override {
        actual = 0;
        return cursor_.read_00bef590(destination, requested, &actual);
    }
private:
    MemoryStream cursor_;
};

bool copy_result(const void* data, std::uint32_t size,
    MemoryStream& output, std::string& error) {
    DWORD copy_error = ERROR_SUCCESS;
    if (memory_stream_from_bytes_00befa40_fragment(data, size, output, copy_error))
        return true;
    error = "MPKG memory copy failed with error " + std::to_string(copy_error) + ".";
    return false;
}

class SourceMaterializationGuard {
public:
    explicit SourceMaterializationGuard(bool* active) noexcept : active_(active) {
        if (active_) *active_ = true;
    }
    ~SourceMaterializationGuard() { if (active_) *active_ = false; }
    SourceMaterializationGuard(const SourceMaterializationGuard&) = delete;
    SourceMaterializationGuard& operator=(const SourceMaterializationGuard&) = delete;
private:
    bool* active_;
};
}

struct MpkgArchive::Impl {
    struct Entry {
        std::string name;
        std::uint32_t local_offset{};
        std::uint32_t data_offset{};
        std::uint32_t compressed_size{};
        std::uint32_t decoded_size{};
        std::uint32_t crc_value{}; // Retained native field; never verified here.
        std::uint16_t method{};
        bool offset_resolved{};
    };

    const Entry* find(std::string_view name) const noexcept {
        for (const auto& entry : entries) {
            if (entry.name.size() == name.size() &&
                (name.empty() || _strnicmp(name.data(), entry.name.c_str(), name.size()) == 0))
                return &entry;
        }
        return nullptr;
    }

    bool parse(std::string& error) {
        const auto* bytes = decoded.data_00bef610();
        const std::uint32_t tail = (std::min)(length, 0xffffu);
        bool found = false;
        //00bb87a0 excludes tail index0 and does not scan a four-byte tail.
        for (std::uint32_t distance = 4; distance < tail; ++distance) {
            const std::uint32_t offset = length - distance;
            if (bytes[offset] == 0x50 && bytes[offset + 1] == 0x4b &&
                bytes[offset + 2] == 5 && bytes[offset + 3] == 6) {
                end_offset = offset;
                found = true;
                break;
            }
        }
        if (!found) {
            error = "MPKG end marker was not found in the native scan interval.";
            return false;
        }
        if (!extent(end_offset, 22, length)) {
            error = "MPKG end record is incomplete.";
            return false;
        }
        const std::uint32_t count = u16(bytes + end_offset + 10);
        directory_size = u32(bytes + end_offset + 12);
        directory_offset = u32(bytes + end_offset + 16);
        prefix = end_offset - directory_offset - directory_size;
        if (directory_offset > length) {
            error = "MPKG directory offset exceeds transformed backing.";
            return false;
        }
        //00bb9700 starts directly at directory_offset, WITHOUT prefix.
        std::size_t cursor = directory_offset;
        entries.reserve(count);
        for (std::uint32_t index = 0; index < count; ++index) {
            if (!extent(cursor, 46, length)) {
                error = "MPKG fixed directory record is incomplete.";
                return false;
            }
            const auto* header = bytes + cursor;
            const std::uint32_t name_length = u16(header + 28);
            const std::uint32_t extra_length = u16(header + 30);
            const std::uint32_t comment_length = u16(header + 32);
            Entry entry;
            entry.method = u16(header + 10);
            entry.crc_value = u32(header + 16);
            entry.compressed_size = u32(header + 20);
            entry.decoded_size = u32(header + 24);
            entry.local_offset = prefix + u32(header + 42);
            entry.data_offset = entry.local_offset + 30u + name_length + extra_length;
            cursor += 46;
            if (!extent(cursor, name_length + extra_length + comment_length, length)) {
                error = "MPKG directory name, extra bytes or comment exceed transformed backing.";
                return false;
            }
            entry.name.assign(reinterpret_cast<const char*>(bytes + cursor), name_length);
            cursor += name_length + extra_length + comment_length;
            entries.push_back(std::move(entry));
        }
        return true;
    }

    MemoryStream decoded;
    MpkgReopenSource reopen_original;
    std::vector<Entry> entries;
    std::uint32_t length{};
    std::uint32_t end_offset{};
    std::uint32_t directory_size{};
    std::uint32_t directory_offset{};
    std::uint32_t prefix{};
    bool materializing_original_source{};
};

MpkgArchive::MpkgArchive() noexcept = default;
MpkgArchive::~MpkgArchive() = default;
MpkgArchive::MpkgArchive(MpkgArchive&&) noexcept = default;
MpkgArchive& MpkgArchive::operator=(MpkgArchive&&) noexcept = default;

bool MpkgArchive::load_00bb9920_fragment(const MemoryStream& encoded,
    MpkgReopenSource reopen_original, std::string& error) {
    error.clear();
    if (impl_) {
        error = "MPKG archive is already loaded.";
        return false;
    }
    const std::int64_t encoded_size = encoded.size_00bef600();
    if (!encoded.has_backing() || !encoded.fully_initialized() || encoded_size <= 0 ||
        encoded_size > (std::numeric_limits<std::int32_t>::max)()) {
        error = "MPKG requires fully initialized encoded backing of 1..INT32_MAX bytes.";
        return false;
    }
    const auto size = static_cast<std::uint32_t>(encoded_size);
    std::vector<std::uint8_t> transformed(size);
    const auto* original = encoded.data_00bef610();
    const std::uint32_t final_block = size / 753u;
    for (std::uint32_t index = 0; index < size; ++index) {
        const std::uint32_t block = index / 753u;
        const std::uint32_t block_length = block == final_block ? size % 753u : 753u;
        //00bb9a49..00bb9a57: GLOBAL index modulo block_length, also for tail.
        const std::uint32_t source = block * 753u + block_length - 1u - index % block_length;
        transformed[index] = static_cast<std::uint8_t>(
            original[source] ^ transform_key[source % 537u]);
    }
    auto candidate = std::make_unique<Impl>();
    candidate->length = size;
    candidate->reopen_original = std::move(reopen_original);
    if (!copy_result(transformed.data(), size, candidate->decoded, error) || !candidate->parse(error))
        return false;
    impl_ = std::move(candidate);
    return true;
}

std::size_t MpkgArchive::entry_count() const noexcept {
    return impl_ ? impl_->entries.size() : 0;
}

bool MpkgArchive::contains_00bb8e00(std::string_view name) const noexcept {
    return impl_ && impl_->find(name) != nullptr;
}

bool MpkgArchive::enumerate_00bb97b0_fragment(std::string_view directory,
    std::string_view extension, std::uint32_t flags,
    std::vector<std::string>& output, std::string& error) const {
    error.clear();
    if (!impl_) {
        error = "MPKG archive is not loaded.";
        return false;
    }
    // Validate the query even when there are no directory records.
    if (resource_enumeration_match_00bee340_fragment(directory, extension, flags, {}) ==
        ResourceEnumerationMatch::unsupported) {
        error = "MPKG enumeration requires a nonempty directory and bounded NUL-free strings.";
        return false;
    }
    std::vector<std::string> selected;
    for (const auto& entry : impl_->entries) {
        const auto match = resource_enumeration_match_00bee340_fragment(
            directory, extension, flags, entry.name);
        if (match == ResourceEnumerationMatch::unsupported) {
            error = "MPKG enumeration encountered an unsupported stored name.";
            return false;
        }
        if (match == ResourceEnumerationMatch::match) selected.push_back(entry.name);
    }
    const auto maximum_count = static_cast<std::size_t>(
        (std::numeric_limits<std::int32_t>::max)());
    if (output.size() > maximum_count || selected.size() > maximum_count - output.size()) {
        error = "MPKG enumeration output exceeds the supported count.";
        return false;
    }
    output.insert(output.end(), std::make_move_iterator(selected.begin()),
        std::make_move_iterator(selected.end()));
    return true;
}

bool MpkgArchive::plan_entry_00bb8d60_fragment(std::string_view name,
    std::uint32_t flags, MpkgEntryReadPlan& output, std::string& error) {
    error.clear();
    if (!impl_) {
        error = "MPKG archive is not loaded.";
        return false;
    }
    if ((flags & 1u) != 0) {
        error = "MPKG entry open rejects flags bit0.";
        return false;
    }
    const auto* found = impl_->find(name);
    if (!found) {
        error = "MPKG entry was not found.";
        return false;
    }
    auto& entry = impl_->entries[static_cast<std::size_t>(found - impl_->entries.data())];
    if (!entry.offset_resolved) {
        if (!extent(entry.local_offset, 30, impl_->length)) {
            error = "MPKG local header exceeds transformed backing.";
            return false;
        }
        const auto* header = impl_->decoded.data_00bef610() + entry.local_offset;
        const std::size_t data_offset = static_cast<std::size_t>(entry.local_offset) + 30u;
        const std::size_t variable_length = static_cast<std::size_t>(u16(header + 26)) + u16(header + 28);
        if (!extent(data_offset, variable_length, impl_->length)) {
            error = "MPKG local name or extra bytes exceed transformed backing.";
            return false;
        }
        entry.data_offset = static_cast<std::uint32_t>(data_offset + variable_length);
        entry.offset_resolved = true;
    }
    MpkgEntryReadPlan candidate;
    candidate.source_offset = entry.data_offset;
    candidate.compressed_size = entry.compressed_size;
    candidate.decoded_size = entry.decoded_size;
    candidate.method = entry.method;
    if (entry.method != 0) {
        candidate.route = MpkgEntryRoute::raw_inflate_from_decoded;
        if (!extent(entry.data_offset, entry.compressed_size, impl_->length)) {
            error = "MPKG compressed extent exceeds transformed backing.";
            return false;
        }
    } else if (entry.decoded_size <= 0x40000u) {
        candidate.route = MpkgEntryRoute::copy_from_decoded;
        if (!extent(entry.data_offset, entry.decoded_size, impl_->length)) {
            error = "MPKG stored extent exceeds transformed backing.";
            return false;
        }
    } else {
        candidate.route = MpkgEntryRoute::reopen_original_range;
    }
    output = candidate;
    return true;
}

bool MpkgArchive::open_entry_00bb8d60_fragment(std::string_view name,
    std::uint32_t flags, MemoryStream& output, std::string& error) {
    MpkgEntryReadPlan plan;
    if (!plan_entry_00bb8d60_fragment(name, flags, plan, error)) return false;
    if (plan.decoded_size == 0 || plan.decoded_size >
        static_cast<std::uint32_t>((std::numeric_limits<std::int32_t>::max)())) {
        error = "MPKG entry materialization requires a decoded size of 1..INT32_MAX bytes.";
        return false;
    }
    if (plan.route == MpkgEntryRoute::copy_from_decoded)
        return copy_result(impl_->decoded.data_00bef610() + plan.source_offset,
            plan.decoded_size, output, error);

    const bool reopening = plan.route == MpkgEntryRoute::reopen_original_range;
    if (reopening && impl_->materializing_original_source) {
        error = "MPKG recursive original-source reopen was rejected.";
        return false;
    }
    SourceMaterializationGuard reentry_guard(
        reopening ? &impl_->materializing_original_source : nullptr);

    std::shared_ptr<InflateSource> original_source;
    if (plan.route == MpkgEntryRoute::reopen_original_range) {
        if (!impl_->reopen_original) {
            error = "MPKG large stored entry requires an original-source reopen callback.";
            return false;
        }
        original_source = impl_->reopen_original(error);
        if (!original_source) {
            if (error.empty()) error = "MPKG original-source reopen returned no source.";
            return false;
        }
        error.clear();
        if (!original_source->seek_absolute(plan.source_offset)) {
            error = "MPKG reopened source seek failed.";
            return false;
        }
    }
    std::vector<std::uint8_t> data(plan.decoded_size);
    if (plan.route == MpkgEntryRoute::raw_inflate_from_decoded) {
        InflateStream inflater;
        const auto initialized = inflater.open_00bbc1d0_fragment(
            std::make_shared<DecodedSource>(impl_->decoded),
            {plan.source_offset, plan.compressed_size, plan.decoded_size});
        if (initialized != InflateStreamStatus::ok) {
            error = "MPKG inflater initialization failed with status " +
                std::to_string(static_cast<int>(initialized)) + ".";
            return false;
        }
        std::uint32_t actual = 0;
        const auto status = inflater.read_00bbc140(data.data(), plan.decoded_size, &actual);
        if (status != InflateStreamStatus::ok || actual != plan.decoded_size) {
            error = "MPKG inflate failed after " + std::to_string(actual) +
                " bytes, status " + std::to_string(static_cast<int>(status)) +
                ", zlib " + std::to_string(inflater.decoder_status()) + ".";
            return false;
        }
    } else {
        std::uint32_t copied = 0;
        while (copied < plan.decoded_size) {
            std::uint32_t actual = 0;
            const std::uint32_t remaining = plan.decoded_size - copied;
            const bool read_ok = original_source->read(data.data() + copied, remaining, actual);
            if (actual > remaining) {
                error = "MPKG reopened source reported more bytes than requested.";
                return false;
            }
            if (!read_ok || actual == 0) {
                error = "MPKG reopened source failed or stopped after " +
                    std::to_string(copied + actual) + " bytes.";
                return false;
            }
            copied += actual;
        }
    }
    return copy_result(data.data(), plan.decoded_size, output, error);
}
}
