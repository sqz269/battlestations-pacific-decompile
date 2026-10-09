# Damageable-class Comment field fragment

Addresses: `0087CBCF..0087CC65`, inside `0087CA80..0087D725`.
`0087CBCF` is the next instruction after Mesh cleanup and prepares the Comment
lookup. It is not a callable Native entrypoint. Names are descriptive hypotheses.

The new ordinary MSVC Win32 C++ fragment reads Comment into the actual descriptor
NativeString at +58h/+5Ch. It borrows the actual Lua row, current owner, still-live
Unique, parent scratch and raw string-pool publications. It does not implement
the entire damageable-class reader, a factory, a registry, model loading or an
application receiver. Worker compilation is separate from Root admission.

## Parent and selected evidence

This packet reuses the retained complete parent/FH3 audit in
`reports/native_damageable_class_lua_orch4.json`, SHA-256
`ff0e9d14717ddfd33456b3dced444d221a6e4164f79618a23fce07865383e31e`.
It does not reopen Native child bodies, unrelated data or handlers. Fresh live
`bsp.py ghidra proto` and `ghidra disasm` queries verified the configured `bsp`
project at `C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`, x86 and
image base 00400000 through the existing client. No Ghidra changes were made.

Independent installed-PE decoding reconfirmed all 3,238 bytes and all 858
instructions of the parent. Its SHA-256 remains
`00d679920bd1b8f8653e9a0ab1de627f954cfadaf48b8bf0642676fde5162808`.
Every PE instruction start equals the complete saved and fresh live listing;
the full files were compared, not just their displayed excerpts. The exact
Comment block is 151 bytes and 46 instructions, SHA-256
`b6dbe45d4a05c9602df91ed9a1cb954acdad71724a3c49ffc23fcf519505d9cd`.
The report retains all selected instructions and all seven direct calls. The
normal successor `0087CC66` begins HP setup and is outside this packet.

The original parent takes the descriptor in ECX, the live LuaObject row on its
stack, and returns with RET 4. Let S be ESP after its E4h locals and four saved
registers. At CBCF, EBX is the actual descriptor and EBP the live row. Unique at
S+94h remains live under parent state 1. The old Mesh field at S+44h is dead;
its stale bytes need not be zero. The old temporary string at S+10h is also dead
although its released header bytes remain. Comment reconstructs both scratch
objects and does not repeat the Name, Unique or Mesh operations.

| Site | Comment operation |
| --- | --- |
| CBCF..CBD9 | push `Comment`, address S+44h, and use the actual row |
| CBDB | B67800 lookup constructs/registers the actual field scratch |
| CBEC | enter state 4 after successful lookup |
| CBF4 | B685C0 exact STRING or empty fallback into actual S+10h header |
| CC00 | enter state 5 after successful string construction |
| CBFE/CC08 | retain destination/source identity test |
| CC11 | resize descriptor+58h from current temporary length, preserve=1 |
| CC16..CC26 | reread source length; if nonzero copy current destination length from current source data to current destination data |
| CC34 | lower to state 4 before temporary cleanup provider calls |
| CC49/CC50 | current raw pool getter and BD1510 return for nonnull data, length+1 with DWORD wrap |
| CC59/CC61 | lower to state 1, then destroy the real tracked field object |

The parent still owns Unique on normal exit and on propagated C++ failure. It
destroys Unique much later at D706 or during its own unwind. This fragment has
no Unique destructor call and takes no copy of Unique or its current index.

## Existing providers and lifetime

Lookup uses the real `native_lua_get_by_name_protected` adapter and B67800 body.
The same-Lua-frame operation preserves actual table/owner/index usage, registers
the actual output address only on success, and restores entry stack height before
throwing `NativeLuaOperationError` on Lua failure. Existing owner/index stability,
tracking capacity and inherited error-handler position requirements still apply.
This is the ordinary C++ transport; Native Lua longjmp identity is held.

B685C0 is composed from the existing exact kind2/LUA_TSTRING predicate and B662B0
getter, followed by the concrete raw 41E870 constructor. The older overload's
NativeStringStorage noexcept-release boundary is not introduced. Non-string
values use the original empty fallback; there is no numeric conversion. The PE
constants were independently checked: CE4E54 is `436f6d6d656e7400` and CE3A0C
begins with `00`. The constructor scans a C string, so embedded NUL truncates it.

The raw constructor overwrites both temporary header words before scanning.
Assignment resizes the actual descriptor header and then reloads its current
length/data and the current source fields. No semantic descriptor cast or copied
header is used. BF7680 is represented by the existing host memmove boundary,
including overlap support and the defined-C++ omission of a zero-byte call.
Current pool publications 01090AA8, 01090AA4 and 01090AA0 are borrowed directly;
the getter is resolved for every relevant allocation/return, never cached here.
Temporary string destruction leaves its stale header bytes unchanged. Actual
Lua destruction clears current kind and preserves its provider's index-shifting
and remaining-byte behavior.

Descriptor, row, Unique, the aligned 14h field scratch, aligned 8h string scratch
and publication cells must not overlap. Addresses and the scratch pointer members
remain stable. Unique stays registered in the actual owner of this parent
invocation. The scratch objects have no live predecessor on entry, but need not
be zeroed. No copied rows/indices/cells, default providers, replay or rollback
is supplied by the interface.

Retained FH3 edges are state 5 -> 4 via C96934/41DD20, state 4 -> 1 via
C96929/B67700, and parent state 1 -> -1 via C9690B/B67700. The new guard owns
only states 4 and 5 and lowers state before each cleanup. Lookup failure owns
neither temporary. Construction failure cleans only the field; it does not add
cleanup for an incompletely constructed string. Resize/copy failure cleans the
completed string then field. Failure during normal string cleanup still cleans
the field, and field-cleanup failure is not retried. Descriptor changes survive.
Secondary failure during guard unwinding meets the ordinary noexcept-destructor
termination boundary. Native stack/register/FH3 identity, fault-sensitive load/
state-store timing, SEH, longjmp and double exceptions remain held.

## Validation and admission boundary

The Comment object and four actual Lua/string/pool provider objects were freshly
compiled with the normal x86 Release `/O2 /Ob2 /MD /W4 /WX /fp:strict /EHsc
/std:c++17` flags. The candidate's complete indexed COFF contains 23 sections,
513 code bytes in nine sections, and 33 relocations. Every code byte, helper
COMDAT, cleanup method, cold/EH handler, unwind funclet and EH metadata was
reviewed. All seven project externals resolve to actual freshly compiled provider
definitions. Indexed provider code/EH/rdata was also compared to the already
reviewed predecessor evidence, preserving physical section and symbol indices;
only anonymous namespace spelling was normalized.

The new branch starts at origin/main `38700ffd4`. Its delta from the previous
baseline revision is six docs/reports only. All 1,967 registered core source files
were hash-compared against the existing baseline build. The unchanged baseline's
normal `scripts/build.ps1` was rerun, including both existing tests, which passed
in 8.66 seconds. This reuses build products only for identical registered inputs;
Comment remains unregistered and is not admitted by that baseline result.

No new test, candidate execution, Native differential comparison, application
receiver, startup or gameplay validation is claimed. Root must register and
validate the reviewed fragment. Whole-parent Source and Native admission stay
held. Source 534 and earlier startup observations are historical context; this
packet adds no whole-function Source count and does not inherit runtime credit.
