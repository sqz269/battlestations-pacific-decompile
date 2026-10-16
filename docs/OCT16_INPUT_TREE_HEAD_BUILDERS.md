# Input tree head builders

Addresses: 0069D1F0, 0069D240, 0069D290, 0069D320.

All four ordinary bodies are complete within the accepted current-CRT allocation domain. The published STL names remain unchanged; the C++ labels are descriptive hypotheses.

| Address | Native bytes / starts | Allocation | Color / nil | Coverage |
|---|---:|---:|---:|---|
| 0069d1f0 | 64 / 17 | 0x9C | +0x98 / +0x99 | complete ordinary |
| 0069d240 | 55 / 17 | 0x2C | +0x28 / +0x29 | complete ordinary |
| 0069d290 | 55 / 17 | 0x18 | +0x14 / +0x15 | complete ordinary |
| 0069d320 | 55 / 17 | 0x24 | +0x20 / +0x21 | complete ordinary |

The Native bodies consume no inputs: incoming ECX and EDX are unused, there are no stacked inputs, plain RET returns the allocation base in EAX. The new public interfaces are zero-input cdecl pointer-return functions. They use a fixed private family instantiated with four literal sizes and offsets.

Each body calls the actual accepted singleton_lifetime_allocate service with SingletonAllocationKind::object and identical native_bytes/host_bytes. It captures the result, independently checks base, wrapping DWORD base+4 and wrapping base+8 before each zero DWORD store, then unconditionally writes color=1 followed by nil=0. A null result skips only the first store: derived addresses 4/8 still participate. No blanket null guard, payload initialization, selflink, count write or nil=1 is added.

The accepted allocator at operator_new at 00BF681B uses current std::malloc, _callnewh retry and std::bad_alloc. This is an explicit current-CRT boundary; Original heap, new-handler, exception identity and fault behavior are unproved. There is no local EH, cleanup, rollback or RawPool context.

Actual Native transfers (owned scope only):

| Function | Site | Native callee | Bound Source service |
|---|---|---|---|
| 0069d1f0 | 0069d1f5 CALL 00BF681B | operator_new | singleton_lifetime_allocate |
| 0069d240 | 0069d242 CALL 00BF681B | operator_new | singleton_lifetime_allocate |
| 0069d290 | 0069d292 CALL 00BF681B | operator_new | singleton_lifetime_allocate |
| 0069d320 | 0069d322 CALL 00BF681B | operator_new | singleton_lifetime_allocate |

Fresh typed project/program verification used C:/Users/sqz269/bsp.gpr and /battlestationspacific.exe. All 229 owned PE/live bytes, 68 current starts and four direct CALL targets match the frozen readiness. Fifty consumed/reference Source/header LF fingerprints still match published Main 9db37215372a19fc81da33accdcdd9d827634715.

The default ./scripts/build.ps1 completed with exit0; all three existing suites passed in 7.45s. Those suites do not execute the new head builders. No new tests, probes, allocation-failure fixtures or Native execution were added.

The actual CMake I386 object contains 868 code bytes and 256 instructions in eight sections: four public bodies 434B/128 instructions plus four private literal copies 434B/128. Every public/private pair has identical full bytes and symbolic relocations. All raw operations after allocation through the nil store match the corresponding Native bytes exactly. All 24 relocations are accounted for: eight actual allocator REL32, eight compiler GS-check REL32 and eight cookie DIR32. No padding, embedded branch data or local EH metadata is present.

Noncode accounting: 305B linker directives, 212B compiler debug and 96B checksum metadata. A 4B toolchain weak Avx2 BSS cell is uninitialized and has no stored raw bytes. It is not a reconstructed game global. Compiler request frames, GS-cookie SDK calls, private/register/flag aliases and pre-link COMDAT copies are qualified Source details, not an Original binary or heap ABI claim.

Existing startup initialize_tree, lifetime, tables, constructor Source and all frozen readiness artifacts remain unchanged. These four entries initialize only the recovered link/color/nil storage. The actual constructor, its cleanup actions and table loader remain separate; this packet makes no whole-constructor, Original FH3/SEH/fault, runtime or gameplay claim.
