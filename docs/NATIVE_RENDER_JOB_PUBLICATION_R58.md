# Raw render-job publication and cleanup (R58)

The render-command executor needs both job singletons in the same actual lifetime
manager as the recovered queue and renderer. Existing job getters used the older
`SingletonLifetimeDomain` projection. `native_render_job_publication` adds full raw
getter compositions using the genuine manager, registration, allocation, and guard
implementations, while retaining the existing actual frame constructor/destructor.

## Native contracts

| Address | Body | Behavior |
|---|---:|---|
| `004C1130` | 192 bytes | Frame getter; allocate `138A8`, construct, publish, reload manager, then register current primary publication |
| `00B0FFB0` | 206 bytes | Preparation getter; allocate 8, stamp three profiles, publish, capture secondary pointer before reloading manager, register captured pointer |
| `00B0F1D0` | 26 bytes | Clear preparation publication and reset secondary profile |
| `00B0F210` | 52 bytes | Primary scalar destructor, optional ordinary free |
| `00B0F1C0` | 8 bytes | Secondary-minus-four deleting thunk |
| `00B0D930` | 41 bytes | Distinct construction-base scalar destructor |

Both getters capture the first manager's real section through enter, registration,
and leave. Cleanup remains armed through normal leave; a registration failure keeps
the published owner. Frame construction failure frees the saved allocation before
guard cleanup. Preparation destruction never unregisters or clears the primary
profile. The source preserves the native null destination rule without claiming
hardware-fault parity.

The raw manager now dispatches registered `CE7550` frame **primary** owners through
the existing frame destructor and `D5E15C` preparation **secondary** owners through
the new adjustment/deletion path. One borrowed context is appended at offset 120;
earlier binding offsets are unchanged. Construction and drain must borrow the same
actual `01090AA0`, `0109CF08`, and `00F8D444` cells and concrete frame bindings.

## Evidence and validation

Fresh PE/live-Ghidra comparisons cover the native functions, profile words, and EH
maps. The frame allocation-cleanup funclet at `00C64EE8` had a truncated nine-byte
Ghidra body: `POP ECX; RET` at `00C64EF1` was omitted after `_free`. The local flow
override was cleared and the full eleven-byte function restored under the write
lock, with its previous definition/comment recorded. The saved/exported listing
now contains all five instructions. Original EH execution remains untested.

- Strict Win32 `/MD /W4 /WX /fp:strict` build and all three CTests passed.
- Both copied-full-native/source getter hot paths preserve an inaccessible manager.
- Native and source preparation cold paths allocate, register once, reuse, and
  drain the actual eight-byte owner through a genuine raw manager and real section.
- Full native/source destructor body and six nonfree scalar cases compare all eight
  bytes plus return/publication state. Three freeing scalar kinds run in both lanes;
  those checks cover return/publication only, without reading freed storage.
- Linked machine inspection verifies genuine frame-constructor, registration, and
  free edges. Same-process resolved I386 runtime files and exact build inputs and
  outputs are archived with hashes.

The native fixture adapts global/import addresses and calls genuine source manager,
allocation, registration, and free providers. It does not execute their original
binary bodies. Frame cold construction/destruction, worker threads, scheduled jobs,
the command executor, exception injection, native ABI, and gameplay remain open.

Evidence: `reports/native_render_job_publication_r58.json` and
`reports/native_render_job_publication_flow_r58.json`.
