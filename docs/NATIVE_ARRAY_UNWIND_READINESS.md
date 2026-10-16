# ArrayUnwind readiness

`source_ready=false`, `source_authorized=false`, Source credit **0**.
This bounded review retains the existing library name `__ArrayUnwind` at
`00BF7C10`. It establishes the selected body's static normal-path contract,
while the actual destructor, SEH, exception-filter/handler and terminate-provider
ownership remain incomplete. It authorizes no Source implementation.

## Exact selected body and evidence

| Selected object | Inclusive bounds | Size |
| --- | --- | --- |
| First body range | `00BF7C10..00BF7C32` | 35 bytes |
| Second body range | `00BF7C5F..00BF7C6D` | 15 bytes |
| Scope window | `00E02D10` through `00E02D2B` | 28 data bytes |

Fresh typed metadata identifies exactly two body ranges totaling **50 bytes**,
with 14 instructions. The 94-byte convex span is not the function body. The
44-byte gap `00BF7C33` through `00BF7C5E` was not captured as a selected byte
range, listed or independently decoded. The complete PE was read for hashing
and section mapping; that file read includes the gap bytes. The full actual body captures
were pinned before substantive analysis; leases were extended only after the
initial metadata confirmed ownership.

The installed PE SHA-256 is
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
Each selected range and the scope window match its fresh live bytes exactly.
Independent Capstone 5.0.7 x86-32 decoding agrees with all 14 captured starts,
lengths, mnemonics, typed flow flags, targets and fallthroughs. No selected
instruction has a flow or fallthrough override. Normal body edges stay within
the selected ranges except the two direct SEH helper transfers and the opaque
destructor transfer.

The four initial supported capture batches verified existing
`C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`, with autostart
disabled. Each captured before/after counter was 6616; the subsequently authorized
three-entry dependency-metadata batch was internally stable at 6618. These dated
records do not establish global database immutability. The earlier 314+338+339 sealed
files and seven working references were pinned before this stage; their bytes,
hashes, file mtimes and family directory mtimes are preserved.

## Physical frame and normal path

The observed prolog transfer is `00BF7C17 CALL 00C07C00`, after pushes of
local size `14h` and scope pointer `00E02D10`. Let E be entry ESP and F the
parent EBP. Instantiating the existing qualified SEH4 contract gives F=E-4
and saved ESP P=F-34h; saved EBX/ESI/EDI are at F-28h/F-2Ch/F-30h, cookie
at F-34h, registration at F-10h, encoded scope at F-8, and try level at F-4.
These are qualified frame formulas, not fresh proof of the actual cookie,
handler, FS chain or dispatcher providers.

The four physical argument homes are interpreted as current end/destination
`[F+8]`, raw DWORD stride `[F+0Ch]`, count `[F+10h]`, and destructor entry
`[F+14h]`. Existing metadata declares a four-argument stdcall void function
with a signed count; the instructions define its edge cases.

The body sets try level to zero using AND, then decrements the **physical count
home modulo 2^32** at `00BF7C20`. JS at `00BF7C23` tests the resulting sign
bit. It does not implement a signed `count <= 0` comparison or test overflow.

Under stable, valid homes and a normally returning provider:

| Initial raw count | Static consequence |
| --- | --- |
| `00000001` through `7FFFFFFF` | That many destructor transfers; final stored count `FFFFFFFF` |
| `00000000` | No destructor transfer; stored count becomes `FFFFFFFF` |
| `80000000` (signed INT_MIN) | First DEC yields `7FFFFFFF`; 2^31 destructor transfers before final `FFFFFFFF` |
| `80000001` through `FFFFFFFF` | No destructor transfer; count still decremented once |

These are instruction-derived consequences, not executed tests. In particular,
an ordinary signed C++ decrement at INT_MIN would not preserve the raw wrap
semantics without an explicit representation contract.

