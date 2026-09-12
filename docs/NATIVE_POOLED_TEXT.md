# Actual pooled line strings and text-buffer reading

`native_pooled_text.hpp/.cpp` reconstruct eleven complete bodies against actual
Win32 storage. The public C++ signatures pass the application's canonical
`NativeStringStorage` explicitly; they are not drop-in original-ABI entry points.
The four-byte line/token header contains only `char*` at +0. This is distinct
from `NativeString`'s eight-byte length at +0/data at +4. Descriptive names remain
hypotheses. Existing atlas semantic implementations and prior Ghidra names and
comments were preserved; this worker made no Ghidra mutations or ledger edits.

| Body | Original ABI | Result / effect |
|---|---|---|
| AEE1E0 | stack char pointer, RET4 | Return nonnull bytes using strlen+1 |
| AEE2A0 | ECX four-byte header, RET | Return captured pointer, clear header |
| AEE2E0 | ECX destination, stack source header, RET4 | EAX destination; copy construction |
| AEE340 | ECX destination, stack bytes, RET4 | EAX printable-prefix length |
| AEE3C0 | ECX line, stack output/index, RET8 | EAX output; construct selected token |
| AEDF80 | ECX line, stack bytes, RET4 | AL case-insensitive equality |
| AF55F0 | ECX buffer, RET | Clear cursor +4 and auxiliary +18h; EAX zero |
| AF5600 | ECX buffer, RET | EAX buffer; initialize six fields, leave +8 |
| AF5660 | ECX destination, stack bytes, RET4 | EAX destination; C-string construction |
| AF56C0 | ECX destination, stack source header, RET4 | EAX destination; release then copy |
| AF5740 | ECX buffer, stack output header, RET4 | AL normalized-line availability |

## Ownership and observed ordering

AEE2E0 and AF5660 are producers: their only object write is the pointer at +0.
Null input produces a null pointer. A nonnull empty C string allocates one byte.
They overwrite a previously live destination without cleanup. AEE2E0 publishes
the new pointer before reloading source.data, including when the headers alias.
AF5660 retains the caller's byte pointer across allocation instead.

Every allocation/return uses the same 00419CC0 singleton and BD1120/BD1510 byte
pool contract as `NativeString`: requested/returned size is strlen+1, native
unused argument is 1. There is no second allocator or stored allocator pointer.
`ActualNativeStringPoolStorage` is the existing bridge for the one canonical
publication slot, shutdown gate and lifetime domain. Its getter timing and
returning/throwing boundary remain those documented by that owner.

AF56C0 returns the captured destination block **before** it reads source.data.
It has no self-assignment guard. A null source leaves the destination header
unchanged after that release, potentially dangling. With an aliased source it
can scan returned storage, then copy from its newly published allocation. These
are native unsafe domains, not repaired assignment semantics. Callback changes
to source headers are observed after release and allocation. AEE2A0 clears its
header after return even if a release callback changed the header.

AEE340 leaves an existing destination untouched if input is null or its first
byte is outside 21h..7Eh. A nonempty prefix releases the old allocation, allocates
count+1, performs the original count-limited copy (including NUL padding), then
terminates using the reloaded destination pointer. Partial/overlapping source
storage and failed allocations retain native preconditions; no bounds or failure
fallback is introduced.

## Token grammar and actual buffer fields

AEE3C0 accepts the initial signed-byte range 20h..7Eh and selects a token before
processing spaces. Repeated spaces advance the token number once. Leading spaces
therefore produce a null token zero and place the first word at index one. A
selected word is copied by AEE340, then copied again into the caller's output
header, then the temporary is returned. Missing tokens construct a null output.
Tabs, control bytes, DEL and high-bit bytes terminate scanning; no quotes, escapes
or generalized whitespace grammar are added. Token output must be fresh storage
or an already released header because the native return performs construction.

AF5600 initialization writes +0Ch/+10h (embedded eight-byte filename string),
+4 cursor, +0, +14h data pointer and +18h auxiliary, in that order. It leaves
+8 extent untouched. The independent AF5850 producer provides stronger layout
evidence: at AF58BA/AF58BC it writes stream size into +0/+8, clears +4/+18h,
publishes an allocation at +14h and reads stream bytes into it. Its full VFS and
buffer lifetime are outside this packet. AF5850's free failure path still has
the Ghidra gap AF58FB..AF590C and is not claimed reconstructed. The API accepts
an actual `void*` buffer rather than inventing its complete class extent.

AF5740 tests +14h for null, compares cursor+4 and extent+8 as signed integers,
scans to LF or extent, skips leading signed bytes <=20h, and retains subsequent
signed bytes >=20h. Interior tabs/controls/high-bit bytes disappear; spaces after
content remain. It NUL-terminates the supplied scratch, increments cursor past
the LF or the extent of a final unterminated line, constructs a temporary pooled
string, assigns the caller output and returns the temporary. EOF leaves output
unchanged. Scanning captures the initial data pointer; the copy phase reloads
+14h, and the loop rereads cursor+4. These accesses preserve observable ordering.

Scratch is an explicit binding for original global F8C2C8. Caller provides enough
writable capacity and the application's shared buffer identity. No implicit
vector, per-call storage, bound or reentrancy policy substitutes for this global.
The AF56FD..AF56FF and AF579D..AF579F listing gaps are three-byte `LEA ECX,[ECX]`
alignment instructions, confirmed from live/installed bytes; neither hides a body
tail. Complete bytes, ends, final instructions and all calls are in the report.

## Verification and limits

Read-only `bsp.py ghidra` batches verified `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`, x86 LE32 at base 00400000, bridge 8089. All eleven
complete bodies matched the installed executable bytes from
`I:/SteamLibrary/steamapps/common/Battlestations Pacific/battlestationspacific.exe`.
`verify_report_calls.py` validates every one of the report's 26 direct call sites.

The preserved scratch experiment `C:/Users/sqz269/bsp-ao-text` contains capture,
relocation generation, C++ probe and build commands. It relocates those actual
bodies, redirects pool/CRT calls and scratch references, and compares original
execution with the reconstruction: **42 observations and 67 pool events match**.
One focused trace covers null/empty construction, copy, assignment/null-source
and self-header ordering, untouched invalid prefixes, token grammar, signed-byte
normalization, final-line cursor, EOF preservation, init and rewind. Returned
blocks are retained by the probe and allocations zero-filled, allowing unsafe
alias ordering to be observed without dereferencing actually freed memory.

The probe compiled this production source with MSVC Win32 `/W4 /WX /fp:strict
/MD /EHsc` and linked `/MANIFEST:EMBED`. The primary integrator performs the
repository build. No permanent tests or shared build metadata were added.
This is original-byte fixture execution with returning pool/CRT hooks, not game
execution, actual pool lifetime validation or ABI replacement. CRT `_stricmp`
and count-copy are host boundaries; only the exercised ASCII comparison is
differentially established. Native SEH handlers/stack unwinding and allocation
failure are not covered. C++ unwinding releases a constructed temporary through
the canonical storage interface; it does not claim native SEH compatibility.
