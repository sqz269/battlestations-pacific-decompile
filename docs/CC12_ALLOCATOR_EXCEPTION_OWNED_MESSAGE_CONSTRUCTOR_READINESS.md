# Allocator exception owned-message constructor: readiness

This read-only packet covers exactly `00BF6340..00BF638D`: **78 bytes and
33 instructions**, ending in `RET 4`. Fresh metadata, the complete live
listing, live bytes and the original PE agree. There is no missing instruction,
x87 operation, unexplained register input or flow gap. The saved library name
**`exception` is retained** without a Ghidra mutation.

Baseline: `fccaff11e849bd760a6384c25a3762345addfe41`. Only this document and
its JSON evidence report are changed. No Source, CMake, ledger, Ghidra, build,
test or probe work is performed and no admission credit is applied.

## Gate and evidence boundary

The verified target remains project `bsp`, file `C:/Users/sqz269/bsp.gpr`,
program `/battlestationspacific.exe`, x86 at image base `00400000`. Each live
query used the verifying `bsp.py ghidra` client. Metadata reports five blocks,
six edges and three calls in the owned body. Its saved prototype is
`undefined __thiscall exception(exception * this, char * * param_1)`.

The decompiler identifies the library match as
`public: __thiscall std::exception::exception(char const * const &)` from
Visual Studio 2005 Release and displays an `exception*` result. The complete
body establishes entry `ECX` as receiver, one stack-word slot address, and
`EAX` as the receiver on ordinary return. These distinct metadata and machine
observations are retained without rewriting the saved type or name.

Whole body SHA-256:
`37d4ff77838e865a70c14dfc5f56b19ed7f1096b0953472608e6c21bb801ef06`.
The JSON retains all 78 bytes, all 33 decoded instructions, the full live
listing and all branch/call targets. No byte at adjacent `00BF638E` was read.
Only the three reached CRT children received metadata queries; their bodies,
Native callers, handlers, profile data, static owners, slot contents, strings,
RTTI and throw data remain outside scope.

## Complete operation schedule

Let `T` be entry `ESP`, `D` entry `ECX`, and `P` the DWORD at `[T+4]`.
`P` is an actual pointer-slot address, not a pre-copied character pointer.

| Address range | Owned behavior |
| --- | --- |
| `6340..6347` | Save `EBX`, capture `P` into `EBX` from `[ESP+8]` after the first push, save `ESI`/`EDI`, retain `D` in `EDI`. |
| `6349..6353` | Publish raw `00D69370` at `D+0`; then load `M0 = DWORD[P]`, test it and branch to `637B` if zero. |
| `6355..635D` | Push `M0`, call `_strlen`, copy its 32-bit result `L` to `ESI`, increment to `N=(L+1) mod 2^32`. |
| `635E..636B` | Push `N`, call `_malloc`, test result `Q`; pop both retained arguments into `ECX`; publish `Q` at `D+4`; branch to `637F` if `Q` was zero. |
| `636D..6379` | Reread `M1 = DWORD[P]`; push `M1`, `N`, `Q`; call `_strcpy_s`; discard its return value, add 12 to `ESP`, jump to `637F`. |
| `637B` | Initial-null-message path: actual DWORD read-modify-write `AND [D+4],0`. |
| `637F..638D` | Write full DWORD `1` at `D+8`, set `EAX=D`, restore `EDI`/`ESI`/`EBX`, return with `RET 4`. |

The initial ownership DWORD is never read by this body. The final write is a
complete four-byte replacement with `1`, including the initial-null and
allocation-null paths. It is not a byte store, Boolean normalization of an
old value or bitwise update. No old message is freed and no previous profile
value is read.

The malloc-result `TEST` survives both `POP ECX` instructions and the `D+4`
publication: those operations preserve flags, so the `636B` branch still
tests `Q`. Even a null allocation is published before ownership becomes `1`.
The null-message path's `AND` has a real prior DWORD read at `D+4`; replacing
it with a plain zero assignment would omit that explicit access contract.

There is no length-overflow check or zero-size normalization. In particular,
`L=FFFFFFFFh` yields `N=0` and is passed to `_malloc`. Nullable allocation is
accepted; there is no throwing allocation wrapper or retry in this body.
If allocation is nonnull, `_strcpy_s(Q,N,M1)` is called with the late slot
value and its result is ignored. Normal return from that child reaches the
ownership write regardless of its integer result. Invalid-parameter,
overlap, error, exception and nonreturn behavior belong to the uninspected
child/provider contract. A successful allocation is not proof of a valid
copied string.

## Slot identity, aliases and access order

`P` is captured before profile publication and retained in `EBX` across the
children under their metadata-declared ordinary cdecl contract. The entry
argument word is not reread to obtain a new slot address. Its first pointed
value is read only after `D+0` changes; its second pointed value is read only
after the nonnull allocation has been stored at `D+4`. There is no late slot
read on the initial-null or allocation-null path.

