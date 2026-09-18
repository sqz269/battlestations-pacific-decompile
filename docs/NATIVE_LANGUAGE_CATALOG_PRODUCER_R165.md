# Raw language catalog producer (R165)

Address: 008D7BC0. Actual catalog: 00F88974/00F88978/00F8897C.

The complete 1483-byte normal producer now composes the actual R161 stream
scanner, R162 pooled rows/list/vector, and R163 hints singleton. It uses borrowed
process publications and raw storage, with caller-owned progress on source
failure. The existing descriptive Ghidra name BSP_GameSettings_BuildLanguageTable
is retained as a hypothesis. The ordinary projected application path remains
separate until its settings/catalog ownership is replaced coherently.

## Native contract

Native entry takes no arguments and returns with RET. The new explicit C++
interface is not a binary replacement or recovered FH3/SEH interface.

1. Any nonzero current catalog count skips all work, including negative values.
2. Construct the extension temporary before the directory temporary. Enumerate
   through current 0109CEEC into an actual intrusive list. Its first word is an
   uninitialized preserved preimage, supplied explicitly in source. Return the
   directory buffer, then the extension buffer, leaving dead headers.
3. Allocate an 828h scanner. Only a nonnull allocation pops a filename into a
   separately owned by-value argument header; BEF2E0 consumes that argument and
   opens the file. No failed-allocation recovery or extra pop is introduced.
4. Get the actual hints owner and inspect its +8 field. Nonzero bypasses filename
   filtering. Otherwise test the inline case-sensitive English C-string prefix,
   followed by French, Italian, German, Spanish and English-authentic prefixes
   through 553C80. Those helper results are tested through AL only. Prefix matches
   permit a suffix after the expected filename. Declined descriptors were already
   opened; destroy/free them through the holder helper before continuing.
5. Zero four raw 8h headers. Peek and stop on token EOF or an unquoted empty token.
   Repeek for each comparison in order: lanfile, lockit_id, voice_dir, fontpath.
   Matching is the existing 438E10 host-CRT source boundary. On a match, reload
   the scanner, accept the keyword, read the string and ignore its success byte.
   An unknown token repeats without consumption and may never return.
6. Each field assignment preserves the inlined allocation schedule. Equal lengths
   retain storage and copy into the current pointer. A changed nonzero length
   allocates first, reloads/returns the old buffer, publishes length then pointer,
   writes NUL, and copies the current length. A changed zero length returns the
   captured old allocation, clears pointer then length, and retries parsing.
7. Append a copied row. Destroy and free the scanner captured by the final peek,
   then reverse-destroy the temporary row. Repeat using current list count.
8. Clear list nodes, reload its sentinel pointer and free it. Do not replace this
   with a helper that performs different frame-header stores.

The explicit operation retains raw temporaries/list/row, scanner ownership,
nested helper progress, pending unpublished string storage, and call sites.
Escaping C++ callbacks require caller cleanup and acknowledgement; replay is
rejected. A native allocation/open failure is not converted into parser success.
The R161 token-size and private-stack-preimage boundaries still apply.

## Validation

- 1777 live Ghidra/PE bytes agree: 1483 producer code bytes and 294 data bytes,
  including the literals, catalog/VFS cells and existing stream/hints data used
  by the comparison. All 65 direct CALL rows are mechanically checked.
- Three unreachable three-byte gaps after short jumps required no byte changes.
  The decoded row-destructor call at 8D814B was excluded from Ghidra's stored
  body after the scanner free, despite appearing in its range listing. Ordinary
  gap detection missed this. The existing locked repair logic cleared the
  verified CRT call overrides, then function recreation restored membership.
  All 65 calls now pass the containing-function check. Existing name, prior
  annotations, saved project, forced snapshot and refreshed exports are retained.
- Strict MSVC Win32 build and all three existing CTests pass.
- One local diagnostic compares the copied original producer with source across
  eight groups: 13,545 identical observed bytes. It uses real raw pooled strings,
  intrusive list nodes, memory streams, hints ownership and manager cleanup.
- Coverage includes positive/negative nonzero count gates, no descriptors, all
  six admitted prefixes, case-sensitive rejection, suffix acceptance, hints
  bypass, opening declined files, uppercase keywords, quoted values, duplicate
  same/different-length assignments, failed empty/missing string reads and an
  empty stream. Both lanes append the same rows despite a false read-success
  byte. Stream/backing, hints, pool and catalog cleanup complete normally.
- Two isolated stall observations reach 32 comparisons without consuming the
  unknown token: 1099 trace bytes match. The fixture deliberately exits from the
  comparison callback at that boundary; it does not claim parser termination or
  cleanup for this path.
- Two isolated failed-open observations both terminate with C0000005 when the
  scanner dereferences the null stream. This records the admitted behavior limit;
  it does not establish native exception handling or recovery.
- One source-only exception after two owned row fields retains that row, scanner,
  stream and remaining list node. Replay rejection and explicit cleanup pass.

Both lanes share the existing R161/R162/R163 helper implementations. Only the
1483-byte producer is newly copied and compared. BDD990 enumeration and VFS open
are controlled callbacks here; they supply actual lists and memory streams, but
do not prove installed-package discovery or ordinary application ownership.
Allocation identities are normalized; row bytes, callback order, read results,
string-read status and resource counters are observed. Original CRT internals,
native FH3/SEH, private stack aliasing, asynchronous mutation, token overflow,
ordinary startup binding and gameplay remain open. No shared game process was
launched or stopped; another harness owned the running game during this packet.

## Follow-up packets

Recover/compose the raw 008D8190 settings loader and remaining path/writer
dependencies, then bind the canonical settings/catalog/hints lifetime into the
application. Use R164 catalog shutdown; do not copy projected objects into raw
storage. Run the mounted-file diagnostic when the shared runtime is available.

Evidence: reports/native_language_catalog_producer_r165.json. Local comparison
and sealed artifacts: local/language_catalog_producer_r165 and local/evidence-r165.
