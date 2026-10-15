# Qualified compiler sampler entry word

The cached-pixel compiler path now has a bounded producer for the complete
sampler entry word. It executes the existing, one-time pixel COM `Release` in
a checked Win32 caller layout and captures the word immediately. The compiler
publishes that result only after the later native owner operations prove that
they did not enter a destructor. No builder value, cache-row value, zero, or
ordinary C++ stack residue is published as a replacement for the capture.

This supports the recorded current Windows/D3D9 profile and the actual cached
pixel path. Other compiler modes, null COM results, changed code/dispatch,
different counters, and unproved later releases remain unknown. The existing
`B3B328` consumption guard and strict full-DWORD sampler API are unchanged.

## Original producer and intervening writes

Let `P` be `B3B3C0`'s steady ESP. Both descriptor calls use `W=P-30h`, which is
the sampler loop's `B-16`. The following offsets are from actual instructions,
not from the C++ compiler's layout.

| Event | Effect on W |
| --- | --- |
| Cached pixel lookup `B3BD66`, then `MOV EBX,EAX` at `B3BD6B` | EBX is the exact returned cache-row pointer. |
| Pixel constructor called at `B3BFB6` | Entry ESP is `P-8`; three EH pushes, `SUB ESP,18h`, and `PUSH EBX` at `B5F9C8` write that row to `P-30h`. The saved register is protected through the constructor. |
| Current D3D9 pixel `Release` at `B3BFD7` | Its guard constructor, D3D9 RVA `489E4`, pushes caller EDI at `P-30h`. EDI is the actual native builder. This replaces the constructor's row. |
| Current `RtlLeaveCriticalSection` | Its entry is `P-24h`; saved EBP is at `P-28h`, and local `[EBP-8]` is W. The contended path initializes this local to zero and can pass it to the actual backoff helper. Capturing the result is necessary. |
| D3D9 device `Release` | The admitted nonfinal path saves only EBP/EDI at `P-28h/P-2Ch`; it does not reach W. The admitted CFG check is the current `RET 0` body. |
| `B5F080` on the fresh null pixel slot | Publishes the wrapper and increments its actual count `1 -> 2`. Current Kernel32 increment reaches only `P-18h`. There is no old-owner release. |
| Caller decrement at `B3BFE7` | Current Kernel32 decrement reaches only `P-0Ch`; the actual `2 -> 1` result excludes `B3BFF7`'s destructor call. |
| Root/mode setup | No intervening write to W. Root and mode keep the existing shared physical-word model. |
| `B5F100` | Replaces W's low stage byte, then copies the complete DWORD into texture row `+4`. Upper bytes are preserved. |

Seven live Ghidra spans, totaling 1,153 bytes, match the original PE. They cover
the sampler, cached pixel lookup/create schedule, compiler tail, shader setter,
texture append, pixel constructor, and initial pass construction. Each live
query verified the existing `bsp` project and `/battlestationspacific.exe`.
Existing function names and comments were preserved; evidence was appended to
`B3B3C0` and `B5F9B0`, the project saved, and their exports refreshed.

## Checked boundary

`src/native_sampler_entry_word_boundary.inc` is compiled by the existing actual
compiler translation unit. Its naked x86 bridge defines P below its own saved
caller state and reproduces the relevant original register/call schedule:

| Input | Binding |
| --- | --- |
| EBX | The retained `B34890` return from this invocation's actual pixel lookup. |
| EDI | The same actual builder as the compiler parent. |
| ESI | The same actual constructed pixel wrapper. |
| EBP | Zero, established natively at `B3BF93`. |
| EDX | The current pixel COM vtable. |
| EAX | Its current `+8` Release entry, recorded by the bridge before dispatch. |
| ECX | The original relative previous-EH-link position `P+D0h`. The admitted D3D9 body overwrites both saved copies before reading them. |
| Flags | The original `CMP COM,0` result before the flag-preserving setup. |
| Stack argument | The same pixel COM pointer; one actual Release call. |

