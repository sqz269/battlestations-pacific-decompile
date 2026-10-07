# LandConvoy owning integer positions

Packet `cc11_scene_convoy_positions_int_payload`; Source baseline `b32d53af9`.
The single Position read in the existing Type1..4/Position1..4 loops now
prefers compatible I/i `integer_value` with `has_integer`. One local called
reader and one looped call replacement cover all 16 logical reads. The index
comment is corrected to positive position VALUE-1. Dimensions, scalar reads,
Reverse, Path, Type/enum resolution, defaults, loop order and slot reset remain
unchanged. No header, global conversion helper or tracked test changes occur.

The original fallback still scans `values.back()` for any type with unchanged
`scene_scan_int`, starting at -1. Missing/malformed/empty/bare-I and raw F/non-I
incidental numeric behavior stay Source compatibility. Source still fills all
slots for zero, skips negative values, and writes positive position-1 only when
position is within the slot count. Later writes still win. Neither native
missing-key fault semantics nor its unchecked upper index are newly reproduced.

## Native fragment and dependencies

Read-only target verified: `C:/Users/sqz269/bsp.gpr`,
`/battlestationspacific.exe`. Names are hypotheses. Spans below give the inclusive
last byte and separate exclusive end. Parent `00743450` has stored body
`00743450..00743B6F`, end `00743B70`, 522 listed instructions and terminal RET
at `00743B6F`. This is a bounded data adoption, not a whole attach/ABI port.

| Observed path | Inclusive bytes; exclusive end | Contract |
| --- | --- | --- |
| Position lookup/copy | `00743787..00743793`; `00743794`, 13 bytes | ECX gets the Type bag from [ESP+10h]; PUSH generated key at 0074378B; Find 0074378C -> 008F2260; MOV EDI,[EAX+0C] at 00743791, with no null/type check. |
| Fill/index/skip | `007437DD..00743826`; `00743827`, 74 bytes | TEST EDI at 007437DD/JNZ 007437DF. Zero uses product-positive fill loop store 00743800; negative skips through signed JLE 00743819; positive store 00743823 uses array[position-1], with no native upper guard. |
| Loop counters | `00743827..0074384E`; `0074384F`, 40 bytes | Position increments 0074382B/CMP4 0074382E/JLE 00743835; Type increments 0074383F/CMP4 00743842/JLE 00743849. Both visit numeric 1..4, independent of authored key order. |

The full supporting Position key/cleanup/decision loop is
`00743720..0074384E`, end `0074384F`, 303 bytes. Key Position at 00CE68B4
has nine bytes including NUL. Name-building calls are Resize 0041DD40 at
00743732, memcpy BF7680 at 0074374D and ConcatInt 004263B0 at 0074376E.
Temporary storage calls 00419CC0/00BD1510 at 007437AF/007437B6 and
007437D1/007437D8 are named dependencies, not newly reconstructed providers.

The supporting Type prefix selects kind6 at 007436A6 after TypeN Find
0074369D; absent/wrong kind selects null at 007436B1. Type Find 00743707
then unconditionally loads its +0C word at 0074370C. Type at 00CE4780 has
five bytes including NUL. Source skips an absent Type subbag and defaults an
unresolved enum to 0; these existing policies do not establish native graceful
failure or an enum ordinal provider. All relevant inspected setup direct calls
are attributed to this parent in the report; all 17 direct rows pass live
verification with zero failures. The three-byte alignment gap
0074371D..0074371F is unchanged. Prior root returning-free tail repair and
522-instruction parent receipt remain supporting history; prior group-merge
stored-tail qualification remains independent. No worker Ghidra mutation occurs.

Owning-I support remains
[SCENE_TYPED_INT_PAYLOAD_CC11.md](SCENE_TYPED_INT_PAYLOAD_CC11.md): explicit
I arm 008F5B48..008F5BDF/end008F5BE0; type0/+0C producer 008F3710,
constructor 008EF140, clone 008F4FF9..008F502C/end008F502D and assignment
008F0716..008F0725/end008F0726. Presence retains data, including zero and -1,
not native ephemeral read-success/+2Ch/replay metadata. The unchanged modern
MSVC `%d` Source provider admits one recognized nonempty explicit I with closed
semicolon, ordinary NUL-free C-locale in-range decimal-prefix token. Historical
BF7533 CRT, overflow/error/locale, implicit/declaration conflict, empty-existing
context, enum identity, native allocation/fault/EH and original ABI remain external.

## Actual connection and focused Source proof

The real class 1Ah SceneReader dispatch still merges library groups then authored
properties and calls the roster reader. Its public `GameSceneEntityRecord`
receives the actual slot/symbol vectors; existing enabled roster and formation
paths consume them. Whole public SceneReader, class/descriptor services, units,
Lua/VFS/emitter and native/game execution are compiled connections, not forced
through invented services by this fixture.

One ignored fixture extends the prior genuine parser/15-library/22-group/private
called production reader scenario. Installed `landconvoy.props` supplies all
16 explicit Position I records: Type1.Position1=0 at line 9 and other 15=-1
at lines 8/10/11/16..19/24..27/32..35. Independent owning child copies survive
parent metadata changes, clearing all diagnostics and opposite diagnostics;
the actual one-slot vector stays 0 with authored symbol us_ambulance. The library
capture remains unchanged. No enum table/ordinal is supplied or claimed: the
existing unresolved-Type0 Source path is retained.

Fresh compatible same-I assignments to Position1 check zero fill, positive 1/2
index, negative -1 skip, lowercase i2tail prefix and copied/cleared/opposite
diagnostics in a two-slot record. Later Type2.Position2 overwrites the second
slot's authored symbol to None, demonstrating actual Source order without a
fabricated ordinal. Raw ANY-type LAST, missing/malformed/empty/bare-I fallback-1
and out-of-range Source upper guard remain unchanged. Dimensions, five scalars,
Reverse, Path and Type resolution carry sentinels/regressions unchanged.

Native-linked numeric admission is two ordinary slots with zero/negative or
in-range positive positions; out-of-range/overflow/allocator and fault behavior
is excluded. The raw/missing/type cases are Source compatibility only. USN1
and JM06 parse and have no LandConvoy entities, so they are parser regressions,
not installed mission/gameplay evidence.

Strict fresh MSVC Win32 parser, changed standalone TU and probe-containing actual
TU compile/link/run exit 0/0/0. The PE014C probe embeds asInvoker with
`/MANIFEST:EMBED`. All 178 unique active production inputs (176 headers and
2 CPP) are stable; the 177-entry include inventory already contains the changed
consumer CPP and is not added again. The probe source gives 179 fixture-inclusive
Source inputs. All 17 installed and 77 pinned support hashes match pre/post.
Current a121efaa0 complete main build inputs were frozen as three libraries and
74 actual Game objects with original-pre/copy/original-post equality; prior
records supplied only filenames. Fresh active parser/SceneContents replace their
older inputs; game_main and old SceneContents are excluded. Uninvoked real
support establishes linkage, not execution. Recipes/hashes are in the ignored
manifest and JSON report. Root post-merge full main build/CTest and independent
verification remain pending; no whole native attach, ABI or game proof follows.
