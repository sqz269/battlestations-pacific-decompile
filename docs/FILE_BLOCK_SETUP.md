# FileBlock identifier preparation and VFS block entry

Addresses: 007fa3a0, 00bd20a0, 00bdf950, 00be0980.

`00bdf950` prepares a FileBlock identifier. `00be0980` enters a named block in
the global VFS manager's observation/tracing state. They receive different
objects and do not define stream byte ranges, retain a stream, or read file
contents. A stream-setup name or implementation would misrepresent them.

The loader lane independently verified that constructor `00be0a30` calls
`00bdf950` with the new `28h` FileBlock object, whose name is at `+14h/+18h`.
It then calls `00be0980` with global manager `[0109ceec]`, the prepared name
wrapper and the caller's second argument. The audited loader caller passes1.
See `VFS_LOAD_PROCESSING_START.md` for that separate caller evidence.

## Fresh evidence and ABI

All live queries used `python tools/bsp.py ghidra ...`; its client verifies
project `bsp`, program `/battlestationspacific.exe`, language
`x86:LE:32:default`, and image base `00400000` before every query. Configuration
names project `C:/Users/sqz269/bsp.gpr` and binary
`I:/SteamLibrary/steamapps/common/Battlestations Pacific/battlestationspacific.exe`.
The current disk SHA-256 is
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.

These complete bodies matched fresh Ghidra bytes to the installed PE. Ends are
exclusive and include RET operands. The approved immediate-callee closure is
limited to `00bd20a0` and `007fa3a0`; string helpers otherwise use explicitly
identified prior evidence or remain callback/allocator boundaries.

| Start | End | Bytes | Observed ABI |
|---|---|---:|---|
| `00bdf950` | `00bdfd80` | 1072 | ECX FileBlock object; no stack args; RET; no stable result contract |
| `00be0980` | `00be0a29` | 169 | ECX VFS manager; stack name wrapper, gate byte; RET8; no stable result contract |
| `00bd20a0` | `00bd216d` | 205 | ECX source string; DL delimiter byte; stack destination token container; RET4 |
| `007fa3a0` | `007fa431` | 145 | ECX list object; stack unsigned increment; EAX updated size on returning path; RET4 |

Three data spans also matched: empty string at `00ce3a0c`, underscore at
`00ce7890`, and the invalid-character set plus diagnostic format around
`00d68520`. Complete hashes and original names/comments are recorded in
`reports/file_block_setup_audit.json`. Captures and complete offline disassembly
are in ignored `exports/bsp/parallel_file_block_setup/`.

## Identifier preparation

The string at FileBlock `+14h` is `{stored_length, data_pointer}`. In the first
phase, `00bdf950` scans it using `strcspn` and replaces matched bytes with `_`.
The exact invalid set at `00d68520` is ` <>=?:;"*+,./\|` (including the leading
space). The scan resumes from the replacement position, which is valid because
underscore is not in that set. Case, hyphens, digits and high bytes are not
otherwise normalized. For valid allocated native strings, an embedded NUL
before stored length also stops `strcspn` and is replaced by `_`; this follows
the call/result/store instructions rather than a separate NUL check.

If stored length is at most32, the function returns through `00bdfd6d` without
tokenization. For longer names it invokes `00bd20a0` with DL `_`. This helper
skips repeated delimiters and appends only nonempty spans to the caller's token
container. Each span is copied through the existing substring helper
`00469840`; the temporary string is released after its insertion call. It does
not borrow pointers into a stream or establish a retained stream lifetime.

The long-name branch selects abbreviation only when token count is3..11
(`count >= 3 && 3*count-1 <= 32`). It clears the destination string, then handles
each token using the native `atol` result:

- A nonzero result retains the complete token text.
- A zero result retains its first byte.
- Underscores separate the resulting tokens; the last token has no suffix.

This is a numeric-conversion predicate, not an all-digits check. For example,
`0`, `00`, and a nonnumeric token take the first-byte path; a token beginning
with a nonzero decimal value can retain its full text. Native CRT overflow and
locale behavior were not newly reconstructed. Long numeric tokens can leave
an output longer than32; there is no final length clamp in this body.

