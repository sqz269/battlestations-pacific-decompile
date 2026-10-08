# CC12: raw physical-stream seek entry, 00BF4F20

## Result and boundary

`raw_seek_native_physical_stream_00bf4f20` is an additional MSVC Win32 raw
entry in the existing physical-stream module. Its complete 32-byte,
12-instruction body matches Original `00BF4F20..00BF4F3F`, except for the
single absolute operand that addresses the genuine `SetFilePointerEx` IAT
cell. The normal production COFF, exact membership of that whole object in
`bsp_core.lib`, and the unique complete linked body were checked before
Original execution. A fresh real-file fixture passed six paired comparisons
and one typed Source call. The normal Win32 build and all three existing
CTests passed.

This admits the bounded raw entry with actual readable HANDLE storage at
receiver+8 and writable `LARGE_INTEGER` storage at receiver+10h, under the real
Win32 API contract. It does not establish a physical-stream class, raw class
table, constructor/destructor, handle producer, lifetime, substream dispatch,
or game integration. The ordinary seek helper and existing raw BF5020 entry
are unchanged. Ghidra was queried read-only; the integrator owns annotations
and shared ledgers.

## Recovered ABI and complete body

Original takes the actual receiver in ECX and distance-low, distance-high,
origin at entry ESP+4/+8/+0Ch. Incoming EDX is unused. Source declares an
explicit unused second fastcall argument to keep those three words on the
stack. The signed 32-bit return preserves the complete Win32 BOOL in EAX;
the final `RET 0Ch` pops the original three arguments. EBX, ESI, EDI and EBP
are untouched by the wrapper and preserved by the called Win32 API.

```text
8B44240C             mov eax,[esp+0Ch]
50                   push eax                 ; origin
8B44240C             mov eax,[esp+0Ch]         ; distance high
8D5110               lea edx,[ecx+10h]         ; direct output address
52                   push edx
8B54240C             mov edx,[esp+0Ch]         ; distance low
50                   push eax
8B4108               mov eax,[ecx+8]           ; actual HANDLE
52                   push edx
50                   push eax
FF15 D822CE00        call dword ptr [00CE22D8]
C20C00               ret 0Ch
```

This preserves the stack-relative loads and their order. It does not replace
the direct output with a temporary, normalize BOOL to C++ bool, or synthesize
success/failure handling. The original PE identifies `00CE22D8` as
`KERNEL32.dll!SetFilePointerEx`; its on-disk thunk bytes are not treated as a
loaded OS address. Ghidra project `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`, and installed-image/live bytes were checked.

Source COFF contains one ordered relocation: offset 25, `IMAGE_REL_I386_DIR32`
(6), target `__imp__SetFilePointerEx@20`. All other 28 bytes match Original.
The fresh linked entry is `20001000`, occurs exactly once, and uses actual PE
IAT cell `2001F020` for the same import. Its map origin is the explicitly
linked current `native_physical_stream_open.obj`. The linked Source IAT was
also checked at runtime against the real `GetProcAddress` result. No other
operand, instruction, call target or global was normalized.

## Fresh fixture and observed behavior

The ignored evidence family is
`local/cc12_file_seek_raw_entry_20261008a/`; final sealed evidence is `run04/`.
The fixture links the current normal module object, one probe object, and
three existing production archives: `bsp_core.lib`, `bsp_lua511.lib` and
`bsp_zlib121.lib`, plus real Windows/CRT import libraries. It is **not** a
zero-BSP-archive fixture. These archives resolve ordinary unexecuted module
dependencies; their presence does not admit those bodies as native providers
for this packet. The executed raw body has only the real OS API callee.

Before either wrapper executes, the driver proves whole-body equality and
the actual linked import. The 32-bit probe then independently rechecks the
loaded Source body and IAT. It places all unchanged Original32 bytes in a
private RX allocation and binds only Original's actual `00CE22D8` IAT cell
to the same OS export, in a private read-only allocation. It supplies no
callback, import surrogate, class table or reconstructed object lifetime.

A real temporary file is created with read/write access, initialized to 64
bytes, and deleted on close. The receiver uses live typed HANDLE and
`LARGE_INTEGER` fields at the required offsets, with a 20h-byte backing
inside a fully compared 40h-byte guarded aggregate. Each pair resets the OS
cursor and output sentinel before calling Source and Original with distinct
incoming EDX seeds.

| Case | Initial cursor | Distance / origin | Full EAX | Output and cursor | Failure error |
| --- | ---: | --- | --- | --- | --- |
| begin | 19 | 7 / FILE_BEGIN | 00000001 | 7 | not asserted |
| current negative | 23 | -3 / FILE_CURRENT | 00000001 | 20 | not asserted |
| end negative | 11 | -5 / FILE_END | 00000001 | 59 | not asserted |
| high word, beyond EOF | 0 | 00000001:00000007 / FILE_BEGIN | 00000001 | 4294967303 | not asserted |
| invalid origin | 11 | 0 / 80000000 | 00000000 | output sentinel unchanged; cursor 11 | 87 |
| invalid handle | 13 | 0 / FILE_BEGIN, INVALID_HANDLE_VALUE | 00000000 | output sentinel unchanged; real-file cursor 13 | 6 |

All six pairs matched complete BOOL EAX, all backing/guard bytes, actual OS
cursor, ESP cleanup and EBX/ESI/EDI/EBP. Failure last-error values matched;
success last-error values were deliberately not an assertion. One additional
typed fastcall Source invocation reached position 3 with guards intact.
The beyond-EOF seek did not grow the file. Both IAT cells and Original code
remained unchanged after the calls; private mappings and file were released.

Microsoft's [SetFilePointerEx contract](https://learn.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-setfilepointerex)
supports the read/write handle, origins, full BOOL/error convention and
beyond-EOF behavior used here. Failure output preservation above is an
observation of this OS, not a promise added to the public API contract.
There is no concurrent-handle, faulting-pointer, asynchronous-I/O, game-path,
or exhaustive Windows-version validation.

## Provenance and retained failures

`run04/inputs/manifest.json` pins pre/post hashes for the installed Original,
current source/header inputs, 202 normal module inputs, 164 probe-consumed
headers, compiler/linker tools, scripts, actual supplied archives and searched
libraries, exact production/probe objects, final PE, and the OS providers.
Copies of recipe inputs, source, consumed probe headers and searched libraries
are retained with matching copy hashes. The actual 32-bit import and export
implementation both resolved to `C:/Windows/SysWOW64/kernel32.dll`; the probe
opened and resolved that file through a real 32-bit handle to disambiguate
Windows filesystem redirection. Whole-body proof completed before execution,
and all recorded pre/post pins matched.

The original attempts remain available: the initial driver import failed
because a local `inspect.py` shadowed Python's standard module; run01 stopped
at a missing Windows header in the probe; run02 retained the exact failed
two-object link with ten unresolved genuine ordinary-module dependencies.
No Original instructions executed in those attempts. Object-level closure
inspection reached 642 production objects, so existing archives were used
instead of adding or fabricating providers. Run03's body proof and execution
passed but its post-execution seal rejected the ambiguous System32 path.
Run04 fixed path observation and completed the seal. Failed evidence was not
rewritten, and no historical fixture family was replayed.

The machine-readable report is
`reports/cc12_file_seek_raw_entry.json`. Its hashes identify the exact sealed
manifest, whole-import proof, executable and logs. This is reconstructed,
build-tested and bounded native differential/ABI evidence, not game validation.
