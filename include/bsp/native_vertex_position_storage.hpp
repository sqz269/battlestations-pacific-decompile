#pragma once
#include "bsp/native_vertex_position_read.hpp"
#include <cstdint>
#include <exception>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Whole raw vertex position storage requires MSVC Win32 and x87 inline assembly.
#endif

namespace bsp {
// Actual three Native decode floats, S+4..F after SUB ESP,0Ch/PUSH ESI.
// No initializer: caller supplies the authentic prior12-byte image. Undefined
// Native formats skip decoding and consume THESE bytes in scale/bias, never
// zeros/default output or a false-return domain gate. Ordinary float objects
// are also the SAME legal outputs for existing unpack/actual half import.
struct NativeVertexPositionStorageScratch final { float decoded[3]; };
struct NativeVertexPositionStorageContext final {
    // Existing bound named D3DXFloat16To32Array import from the genuine retained
    // d3dx9_40 module. Bind before Native entry; no alternate converter/provider.
    const NativeD3dx9Float16Import& half_import;
    // SAME authenticated original RO cells, not numeric copies/new authority.
    const volatile double& byte_divisor_00ce4b48;
    const volatile double& signed_short_divisor_00ce5f98;
    const volatile double& unsigned_short_divisor_00ce5f90;
};
struct NativeVertexPositionStorageAcquired final {
    enum class Phase : std::uint8_t { fresh, entered, complete, failed };
    enum class Boundary : std::uint8_t { none, entry, decode, unpack, half_import, scale_bias, direct_copy };
    Phase phase{Phase::fresh};
    Boundary boundary{Boundary::none}, escaped_boundary{Boundary::none};
    std::uint32_t native_site{}, escaped_site{}, captured_format{};
    void* actual_stream{};
    const volatile std::uint32_t* home_identity{};
    NativeVertexPositionStorageScratch* scratch_identity{};
    void* captured_source{};
    void* returned_output{};
    bool unpack_entered{}, unpack_returned{}, half_entered{}, half_returned{};
    std::exception_ptr failure;
    NativeVertexPositionStorageAcquired() = default;
    NativeVertexPositionStorageAcquired(const NativeVertexPositionStorageAcquired&) = delete;
    NativeVertexPositionStorageAcquired& operator=(const NativeVertexPositionStorageAcquired&) = delete;
};

// WHOLE4768D0..476B40, ECX captured actual logical stream; output/index DWORD
// homes, EAX captured output pointer, RET8. New ordinary Source ABI ONLY.
// home0 is read at its Native epoch, not an entry pointer snapshot. home1 is
// BOTH current input index and mutable integer/FILD scratch for types8/10/12.
// Callers retain the SAME two volatile homes and ordinary12-byte decode image
// through callbacks/failure. Scratch/home backing is stable and separate from
// passive diagnostics/context. Decode image and the two homes are mutually
// separate Native stack regions; preserve their explicit epoch aliases. Actual output
// may overlap input, stream/scale metadata, homes or decode image where genuine
// valid backing permits: exact Native staging, late reads and sequential FP
// stores remain. No typed stream owner, blanket initialization or copied mesh.
//
// Initial+50==0 never reads type/scale metadata or decode scratch: current
// stride*CURRENT index +offset, capture CURRENT output, then current+08 and
// sequential FLD/FSTP directly. Other paths decode all initialized Native
// formats2/3/5/7/8/10/12/13/16. Others preserve actual decode preimage and run
// the SAME current+18/+50 scale/bias/output schedule. No range/type admission.
// Types2/3 use base+stride*index+offset; type13 uses base+4*(stride*index+offset).
// Scalar x87/SSE operation/status/spill/alias order is retained with no C++ FP,
// reset or control-word change. Real475F80 and authentic half import are reused.
//
// Each reached invocation uses a fresh retained acquired record. After normal
// return only, caller may reuse homes/scratch if authentic preimage continuity
// is preserved; historical records keep identities only, never current-value
// permission. After an escape retain the stream/mapping/metadata/output/import/
// scratch/homes and whole graph for explicit disposition. No cleanup, retry,
// rollback, retain, terminal policy, false result or Native-origin inference.
// Phase/exception_ptr denotes passive retention only. Original private stack/
// FH3/SEH/fault/secondary-exception/import ABI/runtime proof remains qualified.
void* read_native_vertex_position_storage_004768d0(void* actual_logical_stream,
    volatile std::uint32_t (&actual_output_index_homes)[2],
    NativeVertexPositionStorageScratch&, const NativeVertexPositionStorageContext&,
    NativeVertexPositionStorageAcquired&);
} // namespace bsp
