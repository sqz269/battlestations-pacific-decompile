# Native particle preparation and borrowed resources

Addresses: 00af40e0, 00af9f50, 00af10a0, 00af10b0, 00af1120, 00af10f0, 00b4d170, 00b4cb10, 00b0d130, 00b0d140, 007099c0

The eleven complete live Ghidra spans match the installed image. The report
records inclusive ends, final instruction lengths, hashes and native prototypes.
Names are descriptive hypotheses, not recovered symbols. New source interfaces
do not establish native exception or application compatibility.

AF40E0 visits embedded pointer rows at variant+10 with a current signed count at
+30. AF9F50 first recursively visits embedded children+3C/current count+4C, then
members+54/current count+68. It calls each actual member's current virtual14,
reloads the member from the row and reloads its table, then calls virtual0C.
The row and table can change during virtual14. Every loop rereads its owner count
after the callback. These offsets describe observed access footprints; full
producer allocation sizes and maximum row capacities remain unestablished.

The explicit preparation dispatch must execute the captured target on the same
actual member. There is no successful default. This packet does not identify or
implement all polymorphic member classes. The probe instruments these boundaries
to check ordering and reentrancy, and does not claim their real behavior.

AF10A0, AF10B0 and AF1120 return borrowed resources+04, +1C and +18. AF10F0 tests
only the low byte of each of its two stack DWORDs: 00/01/10/11 selects +08/+0C/
+10/+14. B4D170 returns +0C and B4CB10 returns +08. B0D130 reads owner+3C and
tail-jumps to B4CB10; B0D140 reads owner+60, calls B4D170, then tail-jumps to
B4CB10. No helper retains or clones the returned object. Existing correct
B4CB10/B0D130 descriptive names are retained. The existing semantic material
shadow sampler interface remains distinct from these actual raw-pointer leaves.

7099C0 captures first-child+34 before storing mask+48, recurses with the same
mask, then reloads the captured child's next-sibling+3C. No scene or transform
notification occurs. Its native signature is ECX node/stack mask/RET4. The
resource selector uses RET8; other leaves and walkers use plain RET.

AF74A0 now directly uses these helpers. AFD2E0 directly invokes the recovered
AFD130/AFD220 cookie-array routines with the current one-bit pattern and retains
the existing reverse member unwind. Model construction still requires the real
member virtual dispatch, canonical ownership bindings, renderer stream factory
and random-range service. Full AF74A0 execution has not been established.

Validation: strict MSVC Win32 compilation and one ignored native-byte probe pass.
The probe executes all eleven original routines, including two relocated tail
jumps. It compares reentrant child/member/variant traversal, negative/zero counts,
all sixteen pairs of selected low/high-byte inputs, borrowed shadow chains and
three full 1488-byte hierarchy images (79 assertions). Original virtual member
implementations, exceptional paths and gameplay were not executed. The call
verifier checks four direct CALL rows; two virtual calls are explicitly indirect.
Two JMP rows are recorded separately and executed by the native-byte probe.

## AM combined validation and saved analysis

The combined strict MSVC Win32 build and both existing seeded CTests passed.
Eight call reports check200 direct CALL rows without failures. All seven focused
replays pass within their documented boundaries. Saved names, native signatures,
full body ranges and old-comment preservation were read back; affected exports
were refreshed. The report embeds the earliest annotation preimages and repair
records. Earlier worker pending notes describe isolated snapshots. No original
exception ABI, complete application composition or gameplay claim is added.
