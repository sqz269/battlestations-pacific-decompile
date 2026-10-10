# Allocation statistics base profile: slot zero and scalar deletion

The actual four-byte word at `00D685E0` is `00BE2890`. The separately admitted
physical `BE2890` body calls `BE27F0` on the captured receiver, tests caller flags
bit zero only after that call returns, conditionally frees the same receiver,
and returns its original address with `RET 4`. This closes the base-profile
deletion identity and ordinary scalar schedule. The raw three-word owner and
its Source binding remain held.

Baseline is `13685f5b7c3c5ac802490705d62eb44e5418e10e`. This packet changes only
this document and its paired report. It makes no Source, CMake, ledger, Ghidra,
build, test, probe, startup or runtime change. Root retains annotation ownership.

## Exact evidence scopes

| Scope | Fresh evidence | Boundary |
| --- | --- | --- |
| Base profile slot zero | Exactly `[00D685E0,00D685E4)`, four original/live bytes `90 28 BE 00`, equal | No adjacent word, additional slot or table extent is admitted. |
| Original PE headers | Five separately authorized metadata windows, 256 bytes total | DOS 64B; PE signature/COFF 24B; ImageBase 4B; SizeOfHeaders 4B; exact four-section table 160B. No generic prefix read. |
| Saved target body | `[00BE2890,00BE28A5)` 21B and `[00BE28A8,00BE28AE)` 6B, original/live equal | These remain the saved Ghidra AddressSet, 27B in two ranges. |
| Physical completion | Separately authorized `[00BE28A5,00BE28A8)` 3B, original/live `83 C4 04` | Decode is `ADD ESP,4`; the three bytes remain undefined and unowned in Ghidra. |
| Physical body | Offline concatenation of retained 21B + new 3B + retained 6B | Thirty bytes, eleven fully decoded instructions; no bounding 30B original/live read was used. |
| Original image integrity | Fresh SHA-256 `b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6` | Whole-file hashing is integrity evidence, separate from Native semantic credit. |

Original-image size is 12,223,752 bytes. Size and modification time remain stable
across the exact header/data reads, whole-image hash and later body reads.
Actual PE `.rdata` metadata maps the slot to file offset `009685E0` (9,864,672).
The slot SHA-256 is
`56c1ac916d0bd93899fdfa2e66a879f29384639b4812e9900c85b5bd7d11b59a`.

The gap proposal initially mentioned tentative offset `008028A5`; that value was
corrected before authorization or execution. Actual `.text` metadata maps the
gap to `007E28A5` (8,267,941), the only executed gap offset. The unexecuted mistake
and correction are retained explicitly. The gap SHA-256 is
`268c54fe317c183f03f29b931f4606343f6824184bd185ed1eca129db5729a8d`.

## Typed identity, saved flow and physical instructions

Every fresh typed batch verifies `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`, language `x86:LE:32:default`, default space `ram`,
and image base `00400000`. All 32 typed responses are accepted at modification
19 before and after their batches. Complete raw response bytes, HTTP statuses,
base64 and hashes are frozen. Twenty-seven additional raw GET responses retain
the exact memory reads, saved listing, prototype/fingerprint and identity checks.
No script endpoint, autostart, analysis, save or mutation fallback was used.
Loaded Java CodeSource remains unattested.

`D685E0` is a defined four-byte DataDB item with no instruction or function owner.
Only after original/live equality identified `BE2890` was that target queried.
It is exactly `FUN_00be2890`, non-thunk, `no_return=false`, with no direct thunk
target. Its complete saved ranges are those in the table above. The reported
prototype `undefined FUN_00be2890(void)` is retained as analysis metadata and
does not recover the register or stack ABI.

All ten saved physical instruction starts match the listing and typed lengths.
The entry is a one-byte instruction with ordinary fallthrough. At `BE28A0`, the
five-byte call has the saved `CALL_RETURN` override: default type
`UNCONDITIONAL_CALL`, effective type `CALL_TERMINATOR`, both recorded fallthrough
fields null. Actual direct target `_free` at `BF65AC` is a thunk with
`no_return=false`; its direct target `_free` at `BF9DC8` also has
`no_return=false`. Those are typed target flags, not newly inspected callee bodies.
The call at `BE2893` names the established `BSP_AllocationStats_Destroy` at
`BE27F0` and has no override.

Each of `BE28A5`, `BE28A6` and `BE28A7` remains an undefined one-byte DataDB item
without any instruction, function owner or instruction flow. Physical decode
recognizes one three-byte instruction at `BE28A5`; that does not create a saved
instruction or enlarge the function. The final metadata confirms the original
split AddressSet and override unchanged. A bounded atomic repair route remains
unattested; the disabled historical repair tool was not run.

## Ordinary scalar schedule

At entry, ECX supplies the receiver and the caller flags word is at `[entryESP+4]`.
The body saves ESI, captures ECX in ESI, then calls `BE27F0`. It next tests bit zero
of the low byte of the actual stacked flags at `[ESP+8]`. A clear bit branches
directly to the return tail. A set bit pushes the captured ESI receiver, calls
`BF65AC`, and consumes that one argument with the physically recovered
`ADD ESP,4`. Both normal paths set EAX to the captured receiver, restore ESI and
execute `RET 4`. No receiver null gate or other flag-bit test occurs in this body.
The returned address is not dereferenced after free.

The destructor call precedes the flag load, conditional free and return. A
future ordinary Source wrapper must preserve those observations under its
explicit valid-object/alias contract. No Source signature, cleanup handler,
destructor failure behavior, original exception ABI, fault equivalence or CRT
equivalence is established by this instruction schedule. The retained Root
`BE27F0` lifecycle evidence supplies normal publication/unregister ordering;
its required exceptional cleanup remains a separate dependency.

## Current Source boundary and remaining work

Current Source still constructs a stack-local semantic `AllocationStatsState`
with a null profile, budget `40000000` and zero startup word. Bounded searches
over `src` and `include/bsp`, and the complete current deletion case list, find
no concrete `D685E0`/`D685F4` deletion binding. The shared dispatcher rejects
unbound profiles. These searches do not claim that every Source body was reviewed.

The genuine `BE2700` metric adapter, current-publication cache consumer, canonical
manager and allocator services, mapped read-only data and retained shared drain
remain available as described in the prior readiness review. They do not create
the missing allocated owner or its lifecycle binding. Current Source/readiness
files are frozen against this packet's Git baseline; retained Root lifecycle
receipts are copied and hash-checked without repeating their Native reads.

An actual three-word heap owner still requires genuine base and derived
deletion dispatch before `BE2750` can publish `0109CEFC` or register it, with
bindings retained through the same canonical manager drain. Constructor,
caller-allocation and destructor failure cleanup remain held. No generic
callback, neighboring-family behavior, fake table, private manager or new
constant metric is substituted. Startup/cache composition, full Native ABI and
gameplay remain unproved. Existing Source648 build/tests are context only.
