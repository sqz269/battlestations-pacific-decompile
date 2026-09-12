#pragma once

struct _GUID;

namespace bsp {

// Source storage adapter for NativeInputEnumerationCalls::append_guid_00a97fa0.
// The actual10h header is backend+E4: preserve +0, use begin/end/capacity-end
// at +4/+8/+C. Its allocation remains owned by the existing A97B00 destructor.
// No shadow vector, additional owner, or original STL entry/ABI is supplied.
//
// MSVC Win32; complete GUID elements and valid representable pointer ranges
// from singleton_lifetime_allocate/free are required. The source may be an
// external GUID or an existing complete element. Growth captures its bytes
// before allocation. Partial overlaps, arbitrary malformed storage, and
// concurrent/reentrant header mutation during allocation are outside scope.
//
// Uses the real returning-capable source CRT invalid-parameter service and
// existing source length-error transport. Their handler/heap/EH identities
// differ from the original static CRT. See docs/NATIVE_INPUT_GUID_STORAGE.md.
void append_input_guid_storage(void* actual_10h_header, const _GUID& value);

} // namespace bsp