The sole modeled preimage is the proved constructor write of EBX to W. The
bridge captures W before any following call and records the COM result.
It restores ESP and all four callee-saved registers. EAX/ECX/EDX and flags are
caller-clobbered. No instruction in the admitted producer paths copies the
bridge's different return PC or its caller's C++ frame into W.

This is a normal-return boundary. It does not reconstruct the original full
compiler frame, private FH3/SEH, an exception through D3D9, or arbitrary hooked
COM implementations. The relative EH-link input is not a claim that the
original exception registration was recreated.

Before dispatch, the compiler checks current module identities, PE metadata,
eleven complete relevant code spans or explicitly bounded nonfinal prefixes,
HIGHLOW relocations, CFG and critical-section import bindings, the current
shader/device vtable entries, and their actual storage relationship. The
current profile includes D3D9 shader Release/guard/counter/AddRef,
device AddRef/nonfinal Release, CFG, NTDLL critical-section exit/backoff, and
Kernel32 increment/decrement. Code is checked again before granting knowledge.
An OS update or different current implementation receives no default credit.

Preflight makes no COM calls and adds no reference. Unsupported preflight uses
the existing single ordinary Release and retains unknown knowledge. Once the
bridge is entered it is never retried.

## Counter and ownership proof

The fresh wrapper constructor initializes actual `+04` to one. Canonical shader
registration borrows that same identity without a retain. The slot operation
now records its actual `fetch_add` result; qualifying requires old slot null,
published incoming identity equal to this wrapper, and prior count one.

The compiler's pixel caller release records its actual `fetch_sub` result.
It preserves the canonical helper's order exactly: decrement the same actual
`+04` with sequential consistency, then call
`dispatch_native_render_actual_owner_zero` only when the prior count was one.
This is the same canonical zero dispatch, not a separate destructor provider.
Qualifying requires prior count two, so no native zero-terminal arm ran.
The ordinary vertex release and all other shared releases are unchanged.

The same renderer retains its actual D3D device throughout this borrowed
compiler invocation. Independent retirement/replacement is excluded by the
domain contract; the GUI parent holds that domain through acquisition and its
later renderer drain. The checked D3D9 guard constructor is called with argument
one and AddRefs this same device before its final Release. Thus a positive
retained baseline n gives `n -> n+1 -> n`, including n=1. Admission excludes
zero/overflowing baselines and additionally requires the observed final count
to equal the initial count. No second AddRef/Release is used to test it.

Only then can the compiler copy the captured word into the existing factory's
fresh shared root/mode scope. A conflicting explicit caller capture fails.
Factory logging may correctly say `entry=unknown` before this compiler
publication. Later sampler-source1 releases still invalidate knowledge unless
their own contract supplies a matching capture or a later established write.

## Validation and retained evidence

The focused probe includes the exact production `.inc`, creates a real Win32
HAL device and pixel shader, and exercises the same bridge/profile. Its
uncontended release returns one and captures the fixture builder. A bounded
contention case demonstrated actual replacement of W, including zero; it does
not rely on that value being constant. Changed module identity is rejected.
The probe retires its shader, device, and API to zero.

The final Win32 build passed both existing CTests (`reconstructed_math` and
`tool_tests`). The unrelated duplicate `spawn_request_id_matches` LNK4006
warning remains. The final contention probe observed 29,170 changed words in
200,000 releases, including zero. The complete 96-byte bridge is identical in
the probe and production COFF objects and in both linked executables; its
SHA-256 is `d480337b401359dd588edf03898dd6e7feb5e995f0149afa3fc96ce78268e06c`.

The report records the final build/CTest results, exact probe counts, full
bridge COFF and linked extents, operand bindings, module identities, hashes,
and frozen artifact bundle. The standalone probe proves the checked boundary
with symbolic fixture row/builder/owner inputs and real COM/OS calls. It does
not execute the original complete compiler frame or the actual GUI acquisition.

The original installation is read only. No GUI06 run is credited by this packet.
Root integration, its combined build, and a fresh actual GUI run remain the
next validation boundary. This adds no newly reconstructed original function.

See [the machine-readable report](../reports/native_sampler_entry_word_cc12.json).
