#include "bsp/native_plane_session_message_lifetime.hpp"
#include "bsp/singleton_lifetime.hpp"

namespace bsp {
static_assert(sizeof(seg42::PlaneChangeStateMessage) == 0x24);
static_assert(sizeof(seg42::UnitMessage) == 0x20);
static_assert(sizeof(seg42::PlaneDisappearMessage) == 0x24);

// Evidence: bsp.py show 00762920 / 00762B00 / 00762BC0 --asm, existing
// bsp.gpr /battlestationspacific.exe. All three bodies TEST [ESP+4],1,
// save ECX in ESI, store 00CE4974 at [ESI], conditionally call 00BF65AC,
// return ESI in EAX and RET4. No payload or other header byte is touched.
// Keep the existing correct Ghidra CG_scalar_deleting_dtor_* names; these
// C++ names identify their message profiles, not newly recovered symbols.
// The root profile is the existing callable translation of 00CE4974.
__declspec(noinline) seg42::PlaneChangeStateMessage*
delete_plane_change_state_message_00762920(seg42::PlaneChangeStateMessage* message,
    std::uint32_t flags) {
    static_cast<volatile NativeSessionMessageStorage&>(message->u.h.base).profile_00 =
        native_session_message_root_profile_00ce4974();
    if ((flags & 1u) != 0) singleton_lifetime_free(message);
    return message;
}

__declspec(noinline) seg42::UnitMessage*
delete_plane_touchdown_message_00762b00(seg42::UnitMessage* message,
    std::uint32_t flags) {
    static_cast<volatile NativeSessionMessageStorage&>(message->h.base).profile_00 =
        native_session_message_root_profile_00ce4974();
    if ((flags & 1u) != 0) singleton_lifetime_free(message);
    return message;
}

__declspec(noinline) seg42::PlaneDisappearMessage*
delete_plane_disappear_message_00762bc0(seg42::PlaneDisappearMessage* message,
    std::uint32_t flags) {
    static_cast<volatile NativeSessionMessageStorage&>(message->u.h.base).profile_00 =
        native_session_message_root_profile_00ce4974();
    if ((flags & 1u) != 0) singleton_lifetime_free(message);
    return message;
}

// Chain 514 evidence: bsp.py show 007625C0 / 007625E0 --asm in existing
// bsp.gpr /battlestationspacific.exe. Both TEST byte [ESP+4],1; save ECX in
// ESI; store 00CE4974 at [ESI]; optionally call 00BF65AC; return ESI; RET4.
// Profiles 00D033C4 (98h) and 00D033D8 (9Ah) select these scalar deletes.
// Retain their correct Ghidra CG_scalar_deleting_dtor_* names. The callable
// root-profile translation and allocator are the same as the plane messages.
__declspec(noinline) seg42::UnitMessage*
delete_ship_wrecked_message_007625c0(seg42::UnitMessage* message,
    std::uint32_t flags) {
    static_cast<volatile NativeSessionMessageStorage&>(message->h.base).profile_00 =
        native_session_message_root_profile_00ce4974();
    if ((flags & 1u) != 0) singleton_lifetime_free(message);
    return message;
}

__declspec(noinline) seg42::UnitMessage*
delete_ship_explode_to_parts_message_007625e0(seg42::UnitMessage* message,
    std::uint32_t flags) {
    static_cast<volatile NativeSessionMessageStorage&>(message->h.base).profile_00 =
        native_session_message_root_profile_00ce4974();
    if ((flags & 1u) != 0) singleton_lifetime_free(message);
    return message;
}
} // namespace bsp
