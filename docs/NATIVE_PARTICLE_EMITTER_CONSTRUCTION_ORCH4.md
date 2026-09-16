# Native particle emitter construction with actual raw-pool composition

## Result and boundary

`src/native_particle_emitter_construction.cpp` adds raw-domain overloads for the
base, Cone, Sphere and SmartArea emitter-definition constructors. They borrow one
`NativeParticleTypeLifetimeContext`, so construction-failure string cleanup uses
its genuine `NativeStringRawPoolContext` and future destruction uses the same
actual `00F8D344` `NativeWeakHandlePool`. No callback, replacement allocator,
shadow string store or second parameter domain is introduced. The existing
`NativeParticleDefinitionBindings` APIs remain unchanged.

These bodies provide the constructor half needed by a later `00AF9FB0`
composition. Allocation, kind selection and parser14 dispatch are outside this
packet. The source interfaces are not ABI-compatible replacements for the native
FH3 entrypoints.

## Complete bodies and native ABI

| Entry | Inclusive end | Bytes | Native interface |
| --- | --- | ---: | --- |
| `00AFA280` | `00AFA34E` | 207 | ECX actual80h owner; stack name8h pointer, word10, word70, flag14; EAX owner; RET10 |
| `00B03940` | `00B03969` | 42 | ECX actual94h Cone owner; same stack words; EAX owner; RET10 |
| `00B02B90` | `00B02BB9` | 42 | ECX actual8Ch Sphere owner; same stack words; EAX owner; RET10 |
| `00B01CB0` | `00B01CD9` | 42 | ECX actual90h SmartArea owner; same stack words; EAX owner; RET10 |

The four spans total 333 bytes through their final native returns. Live Ghidra
reported 68/16/16/16 listed instructions with zero gaps in project
`C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`. Disk and live
bytes match the hashes recorded in the report. The only direct calls are
`AFA2E0 -> 0041DD40`, `AFA2F5 -> 00BF7680`, and each derived constructor's one
call to `00AFA280` at `B03959`, `B02BA9`, or `B01CC9`. There are no register
inputs beyond native ECX and no x87 expressions or indirect calls.

## Sparse base schedule and name contract

`AFA280` first stamps `00CEB130`, stores count1, then stamps `00D5DBC4`. It zeros
the name header `+08/+0C`, counts `+4C/+68`, writes byte `+7C=1`, and copies the
actual source header unless it aliases owner+8. Resize uses the source's current
length; after the callback the body reloads source length, source data,
destination length and destination data before overlap-safe copying. The source
name header remains unchanged and caller-owned; RET10 consumes the four argument
words, not the source allocation.

The remaining stores preserve native order: low byte `+14`, DWORD `+10`, zero
bytes `+15/+1D`, zero parameter pointers `+24/+28/+2C/+20/+34`, DWORD `+70`,
byte `+1C=1`, then zero DWORDs `+6C/+50`. All other bytes retain allocation
preimage, including `+30/+38`, row pointers, padding and all derived slots.

The derived bodies add no initialization beyond replacing profile+0 with
`00D5DEBC`, `00D5DE88`, or `00D5DE48` after successful base construction.

## FH3 state map

`AFA280` installs handler `00CBB013`, which loads FuncInfo `00DF2EEC` and jumps
to the existing `__CxxFrameHandler3` at `00BF6B43`. FuncInfo has magic
`19930522`, maxState2, unwind map `00DF2EDC`, no try blocks or IP map, and flags1.

| State -> next | Funclet | Native action |
| --- | --- | --- |
| 1 -> 0 | `00CBB008` | `0041DD20(owner+8)` using the current name header |
| 0 -> -1 | `00CBB000` | `00BD30F0(owner)`, restoring profile `00CEB130` |

State0 is armed after the base profile store; state1 is armed after the name and
count zeros, before byte+7C and the resize/copy region. The source guard performs
those same ordered actions with the genuine raw string context and terminates if
a second C++ exception escapes cleanup.

The three derived bodies have no FH3 registration, state, or funclet. Their sole
call is `AFA280`; no throwing action follows its successful return. Therefore
they do not call the published
`destroy_native_particle_definition_00afa100(void*, NativeParticleTypeLifetimeContext&)`
during unwind. Adding that call would double-clean an `AFA280` failure and would
not match the native listings. The lifetime entry remains the correct future
cleanup provider once a fully constructed emitter owner needs destruction.

## Validation limit

Strict MSVC Win32 build and existing seeded CTests are recorded in the report.
The ignored focused probe exercises nonzero-filled base/derived storage and the
genuine raw string pool, including current-field reloads, source-name retention,
sparse byte preservation and exact profiles. It does not execute the original
FH3 dispatcher or a native throwing path. Native EH transport, binary ABI
replacement, parser/factory composition and gameplay remain unvalidated.
