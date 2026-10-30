#include "bsp/native_locale_parser_file_storage.hpp"

#include "bsp/game_native_vfs_application.hpp"
#include "bsp/native_filestore_open.hpp"
#include "bsp/native_locale_parser_record_storage.hpp"
#include "bsp/native_string_pool_storage.hpp"
#include "bsp/native_vfs_runtime_bindings.hpp"

#include <cstring>
#include <intrin.h>
#include <memory>
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native locale parser file storage requires MSVC Win32.
#endif

namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == 4 && sizeof(long) == 4 && sizeof(Word) == 4);
Word address(const volatile void* value) noexcept {
    return static_cast<Word>(reinterpret_cast<std::uintptr_t>(value));
}
void* pointer(Word value) noexcept { return reinterpret_cast<void*>(value); }
template<class T> __declspec(noinline) T read(Word location) noexcept {
    T value;
    _ReadWriteBarrier();
    std::memcpy(&value, pointer(location), sizeof value);
    _ReadWriteBarrier();
    return value;
}
template<class T> __declspec(noinline) void store(Word location, T value) noexcept {
    _ReadWriteBarrier();
    std::memcpy(pointer(location), &value, sizeof value);
    _ReadWriteBarrier();
}
Word current_entry(void* stream, Word offset) noexcept {
    const Word table = read<Word>(address(stream));
    return read<Word>(table + offset);
}
} // namespace

struct NativeLocaleParserFileFrame::RecordAttempt {
    NativeLocaleParserRecordContext context;
    NativeLocaleParserRecordFrame frame;
    RecordAttempt* next{};
    RecordAttempt(const NativeLocaleParserFileContext& c,
        NativeLocaleParserFileFrame& owner, void* delimiter)
        : context(c.manager_, &owner.mode_, &owner.index_, owner.scratch_ + 0x48u,
            owner.scratch_ + 0xe0u, owner.scratch_ + 0x178u, delimiter,
            c.strings_, c.pool_, c.data_), frame(context) {}
};

NativeLocaleParserFileContext::NativeLocaleParserFileContext(void* manager,
    const void* filename, game::GameNativeVfsApplication& app,
    game::GameNativeLocalePoolProcess& pool, const game::GameNativeReadOnlyData& data,
    NativeCockpitInterlockedDecrement const volatile& decrement)
    : manager_(manager), filename_(filename), strings_(app.raw_strings()),
      services_(app.borrow_raw_services()), pool_(pool), data_(data),
      decrement_cell_(decrement) {
    if (!services_.strings.uses_raw_pool_cells(strings_))
        throw std::invalid_argument("locale file scanner requires the application's SAME raw pool cells");
    // Genuine SAME data/canonical pool/decrement-cell provenance and lifetime
    // are caller contracts. In particular do not read the import value here.
}

NativeLocaleParserFileFrame::NativeLocaleParserFileFrame(
    const NativeLocaleParserFileContext& context) noexcept : context_(context) {
    progress_.context = &context;
}
NativeLocaleParserFileFrame::~NativeLocaleParserFileFrame() noexcept {
    // Iterative C++ bookkeeping disposal, not native failure recovery. The
    // caller must not discard an unresolved attempt or its borrowed owners.
    while (records_) {
        auto* const current = records_;
        records_ = current->next;
        delete current;
    }
}
void NativeLocaleParserFileFrame::require_entered() const {
    if (!progress_.attempted || progress_.failed || progress_.normal_returned ||
        progress_.phase == NativeLocaleParserFilePhase::prepared ||
        progress_.phase == NativeLocaleParserFilePhase::returned ||
        progress_.phase == NativeLocaleParserFilePhase::failed)
        throw std::logic_error("locale file scanner attempt is not healthy and active");
}
void NativeLocaleParserFileFrame::fail(std::exception_ptr failure) noexcept {
    progress_.failed = true;
    progress_.phase = NativeLocaleParserFilePhase::failed;
    if (!failure_) failure_ = failure;
}

