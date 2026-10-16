# CC12 native CRT `__stricmp` parent 46

The complete Original parent at `00BF7FBF` is reconstructed in
`src/native_crt_stricmp.cpp`, using the existing `00BF7EB7` ASCII leaf and
explicit borrowed services. This provides the parent body for a supplied CRT
domain. It does not provide the live locale flag, errno/TLS owner, invalid
parameter handler, locale comparer, or production GUI comparator composition.

## Original evidence and ABI

The inspected project is `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`. Live export, prototype, listing and bytes agree on
`[00BF7FBF,00BF800F)`: 80 bytes, 32 instructions, seven basic blocks and four
callees. The current library name `__stricmp` is preserved. There was no body
repair or change to a neighboring function. The baseline is
`b8aa1de49f4b2c77c880347df52f88628574ba0f`.

Original ABI: Win32 x86 `int __cdecl __stricmp(char* left, char* right)`,
arguments at entry stack `+4/+8` (after EBP prologue, `[EBP+8/+C]`), result EAX,
caller stack cleanup, ESI preserved. The zero-locale valid arm restores ESI/EBP
and tail-jumps to `00BF7EB7` with the original argument homes. The changed-locale
arm pushes null locale, right, left and caller-cleans 12 bytes after `00BF7EEC`.

| Site | Established behavior |
| --- | --- |
| `00BF7FC5` | Compare live DWORD `0109DE1C` against zero before checking inputs. |
| `00BF7FCD`, `00BF7FF1` | Only the zero-flag arm checks left, then right, for null. |
| `00BF7FF8` | Both valid: tail-jump to `___ascii_stricmp` at `00BF7EB7`. |
| `00BF7FD2` | Either null: call `00BFFB8B`, receiving a writable DWORD pointer. |
| `00BF7FD7..DC` | Push five zero DWORD arguments, then store `0x16` (22 decimal) through EAX. |
| `00BF7FE2` | Call `00BF66EF`; caller-cleans 20 bytes. If it returns, return `0x7FFFFFFF`. |
| `00BF8004` | Nonzero flag: call `__stricmp_l(left,right,nullptr)` at `00BF7EEC`, forwarding even null inputs unchanged. |

The caller establishes the returned errno pointer and five DWORD argument
values. It does not establish the complete private prototypes of `00BFFB8B` or
`00BF66EF`; their current incomplete Ghidra prototypes were not rewritten.

## New Source contract

`compare_native_crt_strings_00bf7fbf` takes const input pointers, a borrowed
`const volatile int32_t&` for the actual changed-locale cell, and a required
`NativeCrtStricmpCallees&`. The service interface supplies the actual errno cell,
five-DWORD invalid-parameter call and three-argument locale comparison. It has
no default service implementation or invented global. The flag is read before
pointer tests; each selected service is invoked once, without replay or fallback.

Zero flag with valid pointers uses the existing Source ASCII leaf. Zero flag
with a null input gets the errno cell, writes 22, invokes the handler with five
zeros, and returns `INT_MAX` only if that handler returns. Nonzero flag delegates
immediately with null locale and returns its result. Service exceptions or
termination are not replaced with a success sentinel. A supplied errno pointer
must identify a valid writable external cell; this function does not fabricate
one or add a null-provider fallback.

This is a new C++ API, not an Original entry-point/register/stack/SEH replacement.
The Original pushes handler arguments before its errno store. The Source
preserves the externally visible errno-before-handler order for the supplied
external-cell domain; aliasing an Original private frame or argument home is
outside that domain. Actual locale/TLS acquisition and invalid-handler behavior
remain service contracts, not recovered implementations in this packet.

## Focused differential evidence

The ignored `local/cc12_native_crt_stricmp_parent_46_probe.cpp` verifies the
installed PE is I386, image base `00400000`, and the entire 80-byte wrapper equals
the audited listing bytes. It copies that complete body and audits these five
external operand replacements before changing it from RW to RX:

| Operand / instruction | Original target | Supplied target |
| --- | --- | --- |
| DWORD operand `00BF7FC7` | `0109DE1C` | Probe volatile changed-locale cell |
| CALL `00BF7FD2` | `00BFFB8B` | Shared errno service |
| CALL `00BF7FE2` | `00BF66EF` | Shared five-DWORD handler service |
| JMP `00BF7FF8` | `00BF7EB7` | Existing Source ASCII leaf |
| CALL `00BF8004` | `00BF7EEC` | Shared three-argument locale service |

All original target operands/opcodes, four internal short branches, and every
unpatched byte are checked. The same services are reached through the Source
adapter and relocated Original. Thus this probe isolates the parent; it does
not independently execute the Original ASCII leaf or actual CRT callees.

The `/MD`, `/O2`, `/EHsc`, `/W4 /WX`, C++17 Win32 build uses
`/link /MANIFEST:EMBED`; the PE resource `24/1/1033` contains the verified
`asInvoker` manifest. Safe executable `local/cc12_crt_stricmp_probe.exe` exited
zero. Four cases passed with equal results and service traces:

| Path | Observed result and effects |
| --- | --- |
| Zero flag, `MiXeD!` / `mixed?` | -30; no service calls or errno change. |
| Zero flag, null left / inaccessible nonnull right | `INT_MAX`; getter then handler once; errno 22 visible to handler; five zeros. |
| Zero flag, valid left / null right | Same error result and ordered effects, covering the second pointer branch. |
| Nonzero flag, null left / inaccessible nonnull right | Locale sentinel -1234567; exact pointers and null locale forwarded; no errno or invalid call. |

These cases are not exhaustive fault, exception, locale, private-frame-alias or
Original ABI tests. The immutable Original install hash is
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`;
the parent body SHA-256 is
`59368fa18343310b9ad2c79521361749d54ef8c2a78fd837a44be5ce67efaf6a`.
Exact input, probe and log hashes are in the JSON receipt.

## Integration boundary

The prior `NativeSettingsChoiceCalls::compare_language_00bf7fbf` host-substitution
ledger record remains historical and separate. This packet does not activate a
new settings/GUI comparer, claim complete CRT ownership, or claim native Text
drawing/gameplay validation. Root owns CMake registration after integration;
the focused probe compiles both new Source files directly. The required Win32
Release repository build passed, with all three existing CTests passing in
10.54 seconds. This checks the baseline CMake composition; it does not include
the new parent until Root registers it. The saved, name-preserving annotation
and refreshed export were checked against the unchanged 80 bytes. Exact build
and annotation receipts are recorded in the paired JSON.
