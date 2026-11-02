# VFS direct Runtime failure retention

Root's VFS shutdown guard asks `GameVfsHost::requires_process_retention()` before
provider destruction. The facade previously returned only its own failed-call
flag or an open title block. Source144 and Runtime142 completion dispatch can
call the SAME Runtime directly, so their retained failure history can exist
while the facade flag remains clear.

The query now also asks the existing `failure_site()`. Its existing no-throw
implementation checks retained title/resource history and, after `core_ready_`,
the SAME Runtime request/completion and name-resolution failure channels. Core
readiness is set only after normal core initialization/publication; the Runtime
has already been retained. The old flag/title short-circuit arms stay intact.
No new provider operation, ownership, Native cleanup or retry policy is added.

This is a positive failure/entry observation. False is not callback last-use,
queue quiescence or disposal authority. Existing Root guards and whole-graph
retention on unresolved failure still apply. Source144 record adoption and
normal callback lifetime closure remain separate work; all GUI selectors stay
OFF. This adds zero Native/vendor or Original ABI/runtime credit.

The finite author plan and exact input pins are retained under ignored
`local/function_source_batch157/`. Registered build, independent review and
actual generated-code admission remain pending until separately recorded.
