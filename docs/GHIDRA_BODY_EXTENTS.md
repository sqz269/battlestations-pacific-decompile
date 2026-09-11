# Stored cleanup function-body extents

Addresses: 0086fe20, 00cdeb00, 00870d00, 0086e8a0.

These four cleanup tails are already decoded, their bytes match the executable,
and their erroneous free-call flow overrides were cleared by earlier packets.
That does not mean the tails belong to Ghidra's stored function bodies.

| Function | First missing body byte | Verified end exclusive |
|---|---|---|
|0086FE20 manager destructor|0086FE98|0086FEBF|
|00CDEB00 name-index atexit|00CDEB2C|00CDEB40|
|00870D00 definition destructor|00870D9F|00870DC2|
|0086E8A0 map iterator erase|0086EB11|0086EB47|

On 2026-09-11, the bridge advertised `run_script_inline` and `run_ghidra_script`
as callable tools. A read-only Java probe, under the coordination lock after
project/program verification, was rejected by the actual execution gate:
`Script execution disabled. Set GHIDRA_MCP_ALLOW_SCRIPTS=1 ... to enable.`
The full response and current function-body readbacks are preserved in
`reports/effect_body_extents.json`. No script executed, no body changed, and the
shared server configuration was not changed or restarted.

A future supported repair must preserve old body ranges, check disk/live bytes,
validate decoded instructions and the final RET, refuse overlap with any other
function, and add only the explicit tail under a lease/write lock. Save and
read back the bodies, refresh exports, and force the function snapshot because
function count does not change. Do not treat this capability rejection as a
failure of the existing reconstruction, which uses the verified tail assembly.
