# A scripted player helm: the image's path (packet cc9_scripted_helm)

2026-09-23. Read-only so far. Names are hypotheses; offsets are unit-relative unless stated.

## 1. Answer

**On USN04 the image refuses the player the Lexington's helm.** The role-1 transfer the HUD sends
(0077C470(unit, 2, 1) at 0064B9A6) is accepted only when the unit's role-1 permission word
unit+18Ch is 9 (PLAYER_ANY) or the player's own slot. usn_19_coralus.lua sets every Lexington role
to PLAYER_AI (line 458) and then opens only EROLF_AA_FLAK, EROLF_AA_MACHINEGUN and EROLE_CAPTAIN
(= 1, the captain's flag bit) to PLAYER_ANY (line 459). Role 1 is EROLF_PILOT (= 2,
scripts/global/luamw_init.lua lines 109-110). It stays at 8, and line 866 resets everything to
PLAYER_AI again. So the transfer fails at the 4Bh receive arm. The player never holds role 1, and
0064B870 never issues a helm order for the Lexington.

**The same read corrects docs/CONTROLLED_UNIT_HELM.md.** unit+184h, the byte that forces a ship
into `cruise` (009F3DF3), is set only when a player **takes role 1**, not role 0 (00780214, below).
On USN04 that never happens. So in the image the idle player's Lexington does not have +184h set.
It is not forced into cruise, and it follows its authored orders under the AI. The host sets its
player-controlled answer for the controlled unit and keeps the Lexington in cruise arm 2, where it
coasts to rest. That is the idle E2 reference's stationary carrier, and on this reading it is not
the image's behaviour.

No harness option was built. Both findings change what the packet should do; see section 5.

## 2. The transfer, 0077C470 and the 4Bh arm

**0077C470**, `__thiscall(unit, uint roleMask, int take)`, `RET 8`:
- Gated on `game+5D4h > 0Ch` or `game+216Ch`, and on session mode `game+1FE4h == 0` (or the client
  test).
- It builds message 4Bh through `BSP_SessionMessage_ConstructBase(4Bh)`, with vtable 00D02C90 and
  these fields:
  - +04h = 1;
  - +18h, +1Ah, +1Ch and +20h = 0;
  - **+24h = roleMask**;
  - **+28h = the local player slot `game+18ECh`**;
  - **+2Ch = take**;
  - +30h = 0.
- It routes the message with `BSP_Session_RouteMessage(msg, 0, 0)` on the unit.

**The HUD call site, 0064B97A-0064B9BD.** It calls `0077C470(unit, 2, 1)` when all of these hold:
- the thrust axis is above 0.1, or the held flag is set;
- unit+1130h == 0;
- `00927F30(unit, 0)`: the local player holds role 0;
- `!00927F30(unit, 1)`: the player does not yet hold role 1.

It then seeds the HUD levers without waiting for the answer: HUD+28h = unit+984h and
HUD+24h = unit+980h.

**The receive, 00780120's 4Bh arm (00780162-007803D9), read here.** With msg+20h == 0:
- **The +184h store (007801F2-00780235):**
  - take == 1, mask bit 2 (role 1) and `0059BBD0(unit, 1, slot)` give `unit+184h = 1` (00780214);
  - take == 0, mask bit 1 and `unit+1ACh == slot` give `unit+184h = 0` (00780235), then
    `0080E290` for a kind-6 unit.
- **The per-role loop,** i = 0..8 over the mask bits. For take == 1 it calls
  `unit->vtable[154h](i, slot)` and relays the message (0077C7B0) only when both hold:
  - the current holder `unit+1ACh+i*4` is 8 or AI-held (`BSP_PartySlot_IsAiHeld`);
  - the permission `unit+188h+i*4` is 9 or equal to the slot.
- Role 0 also rebinds the player record's unit (+4Ch).

**0059BBD0**, `__thiscall(unit, int role, int slot)`, body read whole: it returns
`unit+188h+role*4 == 9 || == slot`.

**The permission words.** They are written by `unit->vtable[148h](mask, value)`. For the instance
vtable 00CFC3D0 that slot is 0077F360, which calls 00927D20. That function stores `unit+188h+i*4 =
value` for every mask bit (`piVar4[-9]` with `piVar4 = unit+1ACh`). SetRoleAvailable (008AB850)
reaches it at 008ABA51 in session mode 0.

**The +184h writer census.** A byte-store scan for `[reg+184h]` over MOV/C6/88/89/C7 forms finds
these unit writers:
- 00928701, the constructor;
- 00780214, set;
- 00780235 and 00780439, clear;
- 009281D6, clear.

Every other hit is on part instances or screens. So role-1 acceptance is the only producer of 1.

## 3. The helm message, 0064B870 to 00816A40

- **The issue.** Each frame, while `00927F30(unit, 1)` holds, 0064B870 quantizes HUD+24h
  (thrust, quarter steps) and HUD+28h (turn, sixth steps). It issues `00816A40(unit, thrust, turn,
  kind 0)` at 0064BB12 (docs/UNIT_ORDER_RECORD.md).
- **00816A40** (00816A40-00816AF1, `RET 0Ch`) builds the 20h-byte record (00815440 clamps both to
  [-2, +2]).
  - It publishes the record through 0080DAD0 into ring slot `[unit+97Ch]`. 0080DAD0 clears the
    slot's +08h, which marks it authoritative, and mirrors it into unit+994h/+998h/+99Ch.
  - **Only in session mode 2** (00816A8D) does it also build message **8Eh** (0075B430 at
    00816AA1). The message carries the record at +1Ch and is sent through 0077C2A0.
- **008141A0** (through arm 00821EBE) applies an 8Eh on the receiving peer. It acts only when the
  sender holds the unit's helm slot, and it backfills with 00812FA0 at the lag-compensated age.
- **In single player (mode 0, this host's value) no 8Eh is sent.** The authoritative slot and the
  confirmed pair come straight from 0080DAD0.
- The existing `--order` option already models that part: `issue_into_ring` re-issues through
  00816A40 every step.
- With role 1 held, cruise takes arm 1, the helm bypass (009E13B4). It sets blk+3F5h, and
  009F3F80 then skips its ring write (009F3FF2). The host honours that byte
  (`drive_order_ring_009f3f80`).

## 4. What a scripted helm would have to fake

A faithful role-1 transfer on USN04 is refused. A harness that "performs the transfer as the image
does" would do nothing. Making the carrier move by helm needs one of these:
- **(a)** A scenario override that opens EROLF_PILOT to PLAYER_ANY on the Lexington, as a
  labelled scenario change equivalent to editing line 459. Then the transfer, the +184h store, the
  cruise arm 1 bypass and the per-frame 00816A40 all run as in the image.
- **(b)** Forcing the role-1 slot without the permission, which is not the image's behaviour.

## 5. Decision needed

- **The idle reference.** On this reading, the image's idle E2 Lexington is AI-driven, with no
  +184h. It is not the host's coasting carrier. The host's "controlled unit implies +184h" stand-in
  would need replacing with the 4Bh role bookkeeping. That is a host fidelity change to the
  reference, not a harness scenario, and it touches the controlled-unit and cruise-arm code.
- **The scripted helm.** It is only possible as scenario (a).
