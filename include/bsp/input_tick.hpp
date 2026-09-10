#pragma once
// Per-frame input tick of GGame::OnMove (004e4a40).
//
// Packet game_frame_input_tick. Addresses reconstructed here:
//   004bec00  input singleton getter (already reviewed, listed for the sequence)
//   00a92c40  input manager per-frame update, called with the raw frame delta
//   00a92370  per-record poll prologue (the previous/current shift)
//   00a91e20  action reset, used by the suppression set at game+5B0h
//   00a919f0  action continue, used by the timed set at game+5BCh
//   00a92aa0  action start with parameters, same set, first visit only
//   00a926f0  parameter-name dispatch onto the listener ("fastRelease"/"holdPress")
//   004b9f40  ordered-set iterator increment for the game+5B0h instantiation
//   004bf830  ordered-set iterator increment for the game+5BCh instantiation
//   004d11d0  ordered-set erase(iterator) for the game+5BCh instantiation
//
// Nothing here is a recovered symbol. Names are hypotheses; offsets carry the
// address that proves them.
#include <cstddef>
#include <vector>

namespace bsp {

// ---------------------------------------------------------------------------
// Action records
// ---------------------------------------------------------------------------
// The input singleton is 24h bytes (constructor 00a93da0: vtable 00d5b630 at +0,
// zeroes at +4h..+18h, 1 at +1Ch and +20h). Its action table is a pointer at +4h
// and a count at +8h; every walk in this packet computes end = base + count*30h,
// so +8h is a count and not an end pointer (00a92c40 at 00a92c7c, 00a922a0 at
// 00a922aa, and the index scaling `id*3 << 4` at 004e4eb9 and 004e4f8b).
//
// Only the fields this packet touches are modelled. The record is 30h bytes; the
// binding array it polls lives at +10h/+14h with a 34h stride (00a92370 at
// 00a9239b) and belongs to the input-settings layer, not to this packet.
struct InputActionRecord {
    bool enabled{false};        // +01h, gate at 00a92c88; a cleared record is skipped
    float previous_hold{0.0f};  // +1Ch
    bool previous_down{false};  // +20h
    float current_hold{0.0f};   // +24h
    bool current_down{false};   // +28h
    // +2Ch holds the listener pointer. It is kept beside the record instead of
    // inside it because the reconstruction owns listeners by index, not address.
    std::size_t listener{0};    // index into InputTickState::listeners, 0 = none
    bool has_listener{false};   // +2Ch != 0
};

// The listener object at record+2Ch. 00a91e20 clears bytes +8h..+13h and floats
// +14h, +18h, +1Ch and +20h, which fixes the extent of its per-frame state.
// 00a91a50 is the classifier that fills it; only the two flags that 00a926f0
// names from strings are recovered, the rest keep their offset as their name.
struct InputActionListener {
    bool pressed{false};        // +08h
    bool press_confirmed{false};// +09h, = (+08h && +0Ah) at 00a91b99
    bool press_aux{false};      // +0Ah
    bool fast_release{false};   // +0Bh, set by name "fastRelease" at 00a92717
    bool release_confirmed{false}; // +0Ch, = (+0Bh && +0Dh) at 00a91bac
    bool release_aux{false};    // +0Dh
    bool hold_fired{false};     // +0Eh
    bool held{false};           // +0Fh
    bool hold_press{false};     // +10h, set by name "holdPress" at 00a9272f
    bool state_a{false};        // +11h
    bool state_b{false};        // +12h
    bool state_c{false};        // +13h
    float since_press{0.0f};    // +14h, cleared on the release branch at 00a91b86
    float since_release{0.0f};  // +18h, cleared on the press branch at 00a91b09
    float held_time{0.0f};      // +1Ch
    float held_time_biased{0.0f}; // +20h, 00a91b27 subtracts 00e12f28 on a repeat
};

// 00a92370 prologue, 00a9237a..00a9238c. The current pair becomes the previous
// pair and the current pair is zeroed before the bindings are polled. Every edge
// in the frame is decided by this one shift.
void begin_action_frame_00a92370(InputActionRecord& record) noexcept;

// 004c43c0, the reviewed edge test. True only on the frame the action goes down:
// current_down && current_hold > 0 && (!previous_down || previous_hold <= 0).
// 00d7a218 is eight zero bytes, so the comparisons are against 0.0f.
bool action_pressed_this_frame_004c43c0(const InputActionRecord& record) noexcept;

// The two booleans 00a92c40 hands to the listener at 00a92ca6..00a92ce0. They
// are the same "down and held for a positive time" predicate applied to the
// previous pair and to the current pair.
bool action_down_previous(const InputActionRecord& record) noexcept;
bool action_down_current(const InputActionRecord& record) noexcept;

// ---------------------------------------------------------------------------
// Effect-list entries
// ---------------------------------------------------------------------------
// Parameter block at node+18h, eight bytes. 00a92aa0 gates the dispatch on the
// first dword being non-zero (00a92abf) and 00a926f0 reads the second as a C
// string, compares it case-insensitively against "fastRelease" (00d5b624, via
// _stricmp 00bf7fbf) and otherwise asks the block itself about "holdPress"
// (00d5b618) through the method at 00425850.
struct InputEffectParam {
    const void* object{nullptr}; // +0h
    const char* name{nullptr};   // +4h
};

// Value type of the ordered set at game+5BCh, 18h bytes at node+0Ch.
// Field addresses: id 004e4f66, amount 004e4f74, expiry 004e4fbf,
// param 004e4f86, started 004e4f70 and 004e4f9d.
struct TimedInputEntry {
    int action_index{0};      // +00h (node+0Ch)
    float amount{0.0f};       // +04h (node+10h), the hold time forced into the record
    float expiry{0.0f};       // +08h (node+14h), compared against the global at 00f876a4
    InputEffectParam param{}; // +0Ch (node+18h)
    bool started{false};      // +14h (node+20h), latched after the first visit
};

// 00a91e20: clear both halves of the record and, when it has a listener, clear
// the listener's twelve flag bytes and four float accumulators. This is what the
// suppression set at game+5B0h applies to every action index it holds.
void reset_action_00a91e20(InputActionRecord& record, InputActionListener* listener) noexcept;

// 00a919f0: continue an injected action. Both halves are set down with the same
// hold time, so previous_down stays true and no edge is produced.
void continue_action_00a919f0(InputActionRecord& record, float amount) noexcept;

// 00a92aa0: start an injected action. The previous half is cleared and the
// current half is set down, which is exactly the rising edge 004c43c0 reports.
// The parameter block is dispatched onto the listener only when param.object is
// non-null; passing a null listener with a non-null object is the native
// null-dereference case and is not reproduced, the call is skipped instead.
void start_action_00a92aa0(InputActionRecord& record, float amount,
    const InputEffectParam& param, InputActionListener* listener) noexcept;

// 00a926f0, the name dispatch reached from start_action_00a92aa0.
void apply_effect_param_00a926f0(const InputEffectParam& param, InputActionListener& listener) noexcept;

// Erase test at 004e4fbf..004e4fcc: `fld [node+14h]; fld [00f876a4]; fcompi st(1);
// jb continue`. The node survives while the global is strictly below its expiry,
// so it is erased once the global reaches or passes it. 00f876a4 has one writer,
// 00874640 at 0087464c, which stores its float argument and derives the integer
// at 00f876b0 as argument/0.05; all five call sites of 00874640 that were read
// pass 0.0f. The global is therefore taken as an explicit input here.
bool timed_entry_expired(const TimedInputEntry& entry, float expiry_reference) noexcept;

// ---------------------------------------------------------------------------
// Frame state and host
// ---------------------------------------------------------------------------
// The slice of the singleton and the game object this packet walks. The two
// containers are MSVC red-black trees, not lists: 004b9f40 and 004bf830 are
// _Tree::iterator::operator++ (right subtree minimum, else climb while the node
// is the right child), reading _Left at +0h, _Parent at +4h, _Right at +8h and
// _Isnil at +11h for the 5B0h set and +25h for the 5BCh set. Both containers are
// {iterator list at +0h, _Myhead at +4h, _Mysize at +8h}, which is why the sizes
// sit at game+5B8h and game+5C4h. Iteration order is the tree comparator's and
// was not recovered; the reconstruction preserves insertion order instead.
struct InputTickState {
    std::vector<InputActionRecord> records;      // singleton+4h, count at +8h
    std::vector<InputActionListener> listeners;  // targets of record+2Ch
    std::vector<int> suppressed_actions;         // game+5B0h, set<int>, value at node+0Ch
    std::vector<TimedInputEntry> timed_actions;  // game+5BCh, set<TimedInputEntry>
};

// One method per native call this packet cannot reconstruct, in frame order.
// There are no default implementations: none of these stands in for recovered
// behaviour.
struct InputTickHost {
    virtual ~InputTickHost() = default;
    // 00f8bbf4 vtable +4h, called with the raw frame delta before any record is
    // touched (00a92c57). ECX is the backend, the delta is the only stack argument.
    virtual void backend_update(float seconds) = 0;
    // Byte 00f8bbf4+D4h, read and cleared at 00a92c5e..00a92c66. When it was set,
    // 00a922a0 re-resolves every record's bindings through 00a91e80.
    virtual bool take_backend_bindings_dirty() = 0;
    // 00a922a0: 00a91e80 on every record. Rebinding is an input-settings concern.
    virtual void rebind_all_actions(InputTickState& state) = 0;
    // 00a92370 after its prologue: evaluate the record's binding array and write
    // current_hold/current_down. The prologue itself is reconstructed here.
    virtual void poll_action_bindings(InputActionRecord& record, std::size_t index) = 0;
    // 00a91a50, ECX = record+2Ch, arguments (delta, previous, current), RET 0Ch.
    // The classifier is analysed but not reconstructed; see docs/GAME_INPUT_TICK.md.
    virtual void listener_classify(InputActionListener& listener, float seconds,
        bool down_previous, bool down_current) = 0;
    // 00f8bbfc, an optional plain function pointer invoked with no arguments
    // after the whole walk (00a92d02..00a92d0d).
    virtual void post_update_hook() = 0;
};

// 00a92c40, __thiscall(this = singleton, float seconds), RET 4. Called from
// OnMove at 004e4a72 with the raw frame delta the game update received.
void update_input_manager_00a92c40(InputTickState& state, float seconds, InputTickHost& host);

// OnMove phase 15, 004e4e6c..004e4fe4, reached only when the game state at
// game+5D4h is 0Dh and game+634h is zero. Walks the suppression set, then clears
// action index 1 when the timed set is not empty (004e4eda..004e4ef3), then walks
// the timed set, starting or continuing each entry and erasing the expired ones.
// Returns the number of entries erased.
int run_input_effect_lists_004e4e6c(InputTickState& state, float expiry_reference);

// The whole per-frame input tick as OnMove orders it: the manager update with the
// raw delta first (phase 3), the effect lists later in the frame (phase 15) and
// only in the in-game state. run_effect_lists carries that guard so a caller can
// reproduce a front-end frame with the same routine.
struct InputTickResult {
    int entries_erased{0};
    bool effect_lists_ran{false};
};
InputTickResult run_game_input_tick(InputTickState& state, float raw_delta,
    float expiry_reference, bool run_effect_lists, InputTickHost& host);

// Action index cleared at 004e4eda..004e4ef3 whenever the timed set is not empty.
// The native code adds a literal 30h to the record base, so this is index 1 and
// not a named control.
inline constexpr int kTimedListGuardAction = 1;

}  // namespace bsp
