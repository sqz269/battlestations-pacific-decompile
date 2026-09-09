# Occlusion-query issue and result methods

The four concrete query-use entries in vtable `00d62ad0` are implemented in
`d3d9_query.cpp`. Their complete bodies were recovered from raw assembly
because several entry addresses were absent from the function snapshot.
No original function creation or Ghidra annotation was performed by this
subtask. These methods contain no unresolved external calls beyond the
real `IDirect3DQuery9` interface.

All four use wrapper ECX with no stack arguments and ordinary RET. Begin,
end, and poll return a boolean in **AL**; some skip paths only write AL, so
upper EAX is not part of the boolean return contract. The getter returns the
full cached DWORD in EAX. The C++ calling convention is new.

| Method | Native behavior |
|---|---|
| `00b5fc30`, begin | If state `+8h` is zero or query `+10h` is null, return true. Otherwise call Issue(2), return whether HRESULT is exactly zero, and leave state/data unchanged. |
| `00b5fc60`, end | If query is null or state is zero, return true without changes. Otherwise call Issue(1), set state to `(HRESULT != 0)` and return `(HRESULT == 0)`. |
| `00b5fca0`, poll | If query is null, return true without changes. Otherwise call GetData on cached DWORD `+ch`, size 4, flags 1. Exactly zero HRESULT sets state to 2 and returns true; all other results return false without a state write. |
| `00b5fce0`, getter | Return cached DWORD `+ch`; do not poll or check state. |

The SDK constants are compile-time checked: Issue(2) is D3DISSUE_BEGIN,
Issue(1) is D3DISSUE_END, and GetData flag 1 is D3DGETDATA_FLUSH. Every success
comparison is **exact S_OK**, not the broader SUCCEEDED macro. S_FALSE and
failed HRESULTs both produce false in the poll API, as in native AL behavior.

Constructor initializes state one and cached samples zero. Successful end
sets state zero (awaiting a completed query result); successful poll sets
state two. Begin permits any nonzero state, including two, but does not itself
change it. End failure stores one. Begin failure preserves the prior state.
These observations describe this wrapper's transitions, not a generalized
three-state query scheduler.

Poll has no state gate, retry loop, sleep, timeout, lost-device check, or reset
request. It calls GetData exactly once even if state is already nonzero.
The actual data member is passed directly, so any API-visible write to that
DWORD is preserved even when a nonzero HRESULT is returned; no temporary
buffer or failed-result rollback was invented. The getter exposes the stored
sample count without asserting freshness, visibility, or completion. Null
query paths return true despite performing no hardware operation; they do
not fabricate a new count.

A caller may repeat polling according to its own documented execution policy,
but that is not behavior inside `00b5fca0`. The reset lifecycle continues to
preserve both state and count across query Release/CreateQuery. Therefore a
successful reset alone does not mark a pending query complete or erase a
previous cached result.

## Implementation and verification scope

Public names are `begin_occlusion_query_00b5fc30`,
`end_occlusion_query_00b5fc60`, `poll_occlusion_query_00b5fca0`, and
`occlusion_query_samples_00b5fce0`. They use the existing concrete owner and
real query methods. No fake result callback, renderer visibility heuristic,
new test target, or HRESULT-normalizing adapter was introduced.

The existing D3D9 triangle draw now runs between the recovered begin/end calls.
After readback, one immediate poll returned false, so the diagnostic harness
now retries the unchanged single-call poll with a two-second bound and 1 ms
sleep. Native false conflates pending and failure; the harness does not claim
to distinguish those HRESULTs. The successful run completed after two polls,
reported state2 and1596 samples, exactly matching1596 green pixels counted in
the non-multisampled readback image. This host waiting policy is not part of
the reconstructed native method or proof of the game's query scheduling.

The Win32 build, both existing CTests and full installed-asset D3D9 probe pass;
see `reports/query_draw_font_fallback_probe.txt`. No new test target was added.
Earlier reset-only documents are superseded for these four methods; caller
scheduling and broader visibility decisions remain separate work.

Every live batch verified project `bsp`, program
`/battlestationspacific.exe`, x86 image base `00400000`. Complete bodies
matched installed PE bytes:

| Range | Bytes | SHA-256 |
|---|---:|---|
| `00b5fc30..00b5fc53` | 36 | `6f87a154f538084841dc63e5a7a2bd215a931daaa2c4d6db57a60765fc6679e8` |
| `00b5fc60..00b5fc92` | 51 | `80da1b92fdf6db46f84b0e527a6b27b9ec2990530383957d39263dfdd0ffbad4` |
| `00b5fca0..00b5fcd3` | 52 | `97522191dc4da543bb7d3c399779b1781f3225f0ebfb3381a90233f8d89d0d90` |
| `00b5fce0..00b5fce3` | 4 | `2e339cdc5de837ec151dbde90057caeabba18ac3ea32e7c2cffc8f95df41a6ff` |

Parent integration created the missing Begin and cached-result function records
after matching raw bytes and dry-run sizes36/4, preserved prior annotations,
applied descriptive names/comments, saved Ghidra and refreshed inventory/exports.
