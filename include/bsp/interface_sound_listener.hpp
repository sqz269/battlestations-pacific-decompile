#pragma once

#include <cstdint>

namespace bsp {
struct CameraAxesCrtAccess;

// Dispatch the CURRENT virtual entry on the supplied captured native object.
// Each entry receives one caller-owned, initially unwritten stack buffer and
// returns a pointer in EAX. The returned pointer need not equal that buffer.
// slot120 supplies the matrix source; slot34 supplies three velocity words.
// These are dispatch contracts, not a claim that every unit has these getters.
// The dynamic type published at game+1ED4 determines its actual slot120 body.
class InterfaceSoundListenerCalls {
public:
    virtual ~InterfaceSoundListenerCalls() = default;
    virtual const void* call_slot120(void* actual_object, void* scratch64) = 0;
    virtual const void* call_slot34(void* actual_object, void* scratch12) = 0;
};

// Borrowed PE publication/constants. References are read at the native sites;
// callbacks may change publications or the interface's applied id. No copied
// game, interface, transform or velocity state is maintained by this provider.
struct InterfaceSoundListenerContext {
    void* volatile& game_00e188a8;
    void* volatile& controlled_00e188d8;
    const volatile std::uint32_t& one_00d7a24c; // initial bits 3F800000
    const volatile float& threshold_00ce3d64; // initial 10000.0f
    const volatile float& compare_00d7a218; // initial +0.0f
    const volatile double& scale_00d7a220; // initial 100.0
    const volatile float* fallback_00f87574; // three live words, initially zero
    InterfaceSoundListenerCalls& calls;
    const CameraAxesCrtAccess& crt;
};

// 0068A670, complete body [0068A670,0068A8A0). Original: ECX interface,
// stack matrix64 destination then velocity12 destination, RET8. This C++ API
// adds explicit bindings and is not a drop-in native entry. actual_interface
// binds the original applied id at +4, NOT the semantic InGameInterfaceManager
// companion. game+19FC must be actual raw camera/node storage accepted by
// refresh_native_camera_world_00b6db70, including canonical raw parent words.
// All pointers reached must be valid; a null game/interface is not synthesized.
// Only the native no-target/no-camera branch creates identity. Sequential
// x87/SSE loads/stores, output alias effects, live reloads, and the native
// comparison-result sqrt/scale behavior are retained. There is no EH cleanup.
void get_interface_sound_listener_0068a670(const void* actual_interface,
    void* matrix64, void* velocity12, InterfaceSoundListenerContext&);
} // namespace bsp
