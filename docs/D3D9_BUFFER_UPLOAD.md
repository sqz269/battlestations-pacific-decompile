# Buffer recreation and uploads

`src/d3d9_buffers.cpp` reconstructs vertex/index recreation (`00b492b0`/`00b49180`),
unlock (`00b4b9d0`/`00b4b820`), and the nonnull, non-diagnostic lock path
(`00b4ba00`/`00b4b850`). The latter two remain separately counted fragments.
Typed bindings project native fields `14h` flags, `18h` capacity, `1ch` cursor,
`20h` lock depth, `24h` dynamic lock count and `28h` COM pointer. Intrusive wrapper
ownership and registry integration are not implemented by this interface.

Creation decodes pool from the low nibble (0..3); pool zero also implies WRITEONLY.
Flag bit `10h` contributes usage 1. Nibble `f00h` maps values 100/200/300/400/500h
to usage bits 2/4000/40/100/80h. Nibble `f000h` equal to 1000h adds DYNAMIC;
`f0000h` equal to 10000h adds WRITEONLY. Flags 1000h therefore reproduce the startup
DEFAULT-pool, DYNAMIC|WRITEONLY buffers. Vertex FVF is zero; indices are INDEX16.
The new interface rejects invalid pool nibbles and reports creation failures,
preserving the previous COM owner. Native assembly passes the device-pointer bits
as the pool for invalid nibbles and does not check HRESULT. On success the port
preserves AddRef-new, Release-old, Release-temporary order.

Lock takes five stack arguments with RET 14h: byte count, extra offset, an unused
DWORD, output base-offset pointer and read-only byte. The new API omits the unused
argument. Non-dynamic buffers use NOSYSLOCK plus optional READONLY. Dynamic buffers
ignore read-only and use DISCARD at cursor zero, NOOVERWRITE thereafter. The API
offset is cursor+extra, but the returned base offset is cursor alone. The native
capacity diagnostic checks cursor+extra, not the requested size. Dynamic calls
increment cursor by count+extra and increment dynamic count and lock depth even
if the API fails. Unlock decrements depth only when the COM pointer exists, and
does so regardless of HRESULT. Recreation/release does not silently rewind cursor.

The port returns INVALIDCALL for null buffers and conditions that invoke the
unresolved diagnostic singleton `004c14c0`, and guards unsigned overflow in its
capacity precondition. It does not supply a replacement for native sentinel
`00f8d4b8`. The omitted branches prevent claiming whole lock-function coverage.

The existing real-device probe recreates both startup-sized buffers after its
surface-reset stage, writes two batches through the recovered lock paths, and
unlocks them. It observes DISCARD then NOOVERWRITE, vertex offsets 0/48 and index
offsets 0/6, final cursors 96/12, two dynamic locks each and zero lock depths.
This verifies API uploads and metadata, not rendered geometry or native-code
differential equivalence. The ordinary Win32 build and existing CTest pass; no
additional test cases were added. See `reports/d3d9_buffer_upload_validation.json`.

Renderer helper `00b1fd90` sets its dynamic-ready flag and invokes these two
recreation methods when resources are absent and the device is not marked lost.
Next: integrate buffer/surface lifecycle with the native reset state machine,
then recover stream/index binding and draw submission rather than substituting
a generic renderer for the game's pipeline.
