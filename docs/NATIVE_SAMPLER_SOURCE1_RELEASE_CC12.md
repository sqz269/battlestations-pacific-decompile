# Descriptor Source1 release word (CC12)

The descriptor `sampler+14h == 1` branch has a bounded nonzero-release proof:
the binding call establishes `W = B-16 = 00B3B2E4`, and the currently observed
Win32 `Kernel32!InterlockedDecrement` does not overwrite W. This does **not**
admit the branch yet. The production Source1 operation does not retain the
actual decrement result, so it cannot distinguish this case from the zero-count
virtual destructor path. Keep its existing unknown-word guard.

This is descriptor Source1, not compiler `source_mode == 1`. The intended
compiler setting remains `source_mode=0`, `load_variants=0`. No runtime launch,
shader replacement, compiler-mode change, shared adapter edit, or Ghidra
annotation was made. The prior cached-mode capture remains independently scoped.

## Exact native boundary

Both internal sites `00B3B2EF` and `00B3B328` belong to `00B3B280`
(`BSP_ShaderBuilder_AddDescriptorSamplers`, descriptive reconstruction name).
Its interface is ECX=builder, stack pass/descriptor, `RET 8`. Let B be its
steady loop ESP after saved EBX/ESI/EDI. EBP is the actual builder, ESI is the
current sampler, `[B+18h]` is the actual pass, and `[B+1Ch]` is the descriptor.
The compiler root/mode shared-W relation comes from the existing caller proof;
this packet does not re-prove the full `B3B3C0` caller.

| Site | Operation and contract |
| --- | --- |
| `B3B2B5` | Tests sampler `+14h == 1`. This field is independent of compiler mode. |
| `B3B2BA/B3B2C5` | Gets the current loader singleton and calls `B1B4D0` with the actual source-name header at sampler `+18h`. EDI captures its returned owner. |
| `B3B2CE` | Null owner skips binding, release and builder `+8Ch` increment. The wrapper's `PUSH CBC760` established W; this is a separate path. |
| `B3B2DF` | Calls `B44CF0` with ECX=pass and arguments current builder `+8Ch`, owner and actual name header. Its return PC is stored at B-16. |
| `B3B2E4..B3B2EF` | EDI remains owner; EDX becomes owner+4; EBX is pass; EBP is builder. Pushes owner+4 at B-4, sets pass `+80h=1`, and calls import cell `CE2220` with return PC `B3B2F5` at B-8. |
| `B3B2F5/B3B2F7` | Tests the actual returned new count. Nonzero branches directly to `B3B32D`. |
| `B3B2F9..B3B2FF` | Zero reads owner vtable slot zero and calls it with ECX=the same EDI owner. No explicit stack argument is pushed; return PC is `B3B301` at B-4. This terminal path remains unqualified. |
| `B3B32D` | After a normal nonnull release, increments builder `+8Ch`. Source1 does not itself call `B5F100`. |
| `B3B369` | Any later executed sampler-state call writes `B3B36E` into B-16, replacing prior uncertainty. |
| `B3B328` | A later Source0 or Source3 reference calls `B5F100`; it writes only the stage byte of B-16 and copies the entire DWORD into the row. |

The `B44CF0` call pushes three arguments before CALL, so its return slot is
exactly B-16. Its ordinary `RET 0Ch` restores ESP=B without replacing that slot.
The following atomic call pushes one argument and its return PC, then the
profiled helper saves EBP at B-12. Its deepest stack access is B-12. It has no
calls or alternate branches; its only non-stack write is `LOCK XADD [ECX],EAX`
to the actual owner+04 count. A real distinct owner count must not alias W.
The helper restores EBP and `RET 4` restores B. Therefore a normally returned
nonzero decrement preserves the proved binding return word `00B3B2E4`.

