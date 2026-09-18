#ifndef BSP_TORPEDO_RELEASE_ORDERS_HPP
#define BSP_TORPEDO_RELEASE_ORDERS_HPP

// The queued release-order count unit+C58h and the pilot control block's attack
// mode ctl+370h: the last two gates between an ordered torpedo aircraft and its
// drop. docs/TORPEDO_RELEASE_ORDERS.md carries the evidence;
// docs/TORPEDO_APPROACH_UPDATE.md owns the approach update that named them and
// docs/TORPEDO_TASK_ARM.md the state machine they feed.
//
// Every name here is a hypothesis, not a recovered symbol. Nothing is a
// binary-compatible layout: the offset constants in the comments are the native
// ones, the structs are not.
//
// Contracts named but not reconstructed: the device walk of 007C0D90 (the
// vtable slots +5Ch, +210h, +1FCh, +220h, +1F0h and the descriptor's +8h),
// 007EE7F0 (the first call of 007EEF30), and the scene-node enable pair
// 00922F30 / 00922F80 that owns unit+5Ch.

namespace bsp {

// ---------------------------------------------------------------------------
// Constants.
// ---------------------------------------------------------------------------

// 007EEF78 PUSH 0x3E7: the count 007EEF30 hands every controlled unit. It is a
// budget, not a magazine: the pilot bot spends one per tick and 999 is large
// enough that the count never runs out inside a mission.
inline constexpr int kReleaseOrderCount_007eef78 = 0x3E7;

// The entity class ids the two capability tests use, from
// docs/ORDNANCE_KIND_IDENTITY.md.
inline constexpr int kEntityKindPlaneKamikaze_17h = 0x17;  // 007B9148 PUSH 0x17
inline constexpr int kEntityKindBomb_2ah = 0x2A;           // 007B917B PUSH 0x2a
inline constexpr int kEntityKindTorpedo_2bh = 0x2B;
inline constexpr int kEntityKindWeaponDevice_25h = 0x25;   // 007C0DB8 PUSH 0x25

// ---------------------------------------------------------------------------
// ctl+370h, the pilot control block's attack mode. Three values, each named
// from the site that writes or tests it.
// ---------------------------------------------------------------------------
enum class PilotAttackMode : int {
    // The value the block starts at. 009D3F60 row 1 and 009D4030 step 6 both
    // send an engaged torpedo task to `prepare` while the mode is this, so a
    // task parked here never reaches `attackrun` or `aim`.
    kHold = 0,
    // 0099B740 at 007ED3F0's call site 0099B774 raises it to this every tick
    // the lead aircraft's task authorises the attack.
    kAttack = 1,
    // 008A4B10 BSP_LuaBinding_PilotStopCloseToShip raises it here through the
    // raise-only setter 007ED430. 009D3210 short-circuits to engaged on it.
    kForced = 2,
};

// 007ED3F0, `void __thiscall(ctl, int mode)`, RET 4: a plain store.
// 007ED430, `void __thiscall(ctl, int mode)`, RET 4: raises only, never lowers
// (CMP at 007ED434, JGE skips the store at 007ED43C).
PilotAttackMode pilot_attack_mode_raise_007ed430(PilotAttackMode current,
                                                 PilotAttackMode requested) noexcept;

// 0099B740 BSP_BotTask_AbandonIfStale, the tail of eight of the ten task
// vtable +54h cruise profiles including the torpedo's 009D4A70. It is the only
// producer of kAttack in the image.
struct PilotAttackModeInputs {
    bool has_control_block_2fc = false;  // task->+2FCh != 0, 0099B749
    bool has_unit_2f4 = false;           // task->+2F4h != 0, 0099B753
    // 0099B757: task->+2F4h == *(void**)(ctl + 3D0h). ctl+3D0h is an array of
    // controlled units, so this is "am I the lead aircraft of the flight".
    bool unit_is_flight_lead = false;
    // 0099B766: task->vtable[38h](). The torpedo task inherits the base
    // 0099B710, which is `MOV AL,1 / RET`, so for this class it is always true.
    bool task_authorises_38h = true;
};
// Returns the mode after the call; kHold in means the call either raises to
// kAttack or leaves it alone.
PilotAttackMode pilot_attack_mode_0099b740(PilotAttackMode current,
                                           const PilotAttackModeInputs& in) noexcept;

// ---------------------------------------------------------------------------
// unit+C58h, the queued release-order count.
// ---------------------------------------------------------------------------

// 007B9140, `char __thiscall(unit, int arg)`, RET 4: can this unit drop
// ordnance at all. A kamikaze plane qualifies on its own class; anything else
// needs a device in the array at unit+974h that holds bomb-family ordnance.
// The torpedo descriptor answers 2Ah as well as 2Bh
// (docs/ORDNANCE_KIND_IDENTITY.md), so a torpedo bomber qualifies here.
struct CanDropOrdnanceInputs {
    bool unit_is_kind_17h = false;     // 007B914A unit->vtable[5Ch](17h)
    int device_count_994 = 0;          // 007B915B unit->+994h
    // 007B917D device->vtable[210h](2Ah, arg), one entry per device at
    // unit+974h, in order. May be null when device_count_994 is zero.
    const bool* device_holds_2ah = nullptr;
};
bool unit_can_drop_ordnance_007b9140(const CanDropOrdnanceInputs& in) noexcept;

// 007BCBE0, `void __thiscall(unit, int count)`, RET 4: the ONE writer of
// unit+C58h in the image outside the constructor. It assigns, it does not
// accumulate, and it zeroes the count whenever any guard fails.
struct ReleaseOrderSetInputs {
    int requested_count = 0;            // 007BCBE6, the pushed argument
    bool scene_node_enabled_5c = false; // 007BCBEC unit->+5Ch
    bool can_drop_ordnance = false;     // 007BCBF4 007B9140(unit, 0)
};
int release_order_count_007bcbe0(const ReleaseOrderSetInputs& in) noexcept;

// 007CCF5 FMUL double [00CEFFB0]: the scale 0079CBD0 applies to the class
// descriptor field when it seeds ctl+390h, the issue threshold.
inline constexpr double kIssueThresholdScale_00ceffb0 = 0.95;

// 007EE7F0, `void __thiscall(ctl, unit)`, the hook 007EEF30 runs first at
// 007EEF3B. It recomputes ctl+374h, the fraction of the flight that still has
// rounds left, and clears ctl+3ECh. 007C1F60 sums the remaining rounds over the
// unit's droppable devices (class 25h, ordnance 2Ah, 006E3500 per device), and
// the CALLING unit discounts one round of its own, because it is about to
// spend it.
struct FlightArmedFractionInputs {
    int controlled_count_3cc = 0;
    // Per controlled unit, in ctl+3D0h order: the scene-node enabled byte
    // unit+5Ch, the remaining round count 007C1F60(unit), and whether this is
    // the unit 007EEF30 was called for.
    const bool* unit_enabled_5c = nullptr;
    const int* unit_rounds_007c1f60 = nullptr;
    const bool* unit_is_caller = nullptr;
};
float flight_armed_fraction_007ee7f0(const FlightArmedFractionInputs& in) noexcept;

// 007EEF30, `void __thiscall(ctl, unit)`: the issuer. It hands
// kReleaseOrderCount_007eef78 to every unit in the control block's array.
struct ReleaseOrderIssueInputs {
    // 007EEF40-007EEF50: FCOMIP on ctl->+390h against ctl->+374h; the whole
    // loop is skipped unless the first is strictly greater. +390h is the
    // threshold 0079CD36 seeds from the class descriptor at unit+538h, field
    // +A0h, times 0.95; +374h is the armed fraction 007EE7F0 recomputes on
    // every call. So a flight issues release orders only while enough of it has
    // already spent its load, which throttles a large formation.
    float authorise_value_390 = 0.0f;      // ctl+390h, the threshold
    float authorise_threshold_374 = 0.0f;  // ctl+374h, the armed fraction
    int controlled_count_3cc = 0;       // 007EEF54
    bool force_flag_378 = false;        // 007EEF62
};
// Whether the loop runs at all.
bool release_orders_should_issue_007eef30(const ReleaseOrderIssueInputs& in) noexcept;
// 007EEF62-007EEF7F, per controlled unit: the force flag short-circuits, else
// the unit must NOT lack a follow target (007B8AD0 returning true skips it).
bool release_order_issue_to_unit_007eef78(bool force_flag_378,
                                          bool unit_lacks_follow_target_007b8ad0) noexcept;

// 007ED3C0, `void __fastcall(ctl)`: clears the count on the lead unit and the
// force flag. Returns the count to write, or leaves it alone when the array is
// empty.
struct ReleaseOrderClearResult {
    bool clear_force_flag_378 = true;   // 007ED3C7, unconditional
    bool write_lead_count = false;      // 007ED3CE, only when ctl->+3CCh > 0
    int lead_count = 0;
};
ReleaseOrderClearResult release_orders_clear_007ed3c0(int controlled_count_3cc) noexcept;

// 0099AF53-0099AFC2, the spend inside BSP_PilotBot_Tick. The guards are, in
// order: the count must be positive, the object embedded at unit+72Ch must
// answer its vtable +38h, unit+5Ch must be set, and 007B9140(unit, 0) must
// hold. Any failure zeroes the count outright rather than leaving it.
struct ReleaseOrderSpendInputs {
    int count_c58 = 0;                        // 0099AF53
    bool manual_release_requested_72c = false;// 0099AF67 (unit+72Ch)->vtable[38h]
    bool scene_node_enabled_5c = false;       // 0099AF70
    bool can_drop_ordnance = false;           // 0099AF78 007B9140(unit, 0)
    bool a_task_took_the_order = false;       // 0099AF9B task->vtable[24h]
};
struct ReleaseOrderSpendResult {
    int count_c58 = 0;        // the value left in unit+C58h
    bool offered = false;     // the arming loop at 0099AF81 ran
    bool cleared = false;     // a guard failed and 0099AFC2 zeroed the count
};
ReleaseOrderSpendResult release_order_spend_0099af53(
    const ReleaseOrderSpendInputs& in) noexcept;

// ---------------------------------------------------------------------------
// The host. One method per native call site on the issue path.
// ---------------------------------------------------------------------------
struct TorpedoReleaseOrderHost {
    TorpedoReleaseOrderHost() = default;
    virtual ~TorpedoReleaseOrderHost() = default;
    TorpedoReleaseOrderHost(const TorpedoReleaseOrderHost&) = delete;
    TorpedoReleaseOrderHost& operator=(const TorpedoReleaseOrderHost&) = delete;

