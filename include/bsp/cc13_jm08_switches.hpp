#pragma once

// Packet cc13-jm08 (read-only audit of JM08's leaderless landing ships, cc9 request).
//
namespace bsp {

// True: after a destroyed ship leaves its formation group (0077BD70, behind
// kDeadMemberLeavesGroupBound), the units host republishes the 007788B0 / 007788D0
// pair to the commands host for the leaving unit and every remaining member. In the
// image both predicates read unit+284h and group+14h live, so a leader hand-over is
// seen at once by 00836920.
//
// What the image does at a LEADER's death (corrected after review of 8fb2ae456):
// - 0077BDDE..0077BE02, before 0070D0C0 at 0077BE40: the dying unit's director
//   (vt114h = 0080E150, [unit+738h]) gets vt5Ch(successor) = 0071C150. While the
//   dying unit is still group+14h (00778890), 0071C150 calls the successor's
//   director vt4Ch(dying director) = 00836680, which runs 00720450: allowFire /
//   allowMove (+3Ch/+3Dh), the previous-command record (+16Ch..+184h), the mode
//   (+30h), the override cleared (0071E610), the ten command records copied (for a
//   kind-6 owner a `land` 00E08FA0 becomes `attackmove` 00E08F78, 0072058A..
//   00720607), stages reset; then 00836240 when src vt48h(2) and src+23Ch.
//   So the NEW LEADER INHERITS THE DEAD LEADER'S QUEUE in place of its own
//   `follow`. In JM08 that queue is Missouri's scene `cruise`, so the successor
//   begins a cruise (its own heading and ordered throttle, 00835E17) rather than
//   stopping. That copy is NOT modelled by this switch (see the report: the host's
//   director state lacks part of the copied record).
// - Members: the follow arm 00836ADC..00836B3B ends each member's `follow` on the
//   dead leader (00836B1A, leader != stored target), and the idle tail 00836DC9
//   re-issues `follow` 00E08F60 on the new leader (00836E13..00836E38).
// - The idle tail's cruise/stop pair (00836E3F) applies only to a leader whose
//   queue empties. Its +28h test follows 00835BF0(1): +28h is reset to -1.0f only
//   when it is >= 0 and int(clock) - +28h > 1; then >= 0.0f issues `cruise`
//   00E08F70, else `stop` 00E08F88.
// False: the commands host keeps the pair pushed at the join (set_unit_formation
// from 0077FA8D), so after a hand-over every member, the new leader included, keeps
// a `follow` aimed at the dead leader, as before.
// No 00BD2F10 draw on any of these paths.
inline constexpr bool kDeadLeaderRepublishesFollowerPairBound = true;  // cc9-sweep2b: paired at 292c293bd on 7 rows (j8v3 loses, see SHIP_AI_OPEN_ITEMS 331)

// True: when the dying unit still leads its group (00778890 at 0071C156) and a
// successor exists (0071C163), leave_group_on_destroy_0077bd70 runs
// GameCommandsHost::copy_director_state_00720450(successor, unit) BEFORE the
// SetLeader (0077BDDE..0077BE02 -> 0071C150 -> 00836680 -> 00720450), so the new
// leader inherits the dead leader's queue. Copied, across three hosts:
// - commands host (copy_director_state_00720450): the mode (+30h), the ten slots
//   and descriptors (+54h..+16Bh), the override cleared (0071E610 through
//   clear_override_in_place, then +188h..+1A0h zeroed), the stage pairs reset
//   (+44h/+48h, +50h) and the slot-0 path object;
// - gunnery host (copy_director_allow_bytes_00720450): +3Ch/+3Dh raw;
// - ship-AI host (copy_fire_target_00836680): 00836680's tail, 00836240 on dst
//   when src vtable[48h](2) (00836790) and src+23Ch.
// Still NOT modelled:
// - +16Ch..+184h, the record 00720850 saves the cleared slot 0 into (0072093A..
//   00720986). No host field. Census (packet cc13-jm08, fragment at 00720450):
//   its only readers are 00720450 itself and 0084DB50 (vtable 00D0BD98 slot 6Ch,
//   the PLANE squadron block), so leaving it out changes no ship.
// - +4Ch, the override pair's byte: no host field.
// - Slots 1..9's path objects (0071FD10 per slot): the host carries slot 0's only.
// - The `land` -> `attackmove` 00E08F78 rewrite for a kind-6 owner (0072058A..
//   00720607): needs 0071DB90's world list; a `land` is copied as is and counted
//   (director_copy_land_kept).
// Ordering. Image: 0077D1A0 posts the 77h at the kill through 0077C2A0 ->
// 0076E520 (appended to session+24Ch; 0076C600 drains it FIFO in the pump,
// 00778542 / 00875E91). A think-phase death (damage control: 00825F20 from the
// entity think 00875E64) is copied at that step's pump, so the destroy flush's
// 0071DDB0 (00875EC9) finds the successor no longer observing the dead leader and
// posts it no 5Dh: the inherited head survives. A death in the post-pump hit drain
// (00875EC4) leaves both the 77h and the successor's 5Dh for the next pump, copy
// first, so the 5Dh ends the inherited head. Host: every kill runs after the
// fan-out and posts the 5Dh at once, which drains at the next pump before this
// copy at the next flush, so the head always survives (one step late; the extra
// 00720850(0) on the successor's old head runs 00835BF0(1) first). Right for a
// think-phase death such as JM08's Missouri, wrong for a post-pump hit death.
// True: the kill site's phase (table below) decides. The image's 77h delivery is
// the whole 0077BD70 (0071C150, SetLeader 0070D0C0, DetachMember 0070E4C0); for a
// ship (whose destroy leave this host otherwise runs at the row-15 flush,
// kDeadMemberLeavesGroupBound) the whole leave runs where the image's pump has
// it: in place at a BeforePump kill, from the posted entry at a HitDrainAfterPump
// one; the flush then finds +284h clear. Non-ships keep the copy alone. The
// flush's leave (flush_pending_entity_queues_009273a0) also needs the ship's
// unit_identity and scene-node flags and skips a ship without them; the whole
// leave here does not test them, so such a ship (a host artefact) now leaves at
// its kill where under OFF the flush never removed it from its group. Believed
// closer to the image, where the destroy 0077D1A0 itself posts the leave (0077D223
// -> 0077C980); those host-only fields have no image counterpart on that path. At a
// HitDrainAfterPump kill the host loopback (FIFO with kSetCommandQueueDelayBound)
// gets the 77h entry first (leader_death_handover_at_kill), then the 5Dh
// (0071DDB0, after it in kill_unit). Lua runs before the next pump, so a LeaveFormation / DisbandFormation
// that meets that posted copy (the unit's own or its leader's) is posted third, as
// 0077C980 / 0077CA60 queue it, and runs at the pump like the image's; every other
// Lua leave stays synchronous.
// The image does deliver a HitDrainAfterPump 77h at the next pump (cc13-deadleadnits):
// - 0076E520 stamps msg+18h with D+174h (0076E5B9) and +0Ch with the current tick
//   (0076E5C6), so the message is due at once.
// - At the next pump, 00780670 resolves D at 0078077E. The handle is released only in
//   the entity destructor 009287B0.
// - The row-15 flush runs only the ship's destroy hook. That is vt+74h 00926390, which
//   tail-calls vt+7Ch 00824B60 OnWrecked. Neither frees D.
// - 00903610 frees an entity on the second expiry pass after it is marked. Row 16 of
//   the next step comes after that step's pump.
// - D+BDh, set by 00923840, has no clearing writer in the entity code. So D passes
//   00780789, and 007807B8 dispatches the 77h through 00780120, which runs the copy.
// - 007807C4 is the arm for a recipient that does not resolve. It keeps the message
//   while it is less than 10 ticks old, then drops it. It is not the copy.
// So the host's always-deliver path matches the image here, and no sub-switch is needed.
// False:the successor keeps its own queue, as before. No 00BD2F10 draw.
inline constexpr bool kDeadLeaderDirectorStateCopyBound = true;  // ON: JM08 San Francisco takes Nevada's attackmove at 527.49 s (SHIP_AI 304)

// The image phase a host kill site stands for, as far as the 77h / 5Dh order
// goes (see kDeadLeaderDirectorStateCopyBound). Only a death between the pump
// (row 9, 00875E91) and the flush (row 15, 00875EC9) ends the inherited head; in
// practice the second hit drain, row 14 (00926700 at 00875EC4), whose records
// were pushed in rows 7..13 (the pump's message arms among them).
//
// Host kill sites (src/game_hosts_gunnery.cpp), mapped:
// | host site                              | phase             | image evidence                                  |
// |----------------------------------------|-------------------|-------------------------------------------------|
// | run_damage_control water/fire death    | BeforePump        | 00825F20 -> 00826B84 -> 008160B0 -> 0093CA20 -> |
// |                                        |                   | 0093C120/0093C210; 00825F20 is tick-vtable slot |
// |                                        |                   | +8h (00CF9068 ...), which the job waves call    |
// |                                        |                   | (wave 3 00874FED..00874FF6 CALL [vt+8]); the    |
// |                                        |                   | waves 00875CC0..00875DFC precede the fan-out    |
// | apply_hit / apply_impact_blast from    | BeforePump        | 0084BC60 / 0084BAD0 push (00926E80) from the    |
// | run_projectiles                        |                   | projectile ticks 006E6490 / 0070C370 / 006E1300 |
// |                                        |                   | (tick slot +8h, a wave) via 0084C430 -> 0084BF00|
// |                                        |                   | drained at row 6 (00875E44)                     |
// | plane_crash_impact_0084bc60 (blast and | BeforePump        | 0084BC60 from 007C0910 from 007CE040, the plane |
// | the plane's own Kill)                  |                   | tick element's slot +8h (00D05EDC and derived)  |
// | kamikaze_detonate_00819a20 blast       | HitDrainAfterPump | message 70h (0080FF30, built at 00814647 /      |
// |                                        |                   | 008146AB in 008145B0 from the contact 009377E0) |
// |                                        |                   | routed 0077C2A0 flags 7 -> 0076E520 -> drained  |
// |                                        |                   | by 0076C600 in the pump; 00821E80 case 70h at   |
// |                                        |                   | 0082218D -> 00819A20 pushes 00819BA7 / 0084BBF9,|
// |                                        |                   | drained at row 14                               |
// Unmapped (LABELLED gap: the copy and the leave wait for the next step's flush,
// leave_group_on_destroy_0077bd70 at game_hosts_ready.cpp's 0092639B site; any
// leave of the successor S before then, Lua or ship_ai, runs while the dead
// leader still leads, so S leaves as a member with its own queue and the flush
// hands the group to S2 with S2's own queue, where the image's S, already the
// leader, would pass its inherited queue on to S2; the dead leader's leave is not
// pulled forward because the image phase, and so whether the 5Dh precedes the
// copy, is unknown):
// the explosion death in run_damage_control (0081B630, its ticker unread);
// land_master_add_damage_0095da00 and the airfield yard kill (00747170, the vt+7Ch
// wreck handler, a flush-phase kill, which keeps the head anyway); script damage
// (apply_script_damage_0095da00); every kill_unit_00926d90 / destroy_unit_0077d1a0
// caller outside the gunnery host (Lua and message-driven kills can come from a
// row-9 or row-15 callback).
enum class DeadLeaderDeathPhase {
    Unmapped,           // not traced: leave_group runs the copy at the flush
    BeforePump,         // copied at the next pump, before the flush's 0071DDB0
    HitDrainAfterPump,  // 77h and the successor's 5Dh both wait for the next pump
};

// True: ship_ai_wake_decompose_00811180 (src/ship_ai_wake_trail.cpp) returns the
// image's along-track distance. 00811180 picks the segment through the nearest
// sample k (00811482..00811584: k == 0 takes head/k1, k == 27h takes k-1/k, else
// the newer neighbour unless its float squared 3D distance is > the older one's;
// 0081157B JBE also keeps the newer one on an unordered compare),
// stores the projection dot(sample[b] - point, normalize(sample[b] - sample[a]))
// at 00811658 into [ESP+48h], and the leg loops 0081166A..00811721 add the b+1
// legs of slots head..head-b onto that same slot. So a member on a trail sample
// gets the full arc length (LST 01 in JM08: 1000, not 950), and a member ahead of
// the leader's head gets a NEGATIVE along (Helena: -500), which 00810630's
// head branch (along <= 0) extrapolates ahead of the leader.
// False: along is the sum of the first `k` legs from the head with no projection
// term, as before: on-sample stations sit one leg (50 m) short and every station
// ahead of the head collapses onto the leader's own point (JM08 Helena's station
// becomes Missouri, which stalls the column at about 120-180 s).
// The across convention (0070D290's left normal from the base at `along`) is
// unchanged. Main-thread x87 (24-bit PC) is modelled as float ops in the image's
// order. No 00BD2F10 draw.
inline constexpr bool kWakeDecomposeSegmentProjectionBound = true;  // ON: JM08/USN04 followers move from step 10 (SHIP_AI 304)

}  // namespace bsp
