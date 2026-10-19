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
  It is virtual (no direct caller) and uses no RNG. The steps are:
  1. It flips +749h.
  2. Each 44h-byte record of the +660h vector that has a node gets 00B6DA70(1.0 when the flag
     is now set, else 0.0; 0). The four 00BF6713 checked-iterator tests are kept.
  3. **Flag clear:** each live effect in +A14h/+A18h is stopped (00867B10), its byte +9 is set
     to 1 and it is released; then 004C9550(0).
  4. **Flag set:** when the class definition [unit+538h]+674h is non-null, it creates one
     008687C0 point effect for each 16-byte position in +678h/+67Ch. The parent is +4A4h. The
     matrix is the identity rotation ([00D7A24C] = 1.0) with the position as the translation
     row. Each new effect is appended through 0074D780.
  5. It always ends with 00815D20(parent +4A4h, flag).

Build: the full `./scripts/build.ps1` (MSVC Win32), with 2/2 existing CTests passing. The bodies
are build-tested only. They are not ABI-compatible and not game-validated.
