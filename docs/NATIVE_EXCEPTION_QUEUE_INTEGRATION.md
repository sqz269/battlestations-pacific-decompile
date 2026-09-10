# Legacy exception owners and queue helpers

Five complete legacy exception-owner functions and four complete queue helpers
are registered in the main Win32 library. The exception owners use the actual
40-byte storage, compose the previously recovered small-buffer string, and
preserve nullable base-message allocation and native cleanup scopes. The queue
helpers access actual configuration and last-command fields and append borrowed
entry/command pointers through existing concrete array reserve operations.

Primary review retained both worker exception source files unchanged and
rechecked their 34 installed code/data spans. The existing native owner fixture
passes against the integrated library with 463 matching observation words,
including copy failure before and after state-zero activation, base-only cleanup,
returning allocator/validation effects and scalar deletion. Original owner code
and FuncInfo use declared host CRT and exception-dispatch services. This is not
compatibility with the original exception transport or modern standard-library
exception layout.

The four queue bodies were checked against all 181 installed instruction bytes
and their complete assembly. Their validation is source/evidence review and
strict compilation, not the exception fixture. The strict MSVC Win32 build and
existing CTest checks pass (2/2); no new test suite was added.

The missing `00411940` copy entry was defined from its verified 25 bytes. Three
returning-free continuations in the two destructor bodies were restored with
old comments journaled and preserved. Nine entries have saved evidence
annotations and fresh exports. The earlier alias-clear interface is retained
inside its upgraded ledger record rather than being counted twice.

The independent alias-assignment discovery was reviewed against its installed
spans, archived source artifacts and pinned subsequent integrations. Its next
two disjoint implementation dependencies are actual pooled-name resize and the
list count/error factory. Preparation-job lifetime is progressing separately.
Native range insertion, record assignment, full command execution and a runnable
game remain unfinished. Evidence: `reports/native_exception_queue_integration_audit.json`.
