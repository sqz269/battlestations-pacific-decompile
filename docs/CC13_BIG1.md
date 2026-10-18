# cc13-big1: the large rows deferred by cc13-small2

Packet cc13-big1 takes the large rows that docs/CC13_SMALL2.md deferred. Code is in
`src/cc13_big1.cpp` and `include/bsp/cc13_big1.hpp`. It is registered on bsp_core and
is not wired into the game.

Every name is a hypothesis, not a recovered symbol. Each body is a new C++ interface over
raw native records, not a drop-in replacement of the original ABI. Every call the image
makes goes through a named `Big1Host` method, at the same point and with the same
arguments. Checked-iterator failures go to the CRT invalid-parameter hook at the same
points as in the image.

Status: reconstructed and build-tested. Nothing here is fixture-tested, ABI-compatible or
game-validated.

## Batch 1

| Address | Name | ABI | Notes |
| --- | --- | --- | --- |
| 004C7AA0 | BSP_World_GetBorderZoneApproach | thiscall(world; out, position, radius, side) RET 10h | 004C7730 for the nearest zone; out = {zone, direction, edge point - radius * normalize(direction), normalize(2 * direction + the x/z unit vector to the corner average)} |
| 005AAC00 | BSP_HudMapIcon_RefreshCarriedUnitIcons | thiscall(icon; template widget) RET 4, EAX | builds a std::set of the HUD icon indices of the classes that the unit's support sites supply; clones one widget per index onto icon+B8h; returns the set size |
| 0064AE20 | BSP_HudShipScreen45_ApplyUnit | fastcall(screen) | VehicleClass[..].NoRepairGUI; throttle seed +44h; gauge flags +104h..+107h and their widgets; 0064AAD0, 0064A9F0 and 0064A960 |

Precision and register notes:

- **004C7AA0.**
  - ECX is not read. It is passed through to 004C7730, which uses it as the world.
  - The y term of the first length is the FLDZ left on the x87 stack at 004C7B2A, so
    the length is (x² + 0·0) + z².
  - Both reciprocals follow FCOMI/JBE, so a zero or NaN length gives 0.
  - The DC-form ops (004C7B2C, 004C7BBA, 004C7C08, 004C7C7C) were read from the bytes
    with tools/x87_dc_form_scan.py.
  - Constants: 0.25 (00D7A348), 0.0 (00D7A258) and 2.0 (00D7A308), all doubles.
  - Modelled at 24-bit precision. The only caller, 0094BFF0, may run on a fixed-step
    job. At 53 bits the unstored partial sums and the constant products would round
    once, at their stores. Which precision applies is uncertain.
- **005AAC00.**
  - The set is a std::set<unsigned> (005A18E0 compares unsigned keys). Its node key at
    +0Ch is passed to vtable[88h] as a zero-extended word.
  - The two list teardowns are inlined in the image and are reproduced node by node.
  - The set's proxy dword is value-initialised here; the image leaves it unset.
  - When the clone is null, the iterator is still advanced.
- **0064AE20.**
  - The constants are 0.5 (00D7A280), 1.5 (00CE3D78) and the float 1.0 (00D7A24C).
  - Each step of (unit+980h + 0.5) / 1.5 rounds to float at 24 bits. A NaN passes both
    clamps unchanged (FCOMIP/JBE, then COMISS/JBE).
  - The bytes pushed for vtable[34h] carry stale upper EAX bits. Only the low byte is
    modelled.
  - +104h is written only on the vtable[5Ch](8) arm.

## Batch 2

| Address | Name | ABI | Notes |
| --- | --- | --- | --- |
| 00641910 | BSP_InGameHudMarkers_AddKind1CMarker | thiscall(markers; unit, is_self, is_friendly, is_target) RET 10h | the IsKindOf(1Ch) sibling of 00642040; on screen a 0063C620 box marker, off screen a 0063D1E0 arrow marker |
| 005E12A0 | BSP_MultiGameLobby_LoadWeatherTable | thiscall(lobby screen) | a private Lua state runs Weather.lua; each Weathers entry with a UniqueID is pushed onto screen+2E0h with its SubScenes ID and Text strings |

Notes:

