# Original CRT OS-error mapper

`00BFFB50` is reconstructed as whole ordinary C++ over the actual mutable
45-pair table beginning at `00E15880`. The native body is 59 bytes / 23
instructions, ends at `00BFFB8A`, and calls no provider. The descriptive Source
name is a hypothesis, not a recovered library symbol.

The native cdecl entry captures its one DWORD input from `[ESP+4]`. It compares
that captured value against each reached current first table word in order,
for at most 45 entries. The first match reads and returns that entry's current
second word. The implementation neither snapshots the table nor supplies a
replacement table. Its extra `actual_pairs90` argument must bind the actual
90 readable DWORD cells; `volatile` retains progressive reached reads.

On no match, unsigned DWORD subtraction/comparison maps the inclusive interval
`13h..24h` to 13. Otherwise the inclusive interval `BCh..CAh` maps to 8 and all
remaining values map to 22. This preserves the full DWORD input domain and the
native CMP/SBB fallback result. There are no new sign, range, null or alignment
guards and no table writes, errno/TLS access, diagnostics, callbacks or SDK call.

The native returns its DWORD result in EAX with plain RET and caller argument
cleanup. All four current incoming native CALL sites were checked:
`00BF9E48`, `00BFDE6E`, `00BFDEFB`, and `00BFFBBE`. Each supplies one DWORD and
consumes the EAX mapping. The saved graph incorrectly attributes some calls to
nearby entries; the report uses current typed containing-function evidence.
These callers are not reconstructed or changed by this packet.

The full live 59 bytes equal the installed PE bytes, SHA-256
`f84cb93a4141603ec2a535fbcf0848c7dd4836fd0b8c1be28e35025d67b3dd3b`.
Target guards identify `C:/Users/sqz269/bsp.gpr`,
`/battlestationspacific.exe`, x86, image base `00400000`. The incoming typed
capture was accepted at modification 3445. No analysis or repair was performed.

The Source draft is complete. Write-locked annotation preserved the original
inventory comment and prior name, appended the bounded evidence, saved the
project, and refreshed the affected shared export. Live comment readback
confirmed preservation. Independent review, registration, the primary's normal
Win32 build and existing checks remain pending. No new tests are added.
The added table binding and C++ signature have no drop-in
Original ABI, original compiler-frame, fault/SEH or runtime execution credit.
This mapper closes no Original heap, errno/PTD, lock/bootstrap or full `_free`
dependency by itself; see `reports/goal_oct15_crt_free_readiness.json`.
