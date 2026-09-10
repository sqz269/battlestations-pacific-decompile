# Native model and render group ownership

This batch connects actual 188h model pool slots to the native 4Ch render group
destructor and adds actual alias-node construction. Fourteen complete entries
are reconstructed: five live model type/bootstrap entries, five model lifetime
and geometry entries, two populated-group destruction entries, and two alias
string/node construction entries. Detailed ABI, native bytes, source hashes,
saved annotations and validation are in
`reports/native_model_group_integration_audit.json` and its component reports.

Model type getters and predicates observe the current shared descriptor rather
than copied tokens. The owner and canonical reference share the actual atomic
at model+04, node hierarchy, logical-release gate and current virtual profile.
An extra queued reference can keep the model alive after its group releases it;
the final reference destroys retained owners and returns its actual pool slot.
Group destruction reloads each later field after callbacks, clears ownership
fields after their calls, destroys the two source arrays in reverse order, and
returns the current name without erasing its header. Alias construction uses
the existing actual 10h node layout and shared string pool, including allocation
callbacks that mutate live source/destination headers and the native captured
node cleanup responsibility on copy failure.

The integrated strict MSVC Win32 build and existing two tests pass. Primary
reruns pass the model type alias/wrap sequence, 2,367 model lifetime observations,
815 observations across two populated groups, and 426 alias-node observations
with 356 compared string bytes. A separate group source/assembly/EH-map review
found no production issue. Native exception maps were inspected; host exception
checks are explicitly separate from native EH execution. These are new C++
interfaces with focused original-machine-code comparisons, not ABI-compatible
replacements or game validation. The previous installed renderer probe remains
the latest visual evidence; this storage/lifetime batch adds no visual claim.

The next contracts are the actual 44h command owner/initialization, checked
alias iteration/erase, and the remaining SBO string/length-error construction
needed by range insertion. The named `00408720` counted-string operation must
not be treated as complete merely from its Ghidra label. Shared queue/job owners,
texture/mesh owners, renderer initialization/reset and world/game startup remain
required before a runnable game reconstruction can be claimed.
