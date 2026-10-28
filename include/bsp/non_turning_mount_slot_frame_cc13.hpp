#pragma once

// Packet cc13-mountfix: the node-0 pose of a rack or base gun. Header-only switch.

namespace bsp {

// kNonTurningMountSlotFrameCc13Bound (OFF): for a rack (BOMBPLATFORM) or base gun (PLANEGUN,
// Rapid_Fixed_Slave_Gun) (non-turning, and not
// MCatapult: node0_keeps_slot_frame in game_hosts_gunnery.cpp), node 0's local in
// device_hierarchy_node_world is the full platform slot frame, not the translation turned
// by 00859550.
//
// Image:
//   0095F500 slot pass (0095FB4E-0095FDDE): rows start as identity (0095FB4E-0095FBC7), row 3
//     = p0 (0095FBF3-0095FC13), row 2 = p2 - p0 (0095FC67-0095FC98), row 1 = (p2 - p0) x
//     (p1 - p0) (0095FD4E-0095FDD8), row 0 stays (1,0,0); then 0085DC80 on the matrix
//     (0095FDDE), which is copied to platform+4Ch.
//   0072DD20 copies platform+4Ch (REP MOVSD, 0072DD60-0072DD6A) as the device model's
//     node-0 local.
//   The per-tick pose (vt[DCh]): TurningGun 0085A270 applies 00859550 RotY/RotX. Racks
//     (00CF96A8 / 00CF9918) and base guns (00CFE0A8 / 00CFE308) use 0072B2D0, which never
//     sets a local, so their node 0 keeps the slot frame.
//   CORRECTION (rev7): MCatapult (00CFAAB8) is not in that set. Its +3BCh is node 0, the
//     model root ([[[gun+360h]+160h]+0Ch], 006ECD80..97, the same root load as
//     0072E973..98A; 006ECE2C..78 builds a node list when there is none). "base" goes to
//     +3C4h and "barrel" to +3C8h (006ECDEF..E21). 006EC620 = 0072B2D0 + 006EC240, and
//     006EC240 is 00859550's code with other angle fields. With a barrel (006EC264..2DF)
//     it rotates node 0 by RotY(-[+4B4h] = rest horz) via 00B646E0, keeping the
//     translation, as the host's RotY(-horz), then puts RotX(-[+4B8h]) on +3C8h. Without
//     a barrel (006EC369..437) both go on node 0. It runs every vt[DCh] tick, gated only by
//     +3BCh (006EC251). MCatapult_InitDevice sets +4B4h = platform+94h (RestAngles horz)
//     and +4B8h = platform+90h, each 0 when FLT_MAX (006ECCC6..CFD, 006ECD0F..D3C).
//     MCatapult +480h is its ammo count, not an angle: 006EB660 reads it, and the 006EBD00
//     reset copies +484h into it.
//   So the host's OFF pose already matches a catapult, including the vertical rest: angles
//     = rest_horz / rest_vert ("rh"/"rv" loaded unnegated, absent = 0 like FLT_MAX ->
//     0), and 00859550's RotX(-vert) goes on the barrel, or on node 0 without one.
//
// Host (OFF): root_local = translation(mount_local), then 00859550 with (-horz, vert) for
// every device. HOST-ONLY for racks and base guns: they get RotY(-horz) (and RotX(-vert)
// without a barrel) on node 0, and lose the slot frame's 3x3.
//
// ON: only the node-0 pose of racks and base guns changes. angles.horz stays as it is for
// every other reader (aim, logs). 00859550 still runs for the barrel local, so a rack's
// barrel pitch is the host's as before (LABELLED; the image's 0072B2D0 does not turn it
// either, but changing it is outside this packet).
// The frame is GameGunRow::mount_basis_slot. It is filled at the ship/plane mount
// (kShipPlatformAttachmentBound) and the land mount (kLandPlatformAttachmentBound). It stays
// identity where there is no slot (007F7110 identity).
//
// PC24: the slot cross product is float multiply/subtract. At PC24 each x87 FMUL/FSUBRP
// rounds to 24 bits, the same as float. 0085DC80 is the existing
// orthonormalize_pose_matrix_0085dc80; its lengths and cross products round per op only
// under kPc24FlightRenderCc13Bound. OFF, it carries that switch's documented double-chain
// debt.
//
// Census (all ship/plane/land models of this installation): 3275 slots in 210 of 683
// models, 59 rotated, 0 degenerate. The rotated slots of racks and base guns are the plane
// BombPlatform keys 50/51 on 8 plane types (1-3 degrees of pitch).
// ON summary line: slot frames, rotated, nonzero horz at mount, and "aimed" (racks and base
// guns whose angles the aim step moved off rest; their OFF node 0 moved with them).
//
// Movers when ON: the 1-3 degree plane BombPlatform racks:
//   - their C1/C2 device hit and blast frames (device_hit_world_frame uses node 0's world);
//   - their render seam (GameGunneryHost hierarchy node);
//   - gun_barrel_muzzle_world_00730762 for any rack or base gun the aim step turned
//     ("aimed" above).
// Racks fire through 006EC8E0 / 006E4D50 via 00727E30 slot 1D8h, not 00730160.
inline constexpr bool kNonTurningMountSlotFrameCc13Bound = false;

}  // namespace bsp
