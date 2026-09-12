# Native Object and Tracer particle loading

Addresses: `00AF8BD0`, `00B0AD50`, `00AF80F0`, `00B0A920`.

These routines operate on the actual particle definitions produced by
`AF89E0`/`B0A0B0`, the existing actual TextBuffer and pooled text interfaces,
the shared `F8D344` parameter pool, and the actual atlas manager/item storage.
Names below are descriptive hypotheses, not recovered symbols. The APIs are
new C++ interfaces; they are not binary replacements.

| Entry | Inclusive end | Original ABI | Coverage |
| --- | --- | --- | --- |
| AF8BD0 Object parser | AF9094 | ECX definition; stack TextBuffer; RET4; AL true | complete |
| B0AD50 Tracer parser | B0B5CB | ECX definition; stack TextBuffer; RET4; AL true | complete |
| AF80F0 Size cache | AF816B | ECX Object; stack parameter; RET4 | complete |
| B0A920 texture list | B0AB8A | ECX Tracer; stack filename; RET4 | complete |

AF8BD0's last physical block jumps back to cleanup; its return is at AF9051.
The other final returns are at B0B5C9, AF8169 and B0AB88. All four full spans
match the installed executable and the verified live BSP program byte for
byte. See the companion report for hashes, call sites and validation results.

## Parsing and field producers

Both parsers clear only definition byte `+64` on entry. They scan for an
opening brace, then perform a further line read even if that scan reached EOF.
A closing brace is consumed; the following line remains unread. EOF and unknown
non-Param lines return true. A Param line first passes its suffix to B015C0;
only an unhandled property reaches the type-specific logic.

For a numeric property, the percent token is parsed to float, its temporary
tokens are released, a real 10h builder is constructed and initialized with
zero endpoints, and the remaining suffix is parsed by AFC470. Its return is
ignored. B00980 handles the shared particle parameters first. Unknown names
still incur the same construction, parsing and cleanup. AFBED0 does not write
builder `+C`; the first value is an explicit incoming stack residue in the
common binding, and that word persists across parameter lines within one
parser invocation. Invalid parameter text has no invented valid-type default.

Each scaled parameter is converted by AFBF60 **before** reading the current
double at D7A358. The native `FLD float / FMUL double / FSTP float` sequence is
retained. There is no float conversion of the scale and no old-parameter
release when a field is replaced. Temporary destruction follows native normal
cleanup order and preserves the shared pool's allocation/release events.

| Object property | Produced field/effect |
| --- | --- |
| Model | Captured current definition vtable+20; stack token2 |
| RotationSpeed | Scaled runtime parameter pointer +80 |
| Size | Scaled parameter passed to AF80F0; +84 pointer and +88 value |

AF80F0 is a **Size** cache, not a rotation helper. It publishes +84 first.
Null stores positive zero at +88; kind WORD +A equal to zero copies the raw
DWORD +4 without arithmetic. Kind one calls AFFA70 at time zero using actual
20h linear rows. Every other nonzero kind calls AFFAE0 using actual 28h cubic
rows. Both returned values spill to float. No kind repair or segment clamp is
introduced. The D5DB00 table and constructor establish the actual Object
domain; D5DB08 references the parser and AF9086 is AF80F0's only direct caller.

| Tracer property | Produced field/effect |
| --- | --- |
| ShowBullet | Byte +B0 = signed atol(token2) > 0 |
| BulletHeadTexture / BulletTailTexture | Actual borrowed atlas item/null +A4 / +A8 |
| TracerTexture | B0A920 over actual descriptor +98/+9C/+A0 |
| MaxSegmentNum / MinSegmentLength / SegmentLifeTime | First builder **float** +80 / +84 / +88 |
| BulletRadius / BulletFadeBegin | Scaled runtime pointer +8C / +94 |
| BulletLifeTime / BulletTailLength | First builder float +90 / +AC |
| OuterColor_R/G/B | Scaled runtime pointers +B4 / +B8 / +BC |
| Glow_R/G/B | Scaled runtime pointers +C0 / +C4 / +C8 |
| Tail_R/G/B | Scaled runtime pointers +CC / +D0 / +D4 |
| Width / WidthSpeed / WidthSpeedEnd | Scaled runtime pointers +D8 / +DC / +E0 |
| TileLength | AFBF60 result +E4; multiplier +0 remains untouched |

B0A0B0 establishes the descriptor's pointer/count/capacity at +98/+9C/+A0.
B0A920 resets only count, then asks AF3A20 for the contiguous frame count.
Zero count performs one direct atlas lookup. A positive count constructs each
actual 8h frame name with AF3B50, looks it up, and stops on the first miss while
keeping prior appends. Negative counts perform no iteration. A null generated
name uses the borrowed actual F8D390 bytes. The atlas lookup returns the same
raw 30h item pointer present in the manager's +4 array; no semantic
TextureAtlasItem conversion, resolver, shadow collection or resource owner is
created.

On a full descriptor, capacity grows using unsigned `2*capacity+2` only when
that wraps to a larger value. The capacity store precedes array allocation.
Multiplication by four saturates its allocation byte count to FFFFFFFF on
overflow. Copying rereads the current descriptor/count; free occurs before
publication of the new pointer. The actual item pointer is appended using the
current descriptor after allocation/free callbacks. Existing item pointers and
capacity survive a subsequent count reset.

