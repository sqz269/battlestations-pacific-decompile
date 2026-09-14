# Native FileStore request and pending insertion

Addresses: `00BE7CD0`, `00BD9E30`, `00BE4DB0`, `00BE5530`, `00BE5630`,
`00BE5750`, `00BE5F00`, `00BE6120`, `00BE62E0`, `00BE63E0`, `00BE6630`,
`00BE6EE0`, `00BE7460`, `00BE79C0`.

`request_native_file_store_00be7cd0` implements the complete normal and
native-false request control flow against actual FileStore storage. The
resident tree is at provider `+14h`, pending tree at `+20h`; each tree has
head/count at `+4/+8`. Pending nodes are the actual `1Ch` allocation with
left/parent/right `+0/+4/+8`, length/data/callback `+0C/+10/+14`, color/nil
bytes `+18/+19`. Existing native search, pending erase, rotations, string
operations and singleton allocation services are reused. No projected
`FileStoreRequests`, `VfsMountContext`, callback list, or shadow map is used.

Source: [header](../include/bsp/native_filestore_request.hpp),
[implementation](../src/native_filestore_request.cpp). Exact span and artifact
hashes, call instructions and ABI details are in
[the report](../reports/native_filestore_request_bk.json).

## Behavior and integration contract

1. Copy and normalize the original eight-byte header through `BEE780`.
   Read the current `0109CEEC` publication and resolve the mutable local
   header through `BDF4C0`. A false result can leave this local name changed;
   destroy it and return false without touching either map.
2. Search the resident tree with the resolved name. A hit returns true,
   without submitting, storing, or invoking this request's callback.
3. Search the pending tree with the same resolved name. A hit has the same
   early true result. It preserves the callback already stored at node `+14`.
4. Construct two owning temporary pairs, then insert their key/callback
   into the actual pending tree. Insertion performs its own duplicate check;
   RequestFile ignores the inserted byte, as the original does. Destroy
   the second temporary before the first, before provider submission.
5. Reload current `0109CEEC`; submit `(resolved, original, BE7B20, 2)` through
   `BDDA10`. The public native API has no flags parameter. A true submission
   returns true after destroying the local header.
6. A false submission erases the pending equal range through `BE79C0`,
   reloads current `0109CEEC`, calls `BD9E30(-1)`, destroys the local header,
   and returns false if that callback returns. A throwing submission has
   no pending erase in the native unwind map.

`NativeFileStoreRequestContext` borrows strings, returning CRT validation,
the actual publication cell, and `NativeFileStoreRequestDispatch`. Dispatch
has three explicit operations: mutable-name `BDF4C0`, four-argument `BDDA10`,
and invocation of the current manager `+90h` target. Each receives the
captured manager for that specific call. Replacing the publication between
resolution, submission and rejection handling remains observable.

The two shared pending helpers used by completion are:

```cpp
find_native_file_store_pending_name_00be5f00(tree, output, name, callbacks);
erase_native_file_store_pending_name_00be79c0(tree, name, strings, callbacks);
```

The callback DWORD supplied by the caller is copied verbatim; no native or
host code invocation occurs in RequestFile itself. The submission callback
identity is always `00BE7B20`. That separate adapter must acquire the
**current** factory and load factory `+8` for completion. It must not capture
the originating store. Root integration owns that adapter and `BE78B0` /
`BE7760` completion/population. Accepted async dispatch must own its native
copies of both name headers before these request temporaries are released.

## Stable request frame and exception boundaries

The caller publishes a `NativeFileStoreRequestAcquired` **before** entering
RequestFile, and passes it as the final argument. Its resolved name, both
temporary pairs, iterator and insertion output remain at stable addresses.
This is necessary because an interrupted `NativeVfsNameResolutionAcquired`
can retain its caller's mutable header. The borrowed original header, store,
context and any nested acquired frames must also remain alive.

Normal returns mark the acquired frame complete and perform the native
cleanup schedule. A source exception records its native call site and
marks the frame failed. The owner must retain it for diagnosis; replay is
rejected and destruction of an incomplete or failed frame terminates.
There is no guessed recovery/cleanup operation for an interrupted provider.
This is an explicit source failure transport, not original FH3/SEH behavior.

The original request map at `E0175C` has states 0/1/2 whose actions are
`CC6E20 -> 41DD20` (resolved), `CC6E28 -> BE5E50` (first pair), and
`CC6E30 -> BE5E70` (second pair). Both pair destructors only release the
current key header. There is no pending-key cleanup action. The retained
source frame does not claim to execute these native unwind actions while
an interrupted nested frame may still borrow its storage.

