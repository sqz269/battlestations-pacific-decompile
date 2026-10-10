# Exact Ghidra body and flow metadata: typed endpoint readiness

The audited GhidraMCP 5.6.0 typed endpoints do not expose the exact function
AddressSets and instruction flow preimages needed for the held atlas/CRT
listing gates. Local Java source and the installed SDK support a small read-only
endpoint in the existing `FunctionService`. No script capability needs enabling.
This packet specifies that endpoint and its BSP client; it implements or deploys
neither. Current Native listing/Source admission gates remain held.

Evidence is in `reports/cc12_ghidra_typed_flow_source_readiness.json`, based on main
`9416c324fcc763c988e83704c422fe86bfa03359`. It pins 17 plugin files, seven BSP files,
installed package metadata and selected installed-JAR bytecode. Only named
`bsp.py ghidra proto 00aeead0 --brief` metadata was queried live, with autostart
disabled and the usual `Client.verify` target checks. No Native bytes, body
listing, decompiler, data or handler was opened in this packet.

## Located source and installed package

The configured Codex `ghidra` bridge runs
`uv run --script J:\tools\ghidra-mcp\bridge_mcp_ghidra.py`.
The local repository is `J:/tools/ghidra-mcp`, HEAD
`0c0299d1588f17bdb457ff8591c98c0b5eae0a68`, with POM version 5.6.0, Java 21 and
Ghidra 12.0.4. Existing changes to `CHANGELOG.md`, `bridge_mcp_ghidra.py` and
`tests/unit/test_mcp_tools.py` were preserved. No reviewed Java service was dirty.

The user extension is
`C:/Users/sqz269/AppData/Roaming/ghidra/ghidra_12.0.4_PUBLIC/Extensions/GhidraMCP`.
Its `lib/GhidraMCP-5.6.0.jar` is 581,085 bytes, SHA-256
`3eb3999c930e844ddda1f74572b2d414cbacc645356593bf2cc44ccc038aad48`.
It exactly matches the embedded JAR in
`J:/tools/ghidra_12.0.4_PUBLIC/Extensions/Ghidra/GhidraMCP-5.6.0.zip`.
The extension properties, manifest and Maven metadata identify version 5.6.0.
These are disk-package facts; loaded JVM class origin and reproducible
source-to-JAR identity were not attested.

## Existing capability boundaries

| Route | Current source | What it establishes |
| --- | --- | --- |
| `get_function_by_address` | `core/FunctionService.java:520-551` | Name, signature, entry, body minimum and maximum. Installed bytecode confirms the min/max getters. It does not return ranges. |
| `analyze_control_flow` | `core/AnalysisService.java:950-1104` | `getBody().getNumAddresses()`, aggregate instruction/flow metrics and derived block details. Details truncate after 100 blocks. Counts, block entries and sizes are not an AddressSet. |
| `get_assembly_context` | `core/XrefCallGraphService.java:1188` | Exact listing starts/context, as already retained by the atlas packet. No required body/flow property fields. |
| `get_function_documentation` | `core/DocumentationHashService.java:312` onward | Documentation, normalized hash and decompiler-oriented work; it is not the missing exact property route. It was not invoked here. |
| Inline/named scripts | `core/ProgramScriptService.java:1260,1815` | Both are gated by the existing script opt-in. The prior atlas response says script execution is disabled; it is pinned historical evidence, not a new probe here. |
| Flow-clear/NoReturn setters | `core/FunctionService.java:1500-1660` | Their old-value getters are inside mutating services. They are not read endpoints. |

Current `AnnotationScanner` includes a generic POST `dry_run` wrapper that opens
a transaction, calls the service and rolls back. That still invokes mutating
code; it is not an attested mutation-free inspection path. It was not called,
and no claim about its runtime rollback behavior substitutes for property reads.

## Smallest concrete endpoint

Add one annotated **GET** method to
`J:/tools/ghidra-mcp/src/main/java/com/xebyte/core/FunctionService.java`:

```java
@McpTool(path = "/get_flow_metadata", method = "GET",
    description = "Read exact current function body and instruction flow metadata without analysis or mutation",
    category = "function")
public Response getFlowMetadata(
    @Param(value = "address", paramType = "address") String address,
    @Param(value = "program", defaultValue = "") String program,
    @Param(value = "max_ranges", defaultValue = "4096") int maxRanges)
```

This is a proposed signature, not compiled implementation. Resolve the explicit
program with `ServiceUtils.getProgramOrError` and address with `parseAddress`.
Use the existing `ThreadingStrategy.executeRead` path. Do not start a transaction
or invoke setters, analysis, disassembly, decompilation, save or scripts.
Bound and validate `max_ranges`; range overflow returns an explicit incomplete
error without a partial body presented as complete.

The response must preserve these distinct observations:

