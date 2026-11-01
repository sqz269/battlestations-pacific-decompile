# Raw atlas buffer parser AEEAF0

The candidate implements the whole ordinary NORMAL payload of `00AEEAF0`
over the actual manager, text buffer, raw headers, arrays and concrete providers.
It is an ignored Source proposal from frozen
`2a094149a200654ea7ddf63226483c2a72bcba35`; no tracked Source, CMake, ledger,
Ghidra project, build, test, Game process, installation or activation was changed.
`reports/cc16_native_texture_atlas_parser_storage.json` records exact pins.

Original ABI is ECX actual manager1Ch, one incoming buffer-pointer DWORD HOME,
RET4, AL Boolean. The proposed C++ interface is
`parse_native_texture_atlas_storage_00aeeaf0(manager, incoming_home, frame, context)`.
Its names describe established behavior; they are not recovered symbols. This
is not an original stack, register, return-high-byte or exception-ABI replacement.

The full 478-row current assembly was inspected through capped `bsp.py show`
pages. Live metadata identifies `/battlestationspacific.exe`, 478 instructions,
78 basic blocks and 56 static calls. The configured saved project is
`C:/Users/sqz269/bsp.gpr`, project `bsp`; no import or annotation was performed.

| Original range | Retained ordinary payload |
| --- | --- |
| AEEAF0..AEEB81 | Capture incoming buffer before rewind; flags0, line0; real AF5740; EOF returns line and AL0. |
| AEEB84..AEEC76 | Token0/TextureAtlas compare and release; mismatch returns line/AL0; token1, genuine exact41E870 TextureName, token release. |
| AEEC76..AEED5B | Copy saved receiver+C descriptor; compute/release DISTINCT BEE390 raw8 without directory assignment; real4BCB80, exact resize/optional space fill; slash construction, captured LEN/DATA append/return. |
| AEED5C..AEEDE1 | Capture renderer BEFORE concat; CURRENT captured receiver profile/slot64 AFTER concat; concrete admitted B319B0 with fresh actual acquired; return fullname; actual manager+10 texture append before body. |
| AEEDE5..AEEEAE | Seek opening brace, immediate second line, loop until closing brace/EOF, skip empty line, token0/TextureItem compare; return keyword then zero it before branch. |
| AEEEB4..AEEF95 | Adopt child before BF681B(30h), preserve null arm; token1, name8, current incoming HOME+C descriptor, genuine AEE4D0; native flags2 then1 normal return/clear order. |
| AEEF96..AEEFF2 | Copy CURRENT input HOME into DISTINCT child AEE620 HOME; always call params, real735F90/current item array append; restore saved texture; nonempty-arm current input reload before next line. |
| AEEFF3..AEF084 | Return current directory, TextureName and line in order; stale homes retained; AL1 even on body EOF. |

The Native S+10..S+50 layout is explicit in `NativeTextureAtlasParserHomes`.
Header-token0 and later item-name token share the same4h HOME. Canonical/slash
use the same raw8; its first DWORD later holds the allocation pointer. Every
helper owns its first writes; no whole-frame memset seeds native preimages.
Static offset assertions document these physical home relations without
claiming original private-stack aliases or ABI identity.

The buffer receiver remains the original saved receiver through header work.
Only the reached nonzero-slash append path reloads it at AEED39. Body seek uses
that saved receiver. An empty body line keeps it; every nonempty arm reloads
the CURRENT incoming HOME at AEEFDB. The parser never writes the incoming HOME.
Each item's separate outgoing parameter HOME receives a current DWORD copy
only after name/keyword cleanup. AEE620 may overwrite that child HOME with F32
and saved x87 CW bits; these writes cannot change the parser's still-live HOME.

`NativeTextureAtlasParserContext` borrows the same genuine raw pool/feature,
RO owner, shared actual F8C2C8 scratch, raw canonicalizer and renderer application.
It embeds exact item/parameter contexts over those same references and checks
raw pool-cell identity, feature-reference identity and actual cache string-cell
identity. It creates no manager, buffer, scratch, feature, singleton publication,
renderer, texture factory or semantic sprite atlas. Real host role authority and
full process lifetime remain required, including F8C26C/F8C2C8 custody.

At AEED5C only the renderer publication value is captured. After the exact concat
returns, the captured receiver's CURRENT numeric profile DWORD is read, the SAME
verified read-only service maps it, and slot64 is captured. The approved finite
parent closure requires this application's genuine normal B32410 birth pointer,
independent of later publication writes, same RO owner, ready lifetime, captured
D5F0A8/B319B0 dispatch and existing cache. Live bytes at D5F10C were
`b0 19 b3 00`. No numeric function pointer is executed. The single actual
`NativeTextureCacheAcquired` belongs to this retained parser frame before entry;
existing B319B0 and its actual creator/cache graph are reused directly.

The returned creator reference transfers to the actual manager+10 array before
the item-body reads, with no additional retain. Null slot tests, signed doubled
capacity choice, real735FF0 reserve, current table/count reloads and current
count increment are literal. Per-item construction uses the real raw AEE4D0
three mutable outgoing DWORD HOMEs. AEE620 executes even on allocation-null;
no recovery/default is added. Real raw735F90 publishes the item before increment,
then the saved texture register is restored. No UV/packed defaults are seeded.

Every matched item list-adopts distinct nonmoving metadata, outgoing HOMEs and
actual constructor/parameter frames before BF681B. All prior children remain.
The allocation, entered helpers, append slot/write/increment history and parent
state/site/failure are retained. A C++ escape records failure and barethrows
without new outer cleanup, token replay, item deletion, array erasure or free.
The existing acquired frame terminates if an incomplete/failed frame is destroyed;
retain the complete failed graph through explicit external disposition.

Caller-owned native backing, initialized reached fields, aliases, live raw pool
and actual provider publications, physical allocator/CRT, CPU/SSE2/DF, serialized
shared scratch and lifetime domains remain required. The incoming HOME and
private Source frames/controls are disjoint from all provider-write extents except
declared raw header/argument aliases. Native FH3/SEH, secondary unwind, hardware/
null faults, unmasked FP, Original Origin, ABI and game parity are unqualified.
The lower text/token/cache providers keep their existing declared error policies;
current Source CRT supplies full `_stricmp` behavior without vendor-body credit.

Dependencies are the sealed raw BEE390 overload, exact426060/4261A0 and raw735F90
proposals, the finite actual renderer custody seam, and frozen raw AEE620 /
AEE4D0/AEE220 leaves. Existing exact41DD40/41E870/BF7680/BF79F0, actual pool,
AF55F0/AF5740/AEE3C0,4BCB80,735FF0 and B319B0 providers are direct calls.
Old semantic and legacy memmove/zero-copy-omission APIs remain untouched.

AF0060, AEF280, AEF3C0, registration/deletion, process host-role production,
common.ats startup and Page/type6 production remain separate work. This proposal
does not activate them or change any of the existing nine held gates. Source
inspection and bounded integrity checks are the only verification here. Compile,
fixture, peer review, Native runtime, original ABI, startup and gameplay credit
are zero pending Root's exact joined-tree review and authorized validation.
