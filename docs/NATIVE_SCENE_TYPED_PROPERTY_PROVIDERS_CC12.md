# Ordinary typed-property providers

The three wrappers `008F3370`, `008F3490` and `008F3510` now have complete
ordinary Source bodies through new C++ interfaces. They reuse seven concrete
current providers and preserve the old model/projection APIs separately. Names
are descriptive hypotheses, not recovered symbols or class identities.

| Entry | Original range, exclusive | Body | Physical entry and normal result |
| --- | --- | --- | --- |
| `008F3370` | `008F3370..008F33B8` | 72B, 25 instructions | ECX actual38h record; stack CString; EAX same record; RET4 |
| `008F3490` | `008F3490..008F3504` | 116B, 36 instructions | ECX actual114h bag; stack key/data/count/full flags; EAX bag; RET10h |
| `008F3510` | `008F3510..008F3584` | 116B, 36 instructions | Same wrapper entry; four-byte elements are opaque bits |

The complete 304 bytes/97 instructions match disk and live memory in the sealed
readiness evidence. There are nine direct calls to seven addresses and no
indirect call or x87 instruction in these three bodies. The array bag is the
ECX receiver, despite the old provisional ledger's stack-argument wording.

`008F3370` reads tag+4 once. Tag2 reads old+C, frees a nonnull current-CRT
allocation, physically clears+C only after the return, then invokes the genuine
raw nullable `duplicate_native_string_00438e40` and stores its result. Other
tags read the genuine declaration at+28, call the known-found raw `0048E840`
getter, and store all32 bits at+C. No tag4-only guard, old-value free on the
enum path, missing-key default, same-pointer shortcut or implicit cleanup is
added. The record and token must remain valid across the old allocation's
release. A raw type4 constructor's opaque identity bits do not by themselves
prove a genuine live declaration.

The array wrappers allocate exactly38h uninitialized bytes through the
canonical current `singleton_lifetime_allocate` service. They invoke actual
raw constructors with different argument orders: `008EF2F0` receives
data/count/full flags; `008EF360` receives count/data/full flags. The unused EDX
formal is supplied explicitly. Every flag bit reaches the constructor; only
its low byte chooses retain/copy. Both then call whole `008F33F0` with the same
actual bag, key and insertion context and propagate its bag result. Original
null allocation still reaches insertion with a null record. That fault path
is outside the successful provider domain; no fallback or recovery is added.

The ownership conditions matter for both retain and copy. A nonzero low flag
byte creates a fresh current-CRT child copy while the caller keeps the input.
A zero low byte only retains the input pointer. For this higher owning
publication, the input must already be a unique genuine allocation in the
same current CRT domain. The caller manages it through construction and then
transfers that allocation with the record at successful publication. The raw
constructor itself creates no ownership transfer. Borrowed, stack, static,
foreign and shared retained buffers cannot enter this owning bag graph: the
accepted record lifetime frees array+20 for tags8..11. Positive/nonwrapping
type8 spans and type9 counts1..3FFFFFFF retain the existing constructor limits;
zero-size/overflow admission is not broadened.

The scalar enum getter borrows the same canonical `GameNativeStringProcess`
raw context and distinct E186ED/E17654 authorities. Array publication borrows
the existing `NativeScenePropertyInsertionContext`, built from the same genuine
string/physical processes after explicit E175B0 startup. E188B4 membership,
E177E4/E17654 property/query roles, key generations and return gates stay live.
No library/enum pool is substituted. Graphs remain finite, unshared, acyclic
and disjoint from the replaced owner, map, node and key. Retire payloads,
records, keys and slots before pool teardown; serialize shared-list mutations.
No wrapper constructs or starts a process, physical pool or application.
Existing raw-string getters retain their accepted lazy behavior.

Original EH was inspected separately. Handler `00CA492B` selects FuncInfo
`00DD75C0`, whose one-entry unwind map at `00DD75B8` points to `00CA4920`.
Handler `00CA494B` selects `00DD75EC`, with map `00DD75E4` and cleanup
`00CA4940`. Each cleanup loads the saved allocation at `[EBP-10h]` and calls
`00BF65AC`; each handler jumps to `00BF6B43`. The wrappers set state0 around
construction and reset state-1 before insertion. The ordinary Source provides
no Original FS/frame/EH transport, constructor-failure cleanup, rollback or
private exception equivalence. Providers must succeed without throwing.

The first strict MSVC Win32 19.51.36244 compile passed with C++17, `/MD /O2
/Ob2 /Oy- /W4 /WX /EHsc /Gy /GL- /fp:strict`. Complete COFF review covered
all11 sections, 42 symbol indices including auxiliaries, six code sections
(455B/182 decoded instructions), 21 code relocations and one metadata
relocation. The scalar wrapper is156B/67 instructions; each array wrapper is
132B/49. Physical read/write helpers and the emitted raw-context getter were
also read completely. Full DWORD flag forwarding, constructor order,
free/zero/duplicate/store order and both insertion paths were checked. Literal
`MD_DynamicRelease` directives, weak AVX datum, debug/chks64 metadata and genuine
compiler security-cookie bindings remain in the evidence without normalization.

The required worktree `./scripts/build.ps1` reached natural exit0 and passed
both existing checks in7.94s. This new TU is intentionally unregistered, so
the normal build is the registered-project regression gate and the separate
strict compile checks these new bodies. No new test, probe or runtime check
was added or run. Root owns registration, shared ledgers, naming and independent
admission. Original register/class/FS/EH/private-CRT/FP/error-state, loader,
startup and game equivalence remain unproven; the whole typed parser is not
closed by this packet.

Ignored `local/WholeSceneTypedPropertyProvidersSource01` retains the frozen
readiness, Source/provider/compiler inputs, Original EH bytes, complete COFF
JSON/listing/dumpbin, exact first compile/build receipts, source-rollout
provenance and final family receipt. The original handles reached terminal
without replay or restart.
