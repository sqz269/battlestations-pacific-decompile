# Derived application retirement: primary review

Accepted worker commit `186f5f04861f9940ad964257c5328343c6663cf3`. The explicit Source wrapper for `00BEA990` writes the volatile DWORD profile `00D68BC8` at the actual receiver, then passes that same receiver and context once to the accepted `00BEA8B0` retirement. It introduces no local guard, return value, storage release or activation. The original complete eleven-byte body establishes its profile store and physical tail jump; this new C++ interface does not establish original ECX, return or tail-call ABI.

Root verified two actual commit objects, nine finite immediate input blobs, four owned outputs, six physically retained helpers and the unchanged full old Source prefixes. The corrected 49-entry proof passed under an audit allowing one supplied ZIP read and one declared receipt write, with subsequent imports, directory reads, processes, network and mutations denied. The earlier draft with a missing report reference was not accepted; its correction changes proof closure, not C++ or the single worker build.

Root ran `./scripts/build.ps1` once: MSVC Win32 Release and all three registered existing checks passed (`reconstructed_math`, `native_math_differential`, `tool_tests`). These checks do not exercise this retirement path. A positive COFF review found the decorated Source definition: 203 text bytes and seven relocations. MSVC inlined the base retirement, preserving the derived profile store followed by the base profile store and genuine provider calls. This compiler evidence is not original binary equivalence.

The function remains dormant. It inherits the base contract's actual borrowed receiver, separate live storage cells, provider/domain/lifetime restrictions and qualified Source C++ cleanup, including the normal Leave failure policy. No outer `007379A0` or scalar-deleting owner, original FH3 frame, hardware unwind, allocator return, quiescence, startup or gameplay claim is admitted.

See `reports/cc12_application_derived_retirement_source_primary.json` and the physically retained proof under `local/cc12_application_derived_retirement_source_primary/`.
