# Native render resources construction

Owned address: `00B14A10`.

`construct_native_render_resources_00b14a10` reconstructs the actual 6ACh
receiver constructor, including raw field initialization, frame-target capture,
two default texture loads, the CCh texture child, the embedded string and the
24h cockpit helper. The existing `BSP_RenderResources_LoadDefaultTextures`
Ghidra name and `void(void)` prototype underdescribe the body; this packet does
not change saved analysis or metadata.

| Routine | Inclusive body | Original ABI | Coverage |
| --- | --- | --- | --- |
| B14A10 | B14A10..B14F5B, 1356 bytes | ECX actual receiver; no stack arguments; EAX original receiver; RET | Complete normal body and nine-state C++ exception projection in the admitted concrete domain |

The live target was verified through BSP CLI against `bsp`,
`/battlestationspacific.exe`, image base 00400000 and configured project
`C:/Users/sqz269/bsp.gpr` on port8089. Live flow reports 302 instructions and
zero gaps. The full listing, not stale pseudocode with unaffiliated register
inputs and a removed B14E69 branch, controls this reconstruction. Original PE
identity, source pins, all call sites and cleanup evidence are in
`reports/native_render_resources_construction_bk.json`.

## Data and calling sequence

ESI becomes original ECX at B14A2A and never changes. EBX is zeroed at B14A57
and remains zero. EDI becomes receiver+684 at B14C12; only B14EFF changes it,
to the current embedded string length. Existing concrete callees preserve
these nonvolatile registers. Both return paths restore all three and return
ESI. The application call at 0073DF01 belongs to 0073D410..0073E52C;
0073DEDF pushes 6ACh, BF681B returns raw storage, and 0073DEFF supplies ECX
only on nonnull allocation.

The source preserves 131 ordered initialization operations, including each
current constant-cell load and retained XMM low-word value. These MOVSS/XORPS
operations perform no floating arithmetic; the source carries their bits in
DWORDs. Unmentioned bytes remain preimages. B0CD80 receives exactly receiver
+2AC and its already-reconstructed interleaved initialization. The vector
prefix words at +68C/+69C and receiver padding are not cleared.

The native allocation requests are exactly40h, CCh and24h, and use the shared
BF681B-compatible CRT allocation/free domain with identical host/native byte
counts. All null branches are retained. The existing allocator returns
nonnull or throws, so those branches are structurally covered, not fixture
evidence of a null-returning provider. Native null40h still reaches an
unchecked frame-target receiver; no successful null-frame fallback is added.

| Site(s) | Callee/provider | Reached contract and cleanup |
| --- | --- | --- |
| B14A31 / B14BEF | B0F020 / B0CD80 | Publish base, then initialize +2AC; RET / RET |
| B14D72, B14EC6, B14F12 | BF681B | cdecl size40h/CCh/24h; ADD ESP,4 after each |
| B14D89 | B1FBB0 | Construct actual40h frame target; RET |
| B14DAC / B14DC9 | current F8D394, D5F0A8+128/+12C | B24DC0 reads current+197C with ignored stack0, RET4; B20090 reads current+198C, RET |
| B14DB6 / B14DD2 | B1FAB0 / B1FB00 | Reread current receiver+1D4, publish/retain surface; RET8 / RET4 |
| B14DE7, B14E5C, B14EF3 | 41DD40 | Raw header resize(9,true), resize(9,true), resize(4,false); RET8 |
| B14E02, B14E77, B14F08 | BF7680 | Overlap-safe CRT copy; ADD ESP,0C. First two use current length+1, final copy uses current length alone |
| B14E20 / B14E95 | current F8D394, D5F0A8+64 | Concrete B319B0 with actual header and flag0; RET8; publish +668/+67C before temporary cleanup |
| B14E40, B14EB5 / B14E47, B14EBC | 419CC0 / BD1510 | Capture buffer then length+1 before current pool getter. Getter takes no arguments, RET; three saved release words remain until BD1510 RET0C |
| B14EDD | B52550 | Construct actualCCh texture child, then publish +34; RET |
| B14F29 | B3C800 | Construct actual24h helper with persistent admitted block; RET; success precedes +0C publication |

Actual literal bytes at D5E474/D5E468/D5E460 are borrowed; the code does not
substitute names or cached copies. F8D394 and each reached profile slot are
loaded again at their native sites. Existing cache/VFS children are retained
individually in caller storage. Their unsupported/failure domains remain
explicit; this packet adds no native callee stubs or abstract host dispatch.

## Cleanup and persistent ownership

CBC3F3 loads FuncInfo DF45B4. Its nine-entry map begins DF45D8:

| State | Next | Funclet | Action |
| --- | --- | --- | --- |
| 0 | -1 | CBC390 | B0F0C0 receiver base destruction |
| 1 | 0 | CBC398 | 41DD20 receiver+684 |
| 2 | 1 | CBC3A6 | B14590 receiver+68C |
| 3 | 2 | CBC3B4 | 4324A0 receiver+69C |
| 4 | 3 | CBC3C2 | BF65AC raw frame-target allocation at EBP-14 |
| 5 | 3 | CBC3CD | 41DD20 temporary header at EBP-14 |
| 6 | 3 | CBC3D5 | Same reusable temporary header |
| 7 | 3 | CBC3DD | BF65AC raw CCh texture allocation at EBP-14 |
| 8 | 3 | CBC3E8 | BF65AC raw24h helper allocation at EBP-14 |

The same local8h region represents ESP+10/+14, with its first DWORD reused
for allocation spills and string lengths. State0 is armed at B14BD3; state3
at B14C6E skips the nonthrowing member initialization states1/2. Copy and resize
precede temporary state5/6. A normal string return disarms that temporary
first, so a throwing current pool getter never retries its cleanup. The map
does not release already-published frame targets, default textures or the CCh
child. The source does not invent those releases. A second escaping MSVC C++
cleanup exception terminates during search; unrestricted hardware faults and
native exception identity are outside the proof.

The caller owns `NativeRenderResourcesConstructionAcquired` before entry.
Its cockpit block reserves both viewport records and scene/lifetime bindings
before B0F020 has any native effect. Its optional helper owner/reference
storage is already address-stable. After successful B3C800, those companions
bind the actual helper without allocation or native retain, before +0C is
usable. There is one actual counter and one authoritative owner/reference.

`helper_completed` is host bookkeeping, not an invented native EH state. If
companion validation unexpectedly fails after B3C800, `helper_binding_failed`
preserves the completed helper allocation identity, partial companions and
camera/viewport block; no native state8 cleanup or automatic recovery occurs.
If B3C800 itself fails, native state8 frees its exact raw helper allocation.
The surviving block remains externally owned, including on helper failure.
Recorded identities are not read as live native storage after free/retirement.

Native final-zero retirement records `helper_retired`; only after all host
borrows end may `forget_retired_helper_after_host_quiescence` discard the
companions. The cockpit block has a separate explicit quiescent reset.
Failed cache/VFS children retain their existing external resolution obligations.
No attempt can be replayed and no local destructor fabricates native rollback.

## Verification boundary

The worker performs source/evidence review and `verify_report_calls.py` only.
The verifier checked 36 call rows with zero failures. Four indirect calls are
identified but their dynamic targets require the separate slot/body evidence.
No new tests, builds, executables, Ghidra writes or CMake registration occur in
this packet. The parent owns strict Win32 integration and any focused fixture.
Source reconstruction is not yet build-tested, fixture-tested, original ABI
compatible, startup-wired or game-validated. `game_hosts.cpp` and actual startup
composition remain named integration dependencies outside the owned files.
