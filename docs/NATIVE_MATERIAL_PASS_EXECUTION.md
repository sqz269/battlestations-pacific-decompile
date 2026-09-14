# Actual material pass execution

The base pass profile `D62A80/+08` is **B5E5E0, exactly `RET 4`**.
The derived profile `D61BE8/+08` is **B454D0**, which forwards the same
entry and a zero override into **B44750**, then returns with `RET 4`.
The draw path at B2C209 therefore depends on the current dynamic pass profile.
The base profile does not upload shaders or draw geometry.

| Routine | Original interface | Body coverage |
| --- | --- | --- |
| B5E5E0..B5E5E2 | ECX unused, one unused stack word, RET4 | Complete three-byte leaf |
| B454D0..B454DE | ECX pass, entry stack word, RET4 | Complete forwarding wrapper |
| B44750..B44B03 | ECX pass, entry/override stack words, RET8 | Complete geometry/plane/apply orchestration; downstream closure pending |
| B43410..B43667 | ECX pass, six stack words, RET18h | Complete apply/upload/callback/draw/diagnostic orchestration; downstream closure pending |
| B1BFA0..B1BFA3 | ECX queue, EAX current queue+30, RET | Complete actual context getter |

These are source interfaces over actual Win32 storage. They do not wrap the
older typed material or renderer state. The context borrows the same renderer,
diagnostics, cached-effect, bank, synchronization, pool and original-profile
domains as the existing raw providers. Numeric vtables select established
source functions and are never called as host addresses.

## Geometry and plane ordering

B44750 reads the captured entry's section. A zero stream count returns before
all other work; otherwise the first stream's current D61D6C/+28 getter must
return something other than FFFFFFFF. The initial instance count selects the
frequency branch, while stream count and stream slots are reread throughout
the loop. The instance branch reloads each bound stream's tag and, except for
80000000, the section instance count. Its renderer is captured before that
getter. The noninstance branch sets streams0 and1 to frequency1 after binding.

The pass's current borrowed effect is compared with shared0108FBF4. Renderer
F8D394 is captured even on an unchanged effect. On change, the new effect is
published before testing byte13C and the current camera-mode distance. The
COMISS/JBE gate preserves unordered behavior. A disabled/nonpositive plane
restores pending planes on the captured renderer.

The positive branch obtains the actual queue through the complete lazy
004C11F0 provider, reads its current context+30 and then scene+0C. It refreshes
the scene world as needed, transforms `(0,0,actual D7A24C)` with normalization
disabled, rereads mode/distance after those calls, and preserves the original
x87 spill/product and SSE negative-zero subtraction. The second world-refresh
decision is captured before that distance calculation. Position snapshots,
the Y/X/Z dot order, final float spill and FCHS are retained. The actual camera
view/projection getters, affine/general inverses, matrix product, transpose and
plane transform feed the captured renderer's actual clip-plane append.
`CameraMatrix`/`CameraPlane` here are the existing fixed float arrays used for
native local scratch; no camera/renderer/owner projection is constructed.

Afterward, the renderer and entry section are reloaded for declaration binding.
The entry mesh supplies its current index owner. The renderer and first stream
are captured before obtaining the base vertex; that renderer's captured profile
selects index binding. The index-presence/section-byte gate changes only BL;
the higher three bytes of the argument retain the captured renderer pattern.
A second getter on that same captured first stream supplies the apply argument.

## Apply and caller registers

The six B43410 stack words are entry, override, indexed DWORD (only its low byte
is used), index owner (unused), index base and vertex base. B44AF2 pushes all six;
B43665 cleans18h. B43410 captures renderer and section before binding render
states then sampler states. It reloads the entry section/material for signed
word104; nonnegative values set render state39 on the current renderer. Vertex
and pixel shader binding still use the initially captured renderer.

The sampler count and PS mask are captured once. Every sampler row and special
byte is reread at its original stage. A skipped pixel row still advances its
pixel slot; special rows use the separate counter starting16. Negative selectors
call the real B17D90, including its unmatched fallback retain and post-increment
fallback reload. This caller does not add a balancing release.

After concrete B42350, stage ends and the prefix form wrapping DWORD counts.
Only a returned VS upload reloads the prefix. The PS gate uses its current stage
end/prefix, but its pushed upload count remains the earlier captured EBP value.
No HRESULT branch or compensation is inserted.

At B435B1, the nonzero material+08 code pointer receives no stack arguments.
The source trampoline seeds ECX=entry, EDX=current section, EAX=callback,
ESI=pass, EDI=initial section, EBX=captured VS count, EBP=captured PS count and
the original TEST flags. It calls the actual code pointer; this is not a default
callback or automatic relocation of original image code. Private stack aliases,
unconstrained caller-saved vector registers and original callback binary
integration remain outside the source interface.

Draw calls use the current renderer and original argument-load order. Statistics
reread the section after drawing, derive primitive-mode vertex counts using the
captured native jump table, reload entry camera mode, capture diagnostics, then
obtain the pass's current effect and call the actual diagnostic accumulator.

## Dependency and exception boundaries

`native_material_constant_build.hpp` is the independent concrete B42350 module.
Its stable function takes actual pass/entry/override, a
`NativeMaterialConstantBuildContext&`, and caller-owned
`NativeMaterialConstantBuildFrame&`. This packet declares that function and
retains the frame by reference; it supplies no stub or callback implementation.
The released raw diagnostic packet62828579 provides the exact B16F80
implementation over `NativeStringRawPoolContext` and six original stack
arguments. Its record/string foundations are preserved as separate dependency
commits, including raw string overloads7D9A6804; their source exception-domain
limits apply unchanged.

The execution frame records a stage and terminal failure. It owns no native
storage, never acknowledges or disarms a failed child, and does not undo any
already-published binding, retain, bank write, draw or statistic. A failed frame
cannot be reused. B44750/B43410 have no original EH registration; actual provider
cleanup remains with those providers. The new C++ interface does not reproduce
original private frames or hardware SEH.

Released dependencies were narrowly cherry-picked: texture leaves514e822b,
geometry leaves90170e87, and their atomic-type/header dependency9ac26670.
Only each packet's own CMake source append was accepted on conflict. No unrelated
main ancestry was merged. The released B16F80 packet and its record/string
foundations were likewise imported without copying unrelated ledger changes.
B42350 is explicitly assigned separately.

## Verification status

The report records complete live=PE spans, original profiles, the primitive
jump table, exact native call rows and the definition/save record for the two
previously undefined virtual leaves. MSVC Win32 strict source compilation passed.
Full build, existing CTests and final artifact capture are recorded in the report
when complete. A static archive build cannot establish linked B44750/B43410
execution while their two concrete downstream providers are absent. No game,
active-renderer, full derived-pass or original EH execution is claimed.
