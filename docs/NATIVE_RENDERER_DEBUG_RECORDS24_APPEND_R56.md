# Sphere record production and dispatcher argument ownership (R56)

`B29270[184]` is renderer virtual `+C8` (`D5F170` contains `B29270`). It takes
the actual renderer in ECX and three stack arguments: sphere16 pointer, camera
pointer, selector DWORD; it returns with `RET0C`. It copies center/radius into
a 24-byte record, stores selector at `+10` and the borrowed camera at `+14`,
then appends to the actual renderer header at `+1D0C`.

This resolves a misleading earlier interpretation of `B45360` mode0:

- `B453D6/B453D7` push entry flags and the already captured camera.
- `B453E3` invokes the model's current virtual `+48` with no stack arguments.
  Those two words remain on the stack. The established model world-sphere
  getter `B6E8C0[99]` returns its sphere address and ends in plain `RET`.
- `B453E7` pushes that returned sphere pointer. `B453EA` invokes the current
  `+C8` slot through the previously captured renderer/table. `B29270` consumes
  all three words with `RET0C`.

Thus camera/flags belong to sphere-record production, rather than to the model
getter. `docs/MATERIAL_ENTRY_DISPATCH.md` and Ghidra evidence now record this
correction. This packet does not implement the remaining `B45360` dispatcher or
claim all dynamic model/override profiles have been resolved.

## Exact producer and append schedules

The public producer uses the R53 guard's SAME actual AA0/D5A0 cells, captures
its section at `+4`, and enters/increments. The four sphere MOVSS reads, camera
store and argument captures precede state0. The radius and selector stores occur
after state0 is armed, immediately before the append call. Normal decrement and
leave remain inside that scope. Camera is borrowed without any retain operation.

Fresh `CBD2B8 -> DF5A78`, unwind map `DF5A70`, and funclet `CBD2B0` show that
state0 invokes only the captured `411EE0` guard cleanup. No record rollback or
owner release is added. A secondary source C++ cleanup exception terminates.
The source uses a bit-preserving MOVD spill across its own exception bookkeeping;
this does not establish original private-frame or hardware-fault equivalence.

`B25750[90]` takes the actual 12-byte header and live 24-byte input. It grows
only when used equals captured capacity, doubles with DWORD wrapping and selects
one unless the signed result exceeds one. It calls the genuine current `B229D0`
reserve, reloads used then data, and computes `data + used*18h` with DWORD wrapping.
A computed-null destination skips every input read. Otherwise four ordered x87
FLD/FSTP pairs copy the sphere and two ordered DWORD operations copy selector and
camera. Finally it increments the current used word, preserving header aliases.

## Evidence and validation

- Fresh live/PE bodies and context bytes include both producers, reserve,
  dispatcher, world-sphere getter, renderer profile slot and exact EH maps.
- `B4540C..B4540F` is an unreachable `LEA ESP,[ESP]` alignment slot: the preceding
  unconditional jump targets `B45412`, the loop targets `B45410`, and live Ghidra
  reports no references to the padding. The saved listing gap is retained.
- Strict MSVC Win32 `/MD /W4 /WX /fp:strict` build and all three existing CTests.
- Six full original90B/source helper comparisons cover separate input,
  forward overlap, header alias (whole128B arena plus x87 status), computed-null
  input skip, and two genuine reserve/growth cases. The copied original helper's
  sole call is relocated to the same existing source `B229D0` provider.
- The compiled helper's full90B match the original except that four-byte call
  displacement; all eight x87 instruction encodings are identical.
- Public producer and genuine guard chain are forced linked, but not executed.
  Seven loaded modules are resolved by file handle in the same Win32 probe and
  verified as I386 on disk. Current inputs, binaries and receipts are frozen.

Original reserve/public producer execution, injected exceptions, full dispatcher,
active renderer, application rendering, visuals and gameplay remain unvalidated.
Source APIs add explicit contexts; original FH3/SEH and complete ABI replacement
are not claimed. No renderer, camera owner or successful callback is fabricated.
