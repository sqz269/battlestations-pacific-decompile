#include "bsp/native_locale_initial_language.hpp"
#include "bsp/native_locale_map_clear.hpp"
#include "bsp/native_physical_file_date.hpp"
#include "bsp/native_string.hpp"

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <intrin.h>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Initial raw locale language requires MSVC Win32.
#endif

namespace bsp {
namespace {
static_assert(sizeof(void*) == 4 && sizeof(std::uint32_t) == 4,
    "requires the Win32 raw address model");

void* at(const void* base, std::uint32_t offset) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(base) + offset);
}
std::uint32_t word(const void* base, std::uint32_t offset) noexcept {
    std::uint32_t result;
    _ReadWriteBarrier();
    std::memcpy(&result, at(base, offset), sizeof result);
    _ReadWriteBarrier();
    return result;
}
void* pointer(std::uint32_t bits) noexcept {
    return reinterpret_cast<void*>(static_cast<std::uintptr_t>(bits));
}
bool overlaps(const void* a, std::size_t an, const void* b, std::size_t bn) noexcept {
    const auto av = reinterpret_cast<std::uintptr_t>(a);
    const auto bv = reinterpret_cast<std::uintptr_t>(b);
    return av >= bv ? av - bv < bn : bv - av < an;
}
} // namespace

NativeLocaleInitialLanguageContext::NativeLocaleInitialLanguageContext(
    void* manager, const void* input, NativeStringRawPoolContext& strings,
    void* pool) noexcept
    : manager_(manager), input_(input), strings_(strings), pool_(pool) {}

NativeLocaleInitialLanguageFrame::NativeLocaleInitialLanguageFrame(
    const NativeLocaleInitialLanguageContext& context) noexcept : context_(context) {}

void NativeLocaleInitialLanguageFrame::fail() noexcept {
    phase_ = Phase::failed;
    normal_result_ = false;
    progress_.failed = true;
}
[[noreturn]] void NativeLocaleInitialLanguageFrame::reject(const char* message) {
    fail(); // latch before diagnostic allocation can reenter
    throw std::logic_error(message);
}
void NativeLocaleInitialLanguageFrame::require_active() {
    if (phase_ != Phase::active || progress_.failed)
        reject("initial locale language cannot resume after failure/reentry");
}
void NativeLocaleInitialLanguageFrame::complete() {
    require_active();
    normal_result_ = true;
    progress_.normal_returned = true;
    phase_ = Phase::returned;
}

void set_native_locale_initial_language_before_tables_00aa09d0(
    const NativeLocaleInitialLanguageContext& c, NativeLocaleInitialLanguageFrame& f) {
    // Rejected replay/mismatch poisons this history before any native read.
    if (&c != &f.context_ || f.phase_ != NativeLocaleInitialLanguageFrame::Phase::fresh)
        f.reject("initial locale language requires its fresh exact Context/Frame");
    f.phase_ = NativeLocaleInitialLanguageFrame::Phase::active;
    f.progress_.attempted = true;
    try {
        if (!c.manager_ || !c.input_ || !c.pool_ ||
            overlaps(&f, sizeof f, &c, sizeof c) ||
            overlaps(&f, sizeof f, c.manager_, 0x4040) ||
            overlaps(&c, sizeof c, c.manager_, 0x4040) ||
            overlaps(&f, sizeof f, c.input_, 8) ||
            overlaps(&c, sizeof c, c.input_, 8))
            f.reject("initial locale language requires live disjoint raw/metadata storage");

        void* const destination = at(c.manager_, 0x4018);
        f.progress_.equality_entered = true;
        const bool equal = equal_native_string_headers_00435c40(c.input_, destination);
        f.progress_.equality_returned = true;
        f.progress_.equal = equal;
        f.require_active();
        if (equal) {
            f.complete(); // Native equal arm reads neither map nor names
            return;
        }
        f.progress_.changed = true;
        if (destination != c.input_) {
            const std::uint32_t requested = word(c.input_, 0);
            f.progress_.resize_entered = true;
            resize_native_string_header_0041dd40(destination, c.strings_, requested, true);
            f.progress_.resize_returned = true;
            f.require_active();
            if (word(c.input_, 0) != 0) {
                const std::uint32_t length = word(destination, 0);
                void* const destination_data = pointer(word(destination, 4));
                const void* const source_data = pointer(word(c.input_, 4));
                f.progress_.copy_entered = true;
                // Preserve all native operand reads even for a zero count.
                if (length != 0) std::memmove(destination_data, source_data, length);
                f.progress_.copy_returned = true;
                f.require_active();
            }
        } else {
            f.progress_.self_skip = true;
        }

        f.progress_.map_clear_entered = true;
        clear_native_locale_map_0073c240(at(c.manager_, 0x14), c.strings_, c.pool_);
        f.progress_.map_clear_returned = true;
        f.require_active();
        const std::uint32_t begin = word(c.manager_, 8);
        f.progress_.names_begin_read = true;
        if (begin == 0) {
            f.progress_.begin_null = true;
            f.complete(); // do not read end on the Native NULL-begin arm
            return;
        }
        const std::uint32_t end = word(c.manager_, 0x0c);
        f.progress_.names_end_read = true;
        // Exact DWORD subtraction and SAR3 bits, without signed C++ casts.
        const std::uint32_t span = end - begin;
        const std::uint32_t count = (span >> 3) |
            ((span & 0x80000000u) ? 0xe0000000u : 0u);
        if (count != 0)
            f.reject("initial locale language reached unsupported nonempty names");
        f.progress_.count_zero = true;
        f.complete();
    } catch (...) { f.fail(); throw; } // retain actual language/map effects; no input action
}
} // namespace bsp