- **00641910.**
  - It takes `Cc13Pkt642Host` (include/bsp/cc13_pkt642.hpp): every callee already has a
    hook there. 0063C620 and 0063D1E0 are called as the pkt642 bodies, as 00642040 does.
  - The clip point's y is [class+A8h] + y. The bounds (0063A6C0) and the off-screen
    arrow use the raw unit position.
  - record+4 is left unwritten when is_friendly and is_target are both set. Otherwise
    it is set when neither is_target nor is_self is set.
  - On screen, the colour is read from 0077A2F0's out buffer. Off screen, it is read
    through the returned pointer, after 00923BE0 and the copy of the distance string.
  - The marker's show flag is set when markers+1Ch is set and the unit's vtable[140h]
    value equals that object's.
  - 0063D1E0 receives is_self as a whole dword.
- **005E12A0.**
  - The LuaStateOwner is a 4CCh-byte stack buffer, the room between [B+E8h] and the EH
    record.
  - The pushed 24h-byte entry has only its two vectors' first/last/end dwords zeroed;
    +0, +4 and +14h are not written.
  - The id is stored after the checked subscript of the last element, even when that
    check calls 00BF6713.

## Batch 3

| Address | Name | ABI | Notes |
| --- | --- | --- | --- |
| 006697E0 | BSP_HudSupportManager_CollectCatalog (new name) | fastcall(screen), EBP frame | for the current team's units in the selected row's unit list: copies the row map (+40h to +50h, +4Ch), reports InferiorFailure units, and appends their 0084D560 supply classes to list +370h |
| 0066A8A0 | BSP_HudSupportManager_RebuildLists (new name) | fastcall(screen) | rebuilds entry list +3E0h from own units (kinds 6 and 0Fh) and the [00F89B3C] records; unit lists +3ACh, +3B8h and +3C4h; the scroll reset; copies +3E0h to +3ECh |

Notes:

- **006697E0.**
  - The selected row is the checked iterator at screen+350h/354h. Its value
    {+8h, a unit list at +0Ch, +18h} is copied, and the list is copied again; the first
    copy is freed by 005A0B80.
  - A unit is taken when its +54h or its +344h object's +54h is the team.
  - The map +40h is probed with 0065B160, and the node is then found again by a linear
    scan from begin.
  - Map +50h is replaced only when its node is a different object: 006641B0 erases
    begin..end, then 0065CE20 copies.
  - The InferiorFailure report goes through unit+3Ch's vtable[198h]. Its bool out skips
    that unit's supply classes.
  - The loop runs while its counter is below the copy's current size.
- **0066A8A0.**
  - ingame_gui.cpp calls this routine `rebuild_list_c_0066a8a0` and calls +3E0h "list C".
    The doc names the lists A (+3E0h), B (+3ACh), C (+3B8h), D (+3C4h) and E (+3ECh).
  - Entries are inserted at the front. The kind-6 insert is inlined; the kind-0Fh insert
    calls 006652C0.
  - The C/B scan's owner checks compare an address with itself (CMP EAX,EAX) and cannot
    fire, so they are omitted.
  - 006681D0 and 00668350 take a trailing by-value tag dword whose stack contents are
    stale. It is passed as 0.
  - The 14h-byte entry's bytes beyond those written keep whatever the stack held.

## Batch 4

| Address | Name | ABI | Notes |
| --- | --- | --- | --- |
| 00674B80 | BSP_HudSupportManagerScreen_Enter | fastcall(screen), EBP frame, EH handler 00C7C5CA | the screen's Enter: panel reset, the SM1STUSE hint, the selected row, one of the unit / catalog / map arms for the name text +170h and the icons +1CCh and +158h, then the points text +1A8h and the open sound |
| 0066EA60 | BSP_HudSupportManagerScreen_Register | fastcall(screen), EBP frame, EH handler 00C7BE50 | vtable +10h: the GUI_support_full page, widget copies, eight environment samples into +10h..+2Ch, the radar-shot texture, size and UV rect on widget +1C0h, the resets |

Notes:

