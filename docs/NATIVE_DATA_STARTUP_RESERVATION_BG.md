# Numeric native-data reservation timing (BG)

The source `GameNativeReadOnlyData` reserves only requested 64 KiB bands of
`CE2000..E07B23`, then loads and hashes the complete supported original PE,
commits selected pages, copies verified `.rdata`, and changes them to read-only.
On failure it releases only reservations it acquired. Its initial reservation
is nevertheless inside its constructor, after Windows loader and dynamic CRT
initialization. The reconstructed graph fixture is a fixed Win32 image at
`30000000`, size `57000`, with an executable TLS directory and embedded
manifest. That probe image does not cover `D10000`. The original game image at
`400000`, size `E2F000`, already contains the original `.rdata` there.

The published BD graph diagnostic recorded 21 complete runs followed by a
`D10000` collision: `MEM_PRIVATE`, committed read/write, allocation base
`D10000`, 36,864-byte queried region, separate from the reported default
process heap `790000`. A manifested child probe derived from the graph fixture
samples `D10000` from an executable TLS callback, a C++ static constructor and
main.
In the final 160 child starts, 11 had `D10000` committed read/write at **all three**
stages. In each collision `GetProcessHeaps` listed `D10000` as one of two heap
handles, distinct from `GetProcessHeap()`. The queried region in these starts
was 20,480 bytes. This establishes a secondary process heap before executable
TLS; its particular creator DLL/call stack is not identified. No assumption
about C++ static initialization or `main` being early enough is valid. Earlier
120- and 160-child timing cohorts each recorded three collisions, including
three in the 160-child TLS cohort. Those earlier cohort binaries were not
retained; the final executable and its 160-start log are hash-pinned. These
finite samples show an intermittent collision; they do not estimate launch
reliability.

An ignored launcher prototype created each probe child suspended. In 120
starts, `VirtualQueryEx` found `CF0000`, `D10000`, `D50000` and `D60000` all
`MEM_FREE` before resume. The launcher reserved each complete band in that
child with fixed-address `VirtualAllocEx(MEM_RESERVE,PAGE_NOACCESS)`; after
resume the child observed `D10000` as reserved in its TLS callback and all four
as individually reserved at main. All 120 children returned zero. These runs
prove an earlier timing point and retained ownership in these children, not a
reliability guarantee across environments or arbitrarily many launches.

One separate suspended child had an intentionally committed read/write block
placed at `D10000` by the prototype. The launcher reserved `CF0000`, got
Windows error 487 on `D10000`, released only its own `CF0000` reservation,
verified `CF0000` free and `D10000` still committed, then resumed the child to
a clean exit. This rollback was reproduced in the resumed packet. No allocation
belonging to an unrelated process was released.

The feasible integration boundary is a controlled `CreateProcessW` suspended
bootstrap that reserves **all** required bands before the child loader runs,
then passes an explicit ownership and handoff contract to the child's native-data
service. The current `GameNativeReadOnlyData` constructor cannot use this
contract: its own `VirtualAlloc(MEM_RESERVE)` rejects an already reserved band.
Adoption must be a coordinated production change that proves the reservation
origin, validates the complete band set, owns commit/protection and releases
only transferred reservations on failure or destruction. The launcher must
also handle failed reservations and child termination before transferring
ownership. Normal direct launch, TLS callbacks and static constructors remain
vulnerable to the preexisting secondary-heap allocation. The prototype did
not run the complete VFS graph under an adopted reservation, and no gameplay
outcome is claimed.

The ignored source, manifested executable, sample logs, launcher, injected
failure probe, audit script and exact SHA-256 hashes are recorded in
`reports/native_data_startup_reservation_bg.json`. No production mapper,
entrypoint, linker setting or original game process was changed.
