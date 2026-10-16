# Actual provided-callee string-header comparison, 00443D00

`less_native_string_headers_with_callees_00443d00` implements the whole
81-byte, 31-instruction ordinary entry in the existing provided-CRT C++ domain.
The established Ghidra name `BSP_NativeString_LessCaseInsensitive` stays intact.
The older two-argument `less_native_string_headers_00443d00` CurrentCRT API,
its implementation, comments and prior reconstruction records remain unchanged.
This packet adds a distinct Source interface and reconstruction record. Its C++
name describes behavior and is not a recovered game symbol.

The two actual eight-byte headers contain LENGTH at +0 and DATA at +4. The
ordered volatile DWORD reads are:

1. Read left LENGTH. If zero, read right LENGTH and return whether it is nonzero.
   High-bit lengths count as nonzero. Both DATA words and all services are skipped.
2. Otherwise read right LENGTH. If zero, return false and skip DATA and services.
3. Otherwise capture right DATA before left DATA, pass both captured pointers,
   the same borrowed actual locale-change cell and the existing
   `NativeCrtStricmpCallees` to `compare_native_crt_strings_00bf7fbf`, and return
   whether its signed result is negative. There are no further header reads.

Native function 00443D00 contains `00443D3D CALL 00BF7FBF`. This is the only owned
transfer. It is neither a null-pointer test nor a length-bounded comparison.
There is no local cleanup, guard, retry, reset, catch or `noexcept` contract.
Exceptions from the existing provided service propagate to the caller.

The required cell is the caller's actual `0109DE1C`, borrowed by volatile
reference. The published service preserves its own cell sampling, ASCII mode,
locale/TLS service, errno and returning invalid-parameter contracts. Its existing
provided callees remain an explicit caller obligation; this API supplies no
default adapter, new callback, locale owner, global, `_stricmp` bridge, RawCtx or
feature word. Empty branches do not sample the cell or invoke any service.

Native has two stack words, RET8 and an AL boolean; upper EAX is incidental.
Source has four stack words, RET10h and a semantic C++ boolean. Actual CMake code
also has an EBP frame and different private argument-load timing: Native loads
the right argument before its first branch, whereas Source loads it on the
selected branch. The Source cell/callee argument loads pass reference identities;
they do not sample the actual cell. Original private argument/frame aliases,
registers, flags, fault timing, FH3/SEH/RTTI, CRT/TLS/heap ownership, runtime and
game equivalence are unproved. Storage must admit the declared volatile raw
DWORD accesses. This is not a drop-in binary replacement.

Default MSVC Win32 Release `./scripts/build.ps1` passed all three existing suites
in 7.47 seconds. These suites are not new comparator body or Original-runtime
tests; no tests, probes or Native execution were added. The actual CMake object
contains 84 code bytes / 37 instructions: public 70/30 and an emitted uncalled
DWORD-read helper 14/7. Every instruction and the sole actual-service REL32 were
reviewed. All three public exits use RET10h; both empty arms skip DATA and
services; right DATA precedes left DATA. There is no local EH, padding, BSS or
weak runtime datum. Non-code sections are compiler linker directives (47B),
debug metadata (212B), and checksum metadata (40B), with no relocations.

The report freezes the installed PE/live/current-start proof, eight published
provider fingerprints, original name/reconstruction preimages, full actual
COFF evidence and build log. Independent Root preliminary review passed 15 checks;
the cited preliminary receipt is immutable, and final frozen-pin acceptance is
pending. Whole hint insertion, unique insertion, fixup and map-index remain
separate Source gates.
