# Directional shadow owner lifetime

This module reconstructs four complete normal bodies and their base-only unwind
action in the existing actual-storage provider domain. It does not construct a
shadow owner or make an arbitrary original object callable through a host vtable.

| Native entry | Inclusive extent | Bytes | Coverage | Original ABI |
|---|---|---:|---|---|
| A8DEC0 | A8DEC0–A8E086 | 455 | complete | ECX actual 508h owner; no public stack arguments; RET |
| A8E160 | A8E160–A8E17D | 30 | complete | ECX owner; stacked DWORD flags; EAX original identity; RET4 |
| A8FCD0 | A8FCD0–A8FCED | 30 | complete | ECX owner; stacked DWORD flags; EAX original identity; RET4 |
| B7BDF0 | B7BDF0–B7BE30 | 65 | complete | ECX actual light; stacked replacement pointer; RET4 |
| CB63D0 | CB63D0–CB63D7 | 8 | complete action | ECX loaded from original frame EBP-10h; JMP BD30F0 |

EA adds only A8E160's 30 bytes: the total is 580 normal-body bytes plus the
eight-byte unwind action, 588 bytes. The prior 558 bytes receive no new credit. Source
interfaces add the borrowed context; the unwind entry receives its captured owner
explicitly. These are not drop-in original-ABI replacements. A8DEC0 preserves
EBX/EBP/ESI/EDI, and the setter/deleting wrapper preserve ESI. No semantic setter
return value is established. Primary repaired the saved A8FCE5 and A8E175
`ADD ESP,4` listing gaps using their verified original bytes. This worker does
not mutate Ghidra; the prior overrides and repair evidence remain recorded.

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

A8FCD0 and A8E160 call the complete destructor first. Only normal return reads the low byte
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
| Captured old base shadow | D5B574, explicitly bound | BD30E0 | A8E160 in this module |
| +504/+10/+14/+18/+1C | D5E5F8 | BD30E0 | B1F8F0, existing viewport retirement/shared CRT |
| +384 | D61948 | BD30E0 | B3F590/B3F2E0, existing texture2D context/pool |
| +4F0..+500 | D5E600 | BD30E0 | B1FCF0/B1FC00, existing frame-target/surface context/shared CRT |

The actual table bindings stay fixed while their words are read live. After the
slot0 check, the real source BD30E0 provider reloads the current owner profile and
the concrete dispatcher reads its current slot4. There is no second decrement or
arbitrary user terminal callback. Unknown profiles/targets or absent bindings
diagnose. Each shadow profile accepts only its corresponding deleting entry.
The context appends optional `table_00d5b574 = nullptr`, preserving existing
aggregate initializers at source level. Base dispatch requires the actual table
binding; direct A8E160 calls need no table lookup. The source context's size
changes, so coupled code must rebuild; binary layout compatibility is not claimed.
The base binding is not a universal constructor preflight requirement. Texture,
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
light+0C and final D5B5D8 profile. A8FD30/A8FA30/A8E2E0 construction is supplied by
the existing directional shadow construction module and DY persistent block.
Its direct base entry can return initialized D5B574 storage. The observed native
derived constructor instead stamps D5B5D8 after base return; a native game
publication of a standalone completed base has not been established. The
destructor's D5B574 stamp alone does not establish a separately releasable owner.
Caller algorithm names remain hypotheses.

A8E160 adds no registry, count operation, cleanup state, or block disposal.
Flags0 ends native lifetime without freeing storage; flags1 requires compatible
shared CRT backing. Native owner deletion does not prove that queued camera
companions, viewport records, cache operations or semantic borrows are quiescent.
The existing persistent block must survive them, and its explicit host-quiescent
reset remains separate. Constructor unwind still directly calls A8DEC0.

The report pins the installed PE, fresh guarded live bytes, prior metadata, all
numeric transfer rows, provider/source inputs, build and generated-code evidence.
There are no new tests or shadow-owner runtime fixtures. Compilation and source order
review do not prove real-game integration, GPU effects, arbitrary callback faults,
or gameplay. Root owns later combined validation and metadata integration.

Historical DU validation at exact clean source `42603ccf159dad821d9b51ceca44e5092d7187a3` passed Win32 Release and both existing CTests with 2627 unchanged tracked build inputs. Its 14 reviewed COFF symbol rows and 21 instruction checks belong to that previous source revision, not EA. The shared 44-byte unwindtable/FuncInfo section and 42-byte funclet/handler section are not independent function bodies. The compiler removed the source disarm store before the known-noexcept base call; ordinary C++ cleanup remained base-only, with no raw FH3/SEH identity claim. No shadow-owner fixture was run.

Primary also cleared the erroneous CALL_RETURN override at A8FCE0 under the Ghidra write lock, decoded the verified three-byte ADD ESP,4 at A8FCE5, and saved the project. There are no remaining call-site listing gaps in A8FCD0. The record preserves the original override value, native byte hash and readback; the shared CRT callee was not edited.

For EA, primary likewise cleared A8E170's CALL_RETURN override to NONE, restored
the three-byte ADD ESP,4 at A8E175, and saved/refreshed the complete 11-instruction
body. The report pins that repair and the read-only EA admission. Primary source
review accepted clean commit `5611325fa323373f94baa935574f296a21bea794` with no
findings. Its single Win32 Release build passed both existing CTests with all
2,631 tracked inputs and the verified ignored seed header unchanged. No compiler
warnings/errors were reported. The complete log, both input maps, four libraries
and current lifetime object are pinned in `local/ea-final-build-stamp.json`.

The frozen runner records the actual after-HEAD string in `same_commit_after`;
independent equality, clean status and input-hash checks confirmed the exact
built revision before metadata edits. The original runner/stamp remain intact.
Primary accepted the complete EA source delta and current generated scheduling
at this exact object. The two emitted 53-byte base/final wrapper sections have
identical bytes and relocations: destruction precedes the low-byte flags load,
bit test and optional free. The reached dispatcher inlines both wrappers after
their exact profile/slot checks and preserves the same late load. The current
setter and destructor retain publication, IAT capture, callback and clear order.
Sixteen code sections, two EH metadata sections and the concrete host vtable
were inspected. Switch tables embedded in code sections remain data, and all
prior DU bytes retain their original credit. This is generated scheduling proof,
not original code/ABI/private EH-frame identity or runtime lifetime validation.

Ghidra now saves A8E160 as `BSP_DirectionalShadowBase_DeletingDestructor` with
native __thiscall, one implicit this and one unsigned flags argument. The bounds
and complete 11-instruction return path are unchanged after the recorded repair;
previous comments are preserved and exports refreshed. The exporter's inventory
name metadata is historical; live prototype/comment readback carries the current
reviewed name. Old DU annotation/build/object records remain explicitly historical.

The exact tested attempt is sealed at `local/ea-validated-attempt.zip`, SHA256
`b3c1bc38243987e9bac423fbfca66325772aa5c04a80c7de4f486e94685169a4`.
All 2,678 payload hashes were reread. It retains all tested inputs, four libraries,
current object, two existing math test executables, build/test configuration and
logs, native/source/generated reviews and Ghidra evidence. Root Git inputs match;
2,622 raw tracked files match and nine differ only in CRLF/LF, archived separately.
The additional ignored seed header matches. No redundant root build was run.
No shadow lifetime fixture, native ABI/private EH or gameplay validation is claimed.