- **00674B80.**
  - The whole body (1212 listing lines) was read from the listing.
  - The constants are 2.0f (00CE3958, modes 4..7) and 0.35f (00CF6560). The strings, read
    from the PE, are "SM1STUSE" (00CF6B8C), "globals.unitclass_spawnpoint" (00CF6B24),
    "-" (00CEED40), "ingame.sm_slots|.: " (00CF6B10) and "ingame.sm_cp|.: " (00CF69CC).
  - The hint string is freed to the sized pool after 0068E600. When the hint was not shown,
    0041E870 rebuilds it in the same stack slot, whose fields still hold the freed block.
  - The 00665D20 tag dword is [B+14h], which is zeroed at entry, so 0 is passed.
  - The unit arm runs when +1F4h is set, the +300h unit is non-null and its byte +5Dh is
    clear. Otherwise the page +34Ch picks the arm: 0 gives the name "-", 1 the catalog
    arm, and anything else the map arm.
  - The catalog arm keeps the team in [B+14h]. Unlike 006697E0, it has no team filter: it
    appends the 0084D560 classes of every unit in the row's list.
  - The catalog arm's device icon: 009552E0(unit; id) and 00443490([[[r+4]+8]+4], DL = 0).
    The texture is [[[c+74h]+34h]+7Ch] from the first lookup; the UV is the four dwords at
    [[c+74h]+34h]+90h from the second. The id is screen+334h (from the map +50h, or
    unit+134h when the map has no entry) or, on the +2ECh arm, unit+134h.
  - The map arm's first test copies the row value. Its `m == 0` branch cannot be taken
    there; it is kept.
  - _ftol2 (00BF7420) is modelled as truncation to int64, keeping the low dword (0 out of
    range).
  - The points-text strings: the not-air arm destroys them through 0041DD20. The other two
    arms free them inline to the sized pool, newest first.
