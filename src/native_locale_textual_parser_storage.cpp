#include "bsp/native_locale_textual_parser_storage.hpp"

#include "bsp/game_native_vfs_application.hpp"
#include "bsp/native_string.hpp"
#include "bsp/native_string_pool_owner.hpp"
#include "bsp/native_string_pool_storage.hpp"
#include "bsp/native_string_vector_raw_storage.hpp"
#include "bsp/singleton_lifetime.hpp"

#include <cstring>
#include <intrin.h>
#include <memory>
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native locale textual parser storage requires MSVC Win32.
#endif

namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == 4 && sizeof(Word) == 4);
Word address(const volatile void* p) noexcept {
    return static_cast<Word>(reinterpret_cast<std::uintptr_t>(p));
}
void* pointer(Word p) noexcept { return reinterpret_cast<void*>(p); }
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
} // namespace

struct NativeLocaleTextualParserFrame::FileAttempt {
    NativeLocaleParserFileContext context;
    NativeLocaleParserFileFrame frame;
    FileAttempt* next{};
    FileAttempt(const NativeLocaleTextualParserContext& c, const void* filename)
        : context(c.manager_, filename, c.app_, c.pool_, c.data_, c.decrement_cell_),
          frame(context) {}
};

NativeLocaleTextualParserContext::NativeLocaleTextualParserContext(void* manager,
    void* input, Word other, game::GameNativeVfsApplication& app,
    game::GameNativeLocalePoolProcess& pool, const game::GameNativeReadOnlyData& data,
    NativeCockpitInterlockedDecrement const volatile& decrement)
    : manager_(manager), input_(input), other_(other), app_(app),
      strings_(app.raw_strings()), services_(app.borrow_raw_services()),
      pool_(pool), data_(data), decrement_cell_(decrement), overrides_(app, data) {
    if (!services_.strings.uses_raw_pool_cells(strings_))
        throw std::invalid_argument("textual option1 requires the application's SAME raw pool cells");
    // No header ownership is acquired here; input is not read and the current
    // decrement value is not captured. The other DWORD has no invented meaning.
}
NativeLocaleTextualParserFrame::NativeLocaleTextualParserFrame(
    const NativeLocaleTextualParserContext& context) noexcept : context_(context) {
    progress_.context = &context;
    progress_.opaque_other_argument = context.other_;
}
NativeLocaleTextualParserFrame::~NativeLocaleTextualParserFrame() noexcept {
    while (files_) {
        auto* const current = files_;
        files_ = current->next;
        delete current; // C++ metadata only; no filename/stream/native cleanup
    }
}
void NativeLocaleTextualParserFrame::require_entered() const {
    if (!progress_.attempted || progress_.failed || progress_.normal_returned ||
        progress_.phase == NativeLocaleTextualParserPhase::prepared ||
        progress_.phase == NativeLocaleTextualParserPhase::returned ||
        progress_.phase == NativeLocaleTextualParserPhase::failed)
        throw std::logic_error("textual option1 invocation is not healthy and active");
}
void NativeLocaleTextualParserFrame::fail(std::exception_ptr failure) noexcept {
    progress_.failed = true;
    progress_.phase = NativeLocaleTextualParserPhase::failed;
    if (!failure_) failure_ = failure;
}
[[noreturn]] void NativeLocaleTextualParserFrame::reject_entry() {
    try {
        throw std::logic_error("textual option1 requires its exact fresh one-shot invocation");
    } catch (...) {
        fail(std::current_exception());
        throw; // MUST be outside this invocation's native-selected cleanup try
    }
}
void NativeLocaleTextualParserFrame::unwind_owned() noexcept {
    // Private actual armed edges, consumed BEFORE their real lower action.
    // These belong only to the admitted outer invocation, not rejected reentry.
    try {
        if (state_ == 1) {
            state_ = 0;
            progress_.selected_vector_edge_consumed = true;
            progress_.selected_vector_entered = true;
            destroy_native_string_vector_004d0fa0(vector_, context_.strings_);
            progress_.selected_vector_returned = true;
        }
        if (state_ == 0) {
            state_ = -1;
            progress_.selected_input_edge_consumed = true;
            progress_.selected_input_entered = true;
            destroy_native_string_header_0041dd20(context_.input_, context_.strings_);
            progress_.selected_input_returned = true;
        }
    } catch (...) {
        std::terminate(); // genuine ordinary-C++ unwind, second cleanup escape
    }
}

