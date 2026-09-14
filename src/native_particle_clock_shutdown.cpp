#include "bsp/native_particle_clock_shutdown.hpp"
#include "bsp/native_particle_clock_publication_base.hpp"
#include "bsp/native_string_pool_storage.hpp"
#include "bsp/singleton_lifetime.hpp"

#include <exception>
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native particle-clock shutdown requires MSVC Win32.
#endif

namespace bsp {
namespace {
using Word = std::uint32_t;
using Context = NativeParticleClockShutdownContext;
static_assert(sizeof(void*) == 4 && sizeof(Word) == 4);
static_assert(sizeof(NativeResourceRecordVectorStorage) == 0x0c);
static_assert(sizeof(NativeRenderResourceRecord) == 0x2c);

void* at(const void* p, Word byte_offset) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<Word>(p) + byte_offset);
}
Word load(const void* p, Word byte_offset = 0) noexcept {
    return *static_cast<volatile Word*>(at(p, byte_offset));
}
void store(void* p, Word byte_offset, Word value) noexcept {
    *static_cast<volatile Word*>(at(p, byte_offset)) = value;
}
NativeResourceRecordVectorStorage& vector_at(void* secondary) noexcept {
    return *static_cast<NativeResourceRecordVectorStorage*>(at(secondary, 4));
}

// Native single memory RMW at 004DDA7D, after record destruction returns.
__declspec(naked) void __fastcall decrement_current_count(void*) noexcept {
    __asm {
        add dword ptr [ecx + 8], -1
        ret
    }
}

void release_current_sink(void* secondary, void* captured_sink, Context& context) {
    const Word profile = load(secondary);
    const void* table;
    if (profile == 0x00ce7d08u) table = context.actual_cache_profile_00ce7d08;
    else if (profile == 0x00ce7d24u) table = context.actual_cache_profile_00ce7d24;
    else throw std::logic_error("particle cache current profile has no recovered raw dispatch");
    const Word target = load(table, 0x10);
    if (target != 0x004ddb40u)
        throw std::logic_error("particle cache current release slot has no recovered raw entry");
    release_native_particle_clock_sink_004ddb40(
        secondary, context.actual_decrement_00ce2220, captured_sink);
}

struct ArrayCleanup {
    NativeResourceRecordVectorStorage& vector;
    Context& context;
    bool armed{true};
    __declspec(noinline) ~ArrayCleanup() noexcept {
        if (!armed) return;
        try { destroy_native_particle_record_array_004ddaa0(vector, context); }
        catch (...) { std::terminate(); }
    }
};
struct BaseCleanup {
    void* owner;
    Context& context;
    bool armed{true};
    __declspec(noinline) ~BaseCleanup() noexcept {
        if (armed) clear_native_particle_clock_publication_base_004b4f10(
            owner, context.actual_publication_00f8d420);
    }
};

__declspec(noinline) void* __cdecl delete_native_particle_clock_impl(
    void* actual_owner, Context* context, volatile std::uint8_t* original_flags_slot) {
    void* const retained_owner = actual_owner;
    destroy_native_particle_clock_00b1b680(retained_owner, *context);
    if ((*original_flags_slot & 1u) != 0) singleton_lifetime_free(retained_owner);
    return retained_owner;
}
} // namespace

void __fastcall clear_native_particle_clock_records_004dda40(
    void* secondary, Context& context) {
    while (load(secondary, 8) != 0) {
        const Word count = load(secondary, 8);
        const Word data = load(secondary, 4);
        void* const sink = reinterpret_cast<void*>(
            load(reinterpret_cast<void*>(data), count * 0x2cu - 4u));
        release_current_sink(secondary, sink, context);
        const Word current_count = load(secondary, 8);
        if (current_count != 0) {
            const Word current_data = load(secondary, 4);
            auto& current_record = *static_cast<NativeRenderResourceRecord*>(
                at(reinterpret_cast<void*>(current_data), current_count * 0x2cu - 0x2cu));
            destroy_native_resource_record_004d45a0(current_record, context.records.strings);
            decrement_current_count(secondary);
        }
    }
    resize_native_particle_record_array_004dc410(vector_at(secondary), context.records, 0);
}

void __fastcall destroy_native_particle_record_array_004ddaa0(
    NativeResourceRecordVectorStorage& vector, Context& context) {
    resize_native_particle_record_array_004dc410(vector, context.records, 0);
    singleton_lifetime_free(reinterpret_cast<void*>(load(&vector)));
}

void __fastcall destroy_native_particle_clock_records_004de290(
    void* secondary, Context& context) {
    store(secondary, 0, 0x00ce7d08u);
    ArrayCleanup cleanup{vector_at(secondary), context};
    clear_native_particle_clock_records_004dda40(secondary, context);
    cleanup.armed = false;
    auto& vector = vector_at(secondary);
    resize_native_particle_record_array_004dc410(vector, context.records, 0);
    singleton_lifetime_free(reinterpret_cast<void*>(load(&vector)));
}

void __fastcall destroy_native_particle_clock_00b1b680(
    void* owner, Context& context) {
    store(owner, 0, 0x00ce7d38u);
    store(owner, 4, 0x00ce7d24u);
    BaseCleanup cleanup{owner, context};
    destroy_native_particle_clock_records_004de290(at(owner, 4), context);
    cleanup.armed = false;
    clear_native_particle_clock_publication_base_004b4f10(
        owner, context.actual_publication_00f8d420);
}

__declspec(naked) void* __fastcall delete_native_particle_clock_004de340(
    void*, Context&, Word) {
    __asm {
        lea eax, dword ptr [esp + 4]
        push eax
        push edx
        push ecx
        call delete_native_particle_clock_impl
        add esp, 0Ch
        ret 4
    }
}

__declspec(naked) void* __fastcall delete_native_particle_clock_secondary_004de360(
    void*, Context&, Word) {
    __asm {
        sub ecx, 4
        jmp delete_native_particle_clock_004de340
    }
}
} // namespace bsp
