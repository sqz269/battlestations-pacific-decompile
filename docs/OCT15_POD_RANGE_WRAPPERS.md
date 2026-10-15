# Native POD range dispatch and fill

Addresses: `0041C4D0`, `0041C580`. Packet `oct15_pod_range_wrappers`.
Existing library-role names are retained:
`STL_Vector12_MoveForwardDispatch_0041C4D0`, `STL_Pod8Range_Fill_0041C580`.
These descriptive names are not recovered source symbols. Both normal
admitted-range bodies are complete; malformed ranges are not repaired.

| Address | Body inclusive | Machine ABI / return | Coverage |
| --- | --- | --- | --- |
| `0041C4D0` | `0041C4D0..0041C4FA` | cdecl first,last,destination; plain RET; EAX from actual move kernel | complete normal admitted ranges |
| `0041C580` | `0041C580..0041C5A2` | cdecl first,last,value; plain RET; EAX final cursor | complete normal admitted ranges |

The original C++ return types are not recovered. The new typed entries expose
the verified EAX machine postconditions rather than copying pseudocode's void
signatures. No C++ pointer subtraction, relational comparison or out-of-array
pointer arithmetic implements either native schedule: raw x86 registers do so.
Ranges must reach their equality endpoint by the native stride, with every
executed source read and destination write admitted in real storage. Empty
ranges require no pointed-storage access. No accessor, pointer host, allocation
callback or generic repair policy is introduced.

Live queries verified project `bsp`, configured at `C:/Users/sqz269/bsp.gpr`,
program `/battlestationspacific.exe`, language and image base. Both complete
pseudocode/listings were read; 17/13 listed instructions have no gaps and reach
their explicit returns. Raw installed `.text` independently confirmed the fill
instruction schedule. Ghidra was not mutated; prior comments were saved in
`local/oct15_pod_range_wrappers_previous_comments.json`.

`0041C4D0` forwards six stack words to accepted actual `00419330`: first,last,
destination,last,last,tag. `0041C4D0` saves incoming ECX as a local word and
`0041C4D9` zeroes only its low BYTE. Its upper 24 bits retain that register's
preimage, rather than becoming a fully zero word. The accepted kernel ignores
all three metadata words, so this qualification does not invent a new input.
`0041C4F7 ADD ESP,1Ch` removes six outgoing words and the local word; plain RET
leaves the caller's three arguments for caller cleanup. EAX remains untouched.

The complete accepted `move_kernel` in `native_game_contact_reports.cpp` was
read, including signed truncating divide-by-12 destination-end computation and
its forward three-word x87 overlap schedule. It has no public callable service.
With explicit primary authorization and expanded file leases, this packet adds
`move_native_vector12_00419330`: a typed six-word cdecl public tail entry. It
compiles to one JMP to that exact existing kernel, whose body is unchanged.
The dispatcher calls that service. There is no copied move loop, substitute,
callback, new allocator or alternative context.

`0041C580` is NOT a raw two-DWORD bit-copy loop. Each iteration loads value[0]
with FLD, advances EAX by 8, compares it to last, stores destination[0] through
FSTP, THEN rereads value[1] and stores destination[1] through x87. The final JNZ
uses the earlier integer comparison flags. Source words are reread every
iteration; no whole value pair is cached. For example, if value[1] aliases the
first destination word, that first store changes the subsequent second read.
This is preserved for every admitted alias arrangement. Finite representation
copies follow the actual instructions; signaling NaNs can quiet and x87 status
can change. A whole-record memcpy or preloaded pair would change behavior.
The empty branch returns EAX=first=last before even loading the value pointer;
nonempty admitted execution ends with EAX=last. Native 32-bit ADD/equality
behavior, including the lack of malformed-range guards, is retained.

The initial index attributed the fill caller to `Catch_All@00420CCF`. Live xrefs
instead identify sites `00420D3F` and `00420D8E` in `STL_inst_00420b60`; live
function-containment queries corroborate both. The separate catch body ends
`00420CE3` and only frees/rethrows. Its context was read, but no catch repair,
new catch contract or mutation ownership was introduced. These caller claims
are separate from the one owned dispatcher call row in the report.

Static verification used actual build COFF, not a separately compiled probe.
All 35 fill bytes match installed PE `.text`. All 39 dispatcher nonrelocation
bytes match; only the four relative CALL-displacement bytes at body offset
23h..26h are masked. The E8 opcode at 22h is compared, and total body length is
43 bytes. The new service is a single E9 rel32 tail-JMP to `move_kernel`.
The tracked JSON report carries extraction, mask provenance and both byte
strings; local artifacts are `oct15_pod_range_wrappers_coff.asm`,
`oct15_pod_range_wrappers_contact_coff.asm`, and the byte-audit script/JSON under
`local/`. This is static instruction evidence, not native execution.

Validation: full warm-cache `./scripts/build.ps1` passed all three existing
suites (`reconstructed_math`, `native_math_differential`, `tool_tests`), 3/3,
in 7.06 test seconds. Log: `local/oct15_pod_range_wrappers_build.log`. No new
routine test suite or runtime fixture was added; existing seeds do not establish
packet-native differential coverage. Post-build header edits were comments
only. The packet does not claim original FP trap/exception ABI, installed binary
replacement, game integration or gameplay equivalence.
