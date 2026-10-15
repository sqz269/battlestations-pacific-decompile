# Scene integer-set rotations

Addresses: 00467740, 00467790.

The packet reconstructs both complete rotations over actual 14h-node storage.
The established native library names are retained. Left/right are hypotheses
established by the promoted link, producer and every observed caller.

| Native address | Source API | Executed span | Coverage |
|---|---|---|---|
| 00467740 | rotate_left_native_scene_integer_set_00467740 | 00467740..0046778D, 78 bytes | complete |
| 00467790 | rotate_right_native_scene_integer_set_00467790 | 00467790..004677E1, 82 bytes | complete |

Native ECX holds the tree, incoming EDX is unused, and the pivot is the sole
stack argument; every return is RET4. Source uses fastcall with explicit unused
EDX. Native EAX retains the promoted node, but all nine observed callers ignore
or overwrite it. No semantic result or unsupported live prototype repair is
introduced.

Producer 004681B0 allocates 14h bytes and writes left+0, parent+4, right+8,
key word+0Ch, color byte+10h and nil byte+11h. The rotations use no key or color.
The full original bodies differ from accepted record-map rotations only in the
nil-displacement byte at offset 17: 19h becomes 11h. These are separate Source
bodies; they do not invoke the record-map providers.

The left body captures pivot.right, copies promoted.left to pivot.right, then
reloads promoted.left before testing child.nil11 and optionally repairing its
parent. It reads current pivot.parent and publishes promoted.parent before
reading current tree.head and head.parent. The root path writes the captured
head.parent. The other path rereads pivot.parent and chooses its left slot iff
that current slot equals pivot, otherwise its right slot. It finally writes
promoted.left=pivot before pivot.parent=promoted. The right body mirrors link
offsets and compares the current parent.right slot. Volatile raw accesses retain
all these external reloads and writes even when storage aliases.

Live xrefs contain four left and five right calls, all reviewed with argument
setup and following result use. The inserting caller is 0046A150; the erasing
caller is 0046A740. Exact site/callee/containing-function rows and original byte
strings are in the JSON report. Neither rotation has a callee or provider gap.
Both had names/tags but no indexed whole reconstruction or existing Source.

The expected live target is C:/Users/sqz269/bsp.gpr, program
/battlestationspacific.exe, x86 image base 00400000. The guarded CLI verifies it
before every live batch. Correct names and existing comments are preserved;
annotation records prior values, takes the shared write lock, saves the project
and refreshes both exports.

Validation at the Source draft: original bytes/layout, all nine caller shapes,
full assembly and static schedule reviewed; build and existing checks pending.
No new routine tests are added. Source completion and compilation do not prove
private-frame aliasing, original mangling/binary ABI, instruction fault or SEH
identity, direct native differential execution, application adoption or game
behavior. Every taken raw access still requires valid backing; there is no
added validation, normalization or rollback.

Final worker validation: `scripts/build.ps1` Release Win32 passed at Source
draft `6328c0e30`, and both enabled existing checks (`reconstructed_math`,
`tool_tests`) passed in 7.45 seconds. The native differential suite is
not enabled in this worker tree; no new entry was directly native-tested.
The log is `local/goal_oct15_scene_integer_set_rotations_build.log`; its hash is in the report.
The inherited duplicate `spawn_request_id_matches` LNK4006 warning remains.
JSON transfer claims, documentation address claims and staged whitespace
checks pass. Listing-gap queries do not expose typed flow/fallthrough
or no-return properties, and no live flow/prototype repair is claimed.