For ordinary nonconcurrent accesses, if `P=D+4`, the late read observes the
published `Q` and supplies it as both destination and source to `_strcpy_s`.
If `P=D`, the initial slot read sees the newly published profile word; no
string validity is inferred. If `P=D+8`, both slot reads precede the final
ownership write, subject to intervening child effects. Child/alias changes
can make `M1` differ from the message used to obtain `N`; no remeasurement or
source-identity check is added. These are conditional alias consequences,
not claims that such inputs are valid strings or valid CRT calls.

The minimum direct receiver extent is 12 bytes. `D+0` and `D+8` require
four-byte writes; `D+4` requires a write on every ordinary path and a prior
read on the initial-null path. `P` requires a readable DWORD, and is read a
second time only on allocation success. The owned instructions do not read
the message's characters; child contracts do. No receiver or slot null guard,
atomicity promise or local catch/cleanup protects earlier partial effects.
For example, a slot fault can follow profile publication, and a late slot or
copy failure can follow message-pointer publication before ownership is set.

## Stack, registers and flags

The saved-register area is `T-12`. The first character pointer stays pushed
across the strlen call while the allocation size is pushed above it. After
malloc returns, two `POP ECX` operations restore `ESP` to `T-12`. On the copy
path, three argument pushes and a cdecl return leave `ESP=T-24`; the explicit
`ADD ESP,12` restores `T-12`. The final restores and `RET 4` produce `ESP=T+8`.
The deepest owned call return-address push reaches `T-28`; children may use
additional stack. There is no local EH setup or x87 work in these 33 operations.

On ordinary completion `EAX=D`; `EBP` is untouched, and the saved words are
popped back into `EBX`, `ESI` and `EDI`. Register preservation assumes normal
child returns and uncorrupted saved stack words. On the initial-null path,
`ECX` remains `D` and `EDX` is unchanged. On allocation-null return, `ECX`
comes from the retained strlen-argument stack word, normally `M0`; child
effects on `EDX` remain unproved. On the copy path, final `ECX`/`EDX` depend on
`_strcpy_s`. No extra register-preservation promise is inferred from its name.

The initial-null path returns arithmetic flags from the memory `AND`:
`CF=OF=SF=0`, `ZF=PF=1`, `AF` undefined. The allocation-null path returns the
same arithmetic values from `TEST Q,Q` with `Q=0`. The copy path returns
arithmetic flags from `ADD ESP,12`, based on the actual stack value; they are
not fixed constants or preserved strcpy flags. Remaining flag effects of
children, hardware faults, exception delivery and arbitrary stack aliasing
are not established.

## Current Source contracts and receipts

The actual header declares `NativeLegacyExceptionStorage::base_message_04`
as **`char*`**, with neither pointer nor pointee const qualification. Its
ownership field is `std::uint32_t`. The 40-byte typed owner has asserted
fields at `+0`, `+4`, `+8`, with its member at `+0Ch`. That actual owner can
provide the leading 12 bytes; bare 12-byte storage is not cast to this type.

The existing admitted base-message and failure-object interfaces accept
**`const void* actual_pointer_slot_address`**. This is an address of a slot,
not a pre-copied `const char*` message value; the headers do not declare a
typed `char**` slot API. The qualifier does not
prove slot stability or Native string ownership. Their different admitted
contracts do not implement this owned-message body. An exact address search
for `00BF6340`/`00BF638D` in `include/bsp` and `src` found no current provider;
that bounded search does not exclude differently named/generated code.

The existing raw copy Source uses callable `strlen`, `malloc` and `strcpy_s`
through `<cstring>`/`<cstdlib>`, with `#pragma function(strlen)`. Its earlier
primary report resolved them to UCRT string/heap imports. Those recorded
addresses and provider identities remain historical Source evidence, not
fresh final-image checks or Native CRT/ABI/failure-policy proof.

The latest default-constructor primary receipt records the successful normal
build, three existing checks, exact 17-byte emitted/selected leaf, two actual
typed calls and all eight current owner EH sections. The private helper has
36-byte FuncInfo, flags `5`, max-state `0`; the public constructor has a
60-byte EH section, flags `1`, max-state `3`, including a terminate entry
before its member guard. These are reviewed Source compiler effects under
the retained typed `noexcept` policy, not Original EH parity and not evidence
of a new owned-message consumer.

All **227** references from the five constructor/copy/cleanup receipts match
their own historical commits. Of those, **221** also match this baseline.
The six historical differences are four CMake references and two older
legacy-owner references. All **57 build pins plus the current primary's
document pin** match current files. The report retains each version and hash;
no older CMake/owner receipt is silently treated as current. Builds, artifact
graphs and execution were not rerun by this audit.

A future primary-selected Source interface must borrow the actual slot
address, preserve both slot reads, allocation/null paths, RMW, full ownership
write and machine schedule. Implementation, registration, emitted review,
Core/consumer/EH review and admission remain separate work. Numeric profiles
create no callable table, RTTI, static owner, Native exception/throw identity
or lifecycle proof. Original placement/ABI, runtime, startup and gameplay
remain unproved; the earlier 24-, 25-, 88-, 22- and 17-byte admissions are not
counted again.
