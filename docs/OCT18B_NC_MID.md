# oct18b nC mid reconstruction

Packet `oct18b-nC-mid` covers exactly the 42 dispatch rows from `009F6C10`
through `00AF37A0`, including the final unsegmented row. Segment 67 is reserved
and untouched. The packet has 35 game-body mappings and seven compiler/library
classifications, with no unresolved body. `reports/oct18b_nc_mid.json` records
every address, descriptive name, C++ symbol, original ABI, evidence, and limit.

The new source and header extend `cc13_seg68`, `cc13_seg69`, `cc13_seg70`,
`cc13_gui1`, and `cc13_terrain`. Their main family files have other active
owners, so these additions live in `src/oct18b_nc_mid.cpp` and
`include/bsp/oct18b_nc_mid.hpp`. There is no game-host wiring. Names are
hypotheses, not recovered symbols; the C++ interfaces are not binary replacements.

## Evidence and behavior

- The squadron filter enabled/disabled leaves reuse the existing segment-68
  filter model. Three identical scalar-deleting wrappers (`009F7110`,
  `009F7130`, `009F7280`) only install `00D21B50`, conditionally call `free`,
  and return the original pointer. These compiler wrappers receive
  `library_instantiation_out_of_scope` and no reconstructed game credit.
- `00A00800` is a cdecl first-word callback, identified only by its reads and
  address-taken site. `00A04C70` maps an index through owner `+10h`, grows the
  cache at `+4h` using `00437BD0`, reloads its data pointer, and reports whether
  the mapped cache word is nonzero. The immediate `00F8A0BD` participates in
  a boolean normalization sequence and is not dereferenced.
- `00A05560` and `00A055B0` are VS2008 vector allocation instantiations for
  120-byte elements: zero the three pointers, check `UINT_MAX/120`, allocate,
  set begin/end/capacity. Their neighboring throw sites identify `vector<T>
  too long`. `00ACAB10` is a typed STL fill over 20-byte float/vector records.
  These receive `library_instantiation_out_of_scope`.
- `00A18570` and `00A2BD40` forward the incoming invocation unchanged through
  virtual slots `+24h` and `+4h`. Their C++ continuations model only dispatch
  control flow; the native target arguments, return registers, and stack
  cleanup remain the continuation's contract. `00A31BF0` is the separate
  compiler adjustor `ECX -= 170h; JMP 00A329D0`, classified out of scope.
- `00A2DA60` searches for an entity in the AI group's member list before
  adjusting the observer base by `-10h` and executing the same removal tail as
  existing `group_remove_entity_00a2d9d0(entity,1)`. The valid-list behavior
  reuses that implementation. As in the existing family, corrupted-iterator
  diagnostics at `00BF6713` are not modeled. `00A31D60` retains its game
  singleton effect: clear `00F8AAFC`, restore `00CE3818`, and optionally free.
- `00A9DE50` preserves recursive center selection, signed half-page arithmetic,
  maximum page clamp, and the selection offset read after page rebuild.
  `00AA64B0` allocates `17Ch`, then constructs or copy-constructs a listbox.
  `00CB7350` identifies FuncInfo `00DEDB94`; unwind map `00DEDB84` has both
  states call `00A9B330` with saved constructor storage. The factory's
  catch/rethrow uses that explicit pool cleanup hook. The allocator wrapper
  at `00A9B7E0` is currently misleadingly tagged as a static destructor stub;
  this packet does not rename that unassigned address.
- GUI leaves preserve the unsigned word at `+ECh`, ordered float4 equality,
  and text flag `+77h`. Assembly is necessary for `00AB6A70`: its virtual tail
  call replaces the input stack word with `3` for a nonzero flag and `0`
  otherwise, a detail the pseudocode omits.
- Terrain leaves preserve empty slots' actual `RET`, `RET 4`, `RET 8`, and
  `RET 0Ch` contracts, object words `+4h/+10h`, and height constants. Bytes
  `F9 02 15 D0` at `00CE4ADC` and `F9 02 15 50` at `00CE4970` are float
  `-1e10` and `+1e10`; identical constant bodies share an implementation.
- `00ADC080` inserts unique pointers into `{data,count,capacity}` at `+44h`.
  It grows a full array to `max(2*capacity,1)`, reloads after reserve, skips a
  null placement destination, and still increments the count. Array validity
  is required; stored pointer ownership remains provisional.
- Height setters `00ADC580` and `00ADFA90` compare against the exact double
  `-1000` at `00CE6658`. An ordered value at/below the threshold writes the
  all-ones sentinel. Otherwise the MSVC Win32 path retains x87 subtraction
  and multiplication on the stack, temporarily sets rounding control to
  truncation, uses `FISTP` to int32, restores control, and stores the low
  16/8 bits. Unordered height takes the conversion path, preserving its
  indefinite-result behavior with masked exceptions. The portable fallback
  is a value model, not x87 ABI proof.
- `00ADF090` stores tile coordinates at `+Ch/+10h` and constructs a matrix
  with diagonal `(scale,1,scale,1)`, where spacing is `[tile+8]+18h` and
  `scale = float(x87(spacing / double_bits(3FC0101020000000)))`. That exact
  double comes from bytes `00 00 00 20 10 10 C0 3F` at `00D5D408`; its
  semantic origin is provisional. The existing `004134F0` matrix copy is
  reused, spacing is reloaded, and integer-coordinate products are each
  rounded to float before the node `+24h` virtual `+38h` matrix call.
- Float grid stores use bit-preserving `MOVSS` semantics; float3 grid get/set
  retain sequential x87 `FLD/FSTP` copies in MSVC Win32. `00AF37A0` copies a
  float3 to `00E13028/2C/30` in component order, preserving bit payloads and
  possible source/destination overlap. The global's meaning remains provisional.

## Validation boundary

All 42 functions were exported from the existing `C:/Users/sqz269/bsp.gpr`,
program `/battlestationspacific.exe`, through the verified Ghidra client.
Assembly was inspected for register inputs, exact stack cleanup, x87,
tail-dispatch arguments, overlapping global constants, and exception cleanup.
All 42 new names were applied through the locked annotation tool, preserving old
names/comments in `local/ghidra-annotations-20261019T054023Z.json`; all 42 audit
rows report `applied`, the project was saved, and all 42 exports were refreshed.

The primary integrator registered the source in CMake and confirmed the combined
MSVC Win32 Release build: all 35 mapped bodies compiled and linked into
`bsp_core` and `bsp_game`. Both existing CTests passed (2/2), recorded in the
integrator's `local/nc_combined_build2.log`. These 35 records now have
`reconstructed_build_tested`, `compiled=true`, `abi_compatible=false`, and
`game_validated=false`; the seven library/compiler classifications retain their
out-of-scope status and receive no source build credit. The worker's separate
`scripts/build.ps1` baseline build and 2/2 existing CTests also passed
(`local/oct18b-nc-mid-build.log`). The exact call-site checker passed all 29
recorded call/tail-jump rows; all 42 assigned addresses are unique, and every
mapped C++ symbol exists. No new test was added. No game was run. No native ABI
compatibility, runtime behavior, or gameplay validation is claimed; unmasked
x87 exception behavior is untested.
