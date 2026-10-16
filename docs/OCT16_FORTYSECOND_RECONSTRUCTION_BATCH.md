# Whole input tree head allocation

Four ordinary entries 0069D1F0 / 0069D240 / 0069D290 / 0069D320 cover 229 Native
bytes and 68 instructions. They allocate 9C/2C/18/24 bytes through the accepted
current CRT service, perform three separate wrapping address checks and zero
stores at base/base+4/base+8, then unconditionally set color=1 before nil=0.
No payload, self-links, nil=1, count or parent publication is introduced.

Twenty-four primary checks passed. All 50 existing provider and parent Source
fingerprints are unchanged. The 194 Native bytes after allocation through the
nil store match emitted code without masking. All eight CODE sections match
the worker bytes and symbolic relocations: 868B/256 instructions, split equally
between four public bodies and unused private literal copies. The 24 relocations
are eight allocator calls, eight compiler GS checks and eight cookie references.
There is no padding, embedded table data or local EH. Compiler request/GS frames
and four unreferenced weak BSS bytes are separate toolchain details.

The actual Core append conflict was union-resolved. Four new name records retain
the established live Ghidra names and four whole function records are appended,
with no old records removed. The head unit and both prior pair units occur once
in Core. Four JSON and four Markdown transfers pass. Four locked annotations
save preimages and the project; four exports are refreshed.

The combined default MSVC Win32 build passes all three existing suites in
7.70s. No new tests, probes or owned-body execution. Current CRT allocation,
original heap/new-handler/exception identity, private frames, GS checks, registers,
flags, raw faults, FH3/SEH, runtime and gameplay remain qualified. The full settings
constructor and actual table loader remain separate unfinished work; their
existing Source and the admitted constructor prefix are preserved.
