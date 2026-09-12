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
