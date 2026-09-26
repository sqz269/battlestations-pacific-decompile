# Native texture-source reached child append

`append_native_texture_source_child_00c307ad_fragment` reconstructs only the
C307AD..C307E1 internal append fragment of C30570. The two new header/source
files and one deferred `bsp_core` CMake registration match the reviewed design.
No caller is activated and the complete C30570 producer remains unimplemented.
Descriptive source names are hypotheses, not recovered native symbols.

The original fragment is53B/19 instructions. It restores the owner from the
producer frame, selects the header at owner+10, reads capacity then CURRENT
count, and reserves on equality. Growth doubles the DWORD with wrap and chooses
the signed result if greater than1, otherwise1. After the genuine735FF0 call
it reads CURRENT count then CURRENT data, computes the wrapped slot address,
skips the store only when that address is zero, and increments CURRENT count
unconditionally. EDI carries the renderer result, including null; EBX is zero
from C30595. C307DC stores the child, C307DE reads/writes count. C307A8 temporary
string return precedes this fragment; owner restoration/iteration at C307E2
follows it. The original producer's register/frame ABI is not a standalone leaf
ABI. The new source interface is cdecl, a payload reference and captured pointer.

Admission requires the actual C30470-created live NativeTextureSourcePayload
at owner+8, its sole nested header, genuine typed735FF0 pointer backing and live
reached source pointer slots. A raw image or equal pointer bits do not establish
that lifetime. This leaf starts one default scalar `void*` lifetime only at the
original reached store, using placement new without parentheses/braces, then
stores through the returned new-object pointer with volatile access. Existing
same-type pointer slots may be transparently replaced there. Unused capacity is
not initialized or read. No payload/header/atomic is constructed again.
Reentrant allocation callbacks may change current fields but must retain these
active lifetimes and valid reached backing; reads after reserve remain fresh.

The leaf adds no child retain/release or rollback. Captured null is stored when
the placement address is nonzero, and count still increments after an address
skip. This is recovered behavior, not safe null-child retirement: C304A0 later
decrements actual child+4. A returned-but-unappended child remains the caller's
existing obligation if temporary string return or reserve fails. No source
flag/FPS/count normalization, positivity guard, null-child guard or cleanup is
added. Local memcpy converts only scalar signed/unsigned bits, not a snapshot
of a header, pointer buffer or publication.

One strict MSVC Win32 build against e64ad38490e20c2c524faeb61c60e80d288cf239
passed with /MD /W4 /WX /fp:strict, followed by all three existing CTests.
No new test or runtime case was added. The compiled leaf is67B/29 instructions
with a sole CALL relocation to the typed735FF0 overload. Its eight field/call
events retain the capacity/count/reserve/current count/current data/slot/count
read/count write order. Native signed JG becomes signed CMOVG; native count
memory ADD becomes fresh MOV/INC/MOV. The address-zero branch rejoins at that
fresh increment. Placement construction and scalar bit conversion add no calls
or FP operations in the leaf. New source code and the53B native fragment are
not byte-identical or original-ABI replacements.

The complete1392B I386 object contains83B in three code sections: the67B leaf
plus unreferenced8B placement-operator and8B signed-bits helpers. Neither helper
has a relocation and the leaf does not call either. Complete raw COFF sections,
code bytes and relocations are archived; `compiled_schedule.json` records the
event/branch lowering. Three selected reserve/procedural/time members remain
exact full-member identities across the preserved old and current libraries,
including81 existing code sections/3979B and every raw section/relocation.
The current1779-member library contains the exact new leaf object. The old
1778-member full library is historical worker child-header build evidence,
not a claim that all later disjoint main providers were identical.

Prebuild evidence pins18 source/build files,27 tools/SDK inputs and nine
preserved artifacts. The actual build launch/poll/completion tool returns and
build log are retained. Two inspection-helper issues are preserved separately:
an input filename was corrected to the actual singleton_lifetime header/source
before build; the first postbuild inspector assumed one code section and was
corrected to include both unreferenced helpers. Neither changed production code
or caused another build. The first filename-failure receipt is explicitly a
stderr transcription, not a raw tool return object.

The native bytes are reused from the exact installed PE and the separately
frozen804B C30570 audit. Readiness lookup/range/enclosing lookup and the frozen
C30000 shard found no prior producer/append fragment credit. This packet adds
one53B fragment record and zero native functions. The full producer audit is
nested separately as non-reconstructed evidence; its remaining751B are not
credited by this source leaf.

Whole-producer prerequisites remain separate: raw Lua fatal/nonlocal error
transfer and cleanup diagnostics, actual stable tracked Lua/string storage,
captured renderer/profile/+64 dispatch and persistent resource operation/credit
domains. Inspected D5F0A8+64 is B319B0, whose genuine provider already exists;
arbitrary other current-profile targets are unresolved. C30570 sets flag1 before
callbacks, stores unchecked FPS and may append null. Native cleanup preserves
flag/FPS/prior appends and has no child-result rollback action. Neither positive
FPS nor nonnull children are guaranteed. No complete producer, Lua transport,
child factory, cache/platform/MSG/XLive/source0/W, full sampler/application,
original native ABI, private FH3/SEH/hardware-fault or game proof is claimed.

Report: `reports/native_texture_source_child_append_cc10.json`.
Frozen archive: `local/cc10_texture_source_child_append_evidence.zip`.
Manifest and detailed schemas are under
`local/cc10_texture_source_child_append_implementation/`: `manifest.json`,
`old_members.json`, `new_members.json`, `complete_coff_sections.json`,
`compiled_sections.json`, `compiled_leaf.json`, `compiled_schedule.json`,
`compile_command.json`, `before_build.json` and `freeze_receipt.json`.
