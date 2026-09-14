# Native one-byte string construction and append (BH)

The saved `bsp` project, `/battlestationspacific.exe`, owns both complete
functions through their real returns. The live bodies match the installed PE
byte for byte. No body repair, missing function definition, flow override edit
or Ghidra mutation was needed for this packet. Names below are descriptive
source hypotheses; the primary integrator owns annotation and shared CMake.

| Entry | Inclusive end | Bytes | Native ABI | Coverage |
| --- | --- | ---: | --- | --- |
| `00531030` | `005310A9` | 122 | ECX actual 8h output header; stack DWORD, low byte used; EAX output; `RET4` at `005310A7` | Complete normal body in the explicit-storage source domain |
| `0054AA70` | `0054AB02` | 147 | ECX actual 8h destination; stack DWORD passed to constructor; `RET4` at `0054AB00`; no semantic result | Complete normal body and temporary return in the same domain |

`construct_native_string_byte_00531030` clears length and pointer **before**
the first `00419CC0` getter and fixed size-two `00BD1120` allocation. Calling
this constructor on a live string therefore loses the previous ownership.
At `00531053` it rereads the length. Unsigned length below one calls `BF7680`
with zero bytes; otherwise it reads and writes the first byte. The pointer is
reread after that copy; a nonnull pointer and current length plus one are
captured before the second getter and `00BD1510` return. It then writes the
new pointer, length one, and terminator in that order. Finally it rereads the
published pointer and stores the input low byte. A NUL input remains a
one-byte string containing NUL; it does not take the empty C-string path.

`append_native_string_byte_0054aa70` constructs the stack temporary first.
`0054AA9B` captures its length into EDI and `0054AAA1` its pointer into EBP;
the FH3 state becomes armed only after construction returns. A zero captured
length skips resize/copy, but a nonnull captured pointer is still returned.
Otherwise it captures the destination's old length in EBX, resizes to the
DWORD sum with preserve set, reloads destination data at `0054AABF`, and
copies the captured temporary length from the captured pointer at offset
old-length. It never rereads either temporary header field on this normal
path. Cleanup disarms the native state, tests the captured pointer and returns
it with captured length plus one through a fresh getter. There is no source
result or destination-header reset after this return.

The source uses existing `NativeStringStorage` operations. Physical enumeration
passes its existing `ActualNativeStringPoolStorage`, whose allocation and
return each resolve `00419CC0` from the application's shared publication;
neither helper creates, owns or caches another pool or lifetime domain.
Ordinary construction takes one getter, allocation and no return. Appending
to an ordinary nonempty string takes four getters in order: allocate the
two-byte temporary, allocate the resized destination, return old destination,
return the temporary. An empty destination skips its old-buffer return.

`00BF4835` now calls the recovered byte append for the backslash. The star
path at `00BF487F..00BF48FA` instead inlines an ordinary C-string
constructor/append; it continues through the existing C-string operations.
This distinction closes the backslash-helper limitation recorded in
`NATIVE_PHYSICAL_ENUMERATION_BF.md`; that older report remains historical.
It does not upgrade the entire enumerator to arbitrary-alias or FH3 parity.

`BF7694..BF769A` tests `source < destination < source+count` and branches to
the backward copy at `BF7844`; `BF785F..BF7862` executes
`STD/REP MOVSD/CLD`. The direct append copy therefore uses `memmove` for
valid, nonwrapping ranges. The reused generic `0041DD40` preserve copy still
uses `memcpy` and keeps its existing nonoverlap contract. A well-formed
actual pool supplies distinct live old/new allocations. Malformed pools,
overlap during that reused resize, wrapping/huge memory spans, invalid
headers, allocator failure and arbitrary aliases into compiler stack slots
are outside the claimed executable source domain. No bounds guards, fallback
buffers or invented allocator state were introduced. The zero-count native
copy is omitted to avoid a C++ null-source copy. The original machine permits
some hardware effects outside that C++ domain; those are not fixture claims.

Normal temporary release is explicit in the source. C++ scope cleanup also
runs on a C++ resize/copy exception, but original FH3/SEH record layout,
handler behavior and exception-time mutation of the stack temporary are not
reconstructed. `NativeStringStorage::release` remains `noexcept`: failure
while acquiring the getter during release is the bridge's existing
terminating source boundary. Original machine ABI compatibility and game
execution have not been validated.

Both owned C++ translation units pass MSVC Win32 `/O2 /MD /W4 /WX`
compilation. After `verify-seeds` passed, one ignored differential scenario
executed the complete original constructor, append and existing resize bodies
against the new source. Live bytes for all three bodies were compared to the
installed PE before execution; only validated direct-CALL operands were
relocated to the copied bodies or explicit fixture getter, allocation, return
and overlap-safe copy boundaries. The actual pool/CRT/FH3 exception bodies
were not executed. The scenario uses a NUL byte with nonzero upper bits,
resurrects a header during allocation, mutates header/buffers during returns,
and checks final publication, destination reload and captured temporary
cleanup. Native and source agree on six getter boundaries, three allocations,
three returns, complete callback order and final bytes/header. This is one
focused regression, not a permanent suite or a gameplay claim.

The private evidence is in `local/bh/`: `byte_probe.cpp`, `native_spans.json`,
`verify-seeds.log` and `byte_probe.exe`. The fixture links the existing primary
`bsp_core.lib` for reused helpers and the freshly compiled new TU. Its binary
has `/MANIFEST:EMBED`. The primary integrator must add
`src/native_string_byte_append.cpp` to `bsp_core`; physical enumeration is
already registered. The structured report records all ten direct CALL sites
inside the two roots and the caller adoption separately; all eleven rows
passed live verification. Read-only flow checks found 45 and 51 listed
instructions respectively, with no gaps. `./scripts/build.ps1` also passed
and its configured `reconstructed_math` CTest passed (1/1). That checkout
build includes the changed enumeration TU but does not compile or link-test
the new unregistered byte helper; its strict TU build and differential fixture
provide the new helper's direct compilation/execution evidence.
