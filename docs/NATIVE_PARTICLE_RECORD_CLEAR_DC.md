# Native particle-record clear and candidate-graph correction (DC)

`004DCAA0..004DCAA7` is a complete eight-byte function: push zero, call
`004DC410`, return. The original interface takes the actual 0Ch record vector
in ECX and has no stack arguments. Its direct source wrapper forwards to the
complete CY raw resize provider, with the additional borrowed context in EDX.
It adds no storage, ownership, rollback, or exception policy.

The preceding `004DCA80..004DCA9F` is a distinct 32-byte wrapper. Its only
call is `004DCA97 -> 004DC2C0`; it never calls `004DC410`. There is no INT3
padding between its RET4 and the clear function. Before DC, Ghidra had no
function at 004DCAA0. The candidate graph's documented linear sweep continued
from 004DCA80 to the next defined start and attributed `004DCAA2 -> 004DC410`
to the preceding function. Refreshing the unchanged function boundaries could
not fix that attribution. This was a range-boundary problem, not stale native
bytes or a changed target executable.

DC defined the reviewed eight-byte range under the Ghidra write lock, after
checking live bytes against the installed executable. It preserves the original
4DCA80 body and does not reconstruct 4DC2C0 or its online-results caller 4DD7C0.
The earlier CY advisory incoming-caller list is corrected in its report; CY's
three verified outgoing calls, reconstructed behavior and sealed prior build
are unaffected. Candidate graph edges remain discovery hints until their
individual instructions and containing function ranges are checked.

No original caller or game execution of 004DCAA0 is established. A branch audit
found no existing address implementation in root, main or the pinned orch5
branches. This packet contributes one new eight-byte body, subject to that
explicit evidence scope. Original FH3 behavior, source caller reachability and
gameplay remain separate validation requirements.
