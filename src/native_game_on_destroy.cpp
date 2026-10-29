#include "bsp/native_game_on_destroy.hpp"
#include "bsp/native_game_construction.hpp"
#include "bsp/native_game_lifetime.hpp"
#include "bsp/native_lua_objects.hpp"

#include <exception>
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native Game OnDestroy requires MSVC Win32.
#endif

namespace bsp {
namespace {
using Word=std::uint32_t;
static_assert(sizeof(void*)==4);
static_assert(sizeof(NativeGameStorage)==0x71a0);

volatile Word& word(NativeGameStorage& game,Word offset) noexcept {
    // Explicit native DWORD address arithmetic; the admitted actual owner is
    // valid at every accessed offset. No synthetic layout or bounds fallback.
    return *reinterpret_cast<volatile Word*>(reinterpret_cast<Word>(&game)+offset);
}
void* pointer(NativeGameStorage& game,Word offset) noexcept {
    return reinterpret_cast<void*>(word(game,offset));
}

// Original004254B0 is exactly C3. Retain the real trace-call epoch and actual
// CE7C98 argument without inventing a trace provider, log effect or callee body.
// This verified no-op is reused with zero additional reconstruction credit.
__declspec(naked) void __cdecl trace_ret_004254b0(const char*) {
    __asm { _emit 0xc3 }
}

void delete_field(NativeGameStorage& game,Word offset,Word call_site,
    Word clear_site,NativeGameLifetimeCalls& calls,NativeGameOnDestroyOperation& o) {
    void* const captured=pointer(game,offset);
    if(captured) {
        o.native_site=call_site;o.captured_address=reinterpret_cast<std::uintptr_t>(captured);
        calls.virtual_scalar(captured,0,1);
        o.native_site=clear_site;word(game,offset)=0;
    }
}
void delete_publication(void* volatile& publication,Word call_site,Word clear_site,
    NativeGameLifetimeCalls& calls,NativeGameOnDestroyOperation& o) {
    void* const captured=publication;
    if(captured) {
        o.native_site=call_site;o.captured_address=reinterpret_cast<std::uintptr_t>(captured);
        calls.virtual_scalar(captured,0,1);
        o.native_site=clear_site;publication=nullptr;
    }
}
} // namespace

NativeGameOnDestroyOperation::~NativeGameOnDestroyOperation() {
    if(phase==Phase::running||phase==Phase::failed)std::terminate();
}
void NativeGameOnDestroyOperation::acknowledge_diagnostic_cleanup() noexcept {
    if(phase==Phase::failed) {
        classes.acknowledge_diagnostic_cleanup();
        phase=Phase::diagnostic_retired;
    }
}

void on_destroy_native_game_004dc5c0(NativeGameStorage& game,
    NativeGameOnDestroyContext& c,NativeGameOnDestroyOperation& o) {
    if(o.phase!=NativeGameOnDestroyOperation::Phase::fresh)
        throw std::logic_error("native Game OnDestroy operation cannot be replayed");
    o.owner=&game;o.context=&c;o.phase=NativeGameOnDestroyOperation::Phase::running;
    try {
        auto& required=c.required_calls;auto& lifetime=c.lifetime;auto& calls=lifetime.calls;
        o.native_site=0x004dc5c9;trace_ret_004254b0(c.actual_trace_00ce7c98);
        o.native_site=0x004dc5d1;word(game,0x5d4)=0x10;
        o.native_site=0x004dc5db;required.call_0046f3e0();
        o.native_site=0x004dc5e2;required.call_004da780(game);
        o.native_site=0x004dc5e9;required.call_004da650(game);

        void* const online=c.actual_online_00f8a2fc;
        if(online) {
            o.native_site=0x004dc5ff;o.captured_address=reinterpret_cast<std::uintptr_t>(online);
            required.current_online_slot_08(online);
            o.native_site=0x004dc601;required.call_00992ac0();
        }

        // B65E80's complete normal actual-storage effects match the existing
        // B669A0 close: capture state04; close iff nonnull and owns00; clear
        // state04 after return; do not clear ownership/tracking metadata.
        o.native_site=0x004dc60c;
        close_native_lua_state_00b669a0(*reinterpret_cast<NativeLuaStateStorage*>(
            reinterpret_cast<Word>(&game)+0x1a0c));
        o.native_site=0x004dc617;
        void* const host=pointer(game,0x1a08);
        o.captured_address=reinterpret_cast<std::uintptr_t>(host);
        required.call_008844f0(host); // Native has no null guard here.

        // Each load follows all preceding callbacks. A callback replacement is
        // cleared after the captured receiver returns, just as in the native.
        delete_field(game,0x1a08,0x004dc62c,0x004dc62e,calls,o);
        delete_field(game,0x21a0,0x004dc644,0x004dc646,calls,o);
        delete_field(game,0x21d8,0x004dc65c,0x004dc65e,calls,o);
        delete_publication(lifetime.publication_00e18678,0x004dc674,0x004dc676,calls,o);
        delete_publication(lifetime.publication_00e1867c,0x004dc68c,0x004dc68e,calls,o);

        o.native_site=0x004dc696;
        calls.NativeGameLifetimeCalls::call_004a9ac0(
            lifetime.classes,lifetime.profile,o.classes);
        o.phase=NativeGameOnDestroyOperation::Phase::complete;
    } catch(...) {
        o.phase=NativeGameOnDestroyOperation::Phase::failed;
        throw;
    }
}
} // namespace bsp
