# Actual scene-record map node leaves

This packet exposes complete ordinary Source for the constructor and string
cleanup below by reusing already accepted concrete raw providers. No existing
provider, legacy semantic cleanup method, or nominal key/value type is changed.

| Routine | Native range | Coverage | Original ABI | C++ symbol |
| --- | --- | --- | --- | --- |
| `00469F50` | `[00469F50,00469FC0)` | complete | ECX actual1Ch node; stack left,parent,right,pair,color; RET14; EAX original node | `construct_native_scene_record_map_node_00469f50` |
| `00468B30` | `[00468B30,00468B4D)` | complete | ECX actual8h string header; RET; no semantic result | `destroy_native_scene_record_map_string_header_00468b30` |

The full `00469F50` assembly agrees instruction-for-instruction with accepted
`00B1ACA0`, including argument capture order, pre-store self-header comparison,
all current storage reads, and the late color-byte read. `00502170` provides a
third matching native body; its profile-map Source is a broader semantic map
implementation with a host allocation boundary, so the complete raw `00B1ACA0`
entry point is the suitable canonical provider. Full installed PE comparison
normalizes only the two relocated call operands; every other byte agrees.

`NativeSceneRecordMapNodeArguments` aliases the existing five-cell binding view
`NativeResourceRegistryNodeArguments`. Bind initialized live DWORD cells for
left,parent,right,pair,color; keep view metadata disjoint from native payload,
argument and local cells. The constructor captures left/right/pair/parent before
stores, compares pair against node+Ch, then writes links at0/4/8 and zeroes the
string header atC/10. Unless the source header is that same address, it invokes
the real raw resize with preservation, rereads current source length, and on
nonzero source length reads destination length, source data, destination data
in that order for the copy. It then reads current pair+8, reads the current
color argument LOW byte, writes mapped14, color18 and nil19=0. Bytes1A/1B remain
untouched. No string object destructor, mapped-value lifetime, additional guard,
local cleanup or rollback is introduced.

The accepted copy provider preserves native backward overlap using host
`memmove`. It omits a standard-library copy when the current destination count
is zero; this existing host-language boundary is inherited rather than claiming
the native call executed in that case. The raw string pool getter/allocation
and current CRT boundaries also remain those of the accepted implementation.

`00468B30` matches the complete canonical `0041DD20` body. It reads data+4 first;
null skips the length read and every pool operation. Otherwise it captures
length+1 with DWORD wrap, resolves the current actual pool even for large or
disabled-small-return cases, and returns the captured buffer. It writes no
header fields, including after provider activity. Raw getter failures escape.
The prior `engine_seg4_6_9_cc13.cpp` semantic table remains separate.

| Containing function | Site | Native callee | Contract |
| --- | --- | --- | --- |
| `00469F50` | `00469F89` | `0041DD40` | Actual string header=node+Ch; current source length, preserve1; callee RET8. |
| `00469F50` | `00469F9F` | `00BF7680` | Destination/source/current destination length; caller ADD ESP,Ch at `00469FA4`; overlap allowed. |
| `00468B30` | `00468B40` | `00419CC0` | No native arguments; following return arguments remain stacked. |
| `00468B30` | `00468B47` | `00BD1510` | Captured block, wrapped byte size, unread1; callee RETCh. |

All these provider bodies and the two canonical raw APIs were inspected.
The single constructor caller is `0046A340` at `0046A398`: the allocated node
is in ECX and five freshly copied argument words are stacked. Destructor xrefs
are the tail transfers at `00C61BFB` and `00C61DF8` in their respective unwind
functions. Their distinct compiler cleanup schedules are outside these two
whole routine bodies; this packet does not claim those FH3/SEH paths in Source.

Live queries verify configured `C:/Users/sqz269/bsp.gpr` and
`/battlestationspacific.exe`. Full byte ranges, prior annotations and native
call-site checks are in `reports/goal_oct15_scene_map_node_leaves.json`.
Ghidra comments preserve prior values under the write lock, then the program
is saved and affected exports refreshed. Strict Win32 build and existing checks
are pending in the initial draft. No new tests, candidate-specific native
differential, original ABI compatibility or gameplay validation are claimed.
