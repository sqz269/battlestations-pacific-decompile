# Raw Movie lifetime readiness

Frozen published base: `f90f5d178da2ef3b8265145cd08ea8e9bc935467`.
The complete native destructor and its four-state unwind map are recoverable,
but a whole populated raw Movie lifetime is **not ready for implementation as
a closed consumer**. Real raw string/base/pool services exist; record-range
and vector-tidy compositions, actual decoder-scalar custody and the reached
widget/model graph are not all supplied. The smallest closed prerequisite is
the two exact raw record-string range contracts described below.

Only this document and its report are changed. No source, CMake, ledger,
Ghidra or frozen-peer mutation, build, test, probe, game or DLL load occurred.
Names are descriptive hypotheses, not recovered original C++ symbols.

## Native whole destructor

`00AAFEF0..00AAFFEF` inclusive is256 bytes/78 instruction starts. ECX is the
actual130h Movie widget, captured in EDI; plain RET, no stack arguments, no
semantic EAX result established. There are seven direct call sites (six unique
callees) and one indirect current-scalar call. This destructor does not return
the widget to its pool or free the widget allocation.

| Site | Capture, call or store |
| --- | --- |
| AAFF0F | Stamp native profile D5C270; identity metadata, never host callable code |
| AAFF15..1F | Capture CURRENT F0 shared pointer, then establish native state3 |
| AAFF29..32 | If nonnull, capture its current slot8, call with flags1, then clear F0 only after return |
| AAFF38..46 | Capture CURRENT124 first, select header120, demote to state2 |
| AAFF51..56 | For nonnull first only, capture CURRENT128 end and call AAF120 over that range |
| AAFF5B..6D | Reread CURRENT124 for BF65AC, then zero124/128/12C |
| AAFF70..7E | Capture CURRENT110 first, select header10C, demote to state1 |
| AAFF89..8E | For nonnull first only, capture CURRENT114 end and call AAF0E0 |
| AAFF93..A5 | Reread CURRENT110 for BF65AC, then zero110/114/118 |
| AAFFA8..B0 | Capture CURRENT100 data, then demote to state0 |
| AAFFB6..CA | If data is nonnull, read CURRENT FC length, wrap length+1, call419CC0 then BD1510 |
| AAFFCF..D9 | Set receiver back to Movie, demote to state-1, call real AA9730 |

The F0 clear deliberately overwrites a replacement made during the scalar
callback. It is conditional on the original captured pointer being nonnull;
there is no final repair or second deletion. Subsequent vector/string callbacks
may change F0 again, and the destructor does not revisit it.

Range destruction uses captured first/end, but backing free uses the **current**
first after those callbacks. Do not free the cached range start instead, retry
a replaced vector, or cache the second vector/name at entry. A null initial
first skips both its range and free, still clearing all three pointer fields.
The opaque allocator words at10C/120 remain untouched. MovieName FC/100 is
released without clearing its header, including callback replacements. Base
retirement subsequently changes profiles/fields in its own native schedule.

## Record contracts and actual string domain

AAF0E0 and AAF120 are each56 bytes/24 starts. Original ECX=first, EDX=end,
two ignored stacked pointer/tag slots, RET8. They capture the end in EDI and
walk first forward by14h or24h respectively until pointer equality. Each step:

1. Capture the record's data pointer at+4; null skips the length read and pool.
2. Otherwise read current length at+0, compute DWORD-wrapped length+1 and call
   419CC0 before BD1510 with that captured buffer/size and the native flag1.
3. Advance by the fixed stride. Do not clear the record header or reread end.

Only the first8-byte NativeString header is destroyed. In particular, AAF120
does **not** scalar-delete the extratitle pointer at record+18. AB00C0's separate
extratitle-clearing logic must not be invented inside this destructor. Any
later child destruction belongs to the actual base/ownership graph.

Genuine `destroy_native_string_header_0041dd20(header,
NativeStringRawPoolContext&)` already implements precisely the data/length/
wrapped-size/getter/return/header-preservation contract. The context borrows
the SAME01090AA8 publication,01090AA4 live return gate and01090AA0 manager.
`GameNativeStringProcess::raw_context()` and `GameVfsHost::raw_strings()` are
actual existing suppliers. The getter runs for every nonnull record, including
large frees and disabled small returns. A cached pool, semantic SizedStoragePool,
`crt_string_storage`, C++ string destructor or fabricated header is not a substitute.

The raw overload permits getter exceptions to escape into the original outer
cleanup scope. The `NativeStringStorage` interface's release is noexcept and
its actual-pool bridge has its own terminating getter-failure qualification;
that interface is not silently interchangeable for exception transport.

No Source implementation of the whole AAF0E0/AAF120 contracts was found.
`GuiMovieFields` instead owns semantic C++ strings/byte vectors, and its existing
destructor calls clear/shrink_to_fit. The semantic MovieSubtitle rows and
`GuiMovieSubtitleHost` insertion/erase requirements are not genuine native
14h/24h record backing or its allocator/lifetime owner.

## Actual partial-unwind map

Handler CB7872 loads FuncInfo DEE388 and jumps to the existing vendor handler;
its internals were not inspected or reconstructed. FuncInfo has magic19930522,
four unwind states, mapDEE368, no try map. The actual map/actions are:

| Current state | Next state | Action | Actual target |
| --- | --- | --- | --- |
| 3 | 2 | CB7864, saved receiver+120 | AAF7A0 extratitle vector tidy |
| 2 | 1 | CB7856, saved receiver+10C | AAF720 subtitle vector tidy |
| 1 | 0 | CB7848, saved receiver+FC | 41DD20 native string destruction |
| 0 | -1 | CB7840, saved receiver | AA9730 raw base destruction |

