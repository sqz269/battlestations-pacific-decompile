#pragma once
#include <cstdint>

namespace bsp {
struct CameraAxesCrtAccess;
struct NativeParticleUnitRandomAccess;

// Dependencies of the raw 00864880 constructor. Their bodies are not replaced.
// Names are hypotheses. Original ABI and byte evidence are in the packet report.
struct RecoveredGunneryTargetCalls {
    void* context;
    // Original 00694A60: ECX subject, EDX callback owner, RET.
    void (*register_observer_00694a60)(void*, void* subject, void* owner);
    // Original unwind 00C94C40 calls 00695870(ECX = record), RET.
    void (*destroy_callback_owner_00695870)(void*, void* owner);
    void (*refresh_world_00414db0)(void*, void* object);
    // Original 0085C3F0: ECX destination, EDX source, stack(axis, angle), RET8.
    // Destination and source are the SAME record+74h pointer in this caller.
    void (*rotate_0085c3f0)(void*, void* destination, const void* source,
                            const float* axis, float angle);
    void* (*global_config_00432650)(void*);
    // Native object virtual +34h: ECX object, stack(out), RET4, EAX vector.
    // The return may alias the supplied scratch, the object or the record.
    const float* (*velocity_slot34)(void*, void* object, float* scratch);
};

struct RecoveredGunneryTargetAccess {
    const RecoveredGunneryTargetCalls* calls;
    const CameraAxesCrtAccess* crt;
    // Borrow the existing application thread/stream domain; no seed or new RNG.
    const NativeParticleUnitRandomAccess* random;
    const volatile float* clock_00f876a4;
    void* volatile* rows_00e19994;
    const volatile double* length_cutoff_00ce3820;
    const volatile float* full_angle_00ce3d9c;
    const volatile float* one_00d7a24c;
};

// Whole ordinary source, new explicit dependency interface; NOT original SEH ABI.
// Original: ECX record (8Ch), stack(owner, target), EAX record, RET8.
// Makes exactly TWO own BD2F10 calls, ECX stream 1: radius, then bearing.
// Does not initialize record+88h (nor the other bytes not written by the image).
void* construct_recovered_gunnery_target_00864880(void* record,
    void* owner, void* target, const RecoveredGunneryTargetAccess&);
} // namespace bsp
