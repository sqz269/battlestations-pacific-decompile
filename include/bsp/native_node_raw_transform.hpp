#pragma once

#include <cstdint>

namespace bsp {

class NativeTracelineRenderServices;
namespace game { class GameNativeReadOnlyData; }

// Borrows the already initialized, retained Original read-only-data service.
// Construction checks only the three genuine Node/Model/Group table aliases;
// it neither reads a receiver nor maps a missing band. This context owns no
// table, node, graph or data-service lifetime. Keep the data service, this full
// context and every reached actual graph/storage alive and disjoint through
// the call and any failure. The actual prefix/provenance remains caller proof.
class NativeNodeMappedRawTransformProfiles final {
public:
    explicit NativeNodeMappedRawTransformProfiles(
        const game::GameNativeReadOnlyData& data);
    NativeNodeMappedRawTransformProfiles(
        const NativeNodeMappedRawTransformProfiles&) = delete;
    NativeNodeMappedRawTransformProfiles& operator=(
        const NativeNodeMappedRawTransformProfiles&) = delete;
private:
    friend struct NativeNodeMappedRawTransformAccess;
    const volatile std::uint32_t* profiles_[3];
};

void __fastcall invalidate_raw_descendants_00b6da30(void* actual);
void __fastcall notify_raw_bounds_00b6dbc0(
    void* actual, NativeTracelineRenderServices* profiles);
void __fastcall set_raw_local_matrix_00b6db10(
    void* actual, NativeTracelineRenderServices* profiles, const void* source);

// Same whole raw bodies, using only reached current +A0/current vptr/+3C.
// Only the admitted actual Node/Model/Group aliases with current B6DBC0 are
// supported. No incoming receiver vptr/38 read belongs in this setter.
// An unsupported reached profile/target throws after prior raw effects; no
// rollback, retry, cycle cutoff, Native ABI/fault or production claim follows.
// Reference overloads leave existing pointer/nullptr calls unambiguous.
void __fastcall notify_raw_bounds_00b6dbc0(
    void* actual, const NativeNodeMappedRawTransformProfiles& profiles);
void __fastcall set_raw_local_matrix_00b6db10(
    void* actual, const NativeNodeMappedRawTransformProfiles& profiles,
    const void* source);

} // namespace bsp
