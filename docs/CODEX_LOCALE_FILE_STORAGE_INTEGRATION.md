# One-file locale scanner integration

This packet composes the actual borrowed-filename fragment `[00AA00B0,00AA0356)`
with the existing VFS routes and completed-record storage helper. It supplies an
ordinary Source interface with a caller-retained, nonmoving attempt. It does not
supply the full AA0020 parser, input/vector custody, raw16 loading, a startup
caller, Original ABI compatibility or runtime evidence.

Primary review covered the complete header/body and the saved Native assembly:
mode1 reads before its unsigned64 position>=length check and before testing the
space; mode2 has no added EOF check; mode3 reads through the captured word entry
and computes its destination from the current index after the call. A real
delimiter creates a stable record node before record effects. After a normal
record, only exact full64 position==length exits.

The frame retains uninitialized evidenced scratch, live count/mode/index DWORDs,
actual streams and an append-only list of typed record attempts. Explicit raw
field compiler fences and the record's private normal-result receipt were added
during review. Caught reentry poisons the attempt and prevents later healthy
completion. Source metadata failures do not widen Native selected cleanup.

The normal footer captures the genuine borrowed decrement cell once, returns
converted then saved-original references through the same callable, and reads
each current table/slot0 only after that decrement returns zero. It performs no
alias suppression, stream-slot clearing, RAII return or retry. Existing matched
VFS zero-reference routes require no new provider. No selected stream cleanup
exists on escape; unresolved graph and provider/code lifetimes remain retained.

Filename backing is needed during the synchronous admitted scan/open. After
return or escape it is historical identity only: frame status, destruction and
replay rejection do not follow it, so outer Native vector cleanup is permitted.
Record history likewise must not follow ended keys or overwritten buffer data.

The Game translation unit is registered once with the existing deferred
`bsp_game` target_sources pattern in root CMakeLists. Startup registry and all
nine raw Text activation flags remain unchanged. Final exact pins, independent
review and canonical build evidence are recorded in
`reports/codex_locale_file_storage_integration.json`.

Exact worker commit `4006752d04b8c85daea84841937d7a3ed0fd2367` passed primary
and independent full Source review, a strict x86 object compile (014C), and
preservation checks for all 14 existing providers. Registration commit
`92344c10355fdf27914d5dcfb4a7288da6159e94` passed the standard Release Win32
build and both existing checks (9.36s). No game or new raw body was invoked.
