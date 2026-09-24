# B107F0 passthrough post and dust continuation

This packet reconstructs only `[00B11EF4,00B120B9)`, 453 bytes of the existing
`BSP_RenderResources_InitializeMembers_PartialEntry`. Its last included
instruction is the five-byte B120B4 call to B52860, inclusive end B120B8.
B120B9 begins the excluded next 20h post allocation. Original ECX service,
three borrowed DWORD argument cells and eventual B13026 RET0C remain those of
the containing function; this fragment neither reads an argument cell nor
executes a native return.

All 453 live Ghidra bytes match the original configured PE, SHA-256
`8fd8b5d1374f2c8e499b80d8aef7e1833be2021207ecedaf27932fc37e8627b6`.
The complete 117-instruction listing has 22 direct calls. The original literal
regions and complete 32-byte B18AC0 wrapper also match live Ghidra and PE.
The saved project is `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`; all queries were read-only. No full-function,
callee implementation or Ghidra annotation claim is added here.

## Actual producer and source contexts

The new service+650 owner is the real 20h allocation from BF681B, constructed
by the existing full B4E470 with name D5E314, vertex count3 and null optional
input. The literal is exactly `Passtrough.mshd`, including its spelling and
its distinct original address. Retain an independent
`NativePostEffect20ConstructionBlock`, which already supplies its canonical
completed-owner companion; do not add a generic pass companion.

The existing service+34 receiver for B52860 comes from the recovered B14A10
producer: B14EC1 pushes CCh, B14EC6 allocates, B14EDD calls B52550, then B14EF0
publishes the actual result to +34. This is an extent/producer contract, not a new allocation
or proof that callbacks cannot replace the field. B120B1 reads its current
value after frame assignment returns. The receiver must still satisfy the
existing CCh provider contract at that read.

Dust requires the distinct `NativePostEffectConstructionContext` used by the
24h B4E840 family, retained through `NativePostEffectDustContext` and an
independent `NativePostEffectDustBlock`. The stage verifies the same actual
registry, nodes, frames, raw names, strings, renderer publication/profiles,
material factory/cache, mesh/section/stream/declaration providers, camera/model
and viewport domains as the original post20 context. It never reinterprets
one context as the other or creates a substitute provider.

## Native ordering and borrowed parameters

1. B11EF6 allocates20h. EDI and retained ESP14 become the raw result; state56
   is armed. On nonnull allocation B11F1F constructs the raw effect-name header,
   then mask20 is set; state57 precedes B11F3F B4E470. No null-init fallback.
2. B11F48 tests mask20 **before** B11F4D publishes the actual result/null to
   service+650. B11F53 disarms **after** publication. A prior+650 value is not
   released. Conditional name cleanup captures data, clears mask20, then reads
   length+1 and the actual current raw-pool singleton/manager/gate. Headers
   remain stale after their native returns; diagnostic flags do not permit retry.
3. Three raw parameter headers and registrations follow. Each name constructor,
   current receiver/source capture, B4CBA0 material getter, real registration
   and current raw-name return retains its original order and state.

| Literal | Actual name | Borrowed source | Record | State | Capture order |
| --- | --- | --- | --- | --- | --- |
| D5E430 | cSceneColorSampleOffset | service+04 | 2 DWORDs, matrix0 | 59 | current+650, then source address |
| D5E2F8 | cDisplacementSampleOffset | service+194 | 2 DWORDs, matrix0 | 60 | current+650, then EBX=source |
| D5E2E4 | cAASampleOffsets | service+F4..183 | 9 float4, 36 DWORDs, matrix0 | 61 | EBP=source, then current+650 |

The first name is the original continuation context's parameter_names[0]. The
two float2 calls compose the existing B18B00 provider. B12074 B18AC0 uses its
established exact expansion to B17E10: wrapping DWORD `4*9`, matrix0, borrowed
source; no new wrapper/library implementation. B4CBA0 is the existing raw
owner+14 material getter. The source bytes remain live borrowed service
storage, not copied float values or fabricated parameter records.

After the second registration, B12017 captures the raw name data **before**
B1201B overwrites EDI with FFFFFFFF. Name disarm/return then uses that value.
EBX has already become service+194; it must not be treated as -1 by later
stages. EBP changes from halfwidth to service+F4 at B12050.

4. B1209F captures current service+1D4 before B120A5 captures current+650.
   B120AC composes the existing B4E2B0 with those captures: publish/retain the
   incoming actual frame before releasing its captured predecessor. The
   genuine current increment/decrement import cells and D5E600 frame profile
   remain authoritative. Do not reuse construction-time frame/post snapshots.
5. Only after that call returns does B120B1 read current service+34. B120B4
   invokes the complete existing B52860 on that captured receiver. No earlier
   snapshot, extra outer-owner companion or additional retain is introduced.

## Existing dust provider boundary

B52860 creates one actual24h B4E840 child using D620AC `oldfilm_dust.mshd`,
vertex count2 and null input, publishing it to the captured receiver+68.
Its retained block supplies the actual child's canonical companion. It does
not release a previous+68 value. After normal name return it captures current
receiver+3C texture before reloading current receiver+68 and material+14, then
binds texture slot0 through the established actual owner domain. The provider
reads current D7A24C at its native site and performs only its evidenced stores.
This caller neither reproduces those stores nor initializes other outer bytes.

The B14A10 service constructor does not thereby admit service-wide teardown.
Its existing CCh texture-owner companion/lifetime prerequisites remain external
to this fragment. Dust's post24 child and +650's post20 already have their
respective canonical views; nested holder/surface/texture paths retain their
existing lifetime rules. No blanket registry, replacement count or duplicate
generic pass view is installed.

## Retention, frontier and evidence limits

The state retains the independent post20/dust blocks, four raw name headers,
borrowed source/material captures, raw/result/publication receipts and the
same original predecessor/context/entry/argument identities. The predecessor
header adds only one successor identity and two phases. Newly meaningful EBX
bits belong to this new state. Both child blocks and every original domain
must remain alive through callbacks and dependent stages. Preparation and
acquisition failures retain diagnostics; existing child failure policy is
authoritative. No caller rollback, raw free, retry or automatic reset is added.

At B120B9: ESI=same service, EBX=service+194, EBP=service+F4, EDI=FFFFFFFF,
ESP14=raw+650 allocation, mask0 and EH state-1. Other original/aligned/half
dimension spills are unchanged. The same three argument cells remain borrowed
and unread here. Four separate retained name headers do not assert native
private-stack aliasing.

The 22 caller call rows plus the supporting B18AD8 wrapper call verify with zero
failures. Strict `scripts/build.ps1` passed MSVC Win32 Release with `/MD`, and
all three existing CTests passed. No new test or runtime probe was added.
Static composition/build evidence does not
establish the full initializer, native ABI/FH3/SEH, arbitrary profiles, service
teardown or gameplay; earlier packets retain their published limits.
