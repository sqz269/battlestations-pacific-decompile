# Actual existing-name resolution

`resolve_native_vfs_existing_name_00bdf4c0` reconstructs the mutable native8h
name route used by the material/effect loaders. It captures the supplied actual
manager once and traverses that manager's current mount list, extension tree and
group list. The existing semantic `resource_lookup` and `vfs_candidates`
containers are not accepted as native storage.

The caller creates one `NativeVfsNameResolutionAcquired` before entering the
operation. Its private storage contains the actual temporary8h headers,14h
device visitor,18h log builder, iterator words and diagnostic call site. The
same input header is mutated. A frame is single use, including after failure;
there is no replay, rollback or replacement manager. A failed frame remains
retained by its caller (destruction terminates), even where native cleanup has
released all of its armed strings. The retained header bytes are diagnostics,
not permission to dereference released buffers or resume the operation.

`NativeVfsNameResolutionContext` borrows the same `NativeVfsDeviceRouteContext`,
its actual pooled-string storage, the live logging publication reference and
the native empty C-string fallback0109CEF0. Its lookup context's `device` link
must identify that same device context. The complete actual device route from
commit42d7a7be is required: `native_vfs_device_route.hpp/.cpp` and the matching
`native_vfs_lookup_routes.hpp/.cpp`. This packet does not duplicate BDBC70,
BDB670/BDB680 or provider current24. An unsupported provider current24 remains
the device context's explicit dispatch boundary. Candidate current08 supports
the verified BDD440 target; other current methods fail explicitly.

The primary's dependency review also identified earlier prerequisites7f688487
(`native_mpak_open.hpp/.cpp`, BB4B20) and7cde1c94
(`native_mpak_enumeration.hpp/.cpp`, BB40C0) in the updated lookup source.
42d7a7be is not a standalone dependency closure against0284fb3c. Their source
registration and any earlier closure remain the primary's integration review;
none of those files is copied into or reconstructed by this packet.

## Bodies and original ABI

All intervals below are inclusive. These are new source interfaces, not binary
ABI replacements and not native SEH entry points.

| Entry | End | Instructions | Original ABI and role |
| --- | --- | ---: | --- |
| BDF4C0 | BDF588 | 65 | ECX manager; stack mutable name; RET4; AL result |
| BDDC80 | BDE9B3 | 1091 | ECX manager; stack mutable name; RET4; AL candidates |
| BDEB40 | BDEBD3 | 37 | ECX manager; stack old/new header pointers; RET8 |
| BDD6E0 | BDD847 | 114 | ECX manager; stack input/output; RET8; AL result |
| BDDAA0 | BDDC72 | 158 | ECX manager; stack output/stem/extension; RET0C |
| BDC680 | BDC8A2 | 186 | ECX manager; stack prefix-output/stem/extension; RET0C |
| BDB1E0 | BDB2D8 | 92 | ECX manager; stack iterator-output/extension; RET8 |
| BDA2C0 | BDA313 | 36 | ECX actual tree; stack key; RET4; EAX upper bound |
| BDAE40 | BDAE63 | 14 | ECX iterator; RET; EAX checked node+8 |
| 4BF5A0 | 4BF5C7 | 16 | ECX iterator; RET; checked forward increment |

Full listings were read, including operand/register provenance. BDDC80 has two
stored-listing holes: BDE0ED..BDE0EF (3 bytes) and BDE7E8..BDE7EF (8 bytes).
Disk decoding shows alignment LEA/NOP bytes after unconditional jumps. Neither
is a reachable fall-through gap; they are not invented functions or repaired
Ghidra bodies. No Ghidra annotations or saved-analysis writes were made.

## Actual producers and traversal

Manager construction BE1DC0 establishes tree54 and group-list60. Tree nodes
are the actual20h nodes produced by BDABA0: links+0/+4/+8, native key+C/+10,
native prefix+14/+18, color+1C and nil+1D. Existing BDA260 and BD9860 operate
on exactly these nodes, so they are reused directly. BDDAA0 calls upper bound
BDA2C0 before lower bound BDA260 and preserves the equal-key node order.

BE10A0's complete producer writes group payload name+0/+4, calls4C3020 at
BE10F8 and BE110D, and stores extension head/count+0C/+10 and prefix
head/count+18/+1C. The group payload starts at its outer node+8. Thus the
outer group node has extension-list owner+10/head+14 and prefix-list
owner+1C/head+20. Value nodes use the actual10h string-list producer4CE6F0:
next+0, previous+4 and native string+8/+C. No host container is cast to one
of these layouts. This packet consumes these producers; it does not replace
search registration or construct a second index.

