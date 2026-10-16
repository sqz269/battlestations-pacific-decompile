# Raw preset key cleanup and Core dependency integration

Whole entry0069E090 covers29Nativebytes and12instructions. It directly reuses
the published REAL RawCtx overload of0041DD20, justified by whole29B equality
after only two E8 operand fields (eight bytes); the other21bytes are exact.
Native69E090 still has getter/return calls, not a fabricated41DD20 call.
Capture data4 before conditional CURRENT length0+1, resolve the getter before
every nonnull block return and leave the header untouched. The earlier
generic NativeStringStorage/noexcept API remains separate and unchanged.
Reversed ledger offset prose is corrected with its old text preserved.

Fourteen primary checks passed. Final19B/8instruction compiler output has one
actual relocation to the correct RawCtx overload, no EH/padding/data reference.
An unused4B BSS section has no file payload. Code and actual relocations match
the reviewed worker. Five JSON and five Markdown transfer claims passed:
two owned calls and three read-only reference/origin transfers. One saved
annotation preserves preimages and one export is refreshed.

The same batch registers existing band_0067_cc13.cpp in Core, retaining its
game/conditional fixture registrations and changing no C++ implementation.
The new raw owner exposed an unresolved Core frontend digit-spinner helper.
Its exact symbol is now defined in the actual Core object/archive linker
index, with exactly one compile item per Core/game target. That worker's
same default build now passes3/3; eight registration checks passed. The raw
owner itself remains separately reviewed and is not published by this batch.

The final combined MSVC Win32 build passes all three existing suites in
8.08s. They cover older math/tool behavior; no new tests, probes or
Native/game execution. SourceRET4/void versus NativeplainRET/conditionalEAX,
pool/manager ownership, private frames, OriginalCRT/FH3/SEH/fault, binaryABI
and game behavior remain unproved. No C7F100 logical action or parent is
promoted. Exact pins, hashes and receipts are in the companion JSON.
