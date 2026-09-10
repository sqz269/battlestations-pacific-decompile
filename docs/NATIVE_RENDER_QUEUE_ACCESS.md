# Actual queue fields and append operations

Four complete native bodies now use actual retained queue, command and pointer
array storage. They do not supply queue construction, registration, renderer
dispatch, scheduling, or full command execution.

`00B1CB30` reads a queue configuration byte at `queue+4+index*8`, writes the
byte output, then reads the DWORD at `queue+8+index*8` and writes the word output.
The byte output may therefore change the source of the later word read. Its
return preserves that captured word's high 24 bits and replaces its low byte
with 1. There is no native bounds check or reconstructed Boolean-only return.

`00B1CB50` reads queue count at `+18`, then command-array data at `+14`, captures
the last command, and copies three source DWORDs to that command's `+1C/+20/+24`
in order. Overlap retains those sequential reads and writes. A valid nonempty
queue is required; this body neither creates nor retains a command.

`00B1CB80` and `00B1CC20` append a borrowed entry or command pointer. They call
the existing concrete specialization reserve only when count equals capacity,
using doubled DWORD capacity with a signed minimum-one decision. After reserve
they reload count and data, compute the destination with native DWORD arithmetic,
and read the source pointer only for a nonnull destination. Count is reloaded
and incremented after the optional store, including the native null-slot branch.
No extra retain, release, rollback or source snapshot is introduced. Existing
reserve valid-span and allocation-service requirements still apply.

`NativeRenderCommandQueueStorage` records the actual 34h layout without member
initializers or automatic ownership. Its existence is a layout contract, not a
completed queue lifetime. The implementation reuses the existing actual 0Ch
pointer-array header and leaves the previous typed projection available.

The four installed bodies were compared byte-for-byte with saved Ghidra and
reviewed against their complete assembly, including source/destination overlap
and partial-register return behavior. The strict MSVC Win32 build and existing
CTest checks pass (2/2). These routine helpers add no new tests; native execution
of these four entries has not been compared in a differential fixture. The
separate exception-owner fixture in the integration batch does not test them.
Binary ABI replacement and gameplay remain unvalidated.

Evidence: `reports/native_render_queue_access_audit.json` and
`reports/native_exception_queue_integration_audit.json`.
