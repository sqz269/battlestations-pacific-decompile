# CC13_SEG51B: the last segment 51 bodies, and the segment 52 census (packet cc13-seg51b)

Static packet, on cc9's gameplay terms. Nothing is wired and there were no game runs. The
names are hypotheses, not recovered symbols.

## 1. Census

- **Segment 52 (008288D0-00851CB0, struck 00836000-00837FFF and 0084E000-0084EFFF):** 729 rows on
  32b1ed132. Every row outside the struck ranges has a reconstruction or ledger status, or is
  STL, template, Catch_All or Unwind. The only exception is `0084B9F0 thunk_0084B9A0`, a single
  JMP. The earlier packet cc13-seg52b (283eea7fd, 200c6423f) and the later cc13-seg52U and
  leftover passes covered it. Nothing was left to build.
  - The index in the main checkout was stale. It listed 19 rows such as 00847FE0, 008482F0 and
    0084FC50 as open. A fresh index in this tree clears all of them.
  - Besides STL, template, Catch_All and Unwind, the census also counts tagged rows as closed.
    These include CG_ scalar deleting destructors and adjustor thunks, and TRIV_ bodies such as
    the 008356D0 getter (`FLD [ECX+244h]; RET`).
  - Three `stl_probable` bookmark rows with EH frames had never been reviewed (packet
    cc13-seg51c). Each was read whole. All three are library code and stay bound; their tag rows
    are now `stl_instantiation` renames under evidence `cc13_seg51c_eh_template`:
    - `00839B40` `STL_Construct30_00839B40`: if the pointer is non-null, a placement copy
      construct of a 30h settings record (00839800). This is the std `_Construct` shape.
    - `0083B350` `STL_Vector_ResizeByValue14_0083B350`: vector resize(n, value) for a 14h-byte
      element. It grows through 0083AB40 (InsertN) or erases through 0083A980, then releases
      the by-value argument's native string. Its caller is 0083B5E0, the game-settings loader.
    - `008484A0` `STL_Vector_CopyConstruct1C_008484A0`: vector copy construct for a 1Ch-byte
      element, through 00664120 (_Buy) plus the uninitialized copy 00847340.
- **Segment 51 (0081AA60-00828870):** 158 rows. All but three had a status. Those three are
  below. None of them is hosted: git grep by address, name and behaviour ("guncamos", deck
  visuals) finds no host body.

## 2. Bodies (`src/cc13_seg51b.cpp`, `include/bsp/cc13_seg51b.hpp`)

Each body was read whole from the listing. Every call is a host hook made at the image's call
site, with the image's arguments.

- **0081BA80** `gun_camos_write_entry_0081ba80`, `__fastcall(ECX = index, EDX = const float[4])`,
  plain RET. It is unreferenced (no direct call and no absolute dword), so it is a tool-side save
  and is not live in a mission. It uses no RNG. The steps are:
  1. It constructs and opens a Lua state owner (00B66BD0, 00B6A020(1)).
  2. If the file system's vtable[8] reports that guncamos.lua exists, it runs the file
     (00B69D40(path, 0)). Otherwise it creates globals.guncamos (00B67580).
  3. Either way it reads globals["guncamos"] into the table.
  4. It sets guncamos[index] = {} and writes elements 1..4 from the colour (00B66580).
  5. It opens guncamos.lua through vtable[4] with mode 35h and writes the table as "guncamos"
     source text with 00B6A390(stream, name, 0, 0, 1, 0).
  6. It releases the stream, destroys the two Lua objects and closes the owner.
  - The first string at ESP+18h is passed to 0041E870 without being zeroed first. 0041E870 may
    be a constructor rather than an assign; the hook keeps the call either way.
