#pragma once

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native unit-owner storage requires MSVC Win32.
#endif

namespace bsp {

class NativeObserverLifetime;
class NativeWeakOwnerDomain;
struct NativeStringRawPoolContext;

// Complete normal 00925CE0..00925EF3 scene-base construction on the actual
// aligned170h prefix. Caller owns the real weak/observer/raw-string domains
// and an established atomic<int32> lifetime in the same +28 field. Preserve
// all untouched bytes, especially world/parent/link preimages. Read actual
// D7A24C twice and perform both ordered x87 matrix copies to the same +74.
// Numeric profile words are evidence, not callable Source vtables. Source
// guards follow the five native unwind owners; Native FH3/SEH/fault/frame
// identity and exceptions within cleanup remain unproven. This does not
// compose 928630, establish world publication, or activate a unit receiver.
void* construct_native_unit_owner_scene_base_00925ce0(
    void* actual_root, NativeWeakOwnerDomain& weak,
    NativeObserverLifetime& observers, NativeStringRawPoolContext& strings,
    const volatile std::uint32_t& actual_one_bits_00d7a24c);

// Qualified 009257A7..009257B9 destructor entry stores, before the held
// 740270 life-reference release and actual child scalar-delete loop. Stamp
// only the three numeric profiles; no release, child dispatch or ownership
// transition is supplied here. These words are not callable Source tables.
void restore_native_unit_owner_scene_destructor_profiles_009257a7(void* actual_root) noexcept;

// Qualified 009257F5..009258B0 normal continuation, ONLY after the real
// life-reference release and complete child loop. The same actual170h root,
// live World/update/sibling/list storage and atomic+28 lifetime are required.
// Real raw unlinks precede names+15C/+154, weak+24, callback+10 and observed+0
// retirement. Five Source guards retain Native cleanup ownership at throwing
// call boundaries; Native FH3/SEH and cleanup-throw behavior remain unproven.
// This never frees the root or supplies the missing release/child dispatch.
void finish_native_unit_owner_scene_retirement_009257f5(
    void* actual_root_after_children, NativeWeakOwnerDomain& weak,
    NativeObserverLifetime& observers, NativeStringRawPoolContext& strings);

// Qualified normal continuation 0077EF27..0077EFA3 of 0077EED0.
// Requires the genuine containing allocation, its original factory preimage,
// successful 00928630(root, 1, unchanged full flag), and all three actual 34h
// recon records successfully constructed by the BF7CD1/805A70/804300 pair.
// The aligned allocation is writable through +30Fh. Bind the record owners to
// root+1E4, install the numeric owner profiles, and construct the first raw
// 0Ch sentinel through the accepted 0077B960/BF681B allocation boundary.
// Numeric profiles are evidence words, not Source-callable virtual tables.
// On success root+29C owns that sentinel; on allocation failure no list is
// acquired and the actual caller still owes the recon/base unwind. This is
// neither the base/record constructor nor its Native EH frame or whole root.
void construct_native_unit_owner_first_list_0077ef27(void* actual_root_after_recon);

// Qualified normal continuation 0077EFA9..0077F053. Requires the successful
// first stage with its live empty 0Ch list. Acquire the 18h sentinel through
// 0077B940, then perform the exact mixed-width observer/physical stores.
// Load the borrowed actual CE4ADC word once, after the +2C8 WORD store and
// before +2E8. On allocation failure the first list remains owned by the
// enclosing caller; this function performs no guessed rollback/unwind.
// Success owns both lists and stops before the actual E188A8/network-mode /
// E0AF20 read and possible 0077EB50 call. It cannot activate a receiver.
void construct_native_unit_owner_second_list_and_fields_0077efa9(
    void* actual_root_after_first_list,
    const volatile std::uint32_t& actual_initial_word_00ce4adc);

// Complete normal raw list leaves. The actual list header has sentinel at
// +4 and DWORD size at +8; nodes contain next/previous at +0/+4. These own
// real CRT allocations from the same allocation domain as the head producers.
// 0077B980 frees all 0Ch nodes, then sentinel, and zeros header+4.
// 0077D930 frees each non-null 18h node+14 payload (zeroing it before the
// node free), then each node. It retains the self-linked sentinel.
// Both clear size and re-read the current sentinel as the Original does.
// No header synthesis, invalid/null-list handling or fault-equivalence claim.
void tidy_native_unit_owner_peer_list_0077b980(void* actual_list_header) noexcept;
void clear_native_unit_owner_holder_list_0077d930(void* actual_list_header) noexcept;

// Qualified normal 0077E42C..0077E456 destructor continuation. Requires the
// actual preceding session/global withdrawal and +2B0 observer destruction,
// and live list ownership. Clear/free the +2A4 holder list, zero +2A8, then
// tidy +298 peer list. Leaves +1E8 recon array, GameEntity/SceneNode, root
// allocation, scalar flags and embedded tick/child destruction to their real
// owner. Not a complete destructor and never a scalar-delete substitute.
void destroy_native_unit_owner_lists_0077e42c(void* actual_root_after_observer) noexcept;

// Complete normal node producers. ECX is unused in the Original; stack
// (next, previous, source), retained EAX result, RET0C. Allocate exactly 18h
// or 0Ch through the same real BF681B domain. Independently guard the wrapped
// +0/+4/+8 destinations; read source DWORDs only after allocation and prior
// link stores. The holder copies all four DWORDs, including its opaque two
// scratch bytes. Source must be actual live storage, not a default record.
void* __stdcall buy_native_unit_owner_holder_node_0077ba60(
    void* next, void* previous, const volatile std::uint32_t* actual_record);
void* __stdcall buy_native_unit_owner_peer_node_0077bb00(
    void* next, void* previous, const volatile std::uint32_t* actual_peer_slot);

// Complete checked count operations with unsigned wrapped subtraction bounds
// 0FFFFFFF and 3FFFFFFF. Overflow retains count and throws the existing owning
// NativeAliasListLengthError with the actual legacy string/logic-error body.
// This is Source exception transport, not Original RTTI/throw/frame ABI.
// These own no node allocation, unlink or rollback.
void grow_native_unit_owner_holder_count_0077dd00(void* actual_list, std::uint32_t increment);
void grow_native_unit_owner_peer_count_0077de40(void* actual_list, std::uint32_t increment);

// Complete normal 0077B030..0077B04B: release non-null scratch+0C and zero it.
// Actual FH3 state0 in 77EB50 reaches this through C8A060. No node is owned.
void release_native_unit_owner_holder_scratch_0077b030(void* actual_record) noexcept;

// Concrete 0077EB50 business body using the real list/count/CRT components.
// Requires live root lists and an aligned, disjoint, genuine 10h scratch
// preimage whose +0A/+0B bytes are supplied by the actual caller/frame domain.
// Those two bytes are copied unchanged; no zero-filled record is invented.
// Borrow the actual peer-argument cell: the holder captures it before the
// first allocation; the peer node reads it after its own later allocation.
// Preserve state0 scratch cleanup only until the fresh peer head/previous
// are captured. After that, a failure leaves prior list effects in place.
// A count failure does not retroactively free the newly allocated node.
// Full Native stack/exception ABI and scratch provenance remain caller gates.
void append_native_unit_owner_peer_0077eb50(
    void* actual_root,
    const volatile std::uint32_t& actual_peer_argument_slot,
    void* actual_holder_scratch_preimage,
    volatile std::uint8_t& actual_dirty_byte_00f87190);

// Qualified normal constructor tail 0077F054..0077F076, after both earlier
// owner stages. Load actual E188A8 once, then its +1FE4 DWORD. Only mode2
// reads E0AF20; only a zero byte appends the explicit peer0 through 77EB50.
// The actual Game publication is required; there is no added null guard.
// Scratch/dirty cells are untouched on either skipped branch. The actual
// scratch preimage and all owning base/array/root lifetimes remain required.
// This still supplies no completed factory, callable profile or tick receiver.
void finish_native_unit_owner_peer_tail_0077f054(
    void* actual_root_after_physical_fields,
    void* volatile& actual_game_publication_00e188a8,
    const volatile std::uint8_t& actual_network_byte_00e0af20,
    void* actual_holder_scratch_preimage,
    volatile std::uint8_t& actual_dirty_byte_00f87190);

// Qualified 0077E3A0..0077E3C7 destructor prefix: ordered numeric profile
// writes to the genuine root before the actual session-release decision.
// These are evidence words, not Source-callable tables. Does not send, unlink,
// destroy a member or establish the enclosing Native EH state4.
void restore_native_unit_owner_destructor_profiles_0077e3a0(void* actual_root) noexcept;

// Complete 0077BEA0..0077BF3B list business schedule. The actual F87194 list
// object has head at+4 and DWORD count at+8; each0Ch node has next/previous/
// value at0/4/8. Capture the sentinel, then the actual target DWORD once, then
// the first node. Remove every match, free the node in the real CRT domain,
// and only then reload/decrement count modulo32. Retain the original sentinel
// for iteration termination while re-reading current head for each check.
// Returning invalid checks call the accepted concrete BF6713 Source boundary;
// its actual SDK policy is qualified, not Native BF66EF handler equivalence.
// No repaired storage, callback provider, count clamp or null-list default.
// Requires real live allocations/header/target storage and the caller's
// synchronization domain; invalid-memory/fault and register ABI unproven.
void remove_native_unit_owner_global_matches_0077bea0(
    void* actual_global_list,
    const volatile std::uint32_t& actual_target_value_cell);

// Qualified 0077E409..0077E41B destructor continuation, after the real
// conditional session-release arm has completed or legitimately skipped.
// Materialize the actual root value cell and remove all its matches from the
// actual F87194 list object. No membership cache or replacement global exists.
// Stops before +2B0 observer destruction; no list/member/root deletion here.
void withdraw_native_unit_owner_global_0077e409(
    void* actual_root_after_session_release,
    void* actual_global_list_00f87194);

// Complete normal 00805A70..00805AE9 record-constructor business effects.
// Requires the actual aligned34h record allocation/preimage and the existing
// observer lifetime domain. Preserve holes11..13/25..27 and owner/index2C/30.
// Initialize embedded callback storage at+14 with the Original mixed widths;
// then re-read+28 and, if non-null, call real6952A0 before clearing that field.
// Return the same actual record address. Source unwind calls the accepted
// raw4B7EF0 child on record+14, matching C90530->4BD350 ownership; neither that
// normal child nor this host guard supplies Original FH3/fault/frame ABI.
// No array iterator, base/root construction, callable profile or activation.
void* construct_native_unit_owner_recon_record_00805a70(
    void* actual_record, NativeObserverLifetime& actual_observers);

// Complete normal 00804300..0080430D retirement leaf: stamp D08E74 at the
// actual record, then invoke accepted raw4B7EF0 at record+14 using the same
// observer domain. Its concrete pair removal/callback destruction owns the
// embedded observer arrays; it does not clear record+28 or free the record.
// Source diagnostic stage is private metadata, not a second native owner.
// No reconstruction of the enclosing vector/FH3 unwind or register tail ABI.
void destroy_native_unit_owner_recon_record_00804300(
    void* actual_record, NativeObserverLifetime& actual_observers);

// Supplied, non-stubbed call boundaries for the complete normal owner bodies
// below. Each name is an address hypothesis. They retain the original receiver
// and stack arguments; this Source interface itself is not the x86 ABI.
struct NativeUnitOwnerEntityCalls {
    virtual ~NativeUnitOwnerEntityCalls() = default;
    // ECX=root; registry_select, full_flag; RET8, EAX=root.
    virtual void game_entity_construct_00928630(
        void* root, std::uint32_t registry_select, std::uint32_t full_flag) = 0;
    // ECX=root; RET0. Completes the GameEntity and SceneNode retirement.
    virtual void game_entity_destruct_009287b0(void* root) = 0;
    // ECX=actual 24h stack message; payload; RET4, EAX=message. Kind4Fh.
    virtual void* entity_release_construct_00779780(void* message, std::uint32_t payload) = 0;
    // ECX=sender root; message, flags, optional output pointer; RET0Ch.
    virtual void route_message_0077c2a0(
        void* sender, void* message, std::uint32_t flags, void* output) = 0;
    // ECX=root+2B0h; RET0. Restore CF9348, unregister target+14h, then
    // destroy its callback owner. This is not a deleting destructor.
    virtual void observer_member_destruct_006e0860(void* member) = 0;
};

// Complete normal 0077EED0..0077F08D constructor schedule. Original ABI:
// ECX=root, one full DWORD flag, RET4, EAX=root. Compose the genuine base,
// three actual34h recon records, owner/profile/list/physical stages, and the
// mode2/offline peer0 arm in image order. Required actual allocation/preimage,
// observer/CRT domains and publication cells are the same as the fragments.
// The Original BF7CD1 iterator is a qualified sequential Source substitution.
// Native C8A0C8/FH3 constructor unwind is not supplied: no EH-path or callable
// native-profile/ABI/gameplay equivalence is claimed by this normal body.
void* construct_native_unit_owner_entity_0077eed0(
    void* actual_root, std::uint32_t full_flag, NativeObserverLifetime& observers,
    const volatile std::uint32_t& actual_initial_word_00ce4adc,
    void* volatile& actual_game_publication_00e188a8,
    const volatile std::uint8_t& actual_network_byte_00e0af20,
    void* actual_holder_scratch_preimage,
    volatile std::uint8_t& actual_dirty_byte_00f87190,
    NativeUnitOwnerEntityCalls& calls);

// Complete normal 0077E380..0077E490 body destructor. Original ECX=root,
// no stack arguments, RET0; it never frees root or interprets a delete flag.
// Preserve the byte/game/mode1 short-circuit reads, kind4Fh stack message
// (payload root+70h, route flags4/output0), global removal, observer/lists,
// reverse recon-record destruction, then genuine GameEntity base teardown.
// BF7C6E is a qualified reverse Source loop. Native C8A053/FH3 unwind and its
// EH-only message79 cleanup are not normal calls and remain unimplemented.
void destruct_native_unit_owner_entity_0077e380(
    void* actual_root, NativeObserverLifetime& observers,
    const volatile std::uint8_t& actual_network_byte_00e0af20,
    void* volatile& actual_game_publication_00e188a8,
    void* actual_global_list_00f87194, NativeUnitOwnerEntityCalls& calls);

} // namespace bsp
