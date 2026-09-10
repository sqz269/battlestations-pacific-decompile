# Native render group destruction

`src/native_render_group_lifetime.cpp` reconstructs the complete actual-storage
entries `00B1D760..00B1D81B` and `00B1D8E0..00B1D8FE`. The new interfaces borrow
the existing 4Ch group storage, canonical native binding/model companions, and
the shared sized string pool. They are not native ABI replacements.

The destructor captures binding+00, decrements that actual owner's atomic+04,
dispatches its current virtual0 on zero, and then clears group+00. It next
reloads model+1C and model+20 separately, unlinks and logically releases each
through `00B6DFA0`, and clears each field after that call. A callback can change
a later field before its load. Models use `NativeModelReference` over the same
actual model prefix, node hierarchy and atomic+04; the resolver only identifies
that canonical companion. Logical release can preserve a model held by a queued
reference, or destroy and return its actual 188h pool slot before this call
resumes. A second aliased group slot observes the existing logical-release gate.

The source arrays at+30 then+24 are destroyed through `00B1D1D0`. Their counts
become zero; data and capacity fields remain stale after ordinary free. No
pointed-to entry is released. The destructor then reads current name+08 and
returns it with unsigned length+04 plus one, without clearing the name fields.
The unknown+3C..4B allocation tail is untouched. Scalar deletion performs that
entire destructor first, ordinary-frees iff flags bit0 is set, and returns the
original address even when it has been freed. Throwing destruction prevents
that outer free.

Original ABI: `00B1D760` takes ECX=group and returns with `RET`;
`00B1D8E0` takes ECX=group, one stack flags DWORD, returns the original address
in EAX, and uses `RET4`. The latter previously had an incorrect returning-free
continuation in Ghidra; the repair preserves its prior name/comments and records
the original metadata in `reports/native_model_group_defined_functions.json`.

EH state1 at `00DF4EC4` cleans the two reverse arrays through `00CBCA3B` and
then state0 cleans current pooled name through `00CBCA30`. It does not release
unvisited binding/model owners on unwind. Scoped cleanup in the reconstruction
preserves that distinction. For valid source-array headers, resize-to-zero and
ordinary free do not throw. Name return is also nonthrowing in the current
shared pool interface. No native SEH execution or malformed-header equivalence
is asserted.

Validation evidence is recorded in `reports/native_render_group_lifetime_audit.json`.
The focused comparison extends the existing model native fixture with populated
groups, actual terminal binding owners, a model surviving logical release,
terminal model pool return, two allocated source arrays, a current pooled name,
and scalar deletion. It does not establish a full native 44h render-command
implementation, full renderer initialization, startup, or game validation.
