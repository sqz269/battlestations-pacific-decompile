#pragma once
#include "bsp/native_model_storage_geometry.hpp"

namespace bsp {
class NativeModelReference;
class NativeMatrixStorageContext;
struct NativeMatrixStorageAccess;

// A real caller-role binding, not Model188 storage relabeled as Matrix194.
// The Model geometry services are the genuine retained renderer/input services;
// only their SAME node/provider identities and real D7A260 role are borrowed.
// Construction samples addresses/metadata, never the sentinel/import contents.
class NativeMatrixStorageGeometryContext final {
public:
    NativeMatrixStorageGeometryContext(const NativeMatrixStorageContext&,
        const NativeModelStorageGeometryContext& actual_model_geometry_services);
    NativeMatrixStorageGeometryContext(const NativeMatrixStorageGeometryContext&) = delete;
    NativeMatrixStorageGeometryContext& operator=(const NativeMatrixStorageGeometryContext&) = delete;
private:
    friend struct NativeMatrixStorageAccess;
    const NativeMatrixStorageContext* const matrix_;
    const NativeModelStorageGeometryContext* const roles_;
};

// One existing whole B75170 execution path, accessible only to concrete checked
// family entries. This does not itself publish a canonical or create a credit.
class NativeModelGeometryKernelAccess final {
    friend class NativeModelReference;
    friend struct NativeMatrixStorageAccess;
    friend void set_native_model_geometry_storage_00b75170(void*,
        NativeModelStorageGeometryFrame&, NativeModelStorageGeometryContext&,
        NativeModelStorageGeometryAcquired&);
    static void require_fresh(NativeModelStorageGeometryFrame&,
        NativeModelStorageGeometryAcquired&);
    static void require_live_model_slot(void*, NativeModelPool&);
    static void reject_returned(NativeModelStorageGeometryFrame&,
        NativeModelStorageGeometryAcquired&);
    static void execute(void*, NativeModelStorageGeometryFrame&,
        NativeNodeBaseDestructionContext&, const volatile std::uint32_t& actual_unchanged_00d7a260,
        NativeModelStorageGeometryAcquired&, const void* issuer,
        const void* context_identity, const void* service_identity,
        NativeModelStorageGeometryContext* legacy_context);
};
} // namespace bsp
