# Actual system fog construction and camera attachment

The raw overloads cover complete B84E50[273B,end B84F61] and
B71940[65B,end B71981] over actual94h `SystemFogOwner` storage and an actual
camera+184 cell. Live Ghidra and installed PE bytes agree. Existing legacy
callers remain valid; the legacy constructor now uses the same native store
engine with its established installed constants. No logical CameraState or
FogState projection is written into a raw cell.

The constructor establishes a real `SystemFogOwner` lifetime by preserving and
restoring all94h allocation bytes around placement construction. It then executes
the full native MOVSS/integer store sequence. Bytes28..67 remain untouched.
The source interface borrows nine constant cells; it copies only their addresses
into a real pointer array for assembly, without assuming reference-member layout
or caching native values. The private16B scratch is fully written before use.

CE386C and D63188 are captured before the first destination write. The sequence
then writes zero08/0C, reads CE7804 once for68/78, reads CE77F8 for7C and CE3950
for80. CE77E8 is captured before zero10 and its value written84; CE77E4 is captured
before zero14/18/1C and written88. CE77E0 is captured before the base profile
stamp and written8C; CE77DC is read after that stamp and before zero20/count1/
concrete profile/zero24. Finally the captured first constants are written6C/70/74
and the last value90. These are raw MOVSS copies, preserving signaling NaNs,
negative zero and aliases to earlier/later destination stores. No eager default
snapshot or C++ float conversion replaces the native accesses.

B71940 captures the incoming argument word before reading current camera184.
Equal identity returns without reading imports. Otherwise it publishes captured
incoming, calls CURRENT CE221C on incoming04 if nonnull, then calls CURRENT
CE2220 on capturedold04 if nonnull. The increment callback may change the
decrement cell, argument or camera slot; the native captured identities survive,
and no final store overwrites callback changes to184.

An observed zero uses the captured owner's CURRENT numeric profile/current0.
Only genuine D63180/current0 BD30E0 is admitted. The existing BD30E0 provider
then reads a fresh profile/current4; only B84F70/flags1 is admitted, and the
existing real scalar body/free is invoked. The separately exposed
`invoke_native_system_fog_zero` provides this same family binding for callers
which already decremented the actual count. Neither path decrements twice,
adds credits, synthesizes an atomic, or registers a host companion. Current
targets lacking a binding are rejected explicitly. B84F70's36B external body
and the live D63180 table are pinned as reuse evidence, not newly reconstructed.

Old/new owners must be genuine live94h objects with their actual volatile-long04
count and matching singleton CRT allocation domain. Every reached CURRENT import
must be nonnull and callable; null remains an unmasked native fault, not a source
exception. Import cells are not eagerly validated or cached together. The table
and source metadata remain valid throughout callbacks; caller-owned storage and
private scratch may not overlap metadata. Quiescence excludes asynchronous
mutation and access after free. These native bodies have no local EH cleanup.
Source callback/provider failures retain the already published field and completed
count changes, without rollback; callers own unresolved credits and disposition.

The already compatible actual94h B84D00/B84D40 scalar setters and existing scalar
terminal remain unchanged. AC59A0's FLDZ/FSTP argument construction, original
allocation cleanup state and creator decrement remain obligations of that caller.
This packet does not implement the full GUI acquisition or install a new graph.

Strict `scripts/build.ps1` passed MSVC Win32 and both configured existing CTests
after synchronization to main84b1117e0. One ignored focused executable passed;
its exact command is retained in `local/output/cc10_system_fog_storage_probe.cmd`
(`/MD /EHsc /std:c++20 /O2 /Gy /W4 /WX /fp:strict`, existing libraries,
`/link /OPT:REF /MANIFEST:EMBED`). `NDEBUG` is absent and rejected at compile time.
No tracked tests were added.

The standard call verifier recognizes and skips the three indirect rows (zero
direct checks, zero failures). All three exact CALL sites/operands were separately
decoded and matched against the pinned live/PE bytes; this does not statically
resolve their runtime targets.

The constructor comparison checks the complete148B output with installed cells
and aliased live cells, including signaling NaN/negative-zero bits and opaque64B
preimages. The setter comparison checks zero/nonzero old counts, callback184
replacement and a newly selected decrement import against copied original code,
using real allocation and terminal providers. Source-only checks cover identity
return without imports and missing current0/fresh4 rejection after prior native
effects. Failure cleanup in the fixture is explicitly separate caller disposition.

338B denotes the full live/PE body extents. The copied constructor relocates nine
four-byte absolute constant operands. The copied setter relocates two four-byte
IAT operands and replaces B71975..B7197C (8B current0 load/call sequence) with an
adapter preserving captured ESI as ECX and calling the genuine shared source
family terminal. Those original virtual-load/call instructions and native scalar
callee are not independently compared. Complete unchanged entry schedules around
these declared boundaries are exercised; native exceptions, private stack/register
ABI, arbitrary derived profiles, application integration and game runtime are
not claimed. All Ghidra access was read-only.
