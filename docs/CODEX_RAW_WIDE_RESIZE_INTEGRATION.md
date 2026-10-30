# Raw wide-string resize integration

The added 004C53E0 overload uses the caller's actual AA8/AA4/AA0 string-pool
cells. It closes a real exception boundary: getter failures propagate through
the raw overload, while the existing NativeStringStorage release interface
remains noexcept. No adapter, semantic header cast or substitute pool is added.

Primary review compared the complete addition with saved Native assembly.
Equal length returns before pointer/getter access. Zero length captures the old
pointer and size before its getter; nonzero resize resolves the allocation pool,
then reads current preserve operands. Old-pointer and size capture precede a
fresh return getter. Only normal return publishes pointer, length and WORD NUL.
The current AA4 gate is still read by the real BD1510 return service.

Raw overlap uses the established memmove convention, and a zero-byte copy call
is omitted after its operands are read. Wrapping sizes and invalid memory domains
remain qualified. Failure does not trigger invented rollback, free or cleanup.

The frozen worker revision is 41d78aea4c10431e5b3dbb0332bb82607d7daa19.
Its strict Win32 object check passes, and whole-file inverse projection verifies
the previous header/source text unchanged. The combined Release Win32 build at
9e0ef9cd5af2541f2bde0cc520b7e060e9c5f31c passes with both existing checks
(2/2, 11.92 seconds). Independent review accepts the exact added Source code
with no scoped blocker. Build checks do not prove Original ABI or runtime parity.

This supplies no parser, loaded locale, raw16 vector provider, startup or runtime
activation. All nine raw Text choices remain false and the frozen EMPTY candidate
retains its source-admission hold.
