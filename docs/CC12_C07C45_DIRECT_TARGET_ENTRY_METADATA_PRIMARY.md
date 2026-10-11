# C07C45 entry: Root review

Worker commit 659c8350641903452abdd17b47dbc402246e4838 retains the original metadata, byte and producer streams. Root independently admits and compares only `8B 4D F0` at C07C45 through C07C47: a 32-bit read through current SS at `(EBP-16) mod 2^32` into ECX. ESP, EBP and EFLAGS remain unchanged on successful normal completion. This explicit address does not require an SS.B=1 stack predicate. No frame, local-variable, exception, provider or ABI interpretation follows.

Root's initial metadata gate passed and its 179 immutable copies are unchanged. Final review passed on its first execution: 20 actual current Git objects, ten LF/index/blob checks and all 536 member pins. The first guarded replay exactly matches both worker receipts and records one archive read, one helper execution, one exclusive receipt write and zero denied events.

One copied producer path failed before manual output or Capstone import. The original producer, preexecution pin, exit result and complete stdout/stderr remain intact; a separate minimal path retry wrote the manual artifact before the first fresh full-field comparison, which passed. This producer failure is separate from decoder, final-review and strict-replay counts, all zero. The worker's one later display truncation is separately retained with its unavailable original-output limit. Prior C1737B IMUL SF evidence remains prior history.

The original PRIMARY publication producer also failed before any public writes because it assumed singular producer fields in a record containing plural fields. Its complete original streams and pins remain; a separate minimal schema retry publishes this report without rerunning decoder or replay evidence.

The remaining 17 owner bytes, C07C48, C07C59 and referents remain unqueried. Source bodies, new builds/tests/probes, runtime and GPR writes were not used. See the paired PRIMARY report for exact pins, histories and scope.
