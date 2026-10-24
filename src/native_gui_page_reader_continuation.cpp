#include "bsp/native_gui_page_reader_continuation.hpp"
#include "bsp/native_lua_service_bindings.hpp"
#include <cstring>
#include <exception>
#include <new>
#include <stdexcept>
#include <type_traits>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native GUI page reader continuation requires MSVC Win32.
#endif

namespace bsp {
static_assert(sizeof(void*) == 4 && sizeof(NativeLuaObjectStorage) == 0x14);
static_assert(sizeof(NativeLuaReaderStorage) == 0x14 && alignof(NativeLuaReaderStorage) == 4);
static_assert(std::is_standard_layout_v<NativeLuaObjectStorage> &&
    std::is_trivially_destructible_v<NativeLuaObjectStorage>);

NativeGuiPageReaderContinuation::NativeGuiPageReaderContinuation() noexcept {
    // User-provided: do not value-initialize native-unwritten bytes.
}
NativeLuaReaderStorage& NativeGuiPageReaderContinuation::live_reader() noexcept {
    return *std::launder(reinterpret_cast<NativeLuaReaderStorage*>(reader_));
}
std::uint32_t NativeGuiPageReaderContinuation::retained_reader_profile() const noexcept {
    // This backing array still exists after a genuine tail ends the typed
    // reader's lifetime. Inspect bytes without re-borrowing a dead reader.
    std::uint32_t profile;
    std::memcpy(&profile, reader_, sizeof profile);
    return profile;
}
NativeGuiPageReaderContinuation::~NativeGuiPageReaderContinuation() {
    // First guard sticky metadata; interrupted native bytes may be unreadable.
    if (native_operation_interrupted()) std::terminate();
    if (phase_ == Phase::awaiting_properties &&
        (retained_reader_profile() != 0x00ce374cu || !owner_ || owner_->state_04 != nullptr))
        std::terminate();
    // Raw backing/trivial objects have no implicit reader/Lua cleanup. A
    // genuine future tail must already have retired them once and quiesced
    // every borrow. This check authenticates no teardown/close history.
}
NativeGuiPageReaderContinuation::Observation NativeGuiPageReaderContinuation::observation() const noexcept {
    return {phase_, prefix_, scripts_, page_, owner_, &globals_, reader_, &scratch_,
        globals_entered_, globals_returned_, globals_adopted_,
        reader_constructor_entered_, reader_constructor_returned_, key_entered_, key_returned_,
        captured_profile_, captured_target_};
}
NativeGuiPageReaderContinuation::PendingProperties
NativeGuiPageReaderContinuation::borrow_for_properties_tail() {
    if (phase_ != Phase::awaiting_properties || !prefix_ || !scripts_ ||
        scripts_->prefix_identity() != prefix_ ||
        prefix_->phase != NativeGuiPageConstructionAcquired::Phase::awaiting_properties_continuation ||
        prefix_->continuation != 0x00ac6806u || prefix_->native_eh_state != 5 ||
        retained_reader_profile() != 0x00ce44fcu || !owner_ || !owner_->state_04)
        throw std::logic_error("GUI reader borrow requires the normal AC6806 properties boundary");
    return {live_reader(), *owner_, context_->lua};
}
void continue_native_gui_page_reader_00ac67cc(NativeGuiPageConstructionAcquired& prefix,
    NativeGuiPageScriptContinuation& scripts, NativeGuiPageReaderContinuation& frame,
    NativeGuiPageReaderContext& context) {
    if (frame.phase_ != NativeGuiPageReaderContinuation::Phase::fresh ||
        scripts.prefix_identity() != &prefix ||
        scripts.phase() != NativeGuiPageScriptContinuation::Phase::awaiting_reader ||
        prefix.phase != NativeGuiPageConstructionAcquired::Phase::awaiting_reader_continuation ||
        prefix.continuation != 0x00ac67ccu || prefix.native_eh_state != 2 || !prefix.actual_page ||
        !context.actual_gui_screen_00d5cb24 || !context.actual_reader_table_00ce44fc)
        throw std::invalid_argument("GUI reader requires a fresh SAME script/prefix and actual literal/table services");
    frame.prefix_ = &prefix;
    frame.scripts_ = &scripts;
    frame.context_ = &context;
    frame.page_ = prefix.actual_page;
    frame.phase_ = NativeGuiPageReaderContinuation::Phase::entered;
    try {
        // Guard the enclosing persistent frame BEFORE borrowing the real owner.
        frame.owner_ = &scripts.lua_owner_for_reader_tail();
        prefix.phase = NativeGuiPageConstructionAcquired::Phase::running;
        NativeLuaServiceBindings::Activation active(context.lua);
        frame.globals_entered_ = true;
        prefix.native_site = 0x00ac67da;
        native_lua_globals_00b67980(*frame.owner_, &frame.globals_);
        frame.globals_returned_ = true;

        prefix.native_site = 0x00ac67e3;
        frame.reader_constructor_entered_ = true;
        frame.globals_adopted_ = true; // Existing constructor owns consumption, including C++ failure.
        ::new (static_cast<void*>(frame.reader_)) NativeLuaReaderStorage(frame.globals_);
        frame.reader_constructor_returned_ = true;

        const auto key_bits = reinterpret_cast<std::uint32_t>(context.actual_gui_screen_00d5cb24);
        auto& reader = frame.live_reader();
        // AC67F1 precedes AC67F5. Required table metadata is borrowed live,
        // never a cached/default own-slot target or callable native address.
        frame.captured_profile_ = reader.profile_00;
        if (frame.captured_profile_ != 0x00ce44fcu)
            throw std::logic_error("GUI reader has no supported current native profile");
        frame.captured_target_ = context.actual_reader_table_00ce44fc[1];
        if (frame.captured_target_ != 0x00bd8e20u)
            throw std::logic_error("GUI reader has no supported current native enter target");
        prefix.native_site = 0x00ac67fc;
        prefix.native_eh_state = 5;
        // BD5790's genuine prelude initializes this SAME fresh output once;
        // opaque/padding retain their existing bytes. No extra constructor.
        frame.key_entered_ = true;
        prefix.native_site = 0x00ac6804;
        reader.enter_key_00bd8e20(frame.scratch_, 0, key_bits);
        frame.key_returned_ = true;

        prefix.native_site = 0x00ac6806;
        prefix.continuation = 0x00ac6806;
        prefix.phase = NativeGuiPageConstructionAcquired::Phase::awaiting_properties_continuation;
        frame.phase_ = NativeGuiPageReaderContinuation::Phase::awaiting_properties;
    } catch (...) {
        prefix.phase = NativeGuiPageConstructionAcquired::Phase::failed;
        frame.phase_ = NativeGuiPageReaderContinuation::Phase::failed;
        throw; // No argument/helper cleanup replay, reader destruction, Lua close or retry.
    }
}
} // namespace bsp
