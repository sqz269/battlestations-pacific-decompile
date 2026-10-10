# CC12 typed flow metadata preparation

Status: **implemented, SDK-built, unit-tested, locally packaged; not deployed or runtime-validated**.
This closes the source preparation described in
`CC12_GHIDRA_TYPED_FLOW_SOURCE_READINESS.md`. Native body/data/handler gates remain held.
The complete machine-readable pins are in
`reports/cc12_ghidra_typed_flow_implementation.json`; its ignored local artifacts
must be retained when the worker worktree is eventually removed.

## Sources and scope

- Plugin worktree: `J:/tools/ghidra-mcp-cc12-typed-flow`, branch
  `agent/bsp-typed-flow`, commit `3d8f2fc1e8f17643ca1894af4da7aa0d1aed0dfc`,
  based on exact `0c0299d1588f17bdb457ff8591c98c0b5eae0a68`.
- Only `FunctionService.java`, `tests/endpoints.json`, and the isolated
  `CHANGELOG.md` changed. The existing annotation registry supplies the route;
  no bridge, router, version, host configuration, or installed file changed.
- `J:/tools/ghidra-mcp` retains its three preexisting dirty files. All 17
  readiness source hashes replayed. Extra snapshots of its dirty changelog and
  unit-test file pin their final state; earlier hashes of those two files were
  not available, so byte-for-byte before/after proof is not claimed for them.
- BSP adds `tools/ghidra_typed_flow.py`, a named `ghidra typed-flow` dispatch,
  and one focused validation file. No reconstruction C++ changed.

## Endpoint and capture contract

`GET /get_flow_metadata` takes explicit `address`, optional `program`, and
`max_ranges` from 1 through 4096. Schema 1 reports the actual DomainFile path,
language, image base, default address space, and project locator name, location,
and absolute `.gpr` marker path. Addresses retain their address-space names.

The endpoint preserves four distinct nullable listing observations:
instruction-at, instruction-containing, code-unit-at, and code-unit-containing.
Instruction flow is null without an exact instruction. With one, it reports
the prototype's default flow, effective flow, FlowOverride, both fallthrough
addresses, the separate fallthrough override flag, and both target lists.
Only statically reported targets of default/effective call flow receive exact
`getFunctionAt` callee flags; no containing-function or indirect-target inference
is made. NoReturn and thunk status remain separate, and thunk lookup uses
`getThunkedFunction(false)` without inheriting the target's NoReturn flag.

Exact/containing queried functions include every real `AddressRange`, inclusive
endpoints, range lengths, counts, and total address count. A bounding interval
never substitutes for the range set. Overflow, count changes, exceptions, or a
changed modification number return an error with `complete: false`, not a
partially admitted body. Getters run through `executeRead`; this new path contains
no transaction, setter, script, analysis, disassembly, decompiler, or save call.

BSP dispatches before its ordinary auto-starting client path. It uses
`Client.verify()` before and after the batch, saves each complete raw HTTP body,
base64 bytes, SHA-256, and status before validation, and rejects malformed or
unavailable responses, contradictory nulls, incomplete ranges, wrong project
marker/program/query identity, and modification changes within or across
responses. Config overrides and output must be JSON files under `local/`.
Maximum batch size is 64 explicit 32-bit addresses. There is no fallback.

After separate Root review and deployment, the command template is:

```text
python tools/bsp.py ghidra typed-flow <Root-approved-addresses> --output local/typed_flow_capture.json
```

This command has not been run against any loaded program. The endpoint is a
current metadata observation, not proof of correct analysis, original ABI,
runtime control flow, Native body admission, or game behavior.

## Validation and package comparison

- `mvn -B clean package assembly:single -DskipTests`: passed with Java 21.0.9
  and Ghidra 12.0.4 SDK. All 14 compile JARs and 12 SDK source declarations are
  retained, including runtime project-locator getters.
- Plugin's required Python unit suite: **294 passed, 1 skipped** for absent
  `dll_exports/`. An isolated `uv` environment supplied the declared test
  dependencies after the host pytest launcher rejected missing `pytest-cov`.
  Test-only `GHIDRA_MCP_URL=http://127.0.0.1:9` avoided the live server.
- Focused BSP tests: **5 passed**, covering discontiguous bodies and gap nulls,
  independent flow/thunk flags, wrong/incomplete metadata, raw preservation,
  cross-batch modification changes, and dispatch without scripts or autostart.
  These use mocked metadata and HTTP, not a loaded endpoint.
