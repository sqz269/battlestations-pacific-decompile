#include "bsp/native_post_effect_dust_initializer.hpp"
#include "bsp/gui_text_material.hpp"
#include "bsp/native_physical_file_date.hpp"
#include "bsp/native_string_pool_storage.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <exception>
#include <new>
#include <stdexcept>
#include <utility>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native post-effect dust initialization requires MSVC Win32.
#endif

namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == 4 && sizeof(NativeString) == 8);
Word word(const volatile void* base, Word byte_offset = 0) noexcept {
    return *reinterpret_cast<const volatile Word*>(
        reinterpret_cast<std::uintptr_t>(base) + byte_offset);
}
void put(void* base, Word byte_offset, Word value) noexcept {
    *reinterpret_cast<volatile Word*>(reinterpret_cast<std::uintptr_t>(base) + byte_offset) = value;
}
void put_byte(void* base, Word byte_offset, unsigned char value) noexcept {
    *reinterpret_cast<volatile unsigned char*>(reinterpret_cast<std::uintptr_t>(base) + byte_offset) = value;
}
void* pointer(Word value) noexcept { return reinterpret_cast<void*>(value); }
void require(bool value, const char* message) {
    if (!value) throw std::logic_error(message);
}
}

__declspec(naked) void* __fastcall get_native_post_effect_material_00b4cbf0(const void*) noexcept {
    __asm {
        mov eax, dword ptr [ecx + 14h]
        ret
    }
}

NativePostEffectDustBlock::~NativePostEffectDustBlock() noexcept {
    if (phase_ != Phase::idle) std::terminate();
}
void NativePostEffectDustBlock::prepare(NativePostEffectDustContext& context) {
    require(phase_ == Phase::idle && context.effect_name_00d620ac,
        "dust initialization requires idle host storage and the actual effect-name bytes");
    context_ = &context;
    phase_ = Phase::preparing;
    try {
        child_.prepare(context.construction);
        acquired_.effect_name_header = &effect_name_;
        phase_ = Phase::prepared;
    } catch (...) {
        context_ = nullptr;
        phase_ = Phase::idle;
        throw;
    }
}
void NativePostEffectDustBlock::cancel_preparation() noexcept {
    if (phase_ != Phase::prepared) std::terminate();
    child_.cancel_preparation();
    acquired_ = {};
    context_ = nullptr;
    phase_ = Phase::idle;
}
void NativePostEffectDustBlock::settle() noexcept {
    if (child_.phase() == NativePostEffectConstructionBlock::Phase::prepared)
        child_.cancel_preparation(); // The native child was never entered.
    phase_ = Phase::settled;
}
void NativePostEffectDustBlock::reset_after_host_quiescence() noexcept {
    if (phase_ != Phase::settled || child_.phase() != NativePostEffectConstructionBlock::Phase::idle)
        std::terminate();
    effect_name_.~NativeString();
    new (&effect_name_) NativeString;
    acquired_ = {};
    context_ = nullptr;
    phase_ = Phase::idle;
}
void NativePostEffectDustBlock::return_name() {
    acquired_.name_return_started = true;
    destroy_native_string_header_0041dd20(&effect_name_, context_->construction.raw_strings);
}
void NativePostEffectDustBlock::unwind(int& state, bool& name_owned) {
    // DF8B4C:0 -> -1/free;1 ->0/masked name;2 ->-1/same name.
    // State2 is present in the original map but never visited by the normal body.
    // Consuming each edge before its call prevents retries. A newer C++ cleanup
    // exception runs the remaining edge and replaces the prior exception; native
    // FH3/double-exception equivalence is explicitly outside this source policy.
    while (state >= 0) {
        const int action = state;
        state = action == 1 ? 0 : -1;
        acquired_.native_state = state;
        try {
            if (action == 0) {
                if (child_.acquired().native_completed) {
                    acquired_.completed_child_preserved_after_host_failure = true;
                } else {
                    acquired_.raw_free_started = true;
                    singleton_lifetime_free(std::exchange(acquired_.raw_child, nullptr));
                }
            } else if (action == 1 || action == 2) {
                if (name_owned) { name_owned = false; return_name(); }
            } else std::terminate();
        } catch (...) { unwind(state, name_owned); throw; }
    }
}

