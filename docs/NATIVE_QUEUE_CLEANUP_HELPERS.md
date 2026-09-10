# Actual queue cleanup helpers

Two complete small native entries now have concrete C++ implementations.
`00B1D590..00B1D5A7` (23 bytes) receives the actual command-pointer array in
ECX, calls the existing complete `00B1CC80` resize with count zero, reloads
the array's current data pointer, ordinary-frees it and returns. It does not
destroy pointed commands or clear the pointer/capacity after free. The supplied
header and shared lifetime allocation domain must remain valid through calls.

`00B1C3C0..00B1C3D1` (17 bytes) receives the actual base address in ECX.
It unconditionally clears singleton publication `00F8D440`, then writes
native table word `00CE3818` at that base and returns. Its new C++ interface
borrows the actual publication slot. There is no identity check, resource
cleanup, unregister, execution or free in this entry.

The complete queue destructor is still separate: it executes queued commands
through `00B1EBE0` before releasing its members. Calling these two helpers
does not implement that behavior. The queue's row array has its own lifetime
family; these helpers make no claim about row destruction.

Complete installed assembly and live Ghidra bytes were reviewed for both
entries. The missing continuation after the pointer-array free at `00B1D59D`
was restored in Ghidra with prior metadata preserved. Evidence and validation
are recorded in `reports/native_queue_cleanup_helpers_audit.json`. The new
interfaces have host calling conventions and allocator boundaries; they are
not original binary replacements or gameplay validation.