| Record | Required fields and getter semantics |
| --- | --- |
| Identity | Schema version, program path/language/image base, canonical query address with address-space identity. |
| Listing | Exact `getInstructionAt` existence/start/length, separate `getInstructionContaining` start, and `getCodeUnitAt` class/type/start/length. A missing instruction yields null flow properties. |
| Instruction override | `getFlowOverride().name()` independently of other flags. |
| Default flow | `getPrototype().getFlowType(getInstructionContext())`, with type text and call/jump/conditional/computed/terminal flags. |
| Effective flow | `getFlowType()` with the same separate flag fields. |
| Fallthrough | `getDefaultFallThrough()`, `getFallThrough()` and `isFallThroughOverridden()` independently; preserve nulls. |
| Targets | Complete `getDefaultFlows()` and `getFlows()` address arrays, with their origin distinguished. |
| Function identity | Separate `getFunctionAt(query)` and `getFunctionContaining(query)` results; never substitute the nearest function. Deduplicate matching records. |
| Exact body | Actual `Function.getBody().getAddressRanges(true)`, emitting **every** inclusive minimum/maximum pair with its address space, plus actual/returned range counts, address count and completion flag. No min/max span, block union or instruction coverage reconstruction. |
| Function flags | `hasNoReturn()`, `isThunk()`, plus direct `getThunkedFunction(false)` identity and its own flags. Preserve a null target and record `recursive=false`. |
| Direct call target metadata | For a default or effective call, label the union of default/effective target addresses and query `getFunctionAt(target)` for each. Record exact-entry presence, NoReturn and direct thunk metadata. Do not guess indirect runtime destinations or replace an interior target by its enclosing function. |

The full range iterator is the evidence, with counts used only to check complete
serialization. A false `exists` at a gap must remain false; no physical decoder
fallback, predicted fallthrough or synthetic instruction belongs in this API.
The proposed getters are present in the installed Ghidra 12.0.4 SDK sources.

Record `DomainObject.getModificationNumber()` before and after the read and reject
changed observations. BSP should also require consistent numbers across a batch.
Equal numbers are a consistency check, not a universal proof of atomicity.
Root's sole-writer/coordination discipline remains necessary before a repair;
the endpoint itself neither repairs nor authorizes changing any property.

## Router, bridge and BSP changes

Paths below are relative to `J:/tools/ghidra-mcp` unless marked BSP.

| File | Required change |
| --- | --- |
| `src/main/java/com/xebyte/core/FunctionService.java` | Add the read method and small serializers for exact address ranges, instruction properties and function/direct-thunk flags. |
| `src/main/java/com/xebyte/GhidraMCPPlugin.java:486-517` | Normally none. Current GUI `AnnotationScanner` already receives `FunctionService` and exposes its schema. |
| `src/main/java/com/xebyte/headless/GhidraMCPHeadlessServer.java:306-339` | Normally none. Headless scans the same service. No headless instance is started by this packet. |
| `src/main/java/com/xebyte/core/AnnotationScanner.java` | Normally none. GET/scalar parameters are supported; the POST dry-run wrapper is irrelevant to this new GET. |
| `src/main/java/com/xebyte/core/EndpointRegistry.java:461` | Legacy explicit registry exists; current audited GUI/headless paths use annotation scanning. Add parity only if retaining this legacy entry path, not to make current discovery work. |
| `bridge_mcp_ghidra.py:747-789,848-896,1058-1068` | Normally none. Existing `/mcp/schema` parser and GET dispatcher support the address/program/integer fields. Preserve the existing working changes. No handwritten wrapper is required. |
| `tests/endpoints.json` | Add the endpoint catalog entry during implementation. Follow that repository's applicable code-change validation then; no tests are run for this readiness document. |
| BSP `tools/bsp.py` | Add a distinct `ghidra typed-flow` parser/dispatch retaining full responses. |

Proposed BSP command:

```text
python tools/bsp.py ghidra typed-flow <address> [<address> ...] --output local/<packet>/typed_flow.json
```

Run `Client.verify` before and after each bounded batch, pass the configured
program explicitly, validate response identity/schema/completeness/null states,
and persist the full response/status before printing a capped summary. The CLI
must reject endpoint-unavailable, incomplete or changing observations. Do not
fall back through script execution, mutators or synthetic range reconstruction.
Existing `ghidra flow-properties` is a separately named script-only route;
`tools/ghidra_flow_properties.py` currently records the disabled-script response.

Root can implement and validate this small plugin/client packet from the pinned
scope. Packaging, replacement and any later restart remain separate concrete
steps; none occurred here. Once a separately reviewed deployment exposes the
endpoint, obtain actual exact metadata before any bounded, locked Native repair.
No script opt-in or host-configuration change is part of the proposed design.

No plugin/BSP tool source, configuration, GPR or original installation was changed.
No build, test, new exact-property result, Native Source admission, runtime or
gameplay credit is claimed by this readiness packet.

Primary review verified and retained all33 artifact pin occurrences and all
eight selected SDK entries/declaration sets. Seven BSP paths match the named
Git baseline and review revision. The scoped endpoint/client preparation may
proceed under the continuing reconstruction goal. It must remain read-only,
preserve existing plugin edits, and return exact current getters. Build/package
evidence does not substitute for later loaded-endpoint/runtime verification;
no listing or Native Source gate is admitted by this readiness review.
