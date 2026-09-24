# Actual scene resource storage and lifetime

Addresses: `B83C50`, `B82ED0`, `B83410`; constructor compiler boundaries
`CC2390`, `CC2398`, `CC23A3`, `CC23AE`, `CC23B9`; destructor boundaries
`CC2310`, `CC2318`, `CC2323`, `CC232E`.

The source operates on the actual `3Ch` allocation: profile0, count4, raw name
header8/C, actual ambient10 and the `28h` registry14..3B. It reuses the genuine
raw string pool, registry, ambient constructor and ambient setter. No enlarged
host scene object, logical registry or second native count is introduced.
Names are descriptive hypotheses, not recovered symbols.

| Routine | Exact extent / last instruction | Coverage |
| --- | --- | --- |
| `B83C50` construct | `[B83C50,B83D4D)`,253B; `B83D4A RET4`,length3 | Complete raw normal body; source cleanup and explicit metadata boundary |
| `B82ED0` destroy | `[B82ED0,B82F82)`,178B; `B82F81 RET`,length1 | Complete raw normal body; source C++ cleanup |
| `B83410` scalar delete | `[B83410,B8342E)`,30B; `B8342B RET4`,length3 | Complete body; current caller flags |
| `CC2390/98/23A3/23AE` | 8/11/11/11B, through `CC23B8` | Native constructor unwind boundaries |
| `CC2310/18/2323` | 8/11/11B, through `CC232D` | Native destructor unwind boundaries |
| `CC23B9`, `CC232E` | 10B each; final JMP at `CC23BE`, `CC2333`,length5 | Missing native FH3 handler definitions; boundary only |

Construction stamps CEB130 and count1, then captures the actual incoming name
argument cell, stamps D63168, and zeros both destination name words. A name
header that aliases resource+8 is still cleared before the self-copy skip.
Otherwise `41DD40` receives current source length and preserve1. After its
callbacks, the caller rereads source length, then current destination length,
destination pointer and source pointer. `BF7680`'s overlap-aware behavior is
represented by `memmove`; a zero-length copy is omitted after its argument
reads. The pool/header provider remains the existing genuine raw one.

The two local allocator bytes are unwritten native scratch. The caller must
supply initialized preimages; the first goes to the registry's used byte.
State1 is armed before ambient10=0 and registry construction. State2 then
protects completed registry/name/base while allocating98h. `B83CFA` overwrites
the original name-argument DWORD with this allocation. State3 covers only the
ambient constructor; `CC23AE` reads that current word for failure free. After
the ambient constructor, state2 is restored. The setter receives a distinct
current requested-ambient argument cell. The final decrement always uses the
captured ambient creator, independently of mutations to the requested cell or
resource10. It reloads the decrement import and dispatches current virtual0
only when that decrement returns zero.

`NativeSceneResourceAcquired` is fresh, caller-owned, address-stable metadata.
Its optional `NativeAmbientReference` is emplaced after completed raw `B7C290`
and before the setter, using exactly the setter's canonical owner registry.
This host-only admission performs no native store, retain or extra decrement.
It may fail independently of a native provider. Such a failure leaves the
completed raw ambient recorded for explicit disposition; the native state2
prefix still cleans registry/name/base. A later setter failure likewise adds
no ambient rollback or creator release. The acquired record and contexts must
outlive final ambient retirement and external quiescence, including surviving
ambient holders after the scene resource has been destroyed.

Destruction stamps D63168 and captures resource10. It decrements that actual
owner through the current import, dispatches its current0 at zero, then clears
resource10 even if the callback changed it. The terminal resolver verifies
that the companion borrows this captured owner's exact+4 atomic. A surviving
ambient's borrowed backlink is **not removed**, preserving the native behavior.
Next the destructor reads/frees current vector begin28, zeros28/2C/30, and
drains the embedded list18. It captures the name block atC, changes native
cleanup state to0, captures current length8+1, calls the current raw pool getter,
and returns the captured block/size. The name header remains untouched. Finally
it disarms cleanup and stamps baseCEB130. Scalar deletion reads only the current
low byte of the actual caller flags cell after this complete destructor; bit0
frees the same owner, and the return is its numeric identity without a dead
payload read.

`NativeSceneResourceReference` is separate postconstruction metadata. It binds
the actual+4 atomic in the same canonical registry, requires current
D63168[0/4]=BD30E0/B83410, and uses the genuine scalar path for terminal or
explicit deletion. Binding adds no reference credit. Metadata retires once,
also after an entered source destruction failure, without reading freed
storage. It does not connect the logical `NativeGuiSceneOwner` or
`SceneAttachmentRuntime` graph; those consumers remain a separate packet.

Constructor FuncInfo DFB768 has four states at DFB748:
0->-1/CC2390 base;1->0/CC2398 name;2->1/CC23A3 registry;
3->2/CC23AE raw allocation. Destructor FuncInfo DFB5BC has three at DFB5A4:
0->-1/CC2310 base;1->0/CC2318 name;2->1/CC2323 registry. Normal destruction
moves directly from state2 to0 before the name return, so a failing pool getter
does not retry name or registry cleanup. The source expresses C++ cleanup;
native FH3/SEH/private EBP and return-address storage are not implemented.

Pre-repair evidence pins552 code bytes,128 EH metadata bytes and8 actual
profile bytes against the installed executable. Returning-free overrides omit
`B82F2C..2E ADD ESP,4`, `B83425..27 ADD ESP,4`, and `CC23B7 POP ECX` /
`CC23B8 RET`. The report lists complete required parent extents and final
instruction lengths. Workers made no Ghidra changes; the archive retains this
pre-repair state for primary review.

Strict MSVC Win32 build and all three existing CTests pass. One ignored probe
compares two original/source outer-body executions, using genuine actual raw
pool, registry, ambient and setter providers: ordinary name and self-header,
allocation preimages, name-argument overwrite, a destructor decrement callback
that changes ambient10 and the current scalar flags, and the surviving backlink.
Actual payloads, head/pairs and relevant argument bytes are compared with
pointer normalization including vector endpoints. Native zero-count virtual
dispatch and native failure unwinding are outside these two comparisons.
Separate source-only checks cover98h allocation failure, host admission failure
with a retained raw creator, and canonical same-count resource/ambient terminal
retirement. No broad tracked suite, game-process modification or gameplay proof
is added.