For each nonnegative post-decrement count, the body loads `[F+8]` into ECX,
subtracts `[F+0Ch]` modulo 2^32, writes ECX back to `[F+8]`, and transfers
through `[F+14h]` at `00BF7C2E`. Thus it updates the destination home **before**
invoking the destructor. With a stable conventional positive stride and valid
end pointer, this visits preceding elements in reverse order. Those input
properties are not validated. A zero stride repeats an address; high-bit strides
and wraparound retain raw DWORD subtraction semantics.

The destructor receives the computed address in ECX. No explicit stack argument
is pushed, no caller stack adjustment follows, and no return value is consumed.
The real provider must return with the call stack balanced and preserve the
borrowed frame/register obligations. This does not establish a deleting-destructor
flag parameter or any particular provider identity. The count, pointer, stride
and destructor homes are reread across iterations; callback aliasing or mutation
of those homes cannot silently be replaced by stable C++ parameters.

When JS selects the normal exit, the body sets try level=-2. Its epilog transfer
is `00BF7C66 CALL 00C07C45`, followed by `RET 10h` at `00BF7C6B`. The qualified
epilog requires ESP=P before transfer and restores ESP=F+4. The outer return
cleans **four DWORD arguments / 16 bytes**. No cdecl caller-cleanup assumption
is compatible with this return instruction.

## Exception boundary and missing owners

The exact 28-byte scope window was separately leased after observing its pointer
and try level zero. Its size follows the qualified EH4 hypothesis of a 16-byte
header plus one 12-byte level-zero record. Its DWORDs are
`FFFFFFFE, 00000000, FFFFFFCC, 00000000, FFFFFFFE, 00BF7C33, 00BF7C5C`.
This is a bounded opaque data observation, not a complete typed scope table.
The candidate filter and handler pointers are in the excluded body gap. Authorized
entry-only metadata queries find **no function and no instruction** at either
pointer: both are undefined length-one DataDB units. Their proto queries genuinely
report `No function found`. No listing repair or extrapolated body was attempted.

If the destructor raises, the count has already been decremented, the destination
home already contains the attempted element address, and the try level remains
zero. Actual dispatch, filtering, parent EBP/ESP restoration and any route to
termination depend on external providers. No direct terminate transfer exists
in the selected 50-byte body. A named/indexed terminate dependency cannot prove
the unreviewed filter/handler transition or its exception policy. Current metadata
names `00C07A75` as `terminate`, non-thunk, with `no_return=false`; that flag is
not evidence that the real service returns. Its metadata body is 44 bytes in two
ranges, `00C07A75` through `00C07A94` and `00C07A9C` through `00C07AA7`; the provider body was
not read. Its entry-only effective flow is FALL_THROUGH with no override.

| Dependency | Current boundary | Required next contract |
| --- | --- | --- |
| `__SEH_prolog4` / `__SEH_epilog4` | Existing qualified frame formulas | Actual frame, FS chain, cookie and dispatcher composition |
| Scope candidates `00BF7C33`, `00BF7C5C` | No function/instruction at either entry; bodies excluded | Explicit ownership/listing decision, then filter/handler behavior and exceptional transitions |
| Destructor `[F+14h]` | ECX input, no explicit stack arguments | Real provider identity, stack/register/frame preservation, mutation and exception behavior |
| Terminate provider `00C07A75` | Named split-body metadata only; no-return flag false | Actual exceptional transition and termination/nonreturn contract |
| Callers `00BF7CB9`, `00BF7D1E`, `00BF9AE7` | Indexed labels only | Real argument validity and provider/frame ownership; no caller bodies reviewed |

The next work must separately own and review these dependencies. A normal-only
destructor loop cannot earn whole-entry Source credit. The prospective interface
must retain four physical homes, raw decrement/subtraction, mutable-frame
semantics, actual providers and outer 16-byte callee cleanup; no C++ interface
has been implemented.

Evidence is in `local/NativeArrayUnwindReadiness01`. The companion JSON supplies
the two exact direct call sites to the repository checker. This review grants
zero Source, build, fixture, physical-ABI, caller, Native-runtime, original-game
or gameplay credit. No compiler, build, tests, probes, target-code execution,
provider/caller body expansion, SDK comparison or Ghidra mutation occurred.
