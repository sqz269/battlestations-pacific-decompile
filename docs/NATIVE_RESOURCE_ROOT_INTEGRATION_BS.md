# Native resource root integration (BS)

Addresses: BE42E0, BE4300, BE4620, BF0280, BF0510, BEA380, BEA700, BE9DF0,
BE9FC0 and their recorded EH support.

Nine complete ordinary bodies (1,030 bytes) now supply actual-storage scalar
and string reads, structured-root construction/publication, and node lifetime.
This closes concrete prerequisites of the B80720 cache-miss load path. It does
not reinterpret the existing typed reader as the original raw owner.

The important preserved behaviors are partial scalar pointer-byte seeds,
space-filled short strings, unsigned budget wrapping, path publication order,
and destruction without seeking unread payload. The new root terminal dispatch
borrows the existing stream dispatcher and pool. Native EH maps establish the
source cleanup order; original FH3/SEH/CRT identity remains separate.

Both new translation units pass strict Win32 compilation. One focused fixture
uses reconstructed memory-stream, pool, reader, root and reference services.
No new repository test cases were added. Its initial runs exposed fixture
errors: trying to map mutable0109DB64 through a read-only-data service and using
offsets for pointer-valued memory-stream fields. Those inputs were corrected;
the source routines were not changed to accommodate the fixture. An early
diagnostic output also allocated before fixed data-band reservation; reservation
now occurs first as in the proven fixture bootstrap.

Detailed evidence and boundaries are in `NATIVE_RESOURCE_STREAM_READS_BS.md`,
`NATIVE_RESOURCE_ROOT_OWNER_BS.md` and their JSON reports. The existing B80720
raw manager/factory/metric/root-dispatch composition, child traversal, parser
singletons/registration, queue update/worker and thread shutdown remain open.
No original executable or gameplay run is claimed for these fixtures.
