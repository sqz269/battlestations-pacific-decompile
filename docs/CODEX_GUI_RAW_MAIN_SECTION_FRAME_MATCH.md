# Main-section Frame constructor identity view

**Source-only library helper; no caller or activation.** Exact baseline
`4a934a3d5225ce85f199bdff2d3abc8e29b05b8a`, containing33536 main-section storage
and19999 sticky-failure behavior. This packet owns only
`native_gui_text_main_section_storage.hpp/.cpp` and this new doc. The Root's
unpublished private Font candidate/harness is not merged or modified.

## Exact interface and constructor capture

Adds one nonvirtual `const noexcept` method:

`bool Frame::matches(const Context&, const Acquired&, Phase expected) const noexcept`.

The actual Frame constructor takes ONLY `Context&` and `Acquired&`. It pins those
references in PRIVATE `context_`/`acquired_`; private `phase_` begins FRESH.
This view compares their exact addresses and the private phase. It reads no
public acquired field, native payload, operand value, Font, section, material,
registry or provider. It invokes no getter/callback/allocation/name constructor.

The temporary8B name storage and parenting frame are private INLINE Frame
members, not caller-supplied arrays or erased external backing. Their backing
is already tied to this exact Frame object. No new arguments, fields, setters,
public operand view, storage initialization or layout/vtable change is needed.
Both public record copy operations remain deleted as before.

This is analogous to the accepted private identity/phase views on the379
geometry Frame and db55 primary Frame. A different Context object or Acquired
object fails even when it holds equal/reference-aliased public values. Changing
Acquired.phase cannot turn this Frame's private FRESH/FAILED phase into COMPLETE.
The real ensure function remains the only friend that advances private phase.

## Meaning and limits

FRESH matching can authenticate the installed context/acquisition before
effects. COMPLETE matching can authenticate which exact invocation returned
through the genuine main-section body. Neither result grants native liveness,
credit, exclusive count1, successful creator exchange, current slot publication,
or physical retirement/pool-return authority. It is not another private issued
db55 receipt and does not create a mesh/section/material owner.

Future admission must still validate exact private db55 and379 provenance,
actual main-section construction/material/cache/name/release history, complete
unfailed public diagnostics, canonical current geometry/section/material and
the qualified sole graph. Future settlement must use independently retained
Model/terminal metadata after entries/companions erase themselves; it must never
rediscover an ended native object from a saved pointer or a phase alone.

The existing successful body, original load/store/import/x87/control-flow order,
and sticky failure behavior are unchanged. A caught recursive failure still
prevents completion; the first escaped diagnostics remain retained. No provider
call, raw85 replay, lifetime-choice/owner/caller route, lazy publication, native
cleanup or new native-body credit is added. The address00533FA0 lease is scoped
coordination for this dependency only; that factory body is not edited/recovered.

## Source validation

Read-only comparison against exact baseline proves the entire existing CPP is
byte-identical after removing ONLY the new method definition. Header comparison
likewise removes ONLY the new declaration/comments: all fields, constructor,
friend, phase getter and private backing declarations remain identical.
Whitespace checks pass. No new tests/framework/public test-only API, compiler,
probe, game, CMake/shared metadata or Ghidra mutation was performed.

Root owns independent review/build. Original register/private-stack/FH3/SEH/FP
ABI, actual allocation/credits/retirement, caller admission, repeated/reentrant
use, gameplay/rendering and runtime acceptance remain separate qualifications.

## Root integration verification

Frozen9a446f8a11bd30def6e00798fb3ec1d1523cc06f was independently reviewed and
merged after the private Font runtime cases. The method reads only the two
private constructor references and private phase; no member data/layout change
or native body change accompanies it. Combined Win32 and existing checks2/2
pass (10.23s), `local/cc16_raw_font_metadata_restored_off_main_section_build.log`.
Source84466023263bcd5d6ff8d1545a0d9ec8f8bfd939 has all four private modes FALSE.
This is compiled library metadata, with no MainSection caller/runtime credit.
