# Native renderer base and singleton subobject lifetime

This packet reconstructs nine complete native bodies over the application's
raw renderer primary object and its +0C singleton subobject. The C++ functions
add borrowed references to the actual `00F8D394` renderer publication and
`01090AA0` singleton-manager publication. They are source interfaces, not
fixed-address ECX/RET ABI replacements. Descriptive names are hypotheses.

| Native body | Original entry and return | Exact scope |
| --- | --- | --- |
| B33A70..B33A8C | ECX primary; EAX primary; RET | D5F1EC, +4/+8 zero, BD1860 section at +4 |
| B33D90..B33D9D | ECX primary; RET | D5F1EC, 0041CC80 releases section at primary+4 |
| B33E10..B33E36 | ECX primary, stack flags; EAX captured; RET4 | Inline primary release, flags bit0 BF65AC free |
| B25F40..B25FD7 | ECX subobject; EAX subobject; RET | Captured singleton section, F8D394 primary publication, BD0C30 registers +0C |
| B25FE0..B26082 | ECX subobject; RET | Current F8D394+0C unregister, clear publication, root profile |
| B26090..B260AD | ECX subobject, stack flags; EAX captured; RET4 | Full subobject teardown, flags bit0 free |
| B283F0..B2844E | ECX primary; EAX primary; RET | Primary then subobject construction, final vtables and fields |
| B284E0..B28537 | ECX primary; RET | Final vtables, subobject teardown, then B33D90 primary release |
| B28540..B2855D | ECX primary, stack flags; EAX captured; RET4 | Full renderer-base teardown, flags bit0 free |

`B33A70` writes primary vtable profile `D5F1EC`, clears +4 and +8 in that
order, then calls the existing raw `BD1860` allocator. Its returned 1Ch
tracked Win32 critical section is stored at primary+4. If allocation or
initialization fails before the call returns, this body does not invent a
rollback. `B33D90` writes the same profile before invoking the existing full
`0041CC80` release on the +4 slot; that release clears the slot only after
deleting/freeing the captured section. `B33E10` has the same inline profile and
release schedule, followed by the scalar free gate.

`B25F40` takes the subobject pointer at primary+0C and writes temporary
subobject profile `D5E610`. The first actual `00415350` getter supplies a
section at singleton manager+10. The native stack guard records that section,
calls `EnterCriticalSection`, and increments its tracked DWORD at section+18.
It computes primary=subobject-0C, publishes **primary** to `F8D394`, then
computes the registration argument from that local primary (nonnull -> +0C).
After the second actual getter it calls the full raw `BD0C30` register body
with that **subobject** pointer. This call does not reload F8D394 for its
argument. The captured first section is decremented/left afterward.

`B25FE0` writes `D5E610`, enters the first getter's captured section, then
reloads the **current** F8D394 publication. It computes the current primary's
+0C subobject (or null) *before* its second `00415350` getter and passes that
captured value to full raw `BCFCA0`. It clears F8D394 only after unregister,
releases the captured first section, then installs root profile `CE3818` on
the receiver subobject. Thus a changed F8D394 unregisters the changed owner's
subobject, not the captured ECX receiver; the source retains this behavior.

`B283F0` calls B33A70 on the primary, then B25F40 on primary+0C. It writes
final subobject profile `D5E76C`, final primary profile `D5E628`, zeroes the
byte at primary+10 and stores `40000000` at primary+18. Apart from those
stores and the primary/subobject bodies, preimage bytes remain untouched.
`B284E0` writes D5E628/D5E76C first, calls B25FE0 on +0C, then B33D90
on primary. Its scalar B28540 captures primary before teardown and tests only
flags bit0 for free. The subobject scalar B26090 instead frees the captured
**subobject** address when bit0 is set; callers must use flags0 for a +0C
interior subobject. Every free requires the matching native CRT allocation
domain. Source `singleton_lifetime_free` uses the host `/MD` CRT boundary.

The native EH maps are explicit: B25F40 uses `DF57BC`, state0 `CBD0E0 ->
00412430`, state1 `CBD0E8 -> 00411EE0`; B25FE0 uses `DF57F0`, state0
`CBD100 -> 00412430`, state1 `CBD108 -> 00411EE0`. B283F0 uses `DF5934`,
state0 `CBD1E0 -> B33D90`; B284E0 uses `DF5960`, state0 `CBD200 ->
B33D90`. Therefore failed registration may leave F8D394 published and a
partial registry entry while root-profile cleanup and primary section release
still run. Failed unregister before clear retains F8D394; the outer primary
release still runs. No native SEH frame identity or hardware-fault compatibility
is claimed.

Three scalar listings omit an instruction after the `_free` thunk at BF65AC:
`B260A5..A7`, `B28555..57` and `B33E2E..30` are each `ADD ESP,4` in the
installed executable. `bsp.py disasm-raw` verifies the exact bytes and the
callee BF65AC->BF9DC8 returns with plain RET. Root must repair these Ghidra
listings under the write lock, then refresh affected exports/ledger evidence.
This worker made no Ghidra changes. Native call rows and installed PE span
hashes are in `reports/native_renderer_base_lifetime.json`.

The packet supplies the base/subobject lifetime only. Root owns the full
renderer B32410/B32920/B339F0, derived +0C adjustor B32900, shared deletion
dispatch, application 1D94h allocation and actual game publication. Those
paths must borrow these same F8D394/01090AA0 cells and honor that the
singleton registry stores primary+0C. The focused local fixture uses the
actual BD0C30/BCFCA0, BD1860/0041CC80 and Win32 critical-section providers
to exercise publication identity, current-slot unregister, primary cleanup and
scalar free; it does not run game rendering, GPU, SDK or network activity.
`./scripts/build.ps1` passed Win32 Release and both configured CTests after
`verify-seeds` matched all eight installed native differential ranges. The
single local fixture compiled source and itself under `/MD /W4 /WX`, linked
with `/MANIFEST:EMBED`, and passed. Neither check is gameplay validation.

Integrated validation at `8f73fbf754bc47dfac527e898c3cf7a4cc6ebfa4`: strict Win32 build and both CTests pass. The existing raw1D94h-owner fixture passes against the exact integrated library, using compatible14h manager storage and real tracked sections. It checks field preimages, primary publication, secondary registration/current-slot unregister, section release and scalar frees. This is source lifetime validation; no original/source differential or application-drain claim is made.

Root restored ADD ESP,4 at B260A5/B28555/B33E2E, saved nine reviewed names/evidence comments while retaining prior comments, refreshed all nine exports and verified24 direct calls. B33D90 is primary section destruction, superseding its earlier generated adjustor classification. The actual derived secondary adjustor is B32900. Native B25F40 computes/pushes the captured registered subobject before its second415350; B25FE0 similarly captures the current publication before that getter. Keep this order distinct from Lua and registry base bodies. `local/checkpoints/8f73fbf7/native-renderer-base-default/validation.json` freezes 3229 artifacts and references the66-artifact worker archive. Full derived lifetime, application composition and gameplay remain open.