## Real dependencies and call evidence

| Native callee | Reviewed contract and use |
| --- | --- |
| AF5740, AEE3C0, AF44C0, AEE2A0 | Existing actual TextBuffer, token, suffix and pooled destruction bodies; same NativeStringStorage |
| B015C0 | Shared property parser, actual atlas/layer/shader domains; called before local property dispatch |
| AFBED0, AFC360, AFC470, AFBF60, AFC1B0, AF4110 | Existing actual builder/parameter producers and cleanup; ignored parse return retained |
| B00980 | Shared actual particle parameter handler, ECX definition and three stack arguments, RET0C |
| AFFA70, AFFAE0 | Existing complete actual linear/cubic curve evaluators; ECX curve, stack time, RET4/ST0 |
| AF3A20, AF3B50, AEFB20 | Shared raw frame counting/naming/atlas lookup bodies; actual pool and raw item identities |
| BF55BE, BF6989 | Real application array allocation/free domains; CDECL stack argument, caller ADD ESP,4 |
| AF9660 via current vtable+20 | Reviewed Object model resource loader body; ECX definition, stack filename, RET4. Required application dispatcher executes the captured target. Its internal VFS/resource/model work is outside this packet. |

AF8D75 loads its target from the current Object vtable+20 after tokenization.
The installed D5DB20 slot points to AF9660, whose body clears/loads actual model
resources and appends them to Object +8C/+90/+94. The interface does not invent
a successful model or shader load. AF80F0 and B0A920 each have one direct call
site, AF9086 and B0B080; parsers are installed at D5DB08 and D5E050. The report
lists every direct native call plus the indirect model call with its owner.

## Ghidra repairs required at integration

The saved function extents are already correct. Per-call false no-return
overrides on returning BF6989 have hidden four continuations:

| Call site | Missing inclusive bytes | Actual continuation |
| --- | --- | --- |
| AF8F48 | AF8F4D..AF8F78 (44) | Retrieve name, ADD ESP,4, clear builder +0/+4/+8, name cleanup scan, continue parsing |
| AF8FCE | AF8FD3..AF8FEF (29) | ADD ESP,4, clear builder +0/+4/+8, AEE2A0 call at AF8FEB, continue parsing |
| B0A9F0 | B0A9F5..B0A9F7 (3) | ADD ESP,4 then publish new descriptor pointer and append |
| B0AAE0 | B0AAE5..B0AAE7 (3) | ADD ESP,4 then publish new descriptor pointer and resume frame loop |

The three-byte alignment gaps AF8BFD..AF8BFF and B0ADBD..B0ADBF follow jumps
and are not missing executable flow. Clear only the four call-site overrides
under the existing Ghidra write lock, disassemble their continuations, preserve
existing comments, apply the proposed names/prototypes, save, and refresh the
four exports. This worker made no Ghidra or shared-ledger mutations.

## Verification and limits

The scratch probe in `C:/Users/sqz269/bsp-aq-object-tracer` relocates the four
installed/live matching byte spans by decoded Capstone operands. It uses real
shared text, parameter, pool, raw atlas and curve implementations and the actual
SysWOW64 d3dx9_40 import. It compares complete normalized definition bytes,
runtime payloads, text cursor and exact string/array events, plus a mutable
percentage operand and all Size cache branches. See the report for execution
status and hashes. The fixture provides actual small raw atlas entries; it does
not claim the renderer has loaded the game's textures.

The probe exits **0**. Object matches 763 string events and 793 interleaved
string/array events; Tracer matches 2,949 and 3,026 respectively. Separate
checks cover EOF/brace handling, three preexisting texture-array hit/miss/frame
cases, an invalid parameter following Const, a scale change during conversion,
and 17 null/constant/linear/cubic Size variants. The existing worktree build
and its single CTest pass. The call verifier checks 155 rows and currently has
one expected mechanical failure: AF8FEB is absent from Ghidra's function body
until the AF8FCE false-no-return continuation is repaired. The integrator has
queued that repair and will rerun the verifier before accepting the packet.

Strict MSVC Win32 compilation uses `/MD /W4 /WX /fp:strict /O2`. The ordinary
worktree build checks the existing CMake graph; the integrator adds these four
owned files' source registration and reruns the current full build and
library-only probe. Native FH3 unwind metadata, arbitrary invalid pointers,
malformed string faults, heap-failure unwind equivalence and gameplay remain
unvalidated. Model/shader/renderer services are explicit real application
boundaries, not successful fixtures or fallback implementations.

## Correction from docs/NATIVE_PARTICLE_TYPE_RESOURCES.md (AR)

Optional same-base resource binding directly invokes complete AF9660 for the captured Object model20 target; current overrides and legacy callers retain the required real dispatcher.

Earlier isolated dispatcher captures remain historical evidence; the optional
composition does not establish complete application wiring or gameplay.
The AQ final integration already repaired AF8BD0's free fall-through and
validated its complete body/call rows; earlier pending-gap notes describe the
pre-repair worker snapshot. AR keeps that repaired normal control flow.
