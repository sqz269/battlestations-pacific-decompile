# Authored map bounds

Addresses: 004E6C00 004D5BD0 004D5EDE 0071C4F0

`src/world_map_bounds.cpp` reads the scene's actual `Map` properties and projects
the bound-selection part of the game owner. Descriptive names are hypotheses,
not recovered symbols. Target: `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`. Evidence: `reports/world_map_bounds.json`.

004E6C00 reads BorderSizeX/Y through008F2260, using CVTSI2SS for integer
properties, then reads sixteen vectors from MultiPlayMapSizes. Missing that
nested bag returns before any global stores. The two size stores end004E713C.
The subsequent004D5BD0 call and twelve004C7150 calls are recorded contracts;
this packet does not reconstruct the full reader or border-object lifetime.

004D5BD0 copies those vectors to game fields, then004D5EDE..004D61BF selects
NW at+711C and SE at+7128. Forced byte+61C, session+1FE4, and mode+614 gate the
multiplayer switch. Modes0..7 select authored pairs when the gate is open;
otherwise NW is(-sizeX/2,0,sizeY/2) and SE is(sizeX/2,0,-sizeY/2).
The multiplier at00D7A280 is a double0.5. Authored inverted bounds are preserved.
The allocation/publication code around this fragment remains unimplemented.

0071C4F0 is complete: ECX game, one XYZ pointer, EAX0/1, RET4. Four x87 JA
branches reject NW.x>point.x, point.x>SE.x, point.z>NW.z, or SE.z>point.z.
Edges are inclusive; Y is ignored. An unordered comparison does not reject by
itself, so inverse conjunctions from pseudocode are not equivalent for NaNs.
Clipping minima are(NW.x,NW.y,SE.z), maxima(SE.x,SE.y,NW.z).

Independent manifested Win32 probes compared the full88-byte0071C4F0 body over
24,576 cases and the738-byte selection fragment over4,032 cases, with zero
mismatches across12 x87 precision/rounding modes. Cases include edges, signed
zeros, denormals, infinities, and NaNs. Selector relocation changes only the
documented globals/jump-table operands and its fixture return. The installed
USN01 scene parsed without errors and produced sizes20000/20000 and default
NW(-10000,0,10000), SE(10000,0,-10000). This is numerical evidence, not gameplay
validation. Existing call verification checked37 rows without failures.

These are new C++ interfaces, not binary overlays. Missing or wrongly typed
required properties throw at this adapter's boundary instead of reproducing
invalid native pointer reads. Global aliasing/reentrancy, native SEH, border
objects, and their registration are outside the demonstrated coverage.

## Border zones (packet `cc9_get_closest_border_zone`, cc9-lua5, 2026-09-28)

Addresses: 004D5BD0 (the rebuild tail 004D61C4..004D6240), 004B6C40, 004C71C0, 004C7150,
004E7142..004E7209, 004C7730, 008AECD0. Reconstructed in `src/world_map_bounds.cpp`
(`build_border_zones_004d5bd0`, `closest_border_zone_004c7730`,
`get_closest_border_zone_008aecd0`). Build-tested only; no fixture compares them with the image.

**The records (V, from the listings).**
- After selecting the bounds, `004D5BD0` allocates three 44h records per edge into the four lists
  at `world+7134h + edge*0Ch`.
  - Each record's `+0` is `004B6C40(edge) / 3.0` (`00D7A2B0`), and `+4` is 2.
  - `004B6C40` gives `SE.x - NW.x` for edges 0 and 2, and `NW.z - SE.z` for edges 1 and 3.
- `004C71C0` lays them out (x87; each corner is summed on the stack and rounded once at its store):
  - `+8` is the edge, `+Ch` the index, `+40h` the running offset along the edge;
  - the last record's length is re-taken as `004B6C40(edge) - offset`;
  - edges 0 (north, `NW.z`, direction +1) and 2 (south, `SE.z`, direction -1):
    - `A.x = B.x = NW.x + offset + 50`, `C.x = D.x = NW.x + offset + length - 50`;
    - the first record's `A.x` loses 1000, and the last record's `D.x` gains 1000;
    - `A.z = D.z = z + 950 * dir`, `B.z = C.z = z + 50 * dir`.
  - Edges 1 (east, `SE.x`, +1) and 3 (west, `NW.x`, -1) mirror this on z, running down from `NW.z`.
  - The constants are 50.0 (`00CE3938`), 1000.0 (`00CE47A0`), 950.0 (`00CE7640`) and ±1.0
    (`00D7A24C`, `00D7A260`).
- The twelve `004C7150(edge, index, side)` calls at the end of `004E6C00` are all constants. Edges 0
  and 2 get sides 0, 1, 0; edges 1 and 3 get 1, 0, 1.
- `004CA930`, a merge of adjacent same-side records, has **no reference** in the image (neither
  rel32 nor absolute), so it is not applied.

**The query, `004C7730(position, side, point_out, direction_out)`.**
- It walks the edges in order 0 to 3 and keeps the records of `side` (any when negative).
- It clamps the position onto the record's B..C edge in x/z and takes the distance. It is 0 when
  the square is at most 1e-10 (`00CE3820`), else the square root.
- It keeps the strictly nearest, seeded at 1e10 (`00CE4970`).
- It writes the point (clamped x, position.y, clamped z) and the outward direction: (0, 0, ±1) for
  edges 0 and 2, (±1, 0, 0) for edges 1 and 3.
- With no match it returns 0 for sides 0 and 1, and retries with -1 for other sides.
  - With -1 and no records at all the image loops forever. The host returns instead.

**Its users.** `008AECD0` `GetClosestBorderZone` (side -1), `007F16D0`'s retreat arm (the
squadron's side, `docs/CONTROLLED_UNIT.md`, "The LOMP10 obedience cases"), `008A4300`
`PilotRetreat`, `004C7AA0` and `009C8D40`.