- **0066EA60.**
  - The whole body (1280 instructions) was read from the listing. The strings and the jump
    table 0066FAE0 were read from the PE.
  - The floats are +2A8h = 0.628, +2A4h = 1.57, +290h = 0.0275 and +294h = 0.05.
  - Each sample is acquired and stored with the inlined ref-pointer assignment.
    InterlockedIncrement runs on the new object. The old object and the temporary are
    released through InterlockedDecrement, with vtable +0h at zero.
  - The fields are, in order of acquisition:

    | Field | Sample |
    | --- | --- |
    | +10h | environment_unavailable |
    | +14h | sm_sp_to_unit |
    | +18h | sm_change_weap |
    | +20h | sm_launch_unit |
    | +1Ch | sm_unit_to_unit |
    | +24h | sm_left_right |
    | +28h | sm_open |
    | +2Ch | sm_close |

  - +2B4h = 3 at 0066EC1B. This is the write that cc9's `kSupportManagerLaunchGroup`
    (air_operations.hpp) cites. This body has no block copy into the screen.
  - The radar-shot name:
    - The mission name is [[00E188A8]+5FCh]+90Ch.
    - The part after the last '/' (or '\' when there is no '/') is kept, with
      count = length - pos - 5, so the last four characters go.
    - It is cut to 20 characters.
    - Modes 0..7 append _ICS, _ICM, _ICL, _ICH, _D, _E, _S and _C through 00425E10.
    - Modes 8 and 9 append nothing.
    - Other modes append "_xxx" inline: a resize, then a memcpy without the terminator.
  - The x87 tail is modelled at 24 bits; this is a main-thread GUI routine. The DC-form ops
    (0066F83F, 0066F8E0, 0066F8E8, 0066F8F6, 0066F91C) come from tools/x87_dc_form_scan.py.
    The DE-form ops (DE F1, DE CA, DE C9, DE E3, DE F3, DE C1) were decoded from the
    opcode bytes.
  - The radar shot's geometry:
    - a = (x1 - x0) / 30000 and b = (z0 - z1) / 30000.
    - The size is {a / m, b / m} times widget +1C0h's size, where m = max(a, b). FCOMI/JBE
      gives a NaN to b.
    - The UV rect at widget+118h is {(x0 + 15000) / 30000, 1 - (z0 + 15000) / 30000,
      (x1 + 15000) / 30000, 1 - (z1 + 15000) / 30000}.
    - The listing computes 1 - v as v * -1 + 1, and u as u * 1 + 0 (or 0 + u * 1), which
      turns a -0 into +0. That is kept.

## Batch 5

| Address | Name | ABI | Notes |
| --- | --- | --- | --- |
| 00428420 | BSP_ProjectileTrace_HitLandingTroops (new name) | thiscall(trace; float) RET 4 | tests a projectile trace's segments against the landing-troop members (2.0 spheres); the first hit is killed, gets a random death animation, and forwards its Damage to the group |

Notes:

- **00428420.**
  - The stack float is never read. The only caller, 0084BF00 (projectile_impact.cpp), passes
    a stream-1 draw (00BD2F10(ECX = 1) at 0084C19F) as that argument. The draw is the
    caller's.
  - The landing groups are the list [[00E188A8]+21D0h]+28h. Each group's box is
    +9Ch..+A8h. Its three member lists start at +B4h, 0Ch apart. A list node holds
    {+8h type, +0Ch member, +34h state, +38h timer}.
  - The segments are boxed from FLT_MAX / -FLT_MAX (00D7A248 / 00D7A244).
  - A member is tested only when vtable[5Ch](3Ah) passes and its state is not 3.
  - The pose refresh copies +74h, or +74h times the parent's +CCh, into +CCh..+108h,
    through float FLD/FSTP copies.
  - The death animation is 00BD2FC0(0) modulo the size of the type's 8-byte vector
    (+60h..+64h). This is a stream-0 draw. A zero size faults (DIV) in the image; the C++
    division keeps that.
  - Only a type with +39h clear (not Defence, per CC13_SOLDIER_TRAFFIC.md) forwards its +1Ch
    (Damage) through 00480C20.

## Batch 6

| Address | Name | ABI | Notes |
| --- | --- | --- | --- |
| 004A0D20 | BSP_LandingTroop_DisembarkSoldiers (new name) | thiscall(troop) RET | the landing-troop update behind cc13_left8b's `landing_troop_update_004a0d20` hook: one list node and path component per soldier of each troop member, with points from the +DCh unit's frame and the troop's two paths |

Notes:

- **004A0D20.**
  - The whole listing was read (2199 instructions, frame SUB 4CCh). B is ESP after PUSH EDI,
    and every `[ESP+n]` was mapped through the pushes in flight. The only caller is
    004A2FC0 (cc13_left8b_seg8.cpp, through its hook).
  - The troop's members (+48h array, +4Ch count) are the soldier-type records that
    00428420's list nodes point at (+8h). Their group is 2 when +38h is clear, else
    (+39h != 0).
  - Group counts ([B+88h]) are `_ftol2(int(+2Ch) * +14h + count)`, gated by +BCh, +C8h,
    +D4h and the [+D8h]+10h / +14h counters.
    - When the gate fails for a +38h member, the image stores 0 at counts[member index],
      not counts[group]. Members 3.. therefore write the frame dwords after the array.
    - Each of those dwords is rewritten before it is read. The C++ gives the array 13
      spare dwords for these stores.
  - With no total, the +B4h list nodes get +38h = 10.0 and +34h = 6. With +DCh set, the
    +38h/+39h counts are re-derived without the running sum.
  - The group weight sums (+14h) are replaced by stream-0 draws in [0, sum]. A member is
    processed when its sum minus its weight drops below zero (COMISS/JBE: a NaN skips).
  - Paths and the "loose" flag:
    - Path a is [+D8h]+0 (+8 for a +39h member) plus 1E4h. Path b is [+D8h]+4 (+Ch when
      +39h and set) plus 1E4h.
    - The flag [B+1Bh] is clear only when path b has exactly 4 points whose ends are at
      least 1e-10 squared and under 10.0 apart.
  - One 40h-byte matrix is built per point of path a. Its rows 3 and 2 are sampled at the
    knot times by 007B03C0. A knot at or past the end time +1Ch is moved to end - 0.01.
  - Straight path b (flag clear):
    - Path b's first point is appended as point 4, then its points are rotated so the one
      farthest from path a's end comes first (007AD180).
    - The farthest-point search reads the stack array [B+358h] before this member writes
      it. That is the previous member's rotation, or uninitialised stack on the first; it
      is zero here.
    - The array holds 5 entries before the 007AF800 buffer at [B+3A0h]. Paths with more
      points would overrun it in the image (not modelled). A zero divisor in the
      `k % (count - 1)` IDIV faults in the image.
    - The longer of the first two spans is u, the other v. The spacing is |u| / (|u| / |v|).
  - Loose path b (flag set): u is the normalised last span of path b. The spacing is the
    knot time of its second-last point over (+2Ch + 1) (FIDIV).
  - Grid defaults: base -0.5 / -0.2 and step 0.5 / 0.3. With the +DCh unit, vtable +24Ch
    points 1 and 0 give the base and the steps:
    - x step: dx / +34h.
    - z step: dz / ((count / +14h) / ((+30h + +34h) / 2)).
    - Each base moves half a step (DC C9 at 004A1ADE).
  - Per soldier (while +2Ch and the group count last):
    - Lateral and speed draws, then a 48h node through 00488E30, _Incsize (00490130) and
      the relink.
    - Node fields: +1Ch slot, +24h row, +18h a new 2Ch-byte 007B28C0 component.
    - Then one point per path-a point, through 007AD330:
      - Two grid points in the +DCh unit's cached world matrix (rebuilt as in 00428420);
        the second is 3.0 + |z offset| further.
      - Or the matrix with row 1 replaced by the global at 00F8758C, orthonormalised
        (0085DAD0). For a +39h member it is preceded by a point 2.0 back along path a's
        first span; otherwise it is jittered along row 0 by (draw(0, 0.2) + 0.9) * lateral.
      - Both are projected through troop+58h (00B62D10, divided by w).
    - The step counter advances along u / spacing.
    - Straight paths add an offset point: v scaled by draws over +20h/+24h, plus u steps,
      added to the rotated first point.
    - Node +3Ch is a draw over a random +8h entry's (+8h, +4h). Node +40h is the start time
      (plus the slot's previous start after the first row), stored to the slot array by
      CVTTSS2SI.
    - The row length is redrawn by `_ftol2(draw(+30h, +34h + 1))`. The group count drops
      by +14h (CVTTSS2SI).
  - Loose paths then sort the list by value +8h (0049F6A0 with predicate 0047B9F0, passed
    as its address).
    - Each state-0 node gets a frame sampled along path b at `r * spacing + draw`. Past the
      end time +1Ch, the time is end - draw (FCOMI/JBE: a NaN takes that arm).
    - The frame is orthonormalised and jittered along row 0, plus 2u for a +38h-clear
      member.
    - Two points are added (pos + 2u, then pos), the direction goes to +28h..+30h, and
      the knots are derived.
  - Node value block [B+1B4h]: dwords +10h and +34h are not written before the copy into
    the node.
    - They hold the identity matrix that a previous loose member wrote there, or stale
      stack on the first (zero here).
    - Node +3Ch keeps that dword when the member's entry count +0Ch is zero.
  - x87: every DE/DC operand order was read from the bytes (local check of all register-form
    FSUB/FDIV; DC C9 at 004A1ADE, 004A2386, 004A2801 and 004A2E08). Modelled at 24 bits.
    The thread that runs 004A2FC0 is not established, so 53-bit is an open uncertainty.
  - The 007B28C0 constructor runs in EH state 0; if it throws, the image frees the
    2Ch-byte block. That unwind is not modelled.

## Batch 7

| Address | Name | ABI | Notes |
| --- | --- | --- | --- |
| 0059D4A0 | BSP_HudMapIcon_PlaceWidget (cc13-ng2 name kept) | thiscall(screen; icon) RET 4 | places the icon's first widget on the HUD map by its anchor mode and corner; the body behind cc13_big5ad's `hud_map_icon_place_widget_0059d4a0` hook (the call sites 005B040C and 005B0420 lie inside 005ADDA0) |

Notes:

- **0059D4A0.**
  - The depth is -2.0 when icon+28h is set, else -12.0 ([00CE7D7C] / [00CE781C]). It is kept
    in the argument slot [B+40h].
  - The widget is the checked front() of the vector +ECh/+F0h. 00BF6713 is called when it
    is null or empty, before 00AA6740 and again before 00AA8240.
  - Anchor mode +11Ch:
    - 0: x = (+554h - +54Ch) / +30h + [00E08890], y = (+550h - +558h) / +34h + [00E08894].
    - 1: the unit (+4h) position through vtable +C4h, read twice. y uses the first call's
      z, x the second call's x.
    - Any other value leaves the position [B+18h..B+20h] unset. The image applies the corner
      offsets to stale stack (what earlier calls at the same depth in 005ADDA0 left there)
      and passes it to 00AA8240. The C++ starts from zero.
  - Corner +120h, by the jump table at 0059D6BC (unsigned; above 3 skips the offsets and the
    depth step):
    - 0: x + 0.02, y + 0.03.
    - 1: x - (w + 0.02), y + 0.03.
    - 2: x + 0.02, y - (h + 0.03).
    - 3: x - (w + 0.02), y - (h + 0.03).
    - Then z + 0.0 (the double [00D7A258], which turns -0.0 into +0.0).
  - The pads are the doubles [00D7A2F8] and [00CEB690], which are 0.02f and 0.03f widened.
    w and h are 00AA6740's size.
  - x87: memory operands only, with no register DE/DC forms. Each sum is rounded at its
    FSTP. Modelled at 24 bits; the caller is a UI path.

## Recorded, not reconstructed

- **004A9BD0 BSP_PlanePartClasses_LoadGlobals.**
  - The planepartclasses.lua globals loader (1118 instructions), called from
    BSP_Game_OnInit 004E3AA0.
  - cc9 hosts the one value it consumes. game_hosts_units.cpp, under
    `GameUnitsHost::Impl::kPlaneDeathModesBound`, reads ExplosionExplosionDelay through
    `read_global_number_pair`, once per process, with defaults {0.6, 1.8}.
  - What the listing confirms:
    - [1] goes to [00E18710] and [2] to [00E1870C] (004AABEC..004AACCD), with no presence
      test or default.
    - 007BBFA0 draws 00BD2F10(ECX = 0, [00E18710], [00E1870C]), so cc9's
      (first, second) = (min, max) matches.
  - The other globals, stored at [00E186F4] through [00E18798], are neither hosted nor reconstructed.
  - Ledger only: `existing_host_substitution_labelled`, coverage partial.

- **00673A10 BSP_HudSupportManager_ConfirmOrder (new name).**
  - The support manager's order action, called from the update 00675C40. It has 1276
    instructions, and its gameplay arms are in cc9's lane.
  - cc9 hosts those arms as labelled models:
    - the shipyard purchase in `GameGunneryHost::shipyard_order` (game_hosts_gunnery.cpp,
      006743E0..00674624);
    - the targeted launch in game_hosts_script_orders.cpp (00674765..006748D7).
  - cc13-big1 checked only the idle-entry search and the start of the A7h loop against the
    listing.
  - The HUD side is neither hosted nor reconstructed. It covers the [supportmanager] and
    spawned messages, and the 00946F00 cost test with the unavailable sound.
  - Ledger only: status `existing_host_substitution_labelled`, coverage partial.

Adoption notes:

- **004C7AA0.** cc13_large61's hook `game_query_004c7aa0` (0094BFF0) can forward to this
  body. Its zone record is the native 44h record. src/world_map_bounds.cpp models
  004C7730 over a BorderZoneSet, and a host can bind the 004C7730 hook to that.
- **005AAC00.** cc13_big5ad's hook `hud_map_icon_refresh_carried_unit_icons_005aac00`
  (005ADDA0) can forward to this body.
- **0064AE20.** cc9's `game_hosts_hud.cpp` (near line 385) records
  `HudShipScreen::apply_unit_0064ae20` under `kHudShipScreenGaugesBound`, as a labelled
  substitution: the flags keep the enter's clear values. This body is the replacement.
  The hooks in cc13_sweepB2 and ingame_gui_cc13 can forward to it.
- **00641910.** Not hosted. hud_markers_runtime.cpp (near line 268) lists it as follow-up
  work.
- **005E12A0.** frontend_screens_cc13's `load_weather_table_005e12a0` hook can forward to
  this body. scene_contents_hosts reads the same table for a different routine.
- **006697E0 and 0066A8A0.** seg27_cc13's `support_collect_catalog_006697e0` and
  ingame_gui's `rebuild_list_c_0066a8a0` hooks can forward to these bodies. cc9 does not
  host either.
- **00674B80.** It is reached only through vtable 00CF6B44 slot +18h. cc9 does not host it.
  Several of its callees have bodies elsewhere: 0066A130, 0066D020, 00669170 and 0066FB00 in
  seg27_cc13. A host can bind the matching hooks to those bodies.
- **00428420.** Not hosted. GUNNERY_OPEN_ITEMS.md and SHIP_AI_OPEN_ITEMS.md list it as
  blocked on hosted landing-troop groups. A gunnery or ships host with troop groups can call
  this body at 0084C1AA.
- **0066EA60.** Reached through vtable 00CF6B44 slot +10h. cc9 does not host it.
  src/hud_screens.cpp lists only its page and widget catalogue. CC13_FE_10.md's "left to
  cc9" listing named it.
