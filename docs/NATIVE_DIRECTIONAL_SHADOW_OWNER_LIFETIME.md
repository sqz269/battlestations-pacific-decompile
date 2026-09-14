# Directional shadow owner lifetime

This module reconstructs three complete normal bodies and their base-only unwind
action in the existing actual-storage provider domain. It does not construct a
shadow owner or make an arbitrary original object callable through a host vtable.

| Native entry | Inclusive extent | Bytes | Coverage | Original ABI |
|---|---|---:|---|---|
| A8DEC0 | A8DEC0–A8E086 | 455 | complete | ECX actual 508h owner; no public stack arguments; RET |
| A8FCD0 | A8FCD0–A8FCED | 30 | complete | ECX owner; stacked DWORD flags; EAX original identity; RET4 |
| B7BDF0 | B7BDF0–B7BE30 | 65 | complete | ECX actual light; stacked replacement pointer; RET4 |
| CB63D0 | CB63D0–CB63D7 | 8 | complete action | ECX loaded from original frame EBP-10h; JMP BD30F0 |

The total is 550 normal-body bytes plus the eight-byte unwind action. Source
interfaces add the borrowed context; the unwind entry receives its captured owner
explicitly. These are not drop-in original-ABI replacements. A8DEC0 preserves
EBX/EBP/ESI/EDI, and the setter/deleting wrapper preserve ESI. No semantic setter
return value is established. The installed A8FCE5 `ADD ESP,4` is omitted from the
live listing; its three bytes and the complete returning free tail are retained
in the byte evidence. This worker does not repair Ghidra metadata.

## Destruction and cleanup

A8DEC0 first stamps D5B574, captures owner+504, captures the current CE2220 callable
at A8DEEC, and arms state0 at A8DEF6. It releases +504, then +10/+14/+18/+1C using
that same captured callable. Each nonnull field is captured before decrement;
zero dispatch reads the captured owner's current profile and current slot0. The
containing field clears only after the release returns. A null field skips both
release and clear.

For each +20/+24/+28/+2C camera, B71990 receives the current actual camera and null
viewport unconditionally. Only after it returns does the destructor reload that
field, independently resolve a nonnull current camera, call B6DFA0, and clear the
field after return. The source uses the existing nonallocating exact-key
`nodes.attachments.find_actual_node` and checks the canonical `NativeCameraReference`.
It creates no second map, companion, identity, or hidden retain. A missing/null
first companion produces a source binding diagnostic rather than a successful
skipped clear. Canonical owners and disposal contracts must survive callbacks and
any deferred queue ownership; child operations are not claimed to allocate nothing.

Next, +384 uses the original captured decrement callable, even if a camera callback
changed the live IAT cell. Groups +4F0/+4F4/+4F8/+4FC and +500 each reload CE2220 only
when their current captured field is nonnull. Each group clears after release.
Borrowed +0C, transient +30 and copied +34 receive no invented cleanup.

Native handler CB63D8 uses FuncInfo DEC624, one-entry map DEC61C: state0 -> -1,
action CB63D0. The action restores only CEB130 through existing BD30F0. Normal
execution disarms at A8E067, then calls BD30F0 at A8E06F. The source's armed local
cleanup has exactly that base-only effect on an ordinary propagated C++ exception:
no retry, unvisited field cleanup, slot clear, or allocation free. Unsupported
profile/slot/companion diagnostics also take this source cleanup; this does not
establish native hardware-fault, arbitrary FH3/SEH or foreign-unwind parity.

A8FCD0 calls the complete destructor first. Only normal return reads the low byte
of the caller-borrowed actual flags word and, for bit0, calls the established
`singleton_lifetime_free` shared CRT provider. It returns the captured original
address, possibly freed. The flags slot must remain valid through destruction;
the source deliberately does not snapshot it before callbacks.

## Replacement and concrete dispatch