void scan_native_locale_parser_file_00aa00b0(
    const NativeLocaleParserFileContext& c, NativeLocaleParserFileFrame& f) {
    auto& p = f.progress_;
    try {
        if (&f.context_ != &c || p.attempted || p.failed ||
            p.phase != NativeLocaleParserFilePhase::prepared)
            throw std::logic_error("locale file scanner requires its exact fresh one-shot frame");
        p.attempted = true;
        p.phase = NativeLocaleParserFilePhase::entered;
        if (!c.manager_ || !c.filename_ ||
            !c.services_.strings.uses_raw_pool_cells(c.strings_))
            throw std::invalid_argument("locale file scanner requires genuine fixed backing and SAME cells");
        f.require_entered();

        // AA00B0..D0: count/mode/index are LIVE actual DWORD objects. No token
        // or WORD-buffer initialization is performed by the Source frame.
        void* const vfs = c.services_.actual_vfs_publication_0109ceec;
        f.count_ = 0u;
        const Word open = current_entry(vfs, 4u);
        f.mode_ = 1u;
        f.index_ = 0u;
        p.open_entered = true;
        f.original_ = c.services_.bindings.open_manager_entry(open, vfs, c.filename_, 2u);
        p.original_stream = f.original_;
        p.open_returned = true;
        f.require_entered();
        p.phase = NativeLocaleParserFilePhase::opened;
        p.conversion_entered = true;
        f.converted_ = convert_native_stored_stream_00bef750(f.original_, c.services_.conversion);
        p.converted_stream = f.converted_;
        p.conversion_returned = true;
        f.require_entered();
        p.phase = NativeLocaleParserFilePhase::scanning;
        auto& calls = c.services_.bindings;

        for (;;) {
            f.require_entered();
            const Word mode = f.mode_;
            if (mode == 1u) {
                const Word slot = address(f.scratch_) + 0x48u + f.index_;
                const Word entry = current_entry(f.converted_, 0x24u);
                calls.source_read(entry, f.converted_, pointer(slot), 1u, &f.count_);
                f.require_entered();
                const auto length = calls.stream_length_entry(
                    current_entry(f.converted_, 0x30u), f.converted_);
                f.require_entered();
                const auto position = calls.stream_position_entry(
                    current_entry(f.converted_, 0x20u), f.converted_);
                f.require_entered();
                if (position >= length) break; // unsigned64 BEFORE space test
                if (read<std::uint8_t>(slot) == 0x20u) {
                    f.mode_ = 2u;
                    store(slot, std::uint8_t{0});
                    f.index_ = 0u;
                } else {
                    f.index_ += 1u;
                }
            } else if (mode == 2u) {
                const Word slot = address(f.scratch_) + 0xe0u + f.index_;
                const Word entry = current_entry(f.converted_, 0x24u);
                calls.source_read(entry, f.converted_, pointer(slot), 1u, &f.count_);
                f.require_entered();
                if (read<std::uint8_t>(slot) == 0x20u) {
                    f.mode_ = 3u;
                    store(slot, std::uint8_t{0});
                    f.index_ = 0u;
                } else {
                    f.index_ += 1u;
                }
            } else if (mode == 3u) {
                const Word entry = current_entry(f.converted_, 0x40u);
                const auto unit = calls.stream_word_entry(entry, f.converted_, &f.count_);
                f.require_entered();
                // Native computes this address AFTER the word call, from the
                // CURRENT index; unlike byte modes it was not captured before.
                const Word slot = address(f.scratch_) + 0x178u + f.index_ * 2u;
                store(slot, unit);
                if (unit != 0x0d00u || read<std::uint16_t>(slot - 2u) != 0x0a00u) {
                    f.index_ += 1u;
                    continue;
                }
                // Pure Source bookkeeping; no finite record limit. Retain each
                // node BEFORE invoking the real completed-record effects.
                std::unique_ptr<NativeLocaleParserFileFrame::RecordAttempt> node(
                    new NativeLocaleParserFileFrame::RecordAttempt(c, f, pointer(slot)));
                auto* const attempt = node.get();
                if (f.last_) f.last_->next = attempt;
                else f.records_ = attempt;
                f.last_ = node.release();
                ++p.record_attempts;
                f.require_entered();
                (void)store_native_locale_parser_record_00aa0130(attempt->context, attempt->frame);
                f.require_entered(); // OUTSIDE record's map/wide selected catches
                if (!attempt->frame.has_normal_result()) // private metadata ONLY
                    throw std::logic_error("locale file record did not return a normal receipt");
                ++p.records_returned;
                const auto length = calls.stream_length_entry(
                    current_entry(f.converted_, 0x30u), f.converted_);
                f.require_entered();
                const auto position = calls.stream_position_entry(
                    current_entry(f.converted_, 0x20u), f.converted_);
                f.require_entered();
                if (position == length) break; // exact full64, NOT >=
            } else {
                // Native spins for an unreached mode. This Source admission
                // rejects out-of-domain mutation, never claims EOF/success.
                throw std::logic_error("locale file scanner reached an unsupported mode");
            }
        }

        f.require_entered();
        p.phase = NativeLocaleParserFilePhase::returning_streams;
        const NativeCockpitInterlockedDecrement decrement = c.decrement_cell_; // AA0327 ONLY
        p.decrement_captured = true;
        p.converted_decrement_entered = true;
        const long converted_result = decrement(
            reinterpret_cast<volatile long*>(address(f.converted_) + 4u));
        p.converted_decrement_returned = true;
        f.require_entered();
        if (converted_result == 0) {
            const Word table = read<Word>(address(f.converted_));
            const Word entry = read<Word>(table);
            p.converted_zero_entered = true;
            calls.source_zero_reference(entry, f.converted_, table);
            p.converted_zero_returned = true;
            f.require_entered();
        }
        // The same captured import; no converted payload/table access here.
        p.original_decrement_entered = true;
        const long original_result = decrement(
            reinterpret_cast<volatile long*>(address(f.original_) + 4u));
        p.original_decrement_returned = true;
        f.require_entered();
        if (original_result == 0) {
            const Word table = read<Word>(address(f.original_));
            const Word entry = read<Word>(table);
            p.original_zero_entered = true;
            calls.source_zero_reference(entry, f.original_, table);
            p.original_zero_returned = true;
            f.require_entered();
        }
        f.require_entered();
        p.normal_returned = true;
        p.phase = NativeLocaleParserFilePhase::returned;
    } catch (...) {
        // AA0020 has NO selected stream cleanup. Keep exact partial refs,
        // buffers/count, all record attempts and borrowed providers; no retry.
        f.fail(std::current_exception());
        throw;
    }
}
} // namespace bsp