    // 007C0DB8, 007C0DC8, 007C0DE4: the device walk over unit+48h that decides
    // whether 007EEF30 is called at all. contract: unread beyond its shape.
    virtual bool unit_carries_droppable_device_007c0d90() = 0;
    // 007C0EE2: the byte the walk raises before the call.
    virtual void set_release_pending_c25(bool value) = 0;
    // 007EEF3B: 007EE7F0(ctl, unit), run before the gate. contract: unread.
    virtual void pre_issue_hook_007ee7f0() = 0;
    // 007EEF40, 007EEF54: the gate fields on the control block.
    virtual ReleaseOrderIssueInputs read_issue_inputs() = 0;
    // 007EEF69: the controlled units at ctl+3D0h, in order.
    virtual int controlled_unit_count() = 0;
    // 007EEF6F: 007B8AD0 BSP_Unit_LacksFollowTarget on one of them.
    virtual bool unit_lacks_follow_target_007b8ad0(int index) = 0;
    // 007BCBEC and 007BCBF4, the two guards inside the setter.
    virtual ReleaseOrderSetInputs read_set_inputs(int index, int requested) = 0;
    // 007BCBFD / 007BCC09: the store itself.
    virtual void write_release_order_count(int index, int count) = 0;
};

struct ReleaseOrderIssueResult {
    bool walk_found_device = false;
    bool gate_passed = false;
    int units_raised = 0;
    int units_zeroed = 0;
};

// 007C0D90 -> 007EEF30 -> 007BCBE0, the whole issue path as one sequence.
// BSP_PlaneTickElement_FixedStep 007CE040 runs it at 007C0D90's call site.
ReleaseOrderIssueResult torpedo_issue_release_orders_007c0d90(
    TorpedoReleaseOrderHost& host);

// ---------------------------------------------------------------------------
// ctl+370h, the pilot control block's attack mode. Three values: 0 hold,
// 1 attack, 2 forced. 009D3F69/009D3F71 and 009D40CB/009D40D2 both send an
// engaged torpedo task to `prepare` only while it is 0, and 009D49A0 arms the
// release countdown only in `prepare`.
//
// An exhaustive census of every store form at a +370h field (28 sites image
// wide, positive control on the two known ones) finds exactly three on this
// object, and an exhaustive rel32 scan of the two setters finds four call
// sites. docs/TORPEDO_ATTACK_MODE.md has the table.
// ---------------------------------------------------------------------------
namespace pilot_attack_mode {
inline constexpr int kHold = 0;
inline constexpr int kAttack = 1;
inline constexpr int kForced = 2;
inline constexpr int kMessageSetMode = 0xBC;   // 007F0204/007F01E8 -> 007F005B
inline constexpr float kForcedRearmSeconds = 2.0f;  // 00CE3958, 009A2872
}  // namespace pilot_attack_mode

// 007ED3F0-007ED3FA, `void __thiscall(ctl, int mode)`, RET 4. An unconditional
// store at 007ED3F4.
int pilot_control_set_attack_mode_007ed3f0(int mode) noexcept;

// 007ED430-007ED442, `void __thiscall(ctl, int mode)`, RET 4. 007ED434 compares
// and 007ED43A JGE skips, so it only ever raises: a signed `<`.
int pilot_control_raise_attack_mode_007ed430(int current, int mode) noexcept;

// 007F0030's message arm at 007F005B-007F0068, reached for message id BCh.
// 007F005B loads the byte payload at msg+20h, and the NEG/SBB/AND 2 idiom turns
// it into 2 when non-zero and 0 when zero. This is the only route to 0 that
// does not go through the closetoship task.
int pilot_control_attack_mode_from_message_007f0068(
    unsigned char payload) noexcept;

struct CloseToShipModeCountdown {
    float timer_550 = 0.0f;    // task+550h
    bool latch_43c = false;    // task+43Ch
    int mode_370 = 0;          // ctl+370h, read only on the expired arm
    float dt = 0.0f;
};

struct CloseToShipModeResult {
    float timer_550 = 0.0f;
    bool latch_43c = false;
    bool lower_to_hold = false;   // 009A285E, the tail jump to 007ED3F0(ctl, 0)
    bool rearmed = false;         // 009A2881
};

// 009A2810-009A288A, `void __thiscall(task, float dt)`, RET 4. The countdown
// the closetoship task's vtable +64h arm (009A2B00, at 009A2B56) runs. It is
// the ONLY path in the image that calls 007ED3F0 with 0.
CloseToShipModeResult closetoship_attack_mode_countdown_009a2810(
    const CloseToShipModeCountdown& in) noexcept;

}  // namespace bsp

#endif  // BSP_TORPEDO_RELEASE_ORDERS_HPP
