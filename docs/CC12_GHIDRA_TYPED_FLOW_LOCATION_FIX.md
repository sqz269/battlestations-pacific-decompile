# CC12 typed-flow Windows project-location correction

The first post-rollout capture was rejected because Ghidra returned project
location `/C:/Users/sqz269/`, while Python's Windows path handling expected
`C:/Users/sqz269/`. The returned marker file was already the correct absolute
`C:\Users\sqz269\bsp.gpr`. This change corrects only the client-side interpretation
of `program.project.location`.

## SDK evidence and narrow correction

Installed Ghidra 12.0.4's `Project-src.zip` contains
`ghidra/framework/model/ProjectLocator.java`:

- Line 77 assigns `location = checkAbsolutePath(path)`.
- Lines 99–106 explicitly document prepending `/` to a Windows drive path and
  treating `c:/...` and `/c:/...` equivalently under Windows.
- Lines 113–136 replace backslashes, prepend the documented slash when needed,
  and retain a trailing slash; `getLocation()` returns that stored value at
  lines 179–180.

The client now removes exactly one initial `/` only when the **location** field
starts with `^/[A-Za-z]:/`. It then performs the existing Windows normalization
and exact comparison with the configured marker file's parent directory.
The actual marker-file validation is unchanged. No location is inferred from
configuration, and full project/program/language/image-base/query identity
remains mandatory. Arbitrary rooted paths, drive-relative paths, file URLs,
wrong drives, and wrong directories do not receive a new interpretation.

No plugin source/JAR, launcher, configuration, or metadata contract changed.

## Offline evidence

Root's original receipt remains unchanged:

```text
J:/PROG/battlestations-pacific-decompile-cc12_resume_integrator/local/cc12_typed_flow_rollout_primary/first_typed_capture.json
SHA-256: 6b746e0a885d171b6331ffd58ac97bf3e5690aafe41a1a61bb83103dca9e084e
```

The old validator reproduced the same location error on all four retained
responses. After correction, the pure validator accepts all four HTTP-200,
schema-1, complete responses at modification number **3**. Both saved identity
checks match configuration; query order, original raw-byte hashes, and unchanged
modification number across the batch also passed. Network access and client
construction were blocked during this replay. The original receipt's rejected
status was preserved; this separate result does not overwrite history.

The existing five focused tests pass. Minimal subcases cover the observed
location, wrong drive, wrong directory, arbitrary rooted/relative/URL forms, and
continued rejection of `/C:/...` in the actual marker-file field.
The machine-readable evidence, SDK hashes/excerpts, source preimages, and full
test output are in `reports/cc12_ghidra_typed_flow_location_fix.json`.

This worker made no new live metadata query, plugin change, restart, GPR
mutation, or Native opening. The mandatory initial `bsp.py brief` used its
existing health/identity probes. Metadata admission remains held for Root's
review, and the loaded JVM class CodeSource remains unattested.


Root reviewed the narrow diff and replayed the exact SDK entry/excerpts, froze
every worker pin, independently accepted the four original raw responses offline,
and reran the five focused tests successfully. A fresh named BSP capture accepts
four responses at modification3 with the actual configured GPR marker identity.
This verifies runtime getter/capture behavior; loaded CodeSource/bytecode identity
and Native body/ABI/repair admission remain distinct. No JAR or GPR change was
needed for the client correction, and the original rejected receipt is preserved.