B7BDF0 captures the incoming pointer at entry and the old light+174 pointer. Equal
identities return without publication, IAT reads, count operations or dispatch.
Otherwise it publishes incoming, retains nonnull incoming through the current
CE221C cell, then decrements captured nonnull old through the current CE2220 cell.
Only final zero reads old's current profile/slot0. There is no subsequent field
clear, restoration, guard retain, owner construction, rollback or local EH cleanup.
Supported synchronous callback replacement therefore leaves the current light slot
as the callback published it; unsynchronized concurrent mutation is not covered.

| Field/route | Supported actual profile | Current slot0 | Current slot4 and concrete provider |
|---|---|---|---|
| Captured old shadow | D5B5D8 | BD30E0 | A8FCD0 in this module |
| +504/+10/+14/+18/+1C | D5E5F8 | BD30E0 | B1F8F0, existing viewport retirement/shared CRT |
| +384 | D61948 | BD30E0 | B3F590/B3F2E0, existing texture2D context/pool |
| +4F0..+500 | D5E600 | BD30E0 | B1FCF0/B1FC00, existing frame-target/surface context/shared CRT |

The actual table bindings stay fixed while their words are read live. After the
slot0 check, the real source BD30E0 provider reloads the current owner profile and
the concrete dispatcher reads its current slot4. There is no second decrement or
arbitrary user terminal callback. Unknown profiles/targets diagnose. D5B574 is the
destruction stamp, not an alias for a supported A8E160 deleting entry. Texture,
surface, viewport, node, queue and camera providers retain their own raw-storage,
profile, pool, registration and noexcept restrictions. In particular B6DFA0 and
canonical camera terminal callbacks retain the existing nonthrowing boundary.
The context's import cells project the parent sites only. Existing child camera,
viewport and frame-target implementations use their established direct Win32
intrusive helpers; arbitrary IAT substitution inside those children is not added.

## Caller and producer evidence

All five incoming setter calls were inspected with full containing listings:
4DF3D6, 504657, 5AB359, 67F39B publish the returned A8FD30 factory owner and later
drop its creator reference; BA1EF7 copies a borrowed owner obtained by B7AAB0 from
another light. B7AAB0 is exactly `[ECX+174] -> EAX; RET`, with no retain. All three
direct A8DEC0 callers were checked: A8E163, A8FCD3, and CB6553 (JMP with ECX from
EBP-58h). A8FCD0 is referenced by D5B5D8+4; CB63D0 by the unwind map.

The producer evidence establishes a 508h allocation, original count1, borrowed
light+0C and final D5B5D8 profile. A8FD30/A8FA30/A8E2E0 construction, allocation
failure and their complete EH states remain separate work; no factory or dummy
initialized owner is supplied. Caller algorithm names remain hypotheses.

The report pins the installed PE, fresh guarded live bytes, prior metadata, all
numeric transfer rows, provider/source inputs, build and generated-code evidence.
There are no new tests or shadow-owner runtime fixtures. Compilation and source order
review do not prove real-game integration, GPU effects, arbitrary callback faults,
or gameplay. Root owns later combined validation and metadata integration.

Primary integration at exact clean source `42603ccf159dad821d9b51ceca44e5092d7187a3` passed Win32 Release and both existing CTests with 2627 unchanged tracked build inputs. All14 reviewed COFF symbol rows have the same complete section bytes and relocation targets/types/offsets as the worker, normalizing only anonymous-namespace path hashes. The shared44-byte unwindtable/FuncInfo section and42-byte funclet/handler section are not independent function bodies. The21 instruction checks retain callback scheduling and late flag loads. The compiler removes the source disarm store before the known-noexcept base call; ordinary C++ cleanup remains base-only, with no raw FH3/SEH identity claim. No shadow-owner fixture was run.

Primary also cleared the erroneous CALL_RETURN override at A8FCE0 under the Ghidra write lock, decoded the verified three-byte ADD ESP,4 at A8FCE5, and saved the project. There are no remaining call-site listing gaps in A8FCD0. The record preserves the original override value, native byte hash and readback; the shared CRT callee was not edited.
