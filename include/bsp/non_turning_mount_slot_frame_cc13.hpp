#pragma once

// Packet cc13-mountfix: the node-0 pose of a non-turning device class. Header-only switch.

namespace bsp {

// kNonTurningMountSlotFrameCc13Bound (OFF): for a gun that does not answer TurningGun (the
// host's gun_answers_turning_22h is false: PLANEGUN, BOMBPLATFORM, CATAPULT), node 0's local
// in device_hierarchy_node_world is the full platform slot frame, not the translation turned
// by 00859550.
//
// Image:
//   0095F500 slot pass (0095FB4E-0095FDDE): rows start as identity (0095FB4E-0095FBC7), row 3
//     = p0 (0095FBF3-0095FC13), row 2 = p2 - p0 (0095FC67-0095FC98), row 1 = (p2 - p0) x
//     (p1 - p0) (0095FD4E-0095FDD8), row 0 stays (1,0,0); then 0085DC80 on the matrix
//     (0095FDDE), which is copied to platform+4Ch.
//   0072DD20 copies platform+4Ch (REP MOVSD, 0072DD60-0072DD6A) as the device model's
//     node-0 local.
//   The per-tick pose (vt[DCh]) applies 00859550 RotY/RotX only for TurningGun (0085A270).
//     Racks (00CF96A8 / 00CF9918) and base guns (00CFE0A8 / 00CFE308) use 0072B2D0, and
//     MCatapult (00CFAAB8) uses 006EC620 = 0072B2D0 + 006EC240. Neither touches node 0's
//     local. 006EC240 poses only the +3BCh launch-plane node and reads +3C8h/+4B4h. It never
//     reads +480h.
//   MCatapult +480h is its ammo count, not an angle: 006EB660 reads it, and the 006EBD00
//     reset copies +484h into it. No angle writer for it exists.
//
// Host (OFF): root_local = translation(mount_local), then 00859550 with (-horz, vert) for
// every device. HOST-ONLY: for a catapult, angles.horz is its RestAngles horizontal rest, so
// the host turns node 0 by RotY(-rest). 52 catapult platforms carry a nonzero rest (0.055 to
// +-2.79 rad).
//
// ON: only the node-0 pose changes. angles.horz = rest stays as it is for every other reader
// (launch direction, aim, logs). 00859550 still runs for the barrel local, so a
// non-turning class's barrel pitch is the host's as before (LABELLED; the image's 0072B2D0
// does not turn it either, but changing it is outside this packet).
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
// models, 59 rotated, 0 degenerate. Non-turning rotated slots are the plane BombPlatform keys
// 50/51 on 8 plane types (1-3 degrees of pitch) and the I-400 Catapult key 200 (12
// degrees).
//
// Movers when ON:
//   - C1/C2 device hit and blast frames (kGunDeviceHitDamageCc13Bound,
//     kGunDeviceBlastDamageCc13Bound; device_hit_world_frame uses node 0's world): catapults on
//     the 52 rest-angled platforms, the I-400 catapult and the plane rack tilt;
//   - the render seam (GameGunneryHost hierarchy node).
// No muzzle mover: catapults and racks fire through 006EC8E0 / 006E4D50 via 00727E30 slot
// 1D8h, not 00730160. Plane guns use the muzzle path, but none has a rotated slot or a rest
// angle.
inline constexpr bool kNonTurningMountSlotFrameCc13Bound = false;

}  // namespace bsp
