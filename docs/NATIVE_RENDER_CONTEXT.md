# Native render context storage and lifetime

`native_render_context.hpp/.cpp` reconstruct the actual `0x18`-byte (24-byte)
context and complete normal bodies at `00B1D120` and `00B1D570`. They also recover
the placement initialization stores inside `00B1EDC0`. Earlier comments saying
"18-byte context" meant hexadecimal `18h`; they were incorrect decimal sizes.

| Native address | Original ABI | Reconstructed boundary |
| --- | --- | --- |
| `00B1D120..00B1D1C8` | ECX=context; RET | Context destructor, including base-profile cleanup |
| `00B1D570..00B1D58E` | ECX=context; stack flags; EAX=original this; RET4 | Destructor, conditional scalar free, original return identity |
| `00B1EDD3..00B1EDF2` | Inline EAX=nonnull storage, ECX=0 | Seven initialization stores only; fragment of `00B1EDC0` |

All ranges use exclusive ends. Names are descriptive hypotheses. The new C++
entry points and companion are host interfaces; they are not native ABI entry
points or drop-in replacements. Win32 static assertions establish the actual
storage size, all six offsets, and a lock-free four-byte atomic.

| Offset | Actual storage |
| --- | --- |
| `00` | Current native vtable identity, initially `00D5E5C4` |
| `04` | The sole actual atomic reference count |
| `08` | Raw retained camera owner identity |
| `0C` | Raw retained second owner identity; its semantic subtype is external |
| `10` | Raw borrowed command backpointer |
| `14` | Raw retained target owner identity |

The placement fragment starts typed lifetime without value initialization. It
stores `00CEB130`, writes count1, stores `00D5E5C4`, and clears fields `08`, `0C`,
`10`, `14` in source order. Allocation, allocation-failure handling, publication
at command+28, retained assignments, and pooled batches are outside this
fragment. It does not claim the complete `00B1EDC0` or `0x44`-byte command owner.

The destructor first writes `00D5E5C4`. For camera, second owner, then target it
captures the current raw identity and decrements that identity's actual `+04`.
Only a zero result invokes a terminal callback. It then clears the containing
slot, even if the callback replaced that slot. The next owner field is loaded
after this callback and clear. It does not decrement context+04 or touch the
borrowed command at+10. Normal completion writes base profile `00CEB130` through
the reconstructed effect of `00BD30F0`.

The scalar deleter calls the destructor, frees through the existing shared
`singleton_lifetime_free` boundary when `flags & 1`, and returns the original
address, including after free. Assembly `00B1D585..00B1D58E` confirms the stack
cleanup and EAX return beyond the decompiler's allocator continuation.

The native destructor's EH handler is `00CBC9C8`, FuncInfo `00DF4E28`, unwind map
`00DF4E20`. Its sole state0 action is `00CBC9C0`: load captured context from
`[EBP-10h]` and tail-call `00BD30F0`, next state -1. It has no remaining-member
release sweep. A local RAII cleanup preserves this base-profile action when a
direct C++ lookup throws; the exception then propagates without clearing that
current field, visiting later fields, or scalar-freeing the context. The queue
companion's terminal contract is nonthrowing. Native SEH/C++ exception dispatch
was inspected and byte-verified, not executed in the differential sequence.

## Binding actual owners

`NativeRenderActualOwners` is a required pure identity lookup. It is called only
after the captured actual count becomes zero. `release_native_render_actual_owner`
then verifies that the returned canonical `RenderCommandReference` borrows the
exact atomic at `raw_identity + 4`, and calls its terminal method. A retained
nonzero owner does not invoke the lookup. Raw aligned atomic lifetime and stable
owner storage until the terminal callback are caller preconditions.

Each concrete companion must inspect its owner's current native profile and
perform its real terminal operation. Unsupported identities or profiles cannot
use a no-op or cached base-destructor fallback. No owner registry, extra count,
synthetic camera/scene/target, or private command/group/queue storage is added.

For an actual native camera, the integration path is
`&NativeCameraOwner::storage -> canonical NativeCameraReference`. That companion
already borrows `storage.node.references_04` and owns the real camera terminal
destruction/pool-return path. A caller's resolver can compare the raw identity to
that storage address and return its existing stable camera companion. The context
field contains the raw storage address, never the host companion address. Any
retained assignment must publish the raw pointer and retain that same actual
counter; this packet does not implement the command's assignment fragment.
Second owners and targets require their own proven concrete terminal companions.

`NativeRenderContextReference` provides the corresponding concrete context
binding. It borrows this storage's actual+04 without initializing or retaining
it, and works with the existing `RenderCommandReference` retain/release helpers.
Its terminal callback checks the current `00D5E5C4` profile and current supplied
slots `00BD30E0`, `00B1D570`, invokes scalar deletion with flag1, then calls the
explicit companion-retirement callback. That callback may remove caller-owned
lookup state or dispose the companion. There is no access to native storage or
the companion after retirement. Base profile `00CEB130` and unverified derived
profiles are rejected. One canonical companion per live context and dependency
lifetimes extending through queued references are caller obligations; direct
destruction is disallowed once this companion owns the terminal path.

## Verification and remaining limits

The guarded CLI verified `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`, for each evidence read. Thirteen exact code, data,
and external-hook spans total 337 bytes and match the installed executable
SHA-256 `b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
The audit records all span hashes and fourteen absolute rebases.

One ignored differential sequence constructs a graph of six actual native
contexts. It executes original context destructor, scalar deleter, zero thunk,
base destructor and exact inline initialization stores in a sparse image.
Unselected pages stay inaccessible and unused committed bytes contain INT3.
There is one five-byte external jump hook at `00BF65AC`, invoking the same real
scalar allocator free as the reconstruction. It then observes retirement at the
same boundary as the host companion disposal callback. The original decrement
IAT binds the real Win32 export. A RET is placed at `00B1EDF2`, outside the
verified initializer fragment, solely to return after its final store. No full
PE loader, game entry point, synthetic retained-owner terminal or broad import
stubs are used. EH metadata/funclets are mapped for evidence, not exercised.

The graph uses real nested context destruction. Its retirement observer replaces
camera/second-owner/target slots at callback boundaries to verify capture and
reload order. It also covers a nonzero retained owner with no resolver call,
preserved count/backpointer, base-profile publication, flags2 without free,
flags3 with free and original return identity, plus queue-helper operations on
the same borrowed context counter. All 343 normalized observation words match.
Normalization covers raw identities, native table addresses and surviving field
values; it does not claim absolute allocation-address or instruction parity.

MSVC Win32 C++17 `/W4 /WX /EHsc /O2` compiled the new source in this focused
fixture. `scripts/build.ps1` passed both existing reconstructed and native math
checks after all eight seed byte comparisons passed. CMake registration belongs
to the primary integration; no tracked test was added. Local scripts and logs
are listed and hashed in `reports/native_render_context_audit.json`.

This is build-tested and fixture-tested lifetime reconstruction. The concrete
camera binding path is supplied by the separate native camera companion;
second-owner/target subclasses, full command allocation/construction/queue,
arbitrary concurrent mutation, allocation failures, native exception execution,
and gameplay/visual validation remain separate. A passing context graph does not
establish those additional boundaries.
