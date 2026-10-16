# Whole scene-property hash iterator pair (CC12)

Two independent MSVC Win32 naked fastcall Source bodies implement the actual
entries below. Their complete strict-compile COFF code equals Original
literally: all 157 bytes / 55 instructions, with no calls, relocations,
undefined helpers or additional code. Root Main registration and admission
remain separate. Names are descriptive hypotheses, not recovered symbols.

| Entry | Half-open range | Bytes / instructions | Original and COFF SHA256 |
| --- | --- | --- | --- |
| Begin008EEB10 | [008EEB10,008EEB51) | 65 / 22 | b9167301cb1875e13c9fd0c9075479b79b62210969d8a67cd5f3108c2aeaff2c |
| Next008EEB60 | [008EEB60,008EEBBC) | 92 / 33 | 3609759b0297a57d1d5234c1cad5899352502be03a0dbbcc6973a0768a7e91a5 |

ECX is actual aligned writable raw12 storage: table+0/current+4/index+8.
Incoming EDX is unused, no arguments are stacked and each body has two plain
RET exits. Begin has no common semantic EAX result, so its ordinary API is
void. Next returns full DWORD zero/one after rereading actual current.
Begin saves/restores EDI on its scan path; Next changes no callee-saved register.
The byte-identical other instances00466BB0/00466C00 are evidence only; these
new entries do not forward to or take address credit from those instances.

The caller-created genuine raw12 region requires initialization of table only.
Begin captures table, tests table+4 and, for zero count, clears current while
leaving index untouched. Otherwise it preserves the actual index stores,
repeated loads and captured-table head reads. Its bound-before-increment scan
can read head64. Preserve that load; never fabricate a sentinel from map+108,
which is owner metadata. Authentic coherent nonempty maps reach a real
nonnull head among the64 buckets.

Next initially reads current. An initially null current returns by rereading
that slot without touching table/index. Otherwise it reads node+C; a nonnull
link is published directly. A null link follows the exact unsigned DWORD index
increment/scan schedule, preserving the captured scan table and final reloaded
current table. Every exit rereads current for a full0/1 result. The literal
four-byte008EEB7C LEA ESP,[ESP] (8D642400) is emitted explicitly and verified
in the entire COFF body. No bounds policy or null-table guard is added.

Supply the genuine live raw61-produced114h property bag's borrowed108h map
at bag+4, or the accepted19Ch enum owner's map at owner+4. Retain actual
canonical process/list/pool/generation and owning-key/node lifetimes. Property
nodes are from E175B0; symbol nodes use distinct E17578; both retain E188B4
and allocator+10 while exposing next+C. No pool/list reset, substitute owner,
class, STL container, callback, constructor or lifetime action is introduced.
Require aligned valid reached DWORD ranges, coherent stable finite unshared
acyclic maps/chains, and iterator/map/nodes/implicit private frames disjoint.
Faults, aliasing, reentry, concurrency, mutation, trim and teardown are excluded.

One fresh own precompile capture pinned31 current producer inputs,5124
prospective compiler headers and100 tools, including all four installed clui
resource candidates. All10 actual sourceDependencies inputs were pre-pinned
and closed against that capture; the current inputs/tools remain unchanged
through postbuild. The worker31 files match sealed readiness exactly. Against
current Main,26 are literal equal; config/target.json and the enum-symbol
insertion/node-pool report JSON files differ only in LF/CRLF, with equal
normalized bytes and parsed JSON. These three primary qualifications remain.
Two Main worktree files, the enum-dictionary lookup header and implementation,
have concurrent staged/unstaged property-lookup edits outside this worker's
stable readiness/precompile/build basis. Their observed bytes are frozen,
with no equivalence or current Main producer readmission claim. These entries
have no CALL providers and include neither file. Root must assess current Main
producer contracts during integration.
No inherited compiler preflight or hermetic toolchain claim is used.

The single strict compile passed /W4 /WX, /O2 /Ob2 /Oy-, /EHsc, /Gy /GL-
and /fp:strict. Complete1073-byte COFF accounting includes both code sections,
all14 symbol slots (nine primary symbols), auxiliary records, directives,
CodeView OBJNAME/COMPILE3, opaque checksum metadata and full string table.
No extra executable helper or unexplained object bytes remain.

The single required normal scripts/build.ps1 invocation passed its configured
reconstructed_math and tool_tests checks (2/2). Its existing LNK4006 duplicate
spawn_request_id_matches warning remains. The new TU is unregistered, so this
normal build does not compile or execute the new bodies. No new test, probe,
body execution or compile/build replay was performed.

Evidence is under local/WholeScenePropertyHashIteratorSource01, sealed with
an exact-self-only readonly regular single-link family receipt including every
nested manifest, actual own tool objects and the own rollout prefix. Existing
sealed local families are preserved. The machine-readable report records exact
file digests, compile/build receipts, dependency closure and metadata audit.
No CMake, registry, ledger or Ghidra mutation is part of this packet.

Static byte equality and the bounded ordinary raw interface do not establish
Original full register/flags/private-frame/binary ABI, historical private CRT,
EH/fault/unwind, caller/key-list/string-list/sort closure, parser/loader,
startup, native runtime or game equivalence. These remain distinct gates.
