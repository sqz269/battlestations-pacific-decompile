#pragma once

namespace bsp {

class NativeStringStorage;
struct NativeStringRawPoolContext;

// Full 00BEE690, originally ECX actual eight-byte length/data header, RET.
// Lowercases ASCII, changes counted backslashes to slashes, trims only byte 20h,
// constructs a pooled substring, then copies it back through actual fields.
// Allocation callbacks can change the source, result, and destination headers.
// Cleanup is armed only after substring construction returns. No stable native
// result is established in EAX, so this interface returns void.
void normalize_native_resource_path_header_00bee690(void* actual_header,
    NativeStringStorage& storage);

// Full 00BEE780, originally ECX output, EDX source, EAX output, plain RET.
// Clears both output fields before the identity branch, without releasing old
// storage, then copies and normalizes. Only successful initial construction arms
// output cleanup: a copy failure keeps partial output; a normalization failure
// releases the current output storage without clearing its header.
// Both APIs use actual raw headers, with no null-header guard or NativeString
// object overlay. NativeStringStorage remains an explicit host pool boundary.
// Evidence, original ABI, and exception limits: docs/NATIVE_POOLED_RESOURCE_PATH.md.
void* copy_construct_native_resource_path_header_00bee780(void* actual_output_header,
    const void* actual_source_header, NativeStringStorage& storage);


// Genuine raw publication variants. Current pool getter failures propagate;
// actual native unwind cleanup terminates if a second cleanup exception escapes.
// Existing NativeStringStorage behavior remains a separate host interface.
void normalize_native_resource_path_header_00bee690(void*, NativeStringRawPoolContext&);
void* copy_construct_native_resource_path_header_00bee780(void*, const void*, NativeStringRawPoolContext&);

} // namespace bsp
