#pragma once
#include "bsp/gameplay_effect_definition.hpp"
#include "bsp/native_string.hpp"
#include "bsp/singleton_lifetime.hpp"
#include "bsp/sound_shutdown.hpp"
#include <cstddef>
#include <cstdint>

namespace bsp {
// The F8BBCC owner produced by A79230 is the streamed-dialog manager. These
// byte stores preserve native padding and fields its constructors do not write.
// No implicit C++ member construction or destruction. NativeString operations
// use the existing actual-header API; references use actual native+4 counts.
struct alignas(4) NativeSoundAlternateOwnerStorage { std::byte bytes[0x234]; };
struct alignas(4) NativeSoundAlternateLogicalStorage { std::byte bytes[0x5c]; };
static_assert(sizeof(void*) == 4);
static_assert(sizeof(NativeSoundAlternateOwnerStorage) == 0x234);
static_assert(sizeof(NativeSoundAlternateLogicalStorage) == 0x5c);

// Required, body-read dependencies. There is no default implementation. The
// table is the actual20h refcounted storage, not portable DialogStreamTable.
// The stream receiver is actual54h storage produced by A877D0, not an FMOD
// channel. The callback takes the actual temporary NativeString in ECX.
class SoundAlternateOwnerDependencies {
public:
    virtual ~SoundAlternateOwnerDependencies() = default;
    virtual void load_stream_table_00a87060(void* actual_table, const NativeString& path) = 0;
    virtual void update_logical_00a78820(void* actual_logical, float dt, float gain) = 0;
    virtual void start_logical_00a783f0(void* actual_logical) = 0;
    virtual void invoke_callback230_ecx(void* callback, NativeString& temporary_name) = 0;
};
struct SoundAlternateOwnerBindings {
    SingletonLifetimeDomain& domain;
    void* volatile& global_00f8bbcc;
    NativeStringStorage& strings;
    GameplayEffectComponentLifetime& references;
    SoundAlternateOwnerDependencies& dependencies;
};

// Full normal body, new C++ interfaces. Base construction publishes/registers
// this; base destruction unregisters CURRENT F8BBCC, clears it, then sets
// CE3818. The manager section captured before the callback is the one unlocked.
// A thrown manager/register/unregister operation unwinds that section then sets
// CE3818, preserving the current global and any partial registration effects.
NativeSoundAlternateOwnerStorage& construct_sound_alternate_base_00a778d0(
    NativeSoundAlternateOwnerStorage&, SoundAlternateOwnerBindings&);
void destroy_sound_alternate_base_00a77970(
    NativeSoundAlternateOwnerStorage&, SoundAlternateOwnerBindings&);
NativeSoundAlternateOwnerStorage& construct_sound_alternate_owner_00a79230(
    NativeSoundAlternateOwnerStorage&, SoundAlternateOwnerBindings&);
void clear_sound_alternate_channels_00a77c10(
    NativeSoundAlternateOwnerStorage&, SoundAlternateOwnerBindings&);
void destroy_sound_alternate_owner_00a78fb0(
    NativeSoundAlternateOwnerStorage&, SoundAlternateOwnerBindings&);
NativeSoundAlternateOwnerStorage* scalar_delete_sound_alternate_owner_00a790d0(
    NativeSoundAlternateOwnerStorage*, std::uint8_t flags, SoundAlternateOwnerBindings&);
void update_sound_alternate_owner_00a789c0(
    NativeSoundAlternateOwnerStorage&, float dt, SoundAlternateOwnerBindings&);

// A78150 and A781C0 are distinct logical channel profiles. Both preserve
// uninitialized +2C and padding; category is the stack argument at +50.
void* construct_sound_alternate_logical_00a78150(void*, std::uint32_t category) noexcept;
void* construct_sound_alternate_logical_00a781c0(void*, std::uint32_t category) noexcept;
void destroy_sound_alternate_logical_00a780b0(void*, SoundAlternateOwnerBindings&);
void destroy_sound_alternate_configuration_00a77dd0(void*, SoundAlternateOwnerBindings&);
void* construct_sound_alternate_record_004c87d0(void*) noexcept;
void destroy_sound_alternate_record_004c87f0(void*, SoundAlternateOwnerBindings&);
void* construct_sound_alternate_physical_pair_00a77d00(void*) noexcept;
void destroy_sound_alternate_physical_pair_00a77d10(void*, SoundAlternateOwnerBindings&);
void* construct_sound_alternate_reference_00a778c0(void*) noexcept;

// Native table +08/+0C/+10 data/count/capacity,14h records beginning with an
// actual NativeString. Reverse string destruction, free current data, base vt.
void destroy_sound_alternate_table_00a791d0(void*, NativeStringStorage&);
void* scalar_delete_sound_alternate_table_00a796f0(void*, std::uint8_t, NativeStringStorage&);
// Stream helpers: A877D0 produces state+20 and name+24. Fade sets bytes A=1,
// B=0 only for states1/2. Name construction clears its destination first.
void request_sound_stream_fade_00a85c00(void* actual_stream) noexcept;
void request_sound_stream_stop_00a86f40(void* actual_stream, NativeStringStorage&);
NativeString& copy_sound_stream_name_00a77ff0(void* actual_stream,
    NativeString& destination, NativeStringStorage&);

// Only routes recovered D58F78 destruction; dependency/reference bindings must
// remain alive while the shared SingletonLifetimeDomain drains this owner.
class SoundAlternateOwnerShutdownRuntime final : public SoundAlternateShutdownHost {
public:
    explicit SoundAlternateOwnerShutdownRuntime(SoundAlternateOwnerBindings& bindings) noexcept
        : bindings_(bindings) {}
    void delete_alternate_slot00(void*, std::uint32_t flags) override;
private:
    SoundAlternateOwnerBindings& bindings_;
};
} // namespace bsp