- **0081C1F0** `refptr_array_push_back_0081c1f0`, `__thiscall(ECX = {data, count, cap}; const
  ref*)`, RET 4.
  - **Name correction:** the elements are 4-byte refcounted pointers (EDI*4 indexing, cap*4
    allocation), not the 8-byte {RefPtr, dword} pairs that cc13-gp80's note described. The
    ledger name is now `TMPL_RefPtrArray_PushBack_0081C1F0`; the Ghidra label is unchanged.
  - When the array is full, cap = 2*cap+2 is stored before 00BF55BE allocates. It copies each
    element with an increment, releases the old slot, frees the old block (00BF6989), then
    appends with an increment and increments count.
- **0081C550** `unit_instance_toggle_deck_visuals_0081c550`, `__fastcall(ECX = unit)`, plain RET.
  It is unreferenced (no direct call, no absolute dword and no vtable slot) and uses no RNG.
  **Uncertain purpose:** on airfields +749h is HangarFailure (cc13_seg30.hpp), so this may be
  the hangar-failure visual rather than "deck visuals". The steps are:
  1. It flips +749h.
  2. Each 44h-byte record of the +660h vector that has a node gets 00B6DA70(1.0 when the flag
     is now set, else 0.0; 0). The four 00BF6713 checked-iterator tests are kept. The
     container-identity tests at 0081C5A6 (`TEST EDI,EDI` / `CMP EDI,EDI`) and 0081C5BB
     (`TEST EDI,EDI`, with EDI = unit+660h) can never fail, so they are not modelled.
  3. **Flag clear:** each live effect in +A14h/+A18h is stopped (00867B10), its byte +9 is set
     to 1 and it is released; then 004C9550(0).
  4. **Flag set:** when the class definition [unit+538h]+674h is non-null, it creates one
     008687C0 point effect for each 16-byte position in +678h/+67Ch. The parent is +4A4h. The
     matrix is the identity rotation ([00D7A24C] = 1.0) with the position as the translation
     row. Each new effect is appended through 0074D780.
  5. It always ends with 00815D20(parent +4A4h, flag).

Build: the full `./scripts/build.ps1` (MSVC Win32), with 2/2 existing CTests passing. The bodies
are build-tested only. They are not ABI-compatible and not game-validated.

## 3. Thunk mop-up (packet cc13-seg51c)

The census of segments 47, 30 and 41 found 17 open rows. Each one is a bare `E9 rel32` JMP
followed by INT3 (Capstone over the disk bytes). Each now has an `existing_recorded_thunk` row
(`cc13_seg51c_jmp_thunk_<addr>`). They have the target's ABI and no code of their own.

| Thunk | Target |
|---|---|
| 007B3C90 | 00695870 BSP_CallbackOwner_Destroy |
| 007B4650, 007B4D80, 007B5250, 007B5770, 007B65D0 | 007B45F0 BSP_LandSharedState_Destroy |
| 007CD2E0 | 007CCD30 BSP_PlaneEngineEffectVector_DestroyRecords |
| 006BF050 | 006BCD20 BSP_AirOps_GetBlock |
| 006C4780 | 006C0F00 BSP_AirOps_FillSlot |
| 006C4B20 | 006C1430 STL_List_Clear (library) |
| 006C6B40 | 006C4B30 BSP_AirOpsSquadronKillObserver_OnNotify |
| 006C9310 | 006C88D0 BSP_AirOpsCallbackList8888_OnNotify |
| 006CDF30 | 006CDD70 BSP_FollowObserverMember_Destroy |
| 006CFAE0 | 006CEFF0 BSP_AirOpsSiteOccupants_OnNotify |
| 0073C330 | 0073C240 BSP_StringMap_Clear |
| 00743F40 | 0095D400 BSP_Unit_ReleaseEffects_0095D400 |
| 007455E0 | 00745200 BSP_IntrusiveList_ClearNodes_00745200 |

`0095D400` had no name record. It is now named after its reconstruction,
`unit_release_effects_0095d400` (src/cc13_seg62.cpp), and its thunk 00743F40 is renamed to
match. The Ghidra labels of 0081C1F0, 0095D400 and 00743F40 were synced from the names ledger
with `ghidra_annotate.py --apply`.
