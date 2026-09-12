# Native MPKG archive and startup callback integration

AX joins the actual MPKG factory/provider, archive, directory and entry-vector
owners to the existing VFS, canonical string pool and retained memory contexts.
The scoped reconstruction covers 22 complete native bodies (3,615 bytes),
strengthens the existing seven-byte BEF610 raw memory-data leaf, and preserves
two vector cleanup fragments plus the 32-byte application callback-store fragment.

The merged source at `9be685a1ed110b290766f66e7398a848bb78800e` passed strict
MSVC Win32 compilation and both CTests after removing temporary sibling source
includes. Four frozen fixture families passed 19 native/source comparisons and
7,111 checks, with six explicitly separate source-only checks. Current source,
dependent headers, linked objects and archive members must match those tested
inputs before promotion; each promoted HEAD is independently built and checked.

| Fixture | Comparisons | Checks | Source-only checks |
|---|---:|---:|---:|
| Entry vector | 10 | 86 | 1 |
| Directory | 5 | 1,162 | 1 |
| Factory/provider | 3 | 1,297 | 2 |
| Whole archive/runtime | 1 | 4,566 | 2 |

The complete archive comparison uses generated ZIP_STORED bytes: 1,002 decoded
bytes, two entries, one full 753-byte block and a 249-byte short block. The full
block boundary has remainder six modulo the short length, so the input exercises
the recovered global-index transform. Original and reconstructed archive,
directory and vector work produced equal 32,772-byte normalized results. The
canonical pool and actual memory counters drain after destruction.

The opener is an explicit boundary that returns a genuine encoded memory
stream. Existing memory/string dependencies use recorded source ABI bridges.
The separate source-only composition checks concrete factory, provider, archive,
directory and stream deletion wiring, early opener failure and startup callback
installation. Both one-byte original RET callbacks execute within that wiring
case. Original BB9900 is exercised by the vector family, not by successful
whole-archive destruction. The auxiliary physical context stays empty and
unpublished; composition owns consistency of the borrowed actual contexts.

The four failed archive attempts and the initial missing directory-support copy
are retained. They exposed probe compilation and fixture setup issues; the
production source did not change after the successful integrated build. The
independent archive review previously corrected one native memory-read ordering
detail before that build.

Original FH3/SEH, arbitrary native stack aliases, simultaneous cleanup failures,
type-ID startup, runtime manager/physical opening and gameplay remain unproven.
The MPKG entry-open/compression route is separate work. A current recursive scan
including ignored and hidden files found no `.mpkg`, `.mpak` or `.pak` extension
in this installation. Generated archive coverage is therefore explicit.

Next bounded candidates are BB8E70/BB8D60/BB8BE0/BB8A90 for actual MPKG entry
opening, BEF840 for copying an actual memory stream, and the actual D64400
inflater owners required by compressed entries. Recheck leases and startup
priority before assignment. The MPAK factory and startup registry also remain
relevant dependencies; their presence alone establishes no installed archive
format or runtime use.

Exact addresses, ABIs, Ghidra repairs, source boundaries, fixture pins and
promotion requirements are retained in `reports/native_ax_integration.json`
and the five packet reports it references. Saved Ghidra annotations preserve
prior comments and mark descriptive names as hypotheses.
