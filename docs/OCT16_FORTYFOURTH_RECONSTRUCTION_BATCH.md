# Checked DWORD copy helper

Whole ordinary 00440C70 covers 48 Native bytes and 18 instructions. Four
read-only producer CALLs cover another 20 bytes without promoting either
parent. The established STL_Copy_00440c70 name is preserved.

The helper uses wrapped DWORD difference and exact SAR2 bits, captures the
wrapped output pointer before any SDK call, and skips only a zero SAR result.
Negative nonzero counts still invoke the accepted current SDK secure-move
effect with unsigned wrapped bytes. It ignores errno and returns the captured
pointer. Original RET0Ch matches the new stdcall arity but does not prove
binary or private argument/frame identity. Original BF67A7 is not reconstructed.

All 20 primary checks pass and all 64 provider fingerprints remain unchanged.
The cited 15-check preliminary receipt is immutable; the final frozen seal is
separate. Both CODE sections and all symbolic relocations match the worker:
191B/70 instructions, split into public 97B/37 and separate SDK 94B/33.
The owned public body does not call the separate SDK helper. ESI captures
the result before all SDK error/invalid-handler/move effects and supplies both
returns. Generic SDK capacity checks include ERANGE; the equal-size/count
public path excludes that arm. All eight relocations are three errno imports,
three invalid-parameter imports and two current memmove calls. Noncode is
47B directives, 212B compiler debug and 40B checksums; no EH, BSS or padding.

The Source merge required no conflict resolution. One name record and one
whole-function record append without removing previous records. Core registers
the unit once. Five JSON and five Markdown transfer claims pass. One locked
annotation saves the original name/comment preimage and project; one forced
export retains all 48 bytes and 18 instruction starts.

The combined default MSVC Win32 build after Main ef70f95 passes all three
existing suites in 7.70s. They do not execute this helper. No new tests or
probes were added. Current SDK error/move behavior remains an admitted external
boundary; Original CRT/heap/validation/feature/fault/FH3/SEH, private aliases,
registers/flags, runtime and gameplay remain unproved. The original secure
wrapper and larger table-loader/vector bodies remain independent unfinished work.
