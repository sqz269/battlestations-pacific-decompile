# LandConvoy owning Boolean Reverse

Packet `cc11_scene_convoy_reverse_bool_payload`; Source baseline `eb4c05ca2`.
Only `convoy_reverse` now calls one local reader that prefers compatible
case-insensitive B plus `has_boolean`, returning `boolean_value!=0`. Otherwise
it evaluates the exact old `scalar("Reverse",0.0f)!=0.0f` fallback. No other
reader, dimension/Position/scalar/Type/Path/slot/order/default/gate/class/Lua or
switch policy changes. Headers and global conversion helpers are unchanged.

The preserved scalar path first returns fallback 0 for missing/empty diagnostics.
Uppercase I tries the LAST token with `scene_scan_int` and converts a successful
integer to float. Next it tries `scene_scan_float` for any type. Finally uppercase
B accepts exact case-sensitive LAST `true`; anything else returns 0. This remains
Source compatibility: raw B TRUE and lowercase b true remain false, numeric
I/F/non-B tokens may be true, and LAST selection remains. Wrong-type owning-B
metadata is ignored; Reverse still ignores owning F/I cache values. The global
FIRST-token case-insensitive Boolean helper is not used or changed.

## Native byte boundary

Read-only target verified: `C:/Users/sqz269/bsp.gpr`,
`/battlestationspacific.exe`. Names remain hypotheses. Parent `00743450` has
stored body `00743450..00743B6F`, exclusive end `00743B70`, 522 listed
instructions and terminal RET at `00743B6F`. Only this 21-byte slice is adopted:
`007434A4..007434B8`, exclusive end `007434B9`.

| Instruction | Contract |
| --- | --- |
| 007434A4 PUSH 00CFF2F0 | Verified `Reverse` plus NUL, eight bytes. |
| 007434A9 MOV ECX,EAX | Uses the holder bag from preceding kind1 setup. |
| 007434AB CALL 008F2260 | Actual property lookup; current direct-call row passes live verification. |
| 007434B0 MOV AL,[EAX+0C] | Loads one BYTE without a null or type guard. |
| 007434B3 MOV [ESI+3A9],AL | Copies that byte unchanged to the convoy. |

There is no DWORD or numeric conversion in this path. Canonical recognized
native type3 true/false supplies byte 1/0, so the owning Boolean projection is
truthful within that domain. Untyped I/F or other record data can have different
low-byte meaning from Source's numeric comparison: word 0x100 has low byte 0,
while the legacy numeric Source path accepts nonzero 256. No native I/F-byte
parity, missing-key graceful recovery or whole-function ABI follows. Native
missing records are dereferenced; Source's false fallback is preserved policy.

Prior complete/partial supporting Boolean bodies remain in
[SCENE_TYPED_BOOL_PAYLOAD_CC11.md](SCENE_TYPED_BOOL_PAYLOAD_CC11.md): B arm
008F5D4F..008F5DDF/end008F5DE0; recognized true 1/false 0; fresh type3/+0C
producer 008F3940; constructor 008EF1F0..008EF220/end008EF221; clone byte arm
008F4F8D..008F4FC1/end008F4FC2; assignment 008F0736..008F0745/end008F0746.
Owning byte/presence retains data including false, not ephemeral read-success,
+2Ch or replay/default action. Admission is one recognized nonempty true/false
B literal with closed semicolon, within the previously admitted Source group
capture/copy domain. Empty/nonliteral/multitoken/implicit/conflicting declarations,
enum identity, native allocator/fault/EH/ownership/reentry/ABI remain external.
Prior convoy returning-free tail repair/522-instruction body and group-merge
stored-tail qualification are unchanged supporting history, not new worker work.

## Actual connection and focused Source fixture

Real class 1Ah SceneReader group/authored merge calls the private roster reader.
Its public record's Reverse value is copied by actual `GameUnitsHost` motion
binding to `c.formation.reverse` at source line 10971, then carried into existing
movement calls 11052/11056/11067. No new movement/math recovery or whole public
SceneReader/units execution is claimed.

The genuine ignored fixture extends the prior parser 15-library/22-group/called
production-reader scenario. `landconvoy.props` line 40 is `Reverse=B false`.
Its owning byte remains false after independent copy, parent overwrite, cleared
and opposite diagnostics; the original library capture remains false. Fresh
recognized B true/TRUE, b true and false/FALSE/b false also survive owning copy
and cleared/opposite diagnostics. Actual public record Reverse changes while
dimensions, Position/scalars, Path, Type resolver and slot/symbol sentinels stay
unchanged. No enum table/ordinal or default service is supplied.

The same focused scenario preserves no-own raw B LAST true/false/TRUE, lowercase
letter behavior, numeric I/F/non-B cases, malformed/missing/empty/bare-B fallback,
wrong-type owning flags and ignored F/I caches. These cases are Source-only
compatibility, not native typed/fault proof. The prior dimensions/Position
scenario remains a regression. Actual USN1/JM06 parse with zero LandConvoy;
there is no installed mission or gameplay witness for this change. No whole
host/descriptor/VFS/Lua/units/emitter scaffold or tracked tests are introduced.

Strict fresh MSVC Win32 parser, changed standalone TU and probe-containing actual
TU compile/link/run exit 0/0/0; PE014C embeds asInvoker with `/MANIFEST:EMBED`.
All 178 unique active production inputs (176 headers+2 CPP) are stable; 177
include entries already contain the consumer CPP, so it is counted once.
Adding probe source gives 179 fixture-inclusive Source inputs. All 17 installed
and 77 pinned support hashes match pre/post. Current 99bef9d8d complete main build
supplied three libraries and 74 actual Game objects, frozen original-pre/copy/
original-post equal; prior records supplied filenames only. Fresh parser and
SceneContents replace active paths, game_main/oldSceneContents are excluded.
Uninvoked real support establishes linkage, not execution. Recipes and hashes
are in the ignored manifest/report. Primary post-merge full main build/CTest
and independent verification remain pending; native ABI and original-game
validation remain unclaimed.