Outside the3..11-token branch, the raw calls at `00bdfc86` and `00bdfc9c` request
substring `(-64,64)` and substring `(0,64)`, respectively, then concatenate the
second result with the first. The cached `00469840` contract adjusts a negative
start by adding it to the requested count and setting start0. Under that prior
contract the first substring is empty and the result is the first64 bytes,
clamped to source length. This is explicitly a composition using prior helper
evidence, not a fresh re-audit of `00469840`. Replacing it with a32-byte limit,
last64-byte suffix, or an assumed hash would change behavior.

The output is stored in the FileBlock object's own string, with temporary
strings and the token container cleaned up. At `00bdfd65` the final container
free is followed by stack cleanup, SEH restoration and RET at `00bdfd7f`.
Captured pseudocode marks that free non-returning even though the complete
body and raw tail include this continuation; the short-name branch also reaches
the shared epilogue. Any analysis repair must inspect that specific call site.

## VFS manager block state

The offsets in this section belong to the VFS manager, not the FileBlock
object passed to `00bdf950`.

| Manager offset | Established use in `00be0980` |
|---|---|
| `+0Ch/+10h` | Current block name string, copied from input after observer notification |
| `+14h` | DWORD nesting depth, incremented once |
| `+78h` | Enables the optional observer call |
| `+79h` | Current gate byte, saved by address for node construction then ANDed with the argument byte |
| `+7Ch` | Native list object passed to node construction and checked size growth |
| `+80h` | List sentinel pointer (`list+4`) |
| `+84h` | List size (`list+8`), grown by one |
| `+88h` | Observer object; virtual `+8` receives the input name wrapper when enabled |

The exact order is: pass the current gate byte's address to node construction
`007f8390`; call checked list-size growth with1; link the returned node before
the sentinel; AND current gate with the input byte; conditionally notify the
observer; increment nesting depth; copy the input name; and, if the resulting
gate byte is nonzero, call the diagnostic helper with `+FileBlock %s`.
The allocated node is linked into manager state and is not removed in this
function. The exact node-constructor internals and destruction path remain
outside this closure.

Observer dispatch happens after the gate change but before depth/name updates.
A reentrant observer can therefore see that intermediate state. There is no
explicit retain/release of the observer or input name wrapper here; observer
internals could have their own lifetime effects. The final diagnostic call's
external output is not established by this audit.

`007fa3a0` is not an unconditional throw. It checks
`increment <= UINT32_MAX - list_size`; on success it adds the increment to
`list+8` and returns that updated size. On overflow it constructs and throws a
`std::length_error` with `list<T> too long`. The existing heuristic name
`STL_xlen_throw_007fa3a0` should be refined or accompanied by this returning-path
comment while preserving its standard-library identity. The caller constructs
its node before this size check, so host failure cleanup must not be assumed
from the successful path.

## Implementation boundary

No C++ change is justified in isolation by this packet. The current typed
`VfsProviderManager` has no FileBlock gate stack, nesting-depth state, paired
leave operation, or real observer. Adding disconnected callbacks or silently
discarding native block state would produce a misleading partial interface.

The next coherent unit is a scoped named VFS block only after its paired exit
and observer contracts are recovered. Loader metadata identifies FileBlock
deleting destructor `00bdebe0` with direct callee `00bdcb30`; the latter is a
candidate exit dependency, not an audited leave implementation. Its relationship
must be checked before claiming balanced nesting or restored flags/names.

For a future implementation, identifier preparation can be separated into a
pure helper with explicit native-string preconditions, but the complete
abbreviation/fallback and numeric-conversion semantics must accompany it.
The verified <=32-byte replacement branch alone is a bounded fragment, not a
complete replacement for `00bdf950`.

This packet provides fresh complete body/data evidence and reconstruction
proposals. It changes no C++, Ghidra state, shared ledger or game files and runs
no build or runtime tests. Existing substring/string-composition evidence is
identified above; native ABI, callback ownership, exceptional cleanup, paired
block exit, and game behavior remain unverified.