BDB1E0 selects only the first matching group. Recorded lengths must agree
before its case-insensitive comparison; its first match is retained across
all subsequent candidate passes. The source rereads the current heads and
links at their native boundaries, including after returning invalid-parameter
callbacks, copies and releases. It neither snapshots the group values nor
deduplicates candidates.

Let F be the full stem, B its basename, E the original extension, A each
different extension in the selected group. The precise candidate sequence is:

1. Direct BDD6E0 of the lowercased/slash-normalized original.
2. Extension-tree prefixes for F.E.
3. Group prefixes for F.E when F differs from B.
4. Extension-tree prefixes for B.E when F differs from B.
5. Group prefixes for B.E; no group means failure here.
6. When F differs from B, all extension-tree prefixes for every F.A, then a
   second extension traversal trying unprefixed F.A followed by group F.A.
7. All extension-tree prefixes for every B.A, then a second extension
   traversal trying group B.A. There is no extra unprefixed B.A probe.

Each F/B inequality is evaluated anew. Parsing uses the FIRST dot beginning
at the final slash index, not the final dot in the name. The slash loop uses
a captured signed length while reloading the name's data pointer per byte.
False can leave the caller's original name normalized.

BDC680 constructs prefix + stem + dot + extension using actual pooled
strings, then reloads current manager virtual08 after releasing its three
concatenation temporaries. D685B4+08 and D68D04+08 both contain BDD440 in the
verified native image. Successful constructed candidates replace the caller's
name with that spelling; this pass does not call the direct output resolver.

## Cleanup and logging evidence

FH3 metadata E0091C/E00940 gives BDDC80 eight states: basename, extension,
full stem, first group candidate, basename group candidate, empty alternate
candidate, nested full alternate candidate, basename alternate candidate.
States3/4/5/7 return to2; state6 returns to5;2->1->0->-1. Funclets
CC6420..CC645F each load the corresponding current header and jump41DD20.
The empty alternate candidate therefore stays armed throughout its nested
group-prefix traversal. BDD6E0 metadata E00840/E00830 arms the visitor before
the copied input; CC6388 destroys the current copy and CC6380 destroys the
visitor via BDB680. BDDAA0's E008F8/E008F0 has one candidate state.

BDC680 E005F0/E005D0 has four states, destroying expanded suffix, stem-dot,
dot, constructed candidate in reverse order. The normal dot cleanup instead
uses its captured data pointer and current length; BDC775 reloads that
pointer only in the nonempty append branch. Normal success copy-outs that
capture data/length before resizing retain those values for their release;
exception funclets always consume the current header. The first F.E tree
success likewise retains the full-stem data/length captured after BDDDE4.
No source catch restores caller fields or invents a creator rollback.

BDF4C0 E00B40/E00B38 arms its normalized-name copy only after construction;
CC65C0..CC65C7 destroys its current header. On success, BDEB40 checks the
live0109CEE8 publication and manager+79, builds `<SRCH><old|new>` and destroys
the same actual builder. E009B4/E009AC and CC6490..CC6497 identify builder
cleanup425F80. There is NO logger/sink call in this native body. No successful
output provider or fabricated logging side effect was added.

Native string release is noexcept in the established pool bridge. Throwing
host allocation/provider/invalid-parameter boundaries are represented by C++
exceptions with the verified native armed-state cleanup, not native SEH
dispatch or arbitrary access-violation recovery. Unarmed partially completed
constructors remain a callee boundary; the persistent diagnostic frame does
not invent a cleanup state for them.

## Validation

Strict MSVC Win32 translation-unit compilation passed with /W4 /WX /fp:strict.
The only temporary dependencies were ignored copies of the two42d7a7be
headers, whose hashes are in the report. This alone is not a complete library
build or runtime validation. Full integration requires that commit's actual
device source and matching lookup source to be linked. The CMake registration
file was leased by another worker during authoring. All 233 direct call rows
passed live verification; the single indirect manager08 call was checked
separately. The primary authorized this source commit with strict-TU-only
validation and will integrate source registration/dependencies and perform the
full build. No game was launched, no
installation was changed, and no permanent tests were added.
