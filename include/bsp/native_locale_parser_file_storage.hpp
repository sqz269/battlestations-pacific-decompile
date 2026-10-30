#pragma once

#include "bsp/game_native_vfs_runtime.hpp"
#include "bsp/native_cockpit_helper_construction.hpp"

#include <cstddef>
#include <cstdint>
#include <exception>

namespace bsp {
struct NativeStringRawPoolContext;
namespace game {
class GameNativeVfsApplication;
class GameNativeLocalePoolProcess;
class GameNativeReadOnlyData;
}

class NativeLocaleParserFileFrame;

// ONE borrowed filename, not the by-value input/override vector of whole AA0020.
// The decrement role is borrowed explicitly from the EXISTING retained renderer:
// renderer.cockpit_viewport_release().decrement_iat_00ce2220. No value is read
// during preparation; this creates no renderer, pool, manager or import resolver.
class NativeLocaleParserFileContext final {
public:
    NativeLocaleParserFileContext(void* actual_locale_manager,
        const void* actual_filename8, game::GameNativeVfsApplication&,
        game::GameNativeLocalePoolProcess&, const game::GameNativeReadOnlyData&,
        NativeCockpitInterlockedDecrement const volatile& actual_decrement_cell);
    NativeLocaleParserFileContext(const NativeLocaleParserFileContext&) = delete;
    NativeLocaleParserFileContext& operator=(const NativeLocaleParserFileContext&) = delete;
    NativeLocaleParserFileContext(NativeLocaleParserFileContext&&) = delete;
    NativeLocaleParserFileContext& operator=(NativeLocaleParserFileContext&&) = delete;
private:
    friend class NativeLocaleParserFileFrame;
    friend void scan_native_locale_parser_file_00aa00b0(
        const NativeLocaleParserFileContext&, NativeLocaleParserFileFrame&);
    void* const manager_;
    const void* const filename_;
    NativeStringRawPoolContext& strings_;
    game::GameNativeVfsRawServices services_;
    game::GameNativeLocalePoolProcess& pool_;
    const game::GameNativeReadOnlyData& data_;
    NativeCockpitInterlockedDecrement const volatile& decrement_cell_;
};

enum class NativeLocaleParserFilePhase {
    prepared, entered, opened, scanning, returning_streams, returned, failed
};
struct NativeLocaleParserFileProgress {
    NativeLocaleParserFilePhase phase{NativeLocaleParserFilePhase::prepared};
    const NativeLocaleParserFileContext* context{};
    void* original_stream{};
    void* converted_stream{};
    std::size_t record_attempts{}, records_returned{};
    bool attempted{}, open_entered{}, open_returned{};
    bool conversion_entered{}, conversion_returned{};
    bool decrement_captured{};
    bool converted_decrement_entered{}, converted_decrement_returned{};
    bool converted_zero_entered{}, converted_zero_returned{};
    bool original_decrement_entered{}, original_decrement_returned{};
    bool original_zero_entered{}, original_zero_returned{};
    bool normal_returned{}, failed{};
};

// Retain this entire nonmoving attempt, all its record nodes and every borrowed
// context/provider/code/renderer owner after any escape. The destructor deletes
// C++ bookkeeping only: NO native cleanup, retry or settlement proof.
class NativeLocaleParserFileFrame final {
public:
    explicit NativeLocaleParserFileFrame(const NativeLocaleParserFileContext&) noexcept;
    ~NativeLocaleParserFileFrame() noexcept;
    NativeLocaleParserFileFrame(const NativeLocaleParserFileFrame&) = delete;
    NativeLocaleParserFileFrame& operator=(const NativeLocaleParserFileFrame&) = delete;
    NativeLocaleParserFileFrame(NativeLocaleParserFileFrame&&) = delete;
    NativeLocaleParserFileFrame& operator=(NativeLocaleParserFileFrame&&) = delete;
    NativeLocaleParserFileProgress progress() const noexcept { return progress_; }
    bool normal_returned() const noexcept {
        return progress_.phase == NativeLocaleParserFilePhase::returned && !progress_.failed;
    }
private:
    friend void scan_native_locale_parser_file_00aa00b0(
        const NativeLocaleParserFileContext&, NativeLocaleParserFileFrame&);
    struct RecordAttempt;
    const NativeLocaleParserFileContext& context_;
    // Prologue4F88 + four pushed registers: only evidenced native buffer offsets
    // are used. This is uninitialized scratch, not a malformed-input length cap.
    alignas(std::uint32_t) unsigned char scratch_[0x4f88u + 0x10u];
    std::uint32_t count_, mode_, index_; // live DWORDs; initialized at native epochs
    void* original_{};
    void* converted_{};
    RecordAttempt* records_{};
    RecordAttempt* last_{};
    NativeLocaleParserFileProgress progress_{};
    std::exception_ptr failure_{};
    void require_entered() const;
    void fail(std::exception_ptr) noexcept;
};

// Reconstructs ONLY the per-file fragment [AA00B0,AA0356), composing the existing
// AA0130 record helper. Actual slot24/40/30/20 order, short-read tails and the
// normal converted-then-original return are preserved. No selected stream
// cleanup exists; no EOF repair, whole-parser/loaded result, native ABI/FH3/SEH,
// lower-provider fencing or runtime equivalence is supplied by this interface.
void scan_native_locale_parser_file_00aa00b0(
    const NativeLocaleParserFileContext&, NativeLocaleParserFileFrame&);
} // namespace bsp
