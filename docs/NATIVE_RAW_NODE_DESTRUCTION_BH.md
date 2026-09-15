# Raw structured-node destruction BH

Address: 00BE9DF0. Discovery base: 3faa70a29aaca3a841251e22f55f7a003a91b6a0.

`destroy_native_structured_node_00be9df0` implements the complete destructor
against an existing actual24h node. It takes the existing
`NativeStringRawPoolContext`, so native string cleanup reaches the actual pool
publication and manager context without a new provider or synthetic global.
The original routine takes ECX=node and RET; the added C++ context means this
source interface is not a drop-in ABI replacement.

## Behavior and field order

The source stamps D68BB4, then checks node+08h. For a nonnull reader it reads
parent+0Ch and, when parent is nonnull, captures the FULL declared payload at
node+1Ch before subtracting it from parent+20h with DWORD wrapping arithmetic.
It reloads node+08h after this debit, decrements reader+60h, and clears node+08h.
It then destroys the actual8h name header at node+10h through the existing raw
pool overload of `destroy_native_string_header_0041dd20`.

`destroy_native_ref_counted_base_00bd30f0` runs on completion or unwind and stamps
CEB130. The native E01A90 unwind map has one state0 -> -1 action,
CC70A0 -> BD30F0, armed before the reader check and disarmed before the normal
base call. Source `__try/__finally` preserves that cleanup schedule, including
exceptions from the string-pool getter. It does not claim the same FH3 metadata.

The destructor neither seeks nor alters node remaining+20h, count+04h,
declared+1Ch, depth+18h, parent+0Ch or name header words. It does not clear the
reader path-name copy, retain/release the parent or free node allocation. Raw
field accesses use volatile DWORD operations to preserve the observed reloads
and stores, including the reader reload after a potentially aliasing parent
write. Valid native storage is a caller precondition; no guard is added.

## Evidence and validation

Current read-only BSP queries verified `bsp.gpr`, program
`/battlestationspacific.exe`, before each analysis query. The full 134-byte
BE9DF0 body and four complete EH spans match the installed PE with SHA256
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
The companion report preserves the three original direct CALL rows with exact
stack/register provenance and their current verification result. EH action
bytes remain data evidence, with no new Ghidra function definitions.

The full Release `scripts/build.ps1` build uses MSVC Win32 `/W4 /WX /fp:strict`.
`verify-seeds` enables both existing CTests: `reconstructed_math` and
`native_math_differential`. The build, both tests, the local probe below and all
three direct native-call rows passed. These tests validate the existing suite, not native
gameplay execution of the new destructor.

One local fault-injection check protects the actual pool-publication page. The
resulting access violation occurs during name cleanup after detachment. An
outer exception handler verifies that CEB130 was stamped, the parent was debited
by the full declared value, reader path count was decremented, the reader was
cleared, and every other node word remained unchanged. This deliberately
inaccessible publication cell tests unwind behavior; it is not a valid-input
gameplay fixture. The local probe links the current built archive with an
embedded manifest, and no new persistent test suite or provider was added.

The report binds the current source and header to their exact compiler command,
read and write tlog blocks, then to the current object. The archive contains one
matching member whose bytes equal that object. Source/header/object/library
sizes and SHA256/SHA512 hashes are recorded, with retained compiler logs,
full build/test output, probe artifacts and native bytes. The full local
evidence inventory is embedded in the report outside its own inventoried folder.

## Integration scope

Only a deferred source registration was added to `cmake/startup.cmake`.
`CMakeLists.txt`, shared ledgers and Ghidra analysis were not changed. Allocation
BEA680/BEA250, reader BF0510 concrete slot48, deleting destructor BE9FC0 and
intrusive wrapper dispatch BE9ED0 remain separate packets. This change closes
the raw destructor source contract, not the entire node lifetime chain.