AAF720 and AAF7A0 are each61 bytes/21 starts, ECX actual header, plain RET.
They capture current first, load end only when first is nonnull, invoke the
matching range helper, reread current first for BF65AC, then clear header+4/+8/+C.
They do not clear the opaque header word or delete non-string record payloads.
These complete vector-tidy Source compositions are also absent.

The normal body demotes **before** entering each cleanup. Failure of F0 scalar
at state3 therefore reaches current extratitle/subtitle/name/base cleanup, but
failure during the normal24h stage at state2 does not retry that partially
destroyed stage. A14h-stage failure reaches name/base only; a name failure
reaches base only; a base failure has no outer cleanup action. Never implement
blanket reverse RAII, parent rollback, a second decoder deletion or replay of
an already demoted range. A partially consumed vector can retain backing and
dangling element headers even after later cleanup; retain all reached provider
and observation metadata independently until explicit failure disposition.
The map establishes actions/order, not native FH3/SEH/fault or cross-ABI C++
exception equivalence, nor a claim that a second cleanup exception can return.

## Scalar, pool and base custody

AB0480..AB049F is the32-byte Movie scalar entry: capture widget, call AAFEF0,
then inspect flags bit0; only after successful destruction return the same
captured slot through F8BD4C/AAD1C0. EAX returns captured widget bits, possibly
after pool return, RET4. Flags0 still destroys the payload and grants no second
destructor permission. Existing Movie pool traits and allocate/free services
match the130h payload/134h slot with slab index at130h. Do not use general
BF65AC to free this widget. Neither entry decrements the widget's+4 count;
flags1 is not a counted-release substitute. Its actual observers/companions
must follow native retirement rather than infer permission from a positive
count. Pool return is a separate future scalar admission.

F0 has a different allocation identity. For the known complete Bink decoder,
D24D98 slot8 is A4C820, the adjusting route into A4C880. The latter destroys
the shared state S and frees captured allocation A=S-10h for flags&1, returning
A. The full normal Source routines exist, but production `MovieDecoderHost`/
`MovieBaseHost`/actual decoder owner and module/texture/string custody do not.
Do not free S, assume every current F0 is this class, or call numeric table words.

Real raw AA9730 already exists, with its native current child/parent/timed/list,
node, name-pool and failure contracts. Its own D5C130 profile uses the actual
base table directly; absence of a Movie derived table alone is not a reason to
substitute or reject that base query. Nonnull4C needs a genuine canonical actual
GeneratedModelNodeLifetime binding, or its separately qualified raw Model
transport. That transport's existing Text/raw118 authentication is not general
Movie admission. Reached children/timed entries still need their real current
scalar, class-query, scene, parent-list self-detachment and observer/zero routes.
The closed production widget dispatcher currently recognizes Group/ClipBox/
Text/Icon, not arbitrary Movie descendants. Empty fresh fields do not prove
the whole populated schedule. Keep every actual node/model/list/observer and
borrower provider alive through the last callback and any partial failure.
The ordinary base-context overload has its established states2/1/0 C++ cleanup;
the separately qualified raw Model overload instead poisons/retains on escape
without base/container unwind. Do not promote that successful-call transport
into original base failure-cleanup equivalence.

## Candidate status and smallest next packet

Constructor54fb69ac5e00304636dcb6609034b8026f50802d and independent review
2d9d919e035f0e21aaeeb31a6b777ebd68104461 are explicit private inputs. Neither
is in this frozen base or promoted here. They establish only qualified raw
default construction and its reviewed store contract, not destruction/caller
admission. Current root/subtitle/child-Page guards are all false and untouched.

Propose the prerequisite **raw Movie record-string lifetime contract**, only
AAF0E0 and AAF120, with new `include/bsp/native_gui_movie_record_lifetime.hpp`,
`src/native_gui_movie_record_lifetime.cpp`, and matching
`docs/CC16_NATIVE_GUI_MOVIE_RECORD_LIFETIME.md` /
`reports/cc16_native_gui_movie_record_lifetime.json`, after fresh leases.
Require actual first/end and the genuine `NativeStringRawPoolContext&`;
compose the existing raw41DD20 at each stride. Use no semantic record class,
allocator, fallback callback, header clear, extra-object deletion or local
rollback. Captured end/forward DWORD arithmetic, empty behavior and partial
prefix failure must stay explicit. Original RET8/dead tags remain evidence,
not an invented private-stack or binary ABI promise. These are library-contract
compositions needed by the in-house destructor, with zero vendor/body credit.

Stop before vector backing free/tidy until the real matching raw-array allocation
and return domain is explicit; before whole AAFEF0 until actual F0/current-scalar
and reached base/graph providers plus independent failure custody are supplied;
and before any scalar/pool or consumer adoption. Root module retention alone
does not retain future borrowed decoder/renderer/VFS/Lua/graph owners. No full
factory/loaded/property/Page consumer, playback, duplicate-body or activation
credit follows from these prerequisites.

Target-verified BSP wrappers checked the configured existing
C:/Users/sqz269/bsp.gpr and /battlestationspacific.exe, x86/image400000,
live/snapshot77631. Complete destructor/range/tidy/scalar assembly and bounded
live EH map/actions were read after pseudocode. The report pins native bytes,
Source/candidate evidence and direct/tail call rows. This is read-only static
readiness, not execution, independent whole-base proof or runtime acceptance.
