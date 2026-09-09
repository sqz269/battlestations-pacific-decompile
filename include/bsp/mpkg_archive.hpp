#pragma once
#include "bsp/inflate_stream.hpp"
#include "bsp/memory_stream.hpp"
#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <string_view>

namespace bsp {
// Return a newly owned source for the ORIGINAL logical archive path. This is
// used only for method-zero entries larger than 0x40000, never as a decoded
// backing substitute. The returned source must have an independent cursor.
using MpkgReopenSource = std::function<std::shared_ptr<InflateSource>()>;

enum class MpkgEntryRoute {
    raw_inflate_from_decoded,
    copy_from_decoded,
    reopen_original_range
};

struct MpkgEntryReadPlan {
    MpkgEntryRoute route{};
    std::uint32_t source_offset{};
    std::uint32_t compressed_size{};
    std::uint32_t decoded_size{};
    std::uint16_t method{};
};

// Owning bounded projection of 00bb9920/00bb9700/00bb8a90/00bb8d60.
// Evidence and native quirks: docs/MPKG_ENTRY_LOADING.md. Not native ABI or
// general ZIP validation. Returned entry streams own independent copied data.
class MpkgArchive {
public:
    MpkgArchive() noexcept;
    ~MpkgArchive();
    MpkgArchive(MpkgArchive&&) noexcept;
    MpkgArchive& operator=(MpkgArchive&&) noexcept;
    MpkgArchive(const MpkgArchive&) = delete;
    MpkgArchive& operator=(const MpkgArchive&) = delete;

    // Uses the entire fully initialized encoded backing, preserving its cursor.
    // Empty or >INT32_MAX input is unsupported. Failure leaves this archive
    // unopened; an open archive cannot be reopened. Allocation/callback
    // exceptions propagate; guarded format/source failures return false/error.
    bool load_00bb9920_fragment(const MemoryStream& encoded,
        MpkgReopenSource reopen_original, std::string& error);

    std::size_t entry_count() const noexcept;
    bool contains_00bb8e00(std::string_view name) const noexcept;

    // First equal stored length + case-insensitive name wins; bit0 rejects.
    // Resolves local name/extra lengths lazily. The plan is a scalar snapshot;
    // the archive retains its backing and the original-source callback.
    bool plan_entry_00bb8d60_fragment(std::string_view name, std::uint32_t flags,
        MpkgEntryReadPlan& output, std::string& error);

    // Buffers the selected route into independent MemoryStream backing using
    // the real copy helper00befa40. Large stored entries require reopen_original.
    // Host bounds/short-read/inflate guards return false and preserve output.
    // Zero decoded size is explicitly unsupported by the current copy helper.
    bool open_entry_00bb8d60_fragment(std::string_view name, std::uint32_t flags,
        MemoryStream& output, std::string& error);

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};
}
