# List count guards

Both routines conditionally throw `std::length_error("list<T> too long")` or
increment the existing list count. The new code reconstructs both normal and
error paths at Source level using the genuine current SDK exception type.
It does not reproduce each native instruction or the native FH3/legacy CRT
object/frame identity. That distinction applies to every completeness claim.

| Native entry | Inclusive byte extent | Original ABI | Coverage | Source entry |
| --- | --- | --- | --- | --- |
| old `STL_xlen_throw_00420140` | `00420140..004201D2`, 147 bytes | thiscall, ECX header, stacked uint32 addition, RET 4 | complete Source normal/error behavior; Native FH3/legacy exception ABI excluded | `increment_native_list_count_00420140` |
| retained `STL_inst_0041fdd0` | `0041FDD0..0041FE62`, 147 bytes | same | complete Source normal/error behavior; Native FH3/legacy exception ABI excluded | `increment_native_list_count_0041fdd0` |

The Source entries use fastcall `(header,unused_edx,addition)`, retaining ECX
header, stacked addition and RET 4. EDX is ignored. This input/cleanup shape
does not establish full native register/flags/stack-frame ABI equivalence.
No Source result is promised.
The existing raw header must have a readable/writable count word at `+8h`
within its `0Ch` extent. No alternate class or owner lifetime is introduced.
Header `+0h/+4h` are untouched; only the normal path writes `+8h`.

## Complete Source word schedule

The native addition parameter is captured at `00420146` / `0041FDD6`, then the
current DWORD count at `00420159` / `0041FDE9`. Each body computes
`uint32(3FFFFFFFh-current)` and compares that value unsigned with addition
(`00420160..0042016A` / `0041FDF0..0041FDFA`). JNC selects normal completion
when the wrapped available count is greater than or equal to addition.

Source addition is a captured by-value input. The Source reads the actual
count once, computes the same unsigned difference,
then either constructs and throws the actual current SDK `std::length_error`,
or stores the wrapped sum of the captured count and addition. The native normal
ADD/store sites are `004201BD..004201BF` / `0041FE4D..0041FE4F`. There is no
intervening external call on that normal path, no count reload, no signed
arithmetic, no clamp and no separate check that current is below the limit.

The optimized Source listing physically reads the count before loading the
stacked argument, whereas Native loads addition first. Both operands are
captured before any error-provider call or count store. Physical argument-read
and fault ordering, and alias/mutation backing for original native frame cells,
are not reproduced. The listing confirms unsigned SUB/CMP/JB, a single normal
sum/store, ECX header, stacked addition and RET 4 in both Source entries; it
also confirms the genuine SDK error constructor and C++ throw service.

| Current | Addition | Source/native decision and normal result |
| --- | --- | --- |
| `00000002h` | `00000003h` | accept, store `00000005h` |
| `3FFFFFFFh` | `00000000h` | accept, store same value |
| `3FFFFFFFh` | `00000001h` | throw; this routine has made no count store |
| `40000000h` | `00000001h` | wrapped available `FFFFFFFFh`; accept, store `40000001h` |
| `FFFFFFFFh` | `00000001h` | wrapped available `40000000h`; accept, store zero |

On error there is no count store before exception construction. Provider-side
effects during exception allocation/handling are not rolled back or overwritten.
Construction can itself fail through the current SDK's genuine exception and
allocation machinery; the Source adds no catch, alternate exception, retry,
success callback or admission policy.

## Native error sequence and qualified Source composition

Each native error arm initializes a local legacy character string with length
zero, capacity 15 and an initial NUL, then assigns the 16-byte message from
`00CE38F8`. The following exact sites were checked against the callee bodies:

| In `00420140` | In `0041FDD0` | Native callee and inspected contract |
| --- | --- | --- |
| `0042018C` | `0041FE1C` | `00408720` counted legacy-string assignment; ECX destination, stack message/count, RET 8 |
| `004201A1` | `0041FE31` | `00411700` constructs native legacy logic-error base/string member from the temporary; RET 4 |
| `004201B8` | `0041FE48` | `00BF6885` C++ throw helper, stack object and ThrowInfo `00D83F98` |

The callers then install native length-error vtable `00D69260` before throwing.
Both owned native bodies also publish/restore the FS exception chain and carry
different FH3 handler addresses (`00C5E3D8`, `00C5E378`). The inspected throw
helper constructs a RaiseException record from `00D693B0`; its flags DWORD is
1 and its exception code is `E06D7363h`. It contains a RET after RaiseException,
but this packet supplies no native handler/control-backing or resume contract.

The Source intentionally implements the whole error operation with
`throw std::length_error("list<T> too long")`, using `<stdexcept>` from the
installed current SDK. The inspected header's real `length_error(const char*)`
constructor delegates to `logic_error`; there is no fabricated exception type
or throw provider. Existing legacy-string/exception services were also read,
but mixing their old object representation with a current C++ throw would
claim an unproved ABI. The Source does not construct those native locals or
preserve their number/order of allocations, vtable stores, copy/unwind paths,
FH3 records, ThrowInfo bytes, original failure sites or CRT handler identity.
Current Source exception allocation, message storage, catching and unwinding
are explicit qualified boundaries. No original instruction-byte equivalence is
claimed for either routine's complete 147-byte body.

## Header producers and all native callers

The registry list constructor `0041E270` and registry producer `00424730`
previously establish sentinel header+4 and count header+8 zero, while preserving
proxy header+0. Those complete producer reconstructions and the existing
`Cc13AvoidZoneRegistryNative` schema agree with this raw interior header.

All five live call sites were inspected. Registry construction has:

* `004247C2 CALL 00420140`
* `0042481E CALL 00420140`
* `0042487A CALL 00420140`

Stream loading has `0042495A CALL 00420140`. Each supplies addition 1 and
the registry+4 list-header address in ECX, after allocating a node and before
linking it. Grid generation has `004238BF CALL 0041FDD0`; it supplies addition
1 and a raw list header at its current record+18h. Its adjacent `0041C0B0`
producer allocates a `0Ch` next/previous/pointer-value node, followed by count
increment and link stores. That body and argument setup were read, but this
packet does not equate that outer record or payload class with the registry.

## Name correction, verification and limits

The old saved name `STL_xlen_throw_00420140` and inventory tag
`stl_throw_site (high, rename)` were recorded before annotation. There was no
name-ledger record at that address. The new descriptive name is
`STL_List_IncrementSizeChecked_00420140`: the verified normal path returns
after incrementing, rather than unconditionally throwing. The generic
`STL_inst_0041fdd0` name is retained. These remain descriptive STL hypotheses.
This packet edits only leased-address name/reconstruction ledgers; primary
owns saved Ghidra name/comment/tag changes and refreshed exports.

Analysis used `C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`, with live
identity/prototypes, complete owned assembly, native error callees and all
caller setups inspected. MSVC x86 `/std:c++17 /EHsc /W4 /WX /permissive- /O2`
compilation passed without warnings, and both emitted public entries were
inspected in `local/goal15c_list_count_guards/native_list_count_guards.asm`.
All 11 report calls and five markdown call claims passed with no failures,
unchecked claims or warnings. The ignored, manifest-embedded Win32 probe at
`local/goal15c_list_count_guards/list_count_guard_probe.cpp` passed all ten
executions: five table cases through each entry, also checking the error type,
message and untouched header words. `probe.cmd` rebuilds and runs it. This
executes only current Source code, not the native routines. Primary owns full
registration/build. No new tracked test framework/target, native differential
test, game execution or native ABI proof is claimed.
