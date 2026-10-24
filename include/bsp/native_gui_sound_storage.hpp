#pragma once
#include "bsp/native_gui_widget_lifetime.hpp"
#include "bsp/native_render_context.hpp"
#include "bsp/native_string.hpp"
#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native GUI Sound storage requires MSVC Win32.
#endif

namespace bsp {
using NativeGuiSoundInterlocked = long (__stdcall*)(volatile long*);

// SAME current raw tracked-instance profile/table and genuine stop body.
// Metadata resolution is side-effect-free; table covers at least three words.
// No semantic SoundInstance cast, fabricated table, default no-op or numeric
// host call is supplied. Provider/table metadata survive terminal callbacks.
class NativeGuiSoundStopBindings {
public:
    virtual ~NativeGuiSoundStopBindings() = default;
    virtual const volatile std::uint32_t* current_instance_table(void* actual_instance,
        std::uint32_t captured_profile) = 0;
    virtual void call_stop_08(void* actual_instance, std::uint32_t captured_target,
        std::uint32_t zero) = 0;
};

struct NativeGuiSoundStorageContext {
    NativeGuiWidgetLifetimeContext& widgets;
    NativeStringRawPoolContext& strings;
    NativeRenderActualOwners& owners; // SAME sample/tracked actual+04 companions.
    NativeGuiSoundStopBindings& stop;
    const volatile std::uint32_t& one_00d7a24c;
    NativeGuiSoundInterlocked const volatile& increment_00ce221c;
    NativeGuiSoundInterlocked const volatile& decrement_00ce2220;
};

// Diagnostic metadata, NEVER reference credits or ownership. Identities can
// already be retired: do not rediscover owners or retry from these pointers.
struct NativeGuiSoundReleaseObservation {
    void* identity{};
    NativeGuiSoundInterlocked captured_decrement{};
    long result{};
    bool entered{}, decremented{}, zero_entered{}, zero_returned{}, complete{};
};
struct NativeGuiSoundAssignmentObservation {
    void* destination_slot{};
    const void* source_slot{};
    void* held{};
    void* incoming{};
    void* replaced{};
    NativeGuiSoundInterlocked captured_increment{};
    bool entered{}, equal{}, held_retain_returned{}, published{};
    bool incoming_retain_returned{}, current_retain_returned{}, complete{};
    NativeGuiSoundReleaseObservation releases[3];
};

// One fresh externally held record per invocation, allocated BEFORE entry.
// No destructor performs native cleanup; interrupted records terminate if
// destroyed. Keep the SAME raw slot, all unresolved credits, contexts, provider
// and candidate backing alive after failure; host must exit before dependency
// drain. Never reset/replay this record or use another record to retry a partial
// Sound lifetime. Public diagnostics are not provenance or an uneditable token.
struct NativeGuiSoundStorageAcquired {
    enum class Phase { fresh, entered, complete, failed };
    enum class Entry { none, construct, copy, destroy, scalar_delete, factory };
    Phase phase{Phase::fresh};
    Entry entry{Entry::none};
    std::uint32_t active_call_site{};
    void* destination{};
    const void* source{};
    void* allocated_slot{};
    bool copy_selected{}, base_returned{}, derived_returned{}, base_destroyed{}, slot_returned{};
    NativeGuiSoundAssignmentObservation assignments[2];
    NativeGuiSoundReleaseObservation releases[4];
    void* stop_receiver{};
    std::uint32_t stop_profile{}, stop_target{};
    bool stop_entered{}, stop_returned{};

    NativeGuiSoundStorageAcquired() noexcept = default;
    ~NativeGuiSoundStorageAcquired() noexcept;
    NativeGuiSoundStorageAcquired(const NativeGuiSoundStorageAcquired&) = delete;
    NativeGuiSoundStorageAcquired& operator=(const NativeGuiSoundStorageAcquired&) = delete;
    bool native_operation_interrupted() const noexcept {
        return phase == Phase::entered || phase == Phase::failed;
    }
};

// Whole normal ACDB30/ACDBA0, SAME fresh DWORD-aligned actual108h destination.
// AA9390(type14) constructs its actual+04/base/container fields. Copy also
// DEFAULT constructs the base (never AA9520), copies only F4 and performs exact
// ACD8E0/ACD9E0 assignments; F8/100 strings start empty. Preserve all base
// unwritten bytes and the pool trailer108. Neither function allocates a slot.
void* construct_native_gui_sound_00acdb30(void* actual_destination,
    NativeGuiSoundStorageContext&, NativeGuiSoundStorageAcquired&);
void* copy_native_gui_sound_00acdba0(void* actual_destination,
    const void* actual_source, NativeGuiSoundStorageContext&, NativeGuiSoundStorageAcquired&);

// Whole normal ACDC40. Genuine live raw base/containers and canonical sample/
// instance companions required. Stamp D5CFD0, sample release, CURRENT stop08(0),
// CURRENT tracked release, raw strings100 thenF8, CURRENT ref-member releases,
// then genuine AA9730. Current stop may replace F0; subsequent reads are fresh.
// No widget-count decrement, implicit ownership map or slot return is added.
void destroy_native_gui_sound_00acdc40(void* actual_sound,
    NativeGuiSoundStorageContext&, NativeGuiSoundStorageAcquired&);

// Whole normal ACDEF0: read CURRENT flags LOWBYTE only AFTER destructor; bit0
// returns SAME slot through existing F8C0EC Sound pool. Identity-only result
// after return; flags/metadata cannot alias freed backing. No return on throw.
void* scalar_delete_native_gui_sound_00acdef0(void* actual_sound,
    const volatile std::uint32_t& current_flags,
    NativeGuiSoundStorageContext&, NativeGuiSoundStorageAcquired&);

// Whole normal AA1890, original ECX optional actual Sound source, plain RET.
// Existing ACD7F0 selector uses the SAME explicitly initialized process pool
// F8C0EC (object108/slot10C), then default/copy construction on that allocation.
// No pool startup, manager, registration, activation or second counter. Source
// interruption retains the entered slot instead of inventing native FH3 unwind.
void* make_native_gui_sound_00aa1890(const void* actual_source_or_null,
    NativeGuiSoundStorageContext&, NativeGuiSoundStorageAcquired&);
} // namespace bsp
