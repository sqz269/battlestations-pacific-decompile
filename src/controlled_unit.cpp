#include "bsp/controlled_unit.hpp"

// docs/CONTROLLED_UNIT.md. Every branch below is the listing's branch in the listing's
// order. No value is defaulted and no call is stubbed: what the packet did not read is
// a host method, not a guess.

namespace bsp {

ControlledUnitGlobals set_controlled_unit_004c0890(bool unit_present,
                                                   ControlledUnitQuery& query,
                                                   SetControlledUnitHost& host) {
    // 004C0893: the store happens before the TEST ECX,ECX branch is taken, so even the
    // null call leaves the global holding null rather than its previous value.
    host.store_controlled_unit(unit_present);

    ControlledUnitGlobals globals{};
    globals.unit_present = unit_present;

    // 004C0899..004C08CE: the resolution, already reconstructed in bsp/unit_instance.hpp.
    const ControlledUnitBind bind = resolve_controlled_unit_004c0890(unit_present, query);

    // 004C08CE..004C0905 against 004C090C..004C0924. Both arms publish; they differ only
    // in the value, which is why publish_listener carries it.
    if (bind.has_target && bind.publishes_anchor) {
        const bool handle = host.driven_listener_handle();
        globals.listener_present = handle;
        host.publish_listener(handle);
    } else {
        globals.listener_present = false;
        host.publish_listener(false);
    }
    return globals;
}

bool unit_is_selectable_00645060(const UnitSelectableInputs& in) noexcept {
    // 00645082..006450BF: the flag block. Each test jumps straight to the AL=0 exit at
    // 00645151, so the order below is the listing's order and is observable only in
    // which test a debugger would stop on.
    if (!in.alive_5c) {
        return false; // 00645091
    }
    if (in.out_of_action_5d) {
        return false; // 0064509B
    }
    if (in.reject_60) {
        return false; // 006450A5
    }
    if (in.reject_5e) {
        return false; // 006450AF
    }
    if (!in.team_matches_owner) {
        return false; // 006450BF
    }

    // 006450C5..0064510C: four trait probes. Two are required, three are rejections.
    if (!in.is_kind_2) {
        return false; // 006450D0
    }
    if (!in.is_kind_0f) {
        return false; // 006450DF
    }
    if (in.is_kind_2a) {
        return false; // 006450EE
    }
    if (in.is_kind_46) {
        return false; // 006450FD
    }
    if (in.is_kind_45) {
        return false; // 0064510C
    }

    // 0064510E..0064511C: vtable +124h must answer true.
    if (!in.vtable_124_allows) {
        return false;
    }

    // 0064511E..0064513A. 00927C50 answering true accepts outright (JNZ 00645145). When
    // it answers false the unit is still accepted, but only through the spectator door:
    // the caller's flag must be set, the team must not be local, and the spectate kind
    // must not be 8.
    if (in.team_query_00927c50) {
        return true; // 00645145
    }
    if (!in.spectate_allowed) {
        return false; // 0064512C -> 0064513C, AL = 0
    }
    if (in.team_is_local) {
        return false; // 00645131, CMP [EBP+19h],AL with AL still 0 from the query
    }
    return in.spectate_kind != kUnitSpectateKindBlocked; // 00645133
}

SelectControlledUnitResult select_controlled_unit_00645600(bool candidate_present,
                                                           SelectControlledUnitHost& host) {
    SelectControlledUnitResult result{};

    // 00645603..0064562D: release the outgoing unit. Guarded on the *global*, not on the
    // candidate, so a null candidate still releases.
    if (host.controlled_unit_present() && host.current_is_kind_6()) {
        host.release_unit_parts();
        host.release_unit_nodes();
        result.released_previous = true;
    }

    // 00645637.
    host.clear_hud_slot();

    // 0064564D..00645679: the eligibility filter. The candidate is dropped when the test
    // fails, or when it is kind 1 and its vtable +124h answers false.
    bool candidate = candidate_present;
    if (!host.candidate_is_selectable()) {
        candidate = false;
    } else if (candidate && host.candidate_is_kind_1() && !host.candidate_vtable_124()) {
        candidate = false;
    }
    result.candidate_rejected = candidate_present && !candidate;

    // 0064567B..0064568C: the republish when nothing is changing. The listing compares
    // the global against the filtered candidate and calls the setter on equality.
    // Presence is all this reconstruction can compare; identity is the host's.
    if (host.controlled_unit_present() == candidate) {
        host.set_controlled_unit(candidate);
        result.refreshed_in_place = true;
    }

    // 00645692..006456BE: detach from whatever the global now holds.
    if (host.controlled_unit_present()) {
        host.unregister_observer();
        result.unregistered = true;
        if (host.controlled_is_kind_5()) {
            host.set_unit_controlled_audio(false); // PUSH 0 at 006456B7
        }
    }

    // 006456C0.
    host.set_controlled_unit(candidate);

    // 006456C5..006456FA: attach to the new global.
    if (host.controlled_unit_present()) {
        host.register_observer();
        result.registered = true;
        if (host.controlled_is_kind_5()) {
            host.set_unit_controlled_audio(true); // [ESP+4] = 1 at 006456EC
        }
    }
    return result;
}

} // namespace bsp
