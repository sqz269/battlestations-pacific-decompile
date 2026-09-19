# Native online process services

R171 supplies the permanent pipe services needed by the actual online manager.
Ordinary `GameStartupHost` construction now runs the seven recovered pipe global
initializers and binds their real Win32 protocol, framing, transport and I/O
adapters to the native IPC worker. This is application composition; no new
native function body or online peer is claimed.

## Ownership and ordering

`GameNativeOnlineProcess` retains one `XLivePipeOriginalData`, one
`XLivePipeGlobalsOwner` and all service adapters at stable addresses until process
exit. The existing image reader validates the exact 200980-byte table digest at
D25D9C and 568-byte fixed-source digest at E12AC8 before any CRT mutation.
Only these read-only original data ranges are consumed by this owner.

The represented CRT order is CD6E16, CD6E2C, CD6E47, CD6E62, CD6E7D, CD6E98,
then CD6F03. Their verified table slots are CE33E0..CE33F4 and CE340C, after the
already bound settings and pending-entity subsets. Intervening unimplemented CRT
entries remain separate work. Six real `std::atexit` registrations retain their
actual return values; the native caller ignores failures. CD6F03 registers no
cleanup. The existing six cleanup bodies execute through those callbacks.

The source service constructs adapters only after the actual acquisition section
exists, shares its section/key/table objects across protocol and framing, then
binds the exact retained `NativeOnlineIpcRuntime` before a worker can start.
`initialize_once` returns cached statuses after success. Access before completion,
replacement of the image or stack policy, and a different worker-runtime address
are rejected. Startup is single-threaded. If initialization throws after CRT
mutation, the process retains the partial graph and calls `_Exit(1)`; this is an
explicit source failure policy, not native FH3 equivalence.

Original loader zeros apply only to the actual global storage. The production
protocol/framing adapters use the existing machine-byte capture boundary for
this process's real allocation and stack preimages. The two IPC stack DWORDs
are a separate explicit policy: ordinary startup supplies zero; the component
check supplies nonzero values and verifies they are retained. Neither policy
claims the original game's stack contents.

## Validation

- Strict MSVC Win32 build and all three existing CTests pass.
- One focused process opens the actual nonzero-mode named server pipe with real
  Win32 security, transport, protocol globals and current-process preimages.
  The recovered send/receive capacity calls return 80/80 bytes. Native close
  succeeds, and the borrowed stop event remains valid for its caller to close.
- The same process checks permanent owner/runtime identity, rejected replacement
  and early access, and a receipt registered before the six native callbacks
  runs after CRT cleanup. No projected owner or recording pipe host is used.
- 2096 live Ghidra/PE bytes agree, covering 15 existing body extents, CRT slots,
  fixed sources, table endpoints and the loader-zero global region. The full
  table SHA-256 is separately checked against the installed PE and by the
  runtime reader. Sixteen direct CALL/tail rows are verified independently.
- Exact build, component, application-attempt and integration results are pinned
  in `reports/native_online_process_r171.json` and immutable local archives.

## Remaining online dependency

The ordinary application still leaves the A40DF0 online-manager constructor
unbound at 73DC7C. Its native IPC path uses mode zero to open the parent's
PID-named pipe. A missing pipe takes the original process-exit path; it is not
equivalent to an empty notification queue. The required original peer remains
unidentified; see [Parent-process pipe dependency](XLIVE_PIPE_PEER_CONTRACT.md).

The component's nonzero-mode server exercises an existing recovered branch and
is not presented as a recovered game startup call or an online peer. No client
worker, encoded peer exchange, live XLive service, online profile callback or
gameplay was tested. Future application owners must close their IPC endpoints
before CRT cleanup destroys the locks. The existing 1000-ms timeout followed by
endpoint free is preserved and does not establish that every worker has joined.
