#pragma once

#include "bsp/live_effect_manager_lifetime.hpp"
#include "bsp/native_unit_observer_endpoint.hpp"

namespace bsp {

// PARTIAL FRAGMENT of00824B60: only00824F39..00824FE4. Entry ESI is the
// same whole unit captured at00824B7E; its preceding effect loop has completed
// or been skipped. Borrow actual existing owning handle cells, not snapshots.
struct NativeUnitWreckEffectReleaseView {
    NativeUnitObserverAlias alias;
    void* volatile& effect_9ec;
    void* volatile& effect_9f0;
    void* volatile& effect_9f4;
};
struct NativeWreckGameView { void* volatile& session_38; };
struct NativeWreckSessionView { const volatile std::uint32_t& mode_60; };

class NativeUnitWreckEffectReleaseAccess {
public:
    virtual ~NativeUnitWreckEffectReleaseAccess() = default;
    // Required mappings to fields of these actual captured owners. No null
    // game recovery or copied session/flags owner is part of the native block.
    virtual NativeWreckGameView borrow_game(void* actual_game) = 0;
    virtual NativeWreckSessionView borrow_session(void* actual_session) = 0;
    // Body read: captured manager+10 pointer span/count+14; null filter means
    // all entries, otherwise pointer equality; call8673B0 for each match.
    // Original ECX manager, one stack filter word, RET4. Required provider.
    virtual void call_008674c0(NativeLiveEffectManagerStorage& actual_manager,
        void* captured_filter) = 0;
    // Body read: decrement CURRENT cell's old owner+4, terminal virtual0 on
    // zero, clear actual cell after callback, then adopt supplied word WITHOUT
    // retain. ECX actual cell, stack replacement, EAX cell, RET4. This fragment
    // supplies literal null. A raw clear would omit required owner release.
    virtual void call_00484620(void* volatile& actual_cell, void* replacement) = 0;
};

struct NativeUnitWreckEffectReleaseContext {
    void* volatile& game_00e188a8;
    NativeLiveEffectManagerStorage* volatile& manager_00f8765c;
    SingletonLifetimeDomain& lifetime_01090aa0;
    NativeUnitWreckEffectReleaseAccess& access;
};

// PARTIAL FRAGMENT, not the handler or its native ABI. Always continues at
//00824FE5 after normal completion, including gate skips. The caller must run
// that required leak/sink continuation; returning here does not complete the
// wreck operation. Exceptions propagate before any later cell is touched.
enum class NativeUnitWreckContinuation : std::uint32_t { at_00824fe5 = 0x00824fe5u };
NativeUnitWreckContinuation native_unit_wreck_effect_release_00824f39(
    NativeUnitWreckEffectReleaseView, NativeUnitWreckEffectReleaseContext&);

// Remaining00824B60..00824F38 and00824FE5..008252B0 are not implemented here.
// No native stack/FH3 transport or installed gameplay binding is claimed.
} // namespace bsp