- `git diff --check` passed. No C++ build is required for these Python/Java/docs
  changes. Java endpoint runtime behavior and loaded class origin remain untested.

The installed JAR has 103 entries including directories. A fresh build of the
exact pristine plugin base reproduced every installed class and resource except
`com/xebyte/version.properties`, whose generated timestamp differs. The new full
build differs from installed in exactly three entries: `FunctionService.class`,
`FunctionService$PrototypeResult.class`, and `version.properties`. The companion's
`javap -p -c -s` output is identical; only its debug line-number table changed.
There are no added or removed entries. Existing public method descriptors remain;
private synthetic lambda suffixes shift because the new endpoint adds a lambda.

A second candidate overlays only the newly compiled `FunctionService.class` in
a copy of the original JAR. All other **102 entries** retain their original
uncompressed byte hashes, including the existing companion, manifest, and all
resources. No new companion class is needed. The installed JAR has no signature
entries. ZIP entry metadata/order/comment are preserved while the container is
reserialized; compressed byte-range identity is not asserted. Full entry hashes,
public/private signatures, code listings, and debug differences are retained in
`frozen/package_comparison/jar_entry_comparison.json`.

| Candidate | Bytes | SHA-256 |
| --- | ---: | --- |
| Installed rollback JAR | 581085 | `3eb3999c930e844ddda1f74572b2d414cbacc645356593bf2cc44ccc038aad48` |
| Full rebuilt JAR | 586004 | `a1e473968541f7030c6efc36b19aae25b699726575523034da7349b046d7b907` |
| Full extension ZIP | 578036 | `4d35ef651c6ac7a6b9bc425d2e5c03ec2816ec8a6c060114e797ca260a8f4b7e` |
| Original-resource overlay JAR | 586003 | `05e5d95e7bb08eae2079ea38fba4d36611498b48462ed1939dbcdc546a24205d` |

## Reviewable install, restart, and rollback plan — not executed

Artifacts are frozen under
`J:/PROG/battlestations-pacific-decompile-cc12_atlas_item_listing/local/cc12_ghidra_typed_flow_implementation/frozen/`.
The complete original extension is in `installed_before/`; the full package is
in `new_package/`; the optional minimal JAR is
`package_comparison/GhidraMCP-5.6.0-typed-flow-overlay.jar`.

1. Root reviews this source commit, entry comparison, exact candidate hash, and
   the saved original files. Choose one candidate explicitly. The overlay keeps
   the old build-timestamp resource, so a version label cannot attest its loading.
2. Root coordinates the sole GPR writer, saves the active
   `C:/Users/sqz269/bsp.gpr` project/program, and closes Ghidra cleanly. Confirm the
   owning JVM has exited. Do not force-kill or discard unsaved analysis. Back up
   the closed project according to the existing preservation workflow.
3. Rehash the installed files against `rollback_files` before any replacement.
   The current extension root is
   `C:/Users/sqz269/AppData/Roaming/ghidra/ghidra_12.0.4_PUBLIC/Extensions/GhidraMCP`.
   Preserve another dated copy. Abort on an unexpected preimage.
4. For the overlay, replace only
   `lib/GhidraMCP-5.6.0.jar` under that root with the selected frozen overlay;
   preserve `extension.properties` and `Module.manifest`. For a full package,
   the ZIP contains exactly those three files and two directory entries; map its
   `GhidraMCP/` contents to the same extension root. Do not copy build SDK JARs,
   source checkout files, or a Python bridge. Verify every selected output hash.
5. Root starts the same Ghidra installation and opens the saved project and
   `/battlestationspacific.exe`. Preserve startup logs, verify project/program
   identity, and separately capture the loaded plugin origin if available. A
   disk hash alone does not attest which class the JVM loaded.
6. Root authorizes a bounded first capture. Retain the raw response and validation
   report. Stop on missing endpoint, schema/identity/modification error, or any
   incomplete range set. Do not switch to scripts, mutate analysis, or advance
   Native body/data/handler gates based on this package preparation.
7. If startup or validation fails, save/close through the same Root coordination,
   restore all three files from `installed_before/` to their original relative
   locations, verify their original hashes, and restart the same saved project.
   Rollback does not involve deleting or reverting the GPR or its saved analysis.

Both candidate packages and all rollback files remain local and undeployed.