void parse_native_locale_textual_option1_00aa0020(
    const NativeLocaleTextualParserContext& c, NativeLocaleTextualParserFrame& f) {
    auto& p = f.progress_;
    // No Native ownership has transferred for THIS invocation. A rejected
    // recursive call must never consume the active outer's state1 vector/input.
    if (&f.context_ != &c || p.attempted || p.failed ||
        p.phase != NativeLocaleTextualParserPhase::prepared)
        f.reject_entry();
    if (!c.manager_ || !c.input_ || !c.services_.strings.uses_raw_pool_cells(c.strings_))
        f.reject_entry();

    p.attempted = true;
    p.phase = NativeLocaleTextualParserPhase::entered;
    f.state_ = 0; // actual admitted formal-input ownership, BEFORE vector stores
    p.input_owned = true;
    try {
        store(address(f.vector_), Word{0});
        store(address(f.vector_) + 4u, Word{0});
        store(address(f.vector_) + 8u, Word{0});
        p.vector_initialized = true;
        void* const vfs = c.services_.actual_vfs_publication_0109ceec;
        f.state_ = 1;
        p.overrides_entered = true;
        append_native_lua_script_overrides_00bdef90(vfs, c.input_, f.vector_, c.overrides_);
        p.overrides_returned = true;
        f.require_entered();
        p.original_append_entered = true;
        append_native_string_vector_004cdc20(f.vector_, c.input_, c.strings_);
        p.original_append_returned = true;
        f.require_entered();

        f.cursor_ = read<Word>(address(f.vector_)); // AA008D CURRENT data
        const Word count = read<Word>(address(f.vector_) + 4u); // AA0091 CURRENT count
        f.end_ = f.cursor_ + count * 8u; // captured DWORD geometry, not a range facade
        p.captured_begin = f.cursor_;
        p.captured_end = f.end_;
        p.cursor = f.cursor_;
        p.phase = NativeLocaleTextualParserPhase::scanning;
        while (f.cursor_ != f.end_) {
            f.require_entered();
            std::unique_ptr<NativeLocaleTextualParserFrame::FileAttempt> node(
                new NativeLocaleTextualParserFrame::FileAttempt(c, pointer(f.cursor_)));
            auto* const attempt = node.get();
            if (f.last_) f.last_->next = attempt;
            else f.files_ = attempt;
            f.last_ = node.release();
            ++p.file_attempts;
            f.require_entered();
            scan_native_locale_parser_file_00aa00b0(attempt->context, attempt->frame);
            f.require_entered();
            if (!attempt->frame.normal_returned()) // private metadata, no old filename read
                throw std::logic_error("textual option1 file attempt lacks normal completion");
            ++p.files_returned;
            f.cursor_ += 8u;
            p.cursor = f.cursor_;
        }

        // Actual nonzero trailing-option branch AA036B -> AA060E. No option0
        // state3/4, sidecar/vector field repair or raw16 service is supplied.
        f.require_entered();
        p.phase = NativeLocaleTextualParserPhase::returning;
        f.state_ = 0; // AA0614 consumes vector edge BEFORE shrinking any element
        p.normal_vector_edge_consumed = true;
        p.normal_vector_shrink_entered = true;
        resize_native_string_vector_00427110(f.vector_, 0, c.strings_);
        p.normal_vector_shrink_returned = true;
        f.require_entered();
        const Word current_data = read<Word>(address(f.vector_));
        p.normal_vector_free_entered = true;
        singleton_lifetime_free(pointer(current_data)); // including null; no header reset
        p.normal_vector_free_returned = true;
        f.require_entered();

        // AA062B captures DATA first. AA063A disarms; AA0648 reads CURRENT len
        // only when captured data is nonnull. Never replace with a header dtor.
        const Word input_data = read<Word>(address(c.input_) + 4u);
        f.state_ = -1;
        p.normal_input_edge_consumed = true;
        if (input_data != 0u) {
            const Word bytes = read<Word>(address(c.input_)) + 1u;
            p.normal_input_return_entered = true;
            auto* const pool = native_string_pool_get_or_create_00419cc0(
                c.strings_.actual_published_01090aa8,
                c.strings_.actual_manager_publication_01090aa0);
            f.require_entered(); // getter may have caught a rejected recursive invocation
            return_native_string_pool_00bd1510(pool, pointer(input_data), bytes,
                c.strings_.actual_small_returns_disabled_01090aa4);
            p.normal_input_return_returned = true;
        } else {
            p.normal_input_null_skip = true;
        }
        f.require_entered(); // own metadata ONLY after ended input/vector payload
        p.normal_returned = true;
        p.phase = NativeLocaleTextualParserPhase::returned;
    } catch (...) {
        const auto failure = std::current_exception();
        f.fail(failure);
        // Only this admitted invocation owns these outer actions. Never add
        // stream/key returns or retry a normal footer edge already consumed.
        f.unwind_owned();
        std::rethrow_exception(failure);
    }
}
} // namespace bsp