`BE6630` has normal body `BE6630..BE66A1`. Its catch-all begins at `BE66A2`:
free the captured node, then rethrow at `BE66B2`, logical inclusive end
`BE66B6`. Ghidra currently stops that catch function at `BE66AA`, leaving
`BE66AB..BE66B6` outside any function. The source implements the complete
free/rethrow sequence. Placement unwind `CC6CE0..CC6CF0` calls RET-only
`401130`, so no key destructor is added for failed node construction.

`BE6EE0` tests unsigned count `>=15555554h`. The length-error path builds
the 19-byte message through `408720`, uses the existing owning native
length-error transport, and destroys its completed message via `4072D0`.
Its native state action is `CC6D60..CC6D67`, descriptor `E01660`.

## Covered bodies and machine interfaces

| Entry and inclusive end | Native interface | Coverage |
|---|---|---|
| `BD9E30..BD9E3A` | ECX manager; discarded stack DWORD; RET4 | complete |
| `BE4DB0..BE4E38` | ECX iterator; RET or invalid-parameter tail | complete |
| `BE5530..BE5582` | ECX tree; name stack; EAX lower bound; RET4 | complete |
| `BE5630..BE5683` | ECX tree; name stack; EAX upper bound; RET4 | complete |
| `BE5750..BE5788` | six stack DWORDs; cdecl RET; final argument ignored | complete |
| `BE5F00..BE5F65` | ECX tree; output/name stack; EAX output; RET8 | complete |
| `BE6120..BE616A` | ECX pair; name/callback-word-pointer stack; RET8 | complete |
| `BE62E0..BE6327` | ECX pair; source pair stack; RET4 | complete |
| `BE63E0..BE644F` | ECX node; left/parent/right/pair/color stack; RET14h | complete |
| `BE6630..BE66A1` plus catch described above | five stack DWORDs; EAX node; RET14h | complete |
| `BE6EE0..BE70CB` | ECX tree; output/left/parent/pair stack; RET10h | complete |
| `BE7460..BE7573` | ECX tree; output/pair stack; RET8 | complete |
| `BE79C0..BE7A2A` | ECX pending tree; name stack; EAX erased count; RET4 | complete |
| `BE7CD0..BE7F6F` | ECX actual store; name/callback stack; AL; RET8 | complete normal/native-false control flow; explicit retained source exception boundary |

Pending ordering is the existing CRT case-insensitive comparison with stored
length zero treated as empty. No length tie-break is introduced. Empty and
case variants follow the native comparator. Insertion publishes node, then
inserted byte, then owner; padding bytes stay untouched. It checks the
current tree count before allocation, reloads/increments it afterwards, and
reuses the existing rotations.
Erase counts a checked iterator range first, then invokes the existing
range eraser. `BE7A0B ADD ESP,18h` establishes all six distance arguments.

The failure wrapper needed a specific correction: `BD9E30` is
`MOV EAX,[ECX+90]; CALL EAX; RET4`. It does **not** push or forward `-1`.
ECX is still the manager at the indirect call. The source dispatch receives
the observed target and manager, with no invented callback error argument.

## Verification and remaining work

All 14 complete normal-body spans match both live Ghidra and the installed
PE. Every live batch verified `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`; Ghidra remained read-only. Names in this packet
are hypotheses and annotation proposals, not recovered symbols. No project
annotation or save was performed.

Strict MSVC Win32 compilation passed. The single ignored actual-storage
fixture passed normalized mutable resolution, original-name preservation,
resident/pending precedence, first-callback preservation, ordinary rejection
rollback, current-manager reload, 13-node red-black invariants and zero
string allocations left after normal teardown. An intentionally retained
interrupted resolver frame preserved its actual header and `BE7D1C` failure
site through process exit. It linked the primary's exact `bsp_core.lib` and
`bsp_zlib121.lib`; their hashes and the fixture hashes are recorded.

The report's call rows pass `tools/verify_report_calls.py`; indirect manager
`+90h` is separately listing-verified. Primary integration must add the TU
to CMake and run `scripts/build.ps1`. This worker did not change CMake,
execute original instructions, run the game, or modify the installation.
Original callable ABI, native exception identity, hardware faults,
concurrent mutation and arbitrary stack aliasing remain unproved.
