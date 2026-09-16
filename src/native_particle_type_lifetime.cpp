#include "bsp/native_particle_type_lifetime.hpp"
#include "bsp/native_ref_counted.hpp"
#include "bsp/native_string.hpp"
#include "bsp/native_string_pool_storage.hpp"
#include "bsp/native_weak_owner.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <initializer_list>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native particle type lifetime requires MSVC Win32.
#endif

namespace bsp {
namespace {
using Word = std::uint32_t;
using Signed = std::int32_t;
void* at(const void* object, Word offset = 0) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<Word>(object) + offset);
}
template<class T> volatile T& field(const void* object, Word offset = 0) noexcept {
    return *static_cast<volatile T*>(at(object, offset));
}
void copy_record(void* destination, const void* source) noexcept {
    // Native REP MOVSD goes forward even if source and destination overlap.
    __asm {
        mov edi, destination
        mov esi, source
        mov ecx, 7
        rep movsd
    }
}
void return_parameter(void* parameter, NativeParticleTypeLifetimeContext& context) {
    if (parameter) {
        destroy_native_particle_parameter_00affdf0(parameter);
        return_native_particle_parameter_00b00090(parameter, context.parameter_pool_00f8d344);
    }
}
struct TypeUnwind {
    void* owner;
    NativeParticleTypeLifetimeContext& context;
    int state = 2;
    ~TypeUnwind() noexcept {
        // DF3468: state2 ->1 descriptor, state1 ->0 current name, state0 ->-1
        // base. These are true unwind actions: a second exception terminates.
        if (state >= 2) destroy_native_particle_type_records_00b00f70(at(owner, 0x68));
        if (state >= 1) destroy_native_string_header_0041dd20(at(owner, 8), context.strings);
        if (state >= 0) destroy_native_ref_counted_base_00bd30f0(owner);
    }
};
} // namespace

void destroy_native_particle_parameter_00affdf0(void* parameter) noexcept {
    const auto type = field<std::uint16_t>(parameter, 0x0a);
    if (type == 1 || type == 2) {
        void* const data = field<void*>(parameter, 4);
        if (data) {
            singleton_lifetime_free(data);
            field<void*>(parameter, 4) = nullptr;
        }
    }
}

void return_native_particle_parameter_00b00090(void* parameter, NativeWeakHandlePool& pool) {
    pool.return_raw_slot_00924420(parameter);
}

void reserve_native_particle_type_records_00b00c20(void* descriptor, Signed requested) {
    if (requested < 1) requested = 1;
    if (field<Signed>(descriptor, 8) >= requested) return;
    const Word bytes = static_cast<Word>(requested) * 0x1cu;
    void* const fresh = singleton_lifetime_allocate({SingletonAllocationKind::object, bytes, bytes});
    Word index = 0;
    Word offset = 0;
    void* destination = fresh;
    while (static_cast<Signed>(index) < field<Signed>(descriptor, 4)) {
        if (destination) copy_record(destination, at(field<void*>(descriptor), offset));
        ++index;
        offset += 0x1cu;
        destination = at(destination, 0x1c);
    }
    singleton_lifetime_free(field<void*>(descriptor));
    field<void*>(descriptor) = fresh;
    field<Signed>(descriptor, 8) = requested;
}

void destroy_native_particle_type_records_00b00f70(void* descriptor) {
    if (field<Signed>(descriptor, 8) < 0) reserve_native_particle_type_records_00b00c20(descriptor, 0);
    while (field<Signed>(descriptor, 4) > 0)
        field<Word>(descriptor, 4) = field<Word>(descriptor, 4) - 1u;
    void* const data = field<void*>(descriptor);
    field<Word>(descriptor, 4) = 0;
    singleton_lifetime_free(data);
}

void destroy_native_particle_type_base_00b00fb0(void* owner, NativeParticleTypeLifetimeContext& context) {
    field<Word>(owner) = 0x00d5ddc0;
    void* const first = field<void*>(owner, 0x1c);
    TypeUnwind unwind{owner, context};
    return_parameter(first, context);
    for (Word offset : {0x2cu, 0x30u, 0x48u, 0x34u, 0x38u, 0x3cu, 0x40u, 0x44u, 0x5cu}) {
        void* const parameter = field<void*>(owner, offset);
        if (parameter) {
            return_parameter(parameter, context);
            if (offset == 0x48) field<Word>(owner, 0x48) = 0;
        }
    }
    const Signed capacity = field<Signed>(owner, 0x70);
    void* const descriptor = at(owner, 0x68);
    unwind.state = 1;
    if (capacity < 0) reserve_native_particle_type_records_00b00c20(descriptor, 0);
    while (field<Signed>(descriptor, 4) > 0)
        field<Word>(descriptor, 4) = field<Word>(descriptor, 4) - 1u;
    field<Word>(descriptor, 4) = 0;
    // The normal inlined schedule captures data AFTER clearing count. The
    // B00F70 unwind body above captures it before that store.
    void* const records = field<void*>(descriptor);
    singleton_lifetime_free(records);
    void* const name_data = field<void*>(owner, 0x0c);
    unwind.state = 0;
    if (name_data) {
        const Word bytes = field<Word>(owner, 8) + 1u;
        auto* const pool = native_string_pool_get_or_create_00419cc0(
            context.strings.actual_published_01090aa8, context.strings.actual_manager_publication_01090aa0);
        return_native_string_pool_00bd1510(pool, name_data, bytes,
            context.strings.actual_small_returns_disabled_01090aa4);
    }
    unwind.state = -1;
    destroy_native_ref_counted_base_00bd30f0(owner);
}

void* delete_native_particle_type_base_00b01130(void* owner, Word flags, NativeParticleTypeLifetimeContext& context) {
    destroy_native_particle_type_base_00b00fb0(owner, context);
    if (flags & 1u) singleton_lifetime_free(owner);
    return owner;
}
} // namespace bsp
