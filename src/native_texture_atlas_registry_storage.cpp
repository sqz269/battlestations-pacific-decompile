#include "bsp/native_texture_atlas_registry_storage.hpp"
#include "bsp/game_native_readonly_data.hpp"
#include "bsp/native_render_context.hpp"
#include "bsp/native_singleton_publication.hpp"
#include "bsp/native_singleton_removal_reorder.hpp"
#include "bsp/native_singleton_vector_registration_wrappers.hpp"
#include "bsp/native_texture_loading_cache.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <cstddef>
#include <new>
#include <stdexcept>
#include <utility>

namespace bsp {
namespace {
using Word = std::uint32_t;
using Phase = NativeTextureAtlasRegistryPhase;
static_assert(sizeof(void*) == 4 && sizeof(long) == 4);
static_assert(sizeof(std::atomic<std::int32_t>) == 4);
static_assert(alignof(std::atomic<std::int32_t>) == 4);
Word word(const void* address, Word byte_offset=0) {
    Word result;
    __asm {
        mov eax, address
        mov edx, byte_offset
        mov eax, dword ptr [eax + edx]
        mov result, eax
    }
    return result;
}
void put(void* address, Word byte_offset, Word value) {
    __asm {
        mov eax, address
        mov edx, byte_offset
        mov ecx, value
        mov dword ptr [eax + edx], ecx
    }
}
std::uint8_t flags_byte(const volatile Word& home) {
    const volatile Word* const address=&home;
    Word result;
    __asm {
        mov eax, address
        movzx eax, byte ptr [eax]
        mov result, eax
    }
    return static_cast<std::uint8_t>(result);
}
void* pointer(Word bits) noexcept { return reinterpret_cast<void*>(static_cast<std::uintptr_t>(bits)); }
Word bits(const void* value) noexcept { return static_cast<Word>(reinterpret_cast<std::uintptr_t>(value)); }
bool negative(Word value) noexcept { return static_cast<std::int32_t>(value)<0; }
bool positive(Word value) noexcept { return static_cast<std::int32_t>(value)>0; }
bool less(Word lhs, Word rhs) noexcept {
    return static_cast<std::int32_t>(lhs)<static_cast<std::int32_t>(rhs);
}

void capture_and_enter(NativeTextureAtlasRegistryBaseFrame& f,
    NativeTextureAtlasRegistryStorageContext& c, Word get_site, Word enter_site) {
    f.active_site=get_site;
    f.first_manager=get_native_singleton_manager_00415350(c.manager_publication);
    f.captured_section=pointer(word(f.first_manager,0x10));
    f.guard[0]=0x00ce37fcu;
    f.guard[1]=bits(f.captured_section);
    if(f.captured_section) {
        f.entered_target=c.enter_iat; // actual per-reached IAT load
        if(!f.entered_target) throw std::invalid_argument("unbound actual CE2218 import");
        f.active_site=enter_site;
        f.entered_target(static_cast<::_RTL_CRITICAL_SECTION*>(f.captured_section));
        f.entered=true;
        put(f.captured_section,0x18,word(f.captured_section,0x18)+1u);
        f.depth_incremented=true;
    }
    f.native_state=1;
}
void leave_captured(NativeTextureAtlasRegistryBaseFrame& f,
    NativeTextureAtlasRegistryStorageContext& c, Word leave_site) {
    if(f.captured_section) {
        put(f.captured_section,0x18,word(f.captured_section,0x18)-1u);
        f.depth_decremented=true;
        f.left_target=c.leave_iat; // load follows the raw decrement
        if(!f.left_target) throw std::invalid_argument("unbound actual CE2210 import");
        f.active_site=leave_site;
        f.left_target(static_cast<::_RTL_CRITICAL_SECTION*>(f.captured_section));
        f.left=true;
    }
}
const volatile Word* texture_table(Word profile,
    NativeTextureAtlasRegistryStorageContext& c) {
    switch(profile) {
    case 0x00d61948u: return c.profile_00d61948;
    case 0x00d61870u: return c.profile_00d61870;
    case 0x00d618b0u: return c.profile_00d618b0;
    default: throw std::invalid_argument("unsupported genuine atlas texture family");
    }
}
Word texture_scalar(Word profile) {
    switch(profile) {
    case 0x00d61948u: return 0x00b3f590u;
    case 0x00d61870u: return 0x00b3f410u;
    case 0x00d618b0u: return 0x00b3f430u;
    default: throw std::invalid_argument("unsupported refreshed atlas texture family");
    }
}
void release_texture(NativeTextureAtlasRegistryTextureRetirement& f,
    NativeTextureAtlasRegistryStorageContext& c) {
    if(!f.decrement) throw std::invalid_argument("unbound captured actual CE2220 import");
    f.decrement_started=true;
    f.active_site=0x00aef100;
    f.decrement_result=f.decrement(static_cast<volatile long*>(pointer(bits(f.actual_texture)+4u)));
    f.decrement_returned=true;
    if(f.decrement_result!=0) return;
    f.active_site=0x00aef106;
    f.profile=word(f.actual_texture);
    f.slot00=texture_table(f.profile,c)[0]; // original captured-profile slot00
    if(f.slot00!=0x00bd30e0u)
        throw std::invalid_argument("unsupported atlas texture current slot00");
    // Genuine BD30E0 reloads the profile and slot04, forwarding flags1. Numeric
    // words validate that concrete route; none becomes a host code pointer.
    f.active_site=0x00bd30e0;
    f.deleting_profile=word(f.actual_texture);
    f.slot04=texture_table(f.deleting_profile,c)[1];
    if(f.slot04!=texture_scalar(f.deleting_profile))
        throw std::invalid_argument("unsupported atlas texture refreshed slot04");
    if(c.textures.texture_context().canonical_actual_owners!=&c.owners)
        throw std::invalid_argument("atlas textures require the same concrete canonical registry");
    if(f.deleting_profile!=0x00d61948u) (void)c.textures.special_contexts();
    // This lookup is pure and is reached ONLY after the exact actual+04 zero.
    // The caller proves this is textures' genuine Source-created companion.
    auto& reference=c.owners.resolve_actual(f.actual_texture);
    auto* const actual_count=std::launder(reinterpret_cast<std::atomic<std::int32_t>*>(
        pointer(bits(f.actual_texture)+4u)));
    if(&reference.reference_count!=actual_count)
        throw std::invalid_argument("atlas companion must borrow captured actual+04");
    f.terminal_companion=&reference;
    f.terminal_started=true;
    f.active_site=0x00aef10d;
    reference.release_zero_references(); // actual family terminal; no second decrement
    f.terminal_returned=true; // no subsequent raw/companion dereference
}

// Full inlined raw pointer-array retirement, including the signed-negative
// capacity/count paths. All operands that Original reloads stay current.
void destroy_array(void* actual, Word begin_at, Word count_at, Word capacity_at,
    NativeTextureAtlasRegistryArrayRetirement& a,
    NativeTextureAtlasRegistryDestroyFrame& f, bool texture) {
    a.started=true;
    a.captured_capacity=word(actual,capacity_at);
    f.native_state=texture?1:0;
    if(negative(a.captured_capacity) && less(a.captured_capacity,1u)) {
        a.allocation_started=true;
        f.active_site=texture?0x00aef12au:0x00aef1cfu;
        // BF55BE is a direct JMP BF681B; same shared source CRT allocator.
        a.replacement=singleton_lifetime_allocate({SingletonAllocationKind::pointer_slots,4,4});
        a.allocation_returned=true;
        a.copy_index=0;
        if(positive(word(actual,count_at))) {
            a.destination=bits(a.replacement);
            do {
                if(a.destination) {
                    const Word current_begin=word(actual,begin_at);
                    const Word value=word(pointer(current_begin+a.copy_index*4u));
                    put(pointer(a.destination),0,value);
                }
                a.copy_index+=1u;
                a.destination+=4u;
            } while(less(a.copy_index,word(actual,count_at)));
        }
        a.replaced_buffer=pointer(word(actual,begin_at));
        f.active_site=texture?0x00aef15bu:0x00aef1fdu;
        singleton_lifetime_free(a.replaced_buffer); // BF6989 -> BF65AC
        a.replaced_free_returned=true;
        put(actual,begin_at,bits(a.replacement));
        put(actual,capacity_at,1u);
        a.replacement_published=true;
    }
    const Word reached_count=word(actual,count_at);
    if(negative(reached_count)) {
        a.fill_displacement=reached_count*4u; // raw32 LEA, including wrap
        do {
            const Word destination=word(actual,begin_at)+a.fill_displacement;
            if(destination) put(pointer(destination),0,0);
            a.fill_displacement+=4u;
        } while(negative(a.fill_displacement)); // actual JS after ADD
    }
    if(positive(word(actual,count_at))) {
        do { put(actual,count_at,word(actual,count_at)-1u); }
        while(positive(word(actual,count_at)));
    }
    a.final_buffer=pointer(word(actual,begin_at)); // BEFORE count0 store
    put(actual,count_at,0);
    a.count_cleared=true;
    a.final_free_started=true;
    f.active_site=texture?0x00aef1b4u:0x00aef254u;
    singleton_lifetime_free(a.final_buffer);
    a.final_free_returned=true;
    // Original leaves begin/capacity (and consumed slot bytes) untouched.
}
} // namespace

NativeTextureAtlasRegistryStorageContext::NativeTextureAtlasRegistryStorageContext(
    const game::GameNativeReadOnlyData& original_data,
    void* volatile& manager, void* volatile& atlas,
    NativeTextureAtlasItemStorageContext& item_context, NativeRenderActualOwnerRegistry& registry,
    NativeTextureLoadOwners& texture_owners,
    NativeTextureAtlasRegistrySectionCall const volatile& enter,
    NativeTextureAtlasRegistrySectionCall const volatile& leave,
    NativeTextureAtlasRegistryDecrement const volatile& decrement)
    : data(original_data),manager_publication(manager),atlas_publication(atlas),
      items(item_context),owners(registry),textures(texture_owners),
      enter_iat(enter),leave_iat(leave),decrement_iat(decrement),
      profile_00d61948(static_cast<const volatile Word*>(data.data_at(0x00d61948,8))),
      profile_00d61870(static_cast<const volatile Word*>(data.data_at(0x00d61870,8))),
      profile_00d618b0(static_cast<const volatile Word*>(data.data_at(0x00d618b0,8))) {}

void* register_native_texture_atlas_registry_base_007364a0(void* actual,
    NativeTextureAtlasRegistryBaseFrame& f, NativeTextureAtlasRegistryStorageContext& c) {
    if(f.phase!=Phase::fresh) throw std::logic_error("atlas registry registration cannot replay");
    f.phase=Phase::entered; f.actual_registry=actual;
    try {
        f.native_state=0;                                      // 7364C0
        put(actual,0,0x00cfea58u);                             // 7364C8
        capture_and_enter(f,c,0x007364ceu,0x007364e7u);
        c.atlas_publication=actual; f.published=true;           // 7364F6
        f.active_site=0x007364fc;
        f.second_manager=get_native_singleton_manager_00415350(c.manager_publication);
        f.current_published_registry=c.atlas_publication;       // 736501
        f.active_site=0x0073650a;
        register_native_singleton_object_00bd0c30(f.second_manager,nullptr,f.current_published_registry);
        f.registered=true;
        leave_captured(f,c,0x00736518u);
        f.phase=Phase::complete;
        return actual;
    } catch(...) { f.failure=std::current_exception(); f.phase=Phase::failed; throw; }
}
void* construct_native_texture_atlas_registry_00737ee0(void* actual,
    NativeTextureAtlasRegistryConstructFrame& f, NativeTextureAtlasRegistryStorageContext& c) {
    if(f.phase!=Phase::fresh) throw std::logic_error("atlas registry construction cannot replay");
    f.phase=Phase::entered; f.actual_registry=actual;
    try {
        f.active_site=0x00737ee3;
        register_native_texture_atlas_registry_base_007364a0(actual,f.base,c);
        put(actual,0,0x00cfeae8u);                              // 737EEA
        put(actual,4,0); put(actual,8,0); put(actual,0xc,0);
        put(actual,0x10,0); put(actual,0x14,0); put(actual,0x18,0);
        f.phase=Phase::complete;
        return actual;
    } catch(...) { f.failure=std::current_exception(); f.phase=Phase::failed; throw; }
}
void destroy_native_texture_atlas_registry_base_007375e0(void* actual,
    NativeTextureAtlasRegistryBaseFrame& f, NativeTextureAtlasRegistryStorageContext& c) {
    if(f.phase!=Phase::fresh) throw std::logic_error("atlas registry base destruction cannot replay");
    f.phase=Phase::entered; f.actual_registry=actual;
    try {
        put(actual,0,0x00cfea58u);                              // 737600
        f.native_state=0;
        capture_and_enter(f,c,0x0073760eu,0x00737627u);
        f.active_site=0x00737636;
        f.second_manager=get_native_singleton_manager_00415350(c.manager_publication);
        f.current_published_registry=c.atlas_publication;       // 73763B
        f.active_site=0x00737644;
        unregister_native_singleton_object_00bcfca0(f.second_manager,nullptr,f.current_published_registry);
        f.unregistered=true;
        c.atlas_publication=nullptr; f.cleared=true;            // 737649
        leave_captured(f,c,0x0073765cu);
        put(actual,0,0x00ce3818u);                              // 737666
        f.phase=Phase::complete;
    } catch(...) { f.failure=std::current_exception(); f.phase=Phase::failed; throw; }
}
void destroy_native_texture_atlas_registry_00aef090(void* actual,
    NativeTextureAtlasRegistryDestroyFrame& f, NativeTextureAtlasRegistryStorageContext& c) {
    if(f.phase!=Phase::fresh) throw std::logic_error("atlas registry destruction cannot replay");
    f.phase=Phase::entered; f.actual_registry=actual;
    try {
        put(actual,0,0x00cfeae8u);                              // AEF0B0
        f.first_item_count=word(actual,8);
        f.item_index=0;
        f.native_state=2;                                    // AEF0BD
        if(positive(f.first_item_count)) {
            do {
                f.captured_item=pointer(word(pointer(word(actual,4)+f.item_index*4u)));
                if(f.captured_item) {
                    auto node=std::make_unique<NativeTextureAtlasRegistryItemRetirement>();
                    node->actual_item=f.captured_item;
                    f.item_calls.push_back(std::move(node)); // adopted BEFORE child
                    auto& call=*f.item_calls.back();
                    call.destroy_started=true; f.active_site=0x00aef0d3;
                    destroy_native_texture_atlas_item_00aee220(call.actual_item,call.destruction,c.items);
                    call.destroy_returned=true;
                    call.free_started=true; f.active_site=0x00aef0d9;
                    singleton_lifetime_free(call.actual_item);
                    call.free_returned=true;
                }
                f.item_index+=1u;
            } while(less(f.item_index,word(actual,8)));
        }
        f.texture_index=0;
        if(positive(word(actual,0x14))) {
            f.captured_decrement=c.decrement_iat;              // AEF0F0, once
            do {
                f.captured_texture=pointer(word(pointer(word(actual,0x10)+f.texture_index*4u)));
                // There is intentionally NO null-texture shortcut.
                auto node=std::make_unique<NativeTextureAtlasRegistryTextureRetirement>();
                node->actual_texture=f.captured_texture; node->decrement=f.captured_decrement;
                f.texture_calls.push_back(std::move(node)); // adopted BEFORE decrement
                f.active_site=0x00aef100;
                release_texture(*f.texture_calls.back(),c);
                f.texture_index+=1u;
            } while(less(f.texture_index,word(actual,0x14)));
        }
        destroy_array(actual,0x10,0x14,0x18,f.texture_array,f,true);
        destroy_array(actual,4,8,0xc,f.item_array,f,false);
        f.native_state=-1; f.active_site=0x00aef262;
        destroy_native_texture_atlas_registry_base_007375e0(actual,f.base,c);
        f.phase=Phase::complete;
    } catch(...) { f.failure=std::current_exception(); f.phase=Phase::failed; throw; }
}
void* delete_native_texture_atlas_registry_00737f10(void* actual,
    const volatile Word& actual_flags_home,
    NativeTextureAtlasRegistryScalarFrame& f, NativeTextureAtlasRegistryStorageContext& c) {
    if(f.phase!=Phase::fresh) throw std::logic_error("atlas registry scalar deletion cannot replay");
    f.phase=Phase::entered; f.actual_registry=actual;
    try {
        f.active_site=0x00737f13;
        destroy_native_texture_atlas_registry_00aef090(actual,f.destruction,c);
        f.reached_flags_byte=flags_byte(actual_flags_home);     // 737F18 AFTER body
        if(f.reached_flags_byte&1u) {
            f.free_started=true; f.active_site=0x00737f20;
            singleton_lifetime_free(actual);
            f.free_returned=true;
        }
        f.phase=Phase::complete;
        return actual;
    } catch(...) { f.failure=std::current_exception(); f.phase=Phase::failed; throw; }
}
} // namespace bsp
