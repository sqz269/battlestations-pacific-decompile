# Unit hull extent production

Addresses: `00810F60`, extent fragment `00810FAF..00811073`; consumed producers
`00960230`, `00B66270`, `00B7F430`; corroborating model-only stores in `0081F980`.

The recovered fragment writes **full width** to unit `+9CC` and **full length**
to unit `+9C8`. `unit_half_width_09cc` retains its historical public spelling,
but now returns the actual stored full width. Its former constant `1.0` and
claim that no producer existed were incorrect.

`00810F60..008110CF` takes the unit in ECX and returns with plain RET. Its
prefix attaches Lua self and allocates/constructs a weapon director; its tail
notifies according to the resource holder's kind and session mode. The new
function covers only `00810FAF..00811073`. It is a new C++ interface, not an
ABI-compatible replacement for the enclosing routine. No Ghidra mutation was
made; the enclosing name proposal remains a hypothesis.

| Input state | Stores, in native order |
| --- | --- |
| unit `+538` class is null | Preserve both prior fields |
| class `+50` model is null | Copy class `+A4` to unit `+9CC`, then class `+A0` to unit `+9C8` |
| Class has a model | `2 * max(-minX, maxX)` to `+9CC`, then `2 * max(-minZ, maxZ)` to `+9C8` |

The model box is local `minXYZ, maxXYZ` at resource `+28..+3C`, also written
by the existing structured-root `BoundingBox` parser (`00B7F525..00B7F55C`).
The new API reuses `HitQueryBounds` as six float words; its caller must supply
the real local box. This is neither a world collision box nor the geometric
span `max-min`: `[-2,7]` produces width 14. Y does not participate.
`0081F9C4..0081FA52` independently confirms the same model formula; that
different enclosing initializer requires a model and has no null-model arm.

The implementation retains the float spills around FCHS and FCOMIP, the JBE
choice of negative minimum on ties or unordered comparisons, and the double
2 at `00D7A308` (`4000000000000000`). The opcode `DC C9` at `0081100C`
multiplies ST(1), leaving the scale available after the X result is stored.
The Z multiplication consumes that retained value. No finite checks,
normalization, absolute-value replacement, ratio, or extent clamping is added.

Class `+A4` is the existing vehicle reader's `Width`: literal `00D0CB40`,
GetByName at `00960354`, GetNumber at `00960363`, FSTP at `00960368`.
`Length` follows at `0096038B/0096039A/0096039F`. The native numeric wrapper
`00B66270` calls `00A67770`, spills to float32, and reloads its ST0 return.
There is no nil guard or positive-width default. Existing
`VehicleClassFieldOffsets::kWidth` and `VehicleClassFields::width` already
record this producer; the concrete mission Lua row now exposes the same key
through the independently owned Lua-reader change.
That Width adapter uses `lua_tonumber` and the existing float32 spill helper,
including native numeric-string conversion; it does not require a numeric tag.

`GameUnitSlot` retains both produced unit fields and the raw loaded class
width. During creation the represented runtime selects the no-model arm,
because it currently owns no class model resource. The hydro input now reads
the retained class `+A4`; it must not read unit `+9CC`, which can differ with
a model. The unit width and length getters return the stored unit values.
A missing class preserves prior semantic storage; fresh storage is zero and
is not asserted to recover an unknown native constructor default.

Verification used an ignored Win32 probe executing the original 197 bytes of
`00810FAF..00811073`, equal to live Ghidra and installed executable bytes.
A register/stack wrapper surrounds the unchanged branches; only the absolute
double-constant operand is relocated. All 1,449 comparisons pass: 816 edge
cases over x87 precision 24/53/64, four rounding modes, and normal or FTZ+DAZ
MXCSR, followed by 633 installed class Width/Length pairs. Comparisons cover
both output float bits, all unrelated unit bytes, x87 exception flags and
stack TOP. They include null class, no model, asymmetric boxes, signed zero,
denormals, infinities, and quiet/signaling NaNs. Condition codes, register ABI,
unmasked exception delivery, SEH, and the enclosing prefix/tail are excluded.

The installed `vehicleclasses.lua` executes in the repository's Lua 5.1.1 with
the real fundamentals and exact conversion/isUnlock functions extracted from
`global/luamw_init.lua`. `PC=true` and an empty `UnlockedClasses` are explicit
fixture inputs; unlock outputs are not compared. All 633 executed rows have
numeric, positive Width, ranging from 1 to 136, across 21 Type strings. This
does not prove which rows the mission instantiated. The primary integrator
owns matching its 77 runtime registrations to actual row IDs and extents.
No installed model box or in-game behavior was validated by this packet.

`./scripts/build.ps1` passes Win32 Release and both existing CTests. No tracked
test was added. The reproducible probe, native span, run result, table export,
and hashes are recorded under `local/` in `reports/unit_hull_extents.json`.
The first configure's Lua download stalled; isolated cached dependency copies
were supplied through CMake FetchContent source overrides before the build.
The Lua worker's two files were borrowed for compilation and excluded from
this commit; integrate its Width reader before building these Units changes.
