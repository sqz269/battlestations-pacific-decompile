# Native unit scene handle

Packet V borrows the existing `NativeUnitObserverAlias` and the same unit's actual
pointer cell at `+4A4h`. Existing `kUnitOffSceneNode`/`kUnitOffModelRoot` describe
that offset. The view creates no scene owner, native layout cast or pointer from
`UnitInstanceState::has_scene_node`. Current `GameUnitsHost` cannot supply this
binding: its boolean is false and it owns no proved native scene pointer.

| Address | Operation / coverage |
| --- | --- |
| `006D1E80[7]` | Complete getter: `MOV EAX,[ECX+4A4h]; RET`; ECX=unit, EAX=borrowed handle |
| `00955448[6]` | Partial producer: `MOV [ESI+4A4h],EAX`; all other `00955420` work excluded |
| `00951FB0[15]` | Complete clear/tail: `MOV [ECX+4A4h],0; JMP 00779AF0`; unchanged receiver |

The getter returns the current cell exactly, including null, without ownership
transfer or caching. The producer's actual EAX originates at `00955429..00955445`:
load unit+360h; when present load that owner's +160h; then load the resulting
pointer's +0Ch. The null+360h arm still reaches the +0Ch load; no valid-null
fallback is inferred. `00955420` first calls `0087BCC0` and later applies pose,
matrix and root registration. The fragment performs only its six-byte store.
It is reached through real leaf initialization, including `00822CDB`, not inferred
from the source host's unimplemented `00928860` placement.

`00959940` separately releases the actual node reference, optionally dispatches
its deleting virtual, and clears +4A4h at `00959A04`. That full ownership work is
outside this packet. Killed-tail `00951FB0` merely clears the cell and delegates;
it does not release the prior pointer.

All 21 existing creator profiles' primary slot `+18h` were checked in live and
installed bytes and point to `006D1E80`. Their slot `+80h` targets are twelve
direct `00951FB0`, eight `007B9590` plane tails and one `0074CD10` land-vehicle
tail; both intermediate paths restore ECX and jump to `00951FB0`. Four additional
referenced primary tables share the getter, without inferred new creators.
Exact per-profile maps are in the report.

The required external `00779AF0` handler queries slot18, conditionally clears the
controlled-node publication, clears world+193Ch, marks positive recon records,
then tail-jumps to `00928C80`. Its existing source file only supplies a partial
base Lua handler. No substitute is supplied here. The required provider receives
the same canonical unit after the cell is cleared; missing bindings are rejected
before the store. No work follows delegation.

Read-only verified Ghidra initially had no function at `006D1E80` and wrongly
named `00951FB0` `CG_adjustor_thunk_00951fb0`. Saved full bytes prove there is no
receiver adjustment. Proposed definition/name correction is assigned to the
integrator; previous values/comments are retained.

Three original-byte cases passed using different native/source layouts,
null/non-null publication, unchanged surrounding bytes and clear-before-provider
order, including provider rebinding. Five spans totaling128 bytes and all table
spans matched installed PE; one tail relocation was validated. Exact fixture
inputs, outputs, objects, tools and libraries remain under `local/` with hashes.
This proves bounded source operations, not scene runtime, reference lifetime,
native exception, complete constructor or gameplay equivalence. Build/CTest and
call-site outcomes are recorded in the report.
