# Concrete XLive pipe service

Addresses: 00A4C030, 00A5DE34, 00A5DE68, 00A5DEAA, 00A5DEE8, 00A5DF20, 00A5DF5E,
00A5DF96, 00A5DFCE, 00A5E055, 00A5E09E, 00A5F416.

`ReconstructedXLivePipeServices` implements the complete `XLiveIpcPipeHost` used
by recovered IPC creation and its worker. It composes the recovered protocol
lifecycle, checked original tables, transport, asynchronous I/O, and frame
capacity/encoding/decoding. No extra native function count is claimed.

The constructor requires protocol and frame views to share both the actual
F8B778 critical section and F8BADC encoded key. It also passes the same verified
table object to both layers. It opens no pipe and launches no worker itself.
Open and close invoke the real transport lifecycle. Reads and writes use the
same transport prefix and OVERLAPPED. Completion forwards the actual caller's
32-bit byte-count address across the MSVC Win32 DWORD boundary: no copied or
precleared output can hide intermediate API writes or alter aliasing.

Use the concrete Win32 protocol, frame, transport and I/O hosts for actual OS
operations, retaining all services, globals and tables through every worker
callback. The caller still supplies established allocation/stack preimages;
original loader-zero global storage is a separate, recovered case. This
composition does not establish unknown native heap/stack bytes, initialize
unrelated CRT globals, or insert a successful IPC substitute.

The native IPC shutdown still waits only 1000 ms before releasing resources,
and the recovered worker can terminate its process on an unexpected protocol
result. This composition preserves those behaviors. Complete application
startup and live asynchronous/gameplay behavior remain unvalidated.

Validation: the composed MSVC Win32 Release build passes `/W4 /WX` and both
existing CTests. The focused local composition fixture verifies canonical-key
rejection and both completion paths with an I/O observer: the observer receives
the exact caller output address, sees its prior value, writes 73, and returns
access denied. The caller retains 73 and the recovered state returns to 3.
This fixture opens no endpoint and supplies no invented protocol preimages.
