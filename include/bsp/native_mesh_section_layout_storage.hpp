#pragma once

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Raw mesh-section layout storage requires MSVC Win32.
#endif

namespace bsp {
struct NativeRefCountedDeleteCalls;
struct NativeHardwareLayoutConstructContext;

using NativeMeshSectionLayoutReferenceOperation = long (__stdcall*)(volatile long*);

// Genuine required transport only. The lookup has no Native/lifetime effects,
// allocation, callback or reentry: return the SAME live raw table represented by
// this captured CURRENT profile. No numeric codeword becomes a host callable.
// Other-target methods execute the actual whole body on the captured receiver;
// no successful default, COM substitute or typed MeshSection cast is supplied.
struct NativeMeshSectionLayoutStorageBindings {
    virtual ~NativeMeshSectionLayoutStorageBindings() = default;
    virtual const volatile std::uint32_t* current_table(void* captured_receiver,
        std::uint32_t captured_profile) = 0;
    virtual void other_reference_current00(void* captured_reference,
        std::uint32_t captured_profile, std::uint32_t captured_target) = 0;
    virtual void* other_stream_descriptor_current24(void* captured_stream,
        std::uint32_t captured_profile, std::uint32_t captured_target) = 0;
    virtual void* other_renderer_layout_current40(void* captured_renderer,
        std::uint32_t captured_profile, std::uint32_t captured_target,
        const void* actual_key_home, void** acquired_before_insertion) = 0;
};

// Pure borrows from the SAME actual stream/layout/renderer domain. The known
// BD30E0 current00 target reuses its whole fresh-profile/current04 Source body;
// scalar_delete must dispatch the actual captured profile's genuine scalar.
// B48CE0 and B2F710 use existing whole raw Source directly. Other targets use
// required transport above. No new native storage, retain, registry or owner.
struct NativeMeshSectionLayoutStorageContext {
    NativeMeshSectionLayoutStorageBindings& bindings;
    NativeRefCountedDeleteCalls& scalar_delete;
    NativeHardwareLayoutConstructContext& hardware_layouts;
    void* const volatile& actual_renderer_00f8d394;
    // Read the SAME genuine callable import home separately at each reached
    // Native CALL; never snapshot these target values at operation entry.
    NativeMeshSectionLayoutReferenceOperation const volatile& increment_00ce221c;
    NativeMeshSectionLayoutReferenceOperation const volatile& decrement_00ce2220;
};

// WholeB86550..B8659E [79B/36ops], ECX actual section, plainRET. Signed current
// +4C controls the loop; each captured nonnull stream is decremented through
// CURRENT CE2220, CURRENT profile/current00 is captured only at zero, then SAME
// slot clears after returned release. Reload signed count after every row,
// including null rows. Set count0 only after a normal loop/initialnonpositive.
// A throwing callback skips that slot's clear and final count reset.
void clear_native_mesh_section_vertex_streams_storage_00b86550(
    void* captured_actual_section, const NativeMeshSectionLayoutStorageContext&);

// WholeB865A0..B8664E [175B/64ops incl3B jumped padding], native ECX actual
// section, stacked meshDWORD home, RET4. This new C++ ABI borrows that real
// live mesh home; it is read only on count0 AFTER any reached prior releases.
// No arbitrary void* -> NativeMeshSectionStorage/typed owner reinterpretation.
//
// actual_key_home identifies caller-retained writable RAW addressable backing,
// with descriptor0 at+00 and the live count word at+10. Initialize only count
// at the native writer. No fixed-array indexing/cap or fresh copied key: after
// each descriptor callback load CURRENT keycount, write key+4*index, THEN read
// CURRENT count again for wrapping increment. Index4 aliases count itself.
// Caller supplies valid ordinary DWORD backing/residue for every reached read,
// including unwritten key words if a genuine callee reads them; no zero fill.
// Caller proves every reached DWORD/adjacent backing valid; no count clamp is
// added. Independent wrapping stream-loop index/slot pointer remain separate.
// The initial signed sectioncount comparison precedes keycount=0, as Native.
//
// On existing layout: whole clear above, reload +50, release captured nonnull
// current layout and clear SAME field only after return, then recheck +50.
// count0: whole rawB73260(mesh,0), CURRENT CE221C on stream+4 without null guard,
// reload sectioncount for append, then reload count AFTER store for increment.
// Loop current stream/profile/+24; read current sectioncount after each call
// and key writes. Only after the loop capture renderer home/profile/current40.
// Store returned layout to captured section+50 after the whole call returns.
//
// acquired_before_insertion is optional external metadata, explicitly nullable,
// using the existing B2F710 output contract. When nonnull it starts empty and
// survives any entered failure; it observes actual acquisition before insertion
// without retaining, cleaning or creating an owner. Other-target transport must
// preserve its genuine acquisition/custody epochs with preprepared metadata.
void rebuild_native_mesh_section_vertex_layout_storage_00b865a0(
    void* captured_actual_section,
    const volatile std::uint32_t& actual_mesh_dword_home,
    void* actual_key_home, const NativeMeshSectionLayoutStorageContext&,
    void** acquired_before_insertion);

// No local cleanup/retry/rollback/termination or implicit destructor is added.
// Caller retains actual section/mesh/key/slot/target/renderer/import/context and
// reached stream/layout/tree/pool backing for the complete Native operation.
// Existing known Source callees keep their established reached-input/lifetime
// preconditions; this loop adds no stream/domain cap. In particular B2F710's
// actual hardware creator still needs its own valid reached key/element domain.
// Partial effects and unresolved acquisitions remain for explicit disposition;
// neither a failed invocation nor a raw release is automatically replayed.
// Original private stack aliases beyond supplied raw homes, actual faults,
// physical register/RET ABI, FH3/SEH, activation and gameplay remain unproved.
// Existing typed bodies/credits, SingleLine and shared integration are unchanged;
// zero additional Original/helper/vendor/duplicate-body/ABI/runtime credit.
} // namespace bsp
