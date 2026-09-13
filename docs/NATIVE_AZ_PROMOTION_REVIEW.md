# Native AZ promotion review

Independent review packet: `orch2_native_az_promotion_review`. The review owns no Ghidra addresses and makes no Ghidra changes.

## Evidence boundary

The five portable fixture families have captured expected results independent of the candidate selected with `--repo`. Existing development executions have matching frozen-input, executable and link-map hashes. Those executions use different candidate commits and remain writable, so they do not establish one final promotion.

| Family | Fixed comparison | Support and coverage limit |
| --- | --- | --- |
| Raw inflater | 1,305 checks; ten original methods plus original constructor | Shared stock zlib and successful allocator support. Two current standalone game objects need unique archive-member equality supplied by the gate. |
| Compressed MPAK entry | 1,468 checks; three original/source caller pairs | Original 415-byte caller uses shared current constructor, conversion, deletion, allocator and stock zlib support. Additional linked core archive members are outside the helper's explicit nine-object list. |
| Enumeration | Fixed JSON including 48 allocations and 48 releases | Link-map-derived current objects, actual project source mapping and compiler-observed includes are retained. String/vector/allocation adapters are shared source support. |
| MPAK runtime | 32 decoded bytes; two original position-leaf comparisons | Conversion is source-only. The helper records probe-observed repository headers, not all linked translation-unit or external headers. |
| VFS device | Eight variants; four original routines totaling 793 bytes | The revised helper links copied current route and lookup objects and requires a fixed baseline. Shared string, traversal, pool and host support remain explicit. Only the probe translation unit is freshly compiled. |

No fixture establishes original FH3/SEH behavior, a complete archive/VFS implementation, drop-in ABI compatibility, installed execution, or gameplay validation.

## Findings sent to the integrator

1. The immutable AY strict-build helper pins libraries, executables, build configuration and repository build inputs, but not standalone object files. Raw replay links two standalone objects without comparing them to `bsp_core.lib`. The promotion gate must prove each directly linked current game object equals one unique member of the strict-proof-pinned archive.
2. Raw and compressed replays capture HEAD only at completion. Their explicitly selected source/object/library hashes are checked before and after, but compiler-observed header hashes are captured after compilation and are not all rechecked. The gate must check one clean full HEAD and current input hashes before and after each stage.
3. Runtime include evidence is limited to headers observed while compiling the probe. Compressed helper source pins do not enumerate additional archive support selected by the linker. The gate must parse actual link maps, map object members through current project files, and retain the missing support pins without claiming additional original-body comparison.
4. Existing raw, compressed and runtime basis manifests and expected output files are writable. Enumeration's basis is read-only. A prior reviewed basis anchor must freeze expected bytes and helper hashes independently of candidate output, then recheck them after execution.
5. The initially reviewed device helper directly compiled copied candidate target sources and accepted a missing baseline. The worker corrected both issues: its revised helper requires `--baseline`, compiles only the probe, copies and links the current `native_vfs_device_route.obj` and `native_vfs_lookup_routes.obj`, and records both current/frozen/hash input rows for the gate's unique archive-member check. The earlier replay04 remains historical development evidence.
6. All reviewed development attempts retain matching frozen bytes but are not fully read-only. A final physical-file manifest must enumerate all retained files, detect additions/removals/hash changes, and mark the completed attempt read-only.

## Verification

The review read all five helpers and the reused `build_proof.py` implementation. An independent local audit checked 383 enumeration, 231 raw, 255 compressed, 199 runtime and 44 device frozen input rows, plus all five executable and link-map hashes. No missing or mismatched frozen bytes were found. The wrong-HEAD negative check was rejected with `Current HEAD differs from expected HEAD`.

Focused calls into the gate used isolated scratch files to verify rejection of changed current source bytes, a stale direct object and changed captured expected-result bytes. Candidate files and actual fixture baselines were not modified. The revised device exact-object replay07 also has 22 matching frozen input rows and matching executable/map hashes; its 22 rows are still writable development evidence.

## Static gate decision

The reviewed gate closes the reported promotion gaps when run with its reviewed, read-only anchor. It verifies one clean full HEAD, retains the strict-build proof, snapshots actual compiler-recorded dependency bytes before the build, rechecks the same dependency set through completion, checks fixed outcomes, authenticates current/frozen input rows and executable/map hashes, and requires unique archive-member equality for both map-selected and directly linked current objects. The anchor, gate source and imported build/helper code are retained in the attempt.

The first reviewed revision could mask a failure by sealing twice and checked candidate state only before the long sealing step. Both were corrected. Sealing now verifies an existing unchanged seal without overwriting it. After sealing, the gate rechecks HEAD, anchor, current input bytes, compiler dependencies, build proof and all replays; only then does it write a separate read-only `-receipt.json` with `status: passed`. A post-seal failure writes an external `-rejected.json` without changing the sealed payload. The payload's `validation.json` deliberately says `validated_before_seal`; it alone is not final acceptance.

A focused check confirmed that repeating an unchanged seal returns the same manifest hash and that adding a file is rejected as `Existing seal has missing or extra physical files`. The reviewed gate hash and helper hashes are recorded in the JSON review report.

Static review is accepted. The final combined attempt is not executed by this report: the integrator must merge this review before selecting the final candidate HEAD, run the gate, and inspect its immutable success receipt and physical-file seal. Any later final-attempt review belongs in ignored local evidence so it does not change that candidate HEAD. No development attempt above is independently promoted by this document.