void initialize_native_post_effect_dust_00b52860(void* actual, std::size_t receiver_bytes,
    NativePostEffectDustBlock& block) {
    require(actual && !(reinterpret_cast<std::uintptr_t>(actual) & 3u) && receiver_bytes >= 0xccu &&
        block.phase_ == NativePostEffectDustBlock::Phase::prepared,
        "dust initialization requires an aligned CCh receiver and prepared persistent host block");
    auto& acquired = block.acquired_;
    auto& c = *block.context_;
    acquired.actual_receiver = actual;
    block.phase_ = NativePostEffectDustBlock::Phase::executing;
    int state = -1;
    bool name_owned = false;
    auto set_state = [&](int value) noexcept { state = value; acquired.native_state = value; };
    try {
        put(actual, 0x20, 0); // B52888: XORPS/MOVSS exact zero bits.
        put_byte(actual, 0x34, 0);
        put(actual, 0x38, 0);
        acquired.raw_child = singleton_lifetime_allocate({SingletonAllocationKind::object, 0x24, 0x24});
        set_state(0); // Allocation failure precedes this state in the original.
        void* child = nullptr;
        if (acquired.raw_child) {
            construct_native_string_cstring_0041e870(&block.effect_name_, c.effect_name_00d620ac,
                c.construction.raw_strings);
            acquired.name_constructed = true;
            name_owned = true;
            set_state(1);
            child = construct_native_post_effect_00b4e840(acquired.raw_child, 0x24,
                block.effect_name_, 2, nullptr, block.child_);
        }
        acquired.returned_child = child;
        put(actual, 0x68, reinterpret_cast<Word>(child)); // B528DE, no prior-value release.
        acquired.child_published = true;
        set_state(-1); // B528E1, BEFORE normal inline name return.
        if (name_owned) {
            // Native BL bit remains set; state-1 prevents unwind from retrying.
            // The raw41DD20 provider captures data, then length+1 and the real
            // pool getter/return without a typed noexcept adapter.
            block.return_name();
        }

        void* texture = pointer(word(actual, 0x3c)); // B5290A: capture BEFORE current child/getter.
        acquired.captured_texture = texture;
        void* current_child = pointer(word(actual, 0x68)); // B5290E, no cached-child fallback.
        acquired.current_post_effect = current_child;
        require(current_child != nullptr, "native dust material getter requires a valid current post-effect");
        void* material = get_native_post_effect_material_00b4cbf0(current_child);
        acquired.current_material = material;
        require(material != nullptr, "native dust texture binding requires a valid current material");
        acquired.texture_bind_started = true;
        set_native_material_texture_00b189f0(*static_cast<NativeMaterialStorage*>(material), 0,
            texture, c.construction.destruction.actual_owners);

        const Word one = word(&c.construction.node_constants.one_00d7a24c); // B52922: MOVSS bit copy.
        put(actual, 0x6c, 0); put(actual, 0x78, 0); put(actual, 0xa0, 0);
        put_byte(actual, 0xb4, 0); put(actual, 0xb8, 0);
        put(actual, 0x08, 0); put(actual, 0x0c, 0); put(actual, 0x10, one);
        put_byte(actual, 0xbc, 1); put(actual, 0xc0, 0); put(actual, 0x14, 0);
        put(actual, 0xc4, 0); put(actual, 0xc8, 0);
        acquired.normal_body_completed = true;
        block.settle();
    } catch (...) {
        try { block.unwind(state, name_owned); }
        catch (...) { block.settle(); throw; }
        block.settle();
        throw;
    }
}
} // namespace bsp
