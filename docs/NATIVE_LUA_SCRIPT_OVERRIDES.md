# Native Lua script override candidates

Addresses: `00BDEF90`, `00BDEF80`, `004BCB80`, `004CDC20`, `00419CA0`.

This packet implements five complete routines over the existing actual eight-byte
`NativeString` and twelve-byte `NativeStringVectorStorage`. Names are descriptive
hypotheses, not recovered symbols. It supplies the previously required BDEF90
producer in `NATIVE_LUA_FILE_LOADING.md`; VFS manager/provider ownership remains a
separate contract. The C++ interfaces are not drop-in original ABI replacements.

| Address | Original ABI | Reconstructed behavior |
| --- | --- | --- |
| BDEF90 | ECX manager; stack path/output; RET 8 | Append existing script candidates in registered suffix order |
| BDEF80 | ECX manager; stack suffix; inherited RET 4 | Add 48h to receiver and tail-jump to string-vector append |
| 4BCB80 | ECX string; stack byte-set/signed start; RET 8 | Reverse search counted bytes for any byte in a C-string set |
| 4CDC20 | ECX vector; stack string; RET 4 | Grow only at count == capacity, construct copied entry, then increment live count |
| 419CA0 | ECX string; EAX pointer; RET | Return data or the external E17654 empty fallback |

## Producer and native ordering

BE1DC0 establishes the suffix header: BE1DD8 captures ECX in ESI, BE1DE4 zeroes
EBX, and BE1E54/57/5A store zero at manager+48/+4C/+50. BDEF80 and 4CDC20
establish the actual eight-byte element stride. Only these fields are accessed;
this packet does not invent the remaining manager layout.

BDEF90 searches backward for `/`, then searches forward from that position for
the first `.`. Backslash is not the separator here. A dotted filename splits at
its first dot following the last slash; a dot in an earlier directory component
does not split it. Counted bytes and C-string searches retain their different
semantics. The substring temporary is copied to the stem and destroyed before
the path's current data pointer is reread to construct the extension.

The suffix begin/end pointers are captured at BDF11C..BDF12E. For each suffix,
the routine builds `stem + "_" + suffix + extension` with three actual native
concatenations. It destroys the intermediate strings before calling the current
manager virtual method at +08 (BDF227). A nonzero AL accepts the current candidate
header, including any mutation by that method. Accepted candidates are appended;
prior output and duplicate suffixes are preserved. The manager's vtable is
reread for each candidate, while the captured suffix range is not refreshed.

The underscore's normal release uses its captured pointer/length. Exceptional
cleanup reads current headers. FuncInfo E00A90 has ten states, with predecessors
`[-1,0,1,1,3,4,5,4,3,1]` and cleanup thunks CC6550..CC6580. The rebuilt function
preserves the temporary lifetime sequence with SEH finalizers. The two diagnostic
calls target 4254B0, verified as a single RET; the rebuilt producer omits them.

4CDC20 doubles the capacity with signed 32-bit wrap and clamps the request to at
least one, only when count equals capacity. It zeroes the destination before
copying, so aliasing the source to that destination abandons the previous buffer
and copies the newly empty header. Count is incremented after the copy. Its one
unwind state computes the pending destination and calls 401130, a bare RET;
there is no allocated-copy rollback. The existing reserve/resize/destructor
implementations remain the actual vector dependency.

## Validation and boundaries

The strict MSVC Win32 build and both existing CTests pass. One ignored differential
fixture executes the five complete original routines and relocates forty prior
dependency bodies (only reached dependencies execute), using Lua 5.1.1 and the established
native string pool. It checks 150 reverse-search combinations plus embedded NUL,
fallback pointer identity, vector aliasing/growth, nine path shapes, duplicate
suffixes, prior output, live candidate/vtable changes, and captured-range behavior.

The original and rebuilt complete DoFile/file/override sequence agrees on nested
duplicate execution (`MCppPP`). Both also load the installed debugshader,
alphablend, and dx9 include bytes through explicit fixture streams. A rebuilt
existence exception releases all temporary strings. Eleven original EH states
are checked as metadata; original exception-handler execution is not claimed.

The original fixture binds 469840 and 4261A0 to previously reconstructed helpers,
stock CRT/Lua through ABI adapters, and callable original-ABI VFS existence and
stream services. The required actual manager +08 method remains external; this
does not validate native physical/archive VFS routing, arbitrary callback
mutation, malformed lengths, binary replacement ABI, or game rendering/gameplay.
See `reports/native_lua_script_overrides.json` for call-site rows, PE spans,
annotation preservation, source/artifact hashes and tested commit.

## Follow-up packets

Compose the actual VFS open/existence owner and stream dispatcher with these
consumers, then connect the native fundamentals cache and shader descriptor
bootstrap. Preserve the callable-vtable boundary until numeric native identities
are resolved through verified methods and owners.