This constant is conditional on the exact native call placement and provider;
it is not an initial/default sampler word. If it is next consumed with stage
zero or one, the resulting row word is respectively `00B3B200` or `00B3B201`.
Intervening native writes can establish another word. Any later Source1 release
needs its own observation and qualification.

## Provider and source evidence

Five saved-Ghidra spans, totaling 961 bytes, match the installed original PE:
`B3B280` (306), `B1B4D0` (280), `B44CF0` (106), `B44690` (188), and `B5F100`
(81). Each live batch used the bsp wrapper's target verification for project
`bsp`, `/battlestationspacific.exe`, x86-32 and image base `00400000`, with
configured project file `C:/Users/sqz269/bsp.gpr`.

The original PE import at `CE2220` is `KERNEL32.dll!InterlockedDecrement`.
A separate real 32-bit process resolved the current export to RVA `31AB0` and
read its complete 20-byte body. It matches both SysWOW64 Kernel32 on disk and
the accepted cached-entry profile:

```
8bff558bec8b4d0883c8fff00fc101485dc20400
```

This was a read-only loaded-provider observation, not a release execution or
a game module observation. The Win32 loader reports the virtualized path
`C:\WINDOWS\System32\KERNEL32.DLL`; the matching physical x86 disk file is
`C:/Windows/SysWOW64/kernel32.dll`. The evidence records process ID, module
base, entry VA, PE identity, bytes and hashes. No relocation falls in the body.
The first read-only script attempt was refused by the process's default script
execution policy; the second used process-scoped `-ExecutionPolicy Bypass`.
Neither attempt launched the game or changed persistent policy.

Current source deliberately records the binding word, performs the real
canonical release, then invalidates knowledge when no matching release capture
exists. `release_native_render_actual_owner` performs the same actual `+04`
sequentially consistent `fetch_sub`, dispatching the canonical zero terminal
only when the prior count is one; it discards that observed prior count.
The Source1 step records release entry/return and owner identity but not the
count. Root/mode evidence arrays are supplied before the invocation and remain
borrowed; pre-filling them with this constant would bypass the missing proof.

Do not infer the count from a nominally successful binding. `B44CF0` has a
normal null-allocation arm, and `B44690` reads the current retained slot after
string callbacks, with an equality skip before increment. Even with a nonnull
binding, the operation must observe the atomic result rather than reread the
count or assume `2 -> 1`.

## Smallest next packet

An independent bounded proof is feasible for the nonzero path without another
96-byte release bridge. Add Source1-specific receipt state for the actual
decrement's prior count, same owner identity, sampler index and invocation,
preserving the canonical atomic operation and zero-dispatch ordering. Admit
W=`00B3B2E4` only after normal return, a conservative valid prior count greater
than one, and a match of the exact original atomic provider profile. Preserve
unknown state when these conditions are absent. Any explicit caller capture
must agree with a newly established word rather than silently overriding it.

Review the production source/header changes, actual owner/domain lifetime and
compiled call/atomic ordering. Use one focused original/source case with a
real loader-returned owner and actual binding, followed by an existing Source0
consumption; retain its exact source/header/COFF/provider closure, actual count
and owner observations, and rootable call coverage. A synthetic LONG-only test
would establish the atomic helper alone and cannot establish this owner path.
Keep preparation failures and captured identities; do not repeat a timed-out
target launch as a substitute for observation.

If the observed prior count is one, retain unknown W and resolve the exact
owner's current vtable-zero terminal (including any nested provider stack
writes) in a separate bounded packet. This review does not name a universal
terminal for every resource returned by the sampler loader.

The real `fakemotionblur.mshd` descriptor and its reached sampler/state order
still require observation; nothing here proves it follows this candidate path.
No Source1 production admission, source/native differential, COFF validation,
whole caller ABI/FH3/SEH, game startup or gameplay credit is claimed.

The [machine-readable report](../reports/native_sampler_source1_release_cc12.json)
pins the immutable local archive, source/header inputs and all bounded checks.
