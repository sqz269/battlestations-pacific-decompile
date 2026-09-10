# Preparation-job lifetime, pooled strings and alias-count integration

The actual eight-byte preparation job now allocates, publishes and registers
its secondary subobject with the shared lifetime manager. Its four destruction
entries preserve the original publication clear, secondary adjustment and
conditional free. Execution and scheduling remain separate dependencies.

`resize_native_string_header_0041dd40` operates on an existing eight-byte pooled
string header; `NativeString::resize_0041dd40` delegates to it. It preserves
the captured initial length and the later header reads across allocation and
release. The complete alias-list count guard at `004CE780` uses the reconstructed
legacy exception owner and SBO string to throw an owning host exception. Its
temporary cleanup begins only after counted string assignment succeeds.

Primary review verified 65 native spans against both the installed image and
the existing Ghidra program, plus 39 archived or shared evidence artifacts.
The source and headers match the three worker commits. A documentation error
in the count packet was corrected: `list<T> too long` contains sixteen visible
bytes; the original counted copy excludes the NUL at `00CE3908`. The source
already used the correct count.

The strict MSVC Win32 build and both existing CTests passed. Three existing
private fixtures were rebuilt and run against the integrated library:

- Preparation-job lifetime: captured secondary registration, after-unlock
  publication reload, actual shared-manager drain, primary free, and body/base
  table reset all matched original code.
- Pooled-string resize: 483 normalized words matched, including callback changes
  to the actual header and the declared native zero-byte-copy omission.
- Alias-count growth: 127 normalized ownership/count words matched, including
  the original factory and ThrowInfo with actual host CRT throw/catch/destruction.
  Owner-construction failure causes one additional host rethrow; this declared
  exception-ABI difference is excluded from the normalized trace.

Seven complete native bodies and one supporting unwind fragment were added to
the sharded reconstruction ledger. Three previously missing preparation-job
entries were defined; both returning-free continuations were restored. Eight
annotations preserve old names and comments, the project is saved, and all
eight affected exports are refreshed. `Unwind@00C65A80` retains its existing
funclet name. Full evidence is in
`reports/native_job_string_count_integration_audit.json`.

These are new C++ interfaces with explicit allocator, runtime and valid-storage
boundaries. The preparation-job fixture does not inject a native exception.
Queue construction still requires renderer shutdown, and queue destruction
executes queued commands before releasing its arrays. Neither full execution
nor a runnable, gameplay-validated reconstruction is established by this batch.
