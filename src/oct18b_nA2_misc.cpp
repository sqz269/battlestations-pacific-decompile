#include "bsp/oct18b_nA2_misc.hpp"
#include <cstddef>
#include <cstring>
extern "C" {
#include <lua.h>
}

namespace bsp::oct18b_nA2 {
namespace {
template<class T> T get(const void* p,std::size_t offset) noexcept {
    T value; std::memcpy(&value,static_cast<const std::byte*>(p)+offset,sizeof value); return value;
}
template<class T> void put(void* p,std::size_t offset,T value) noexcept {
    std::memcpy(static_cast<std::byte*>(p)+offset,&value,sizeof value);
}
}
void ignore_argument_004f16f0(std::uint32_t) noexcept {}
std::uint32_t return_zero_004f17d0() noexcept { return 0; }
void empty_leaf_0057fac0() noexcept {}
void* clear_word_0052d500(void* p) noexcept { put<std::uint32_t>(p,0,0); return p; }
std::uint8_t high_byte_00534550(std::uint32_t value) noexcept { return static_cast<std::uint8_t>(value>>24); }
std::uint32_t word_ec_0054d450(const void* p) noexcept { return get<std::uint32_t>(p,0xec); }
void* member_c_00553060(void* p) noexcept { return static_cast<std::byte*>(p)+0xc; }
void set_fields_0057d350(void* p,std::uint32_t a,std::uint32_t b,std::uint8_t c) noexcept {
    put(p,0x9c,a); put(p,0xa0,b); put(p,0xa4,c);
}
std::uint32_t class_kind_matches_bits_005457f0(const void* p,std::uint32_t requested) noexcept {
    const auto kind=get<std::uint32_t>(p,8);
    const bool matches=kind==requested ||
        (requested==1 && (kind==2 || kind==3)) ||
        (requested==4 && (kind==5 || kind==6 || kind==7)) ||
        (requested==0xc && (kind==0xf || kind==0xd || kind==0xe)) ||
        (requested==8 && ((kind>=9 && kind<=0xf) || kind==0x12 || kind==0x13));
    return (kind&0xffffff00u)|(matches ? 1u : 0u);
}
bool lua_stack_is_nil_006b7ee0(const void* p,std::int32_t index) {
    return lua_type(get<lua_State*>(p,4),index)==LUA_TNIL;
}
bool lua_stack_is_table_006b7f00(const void* p,std::int32_t index) {
    return lua_type(get<lua_State*>(p,4),index)==LUA_TTABLE;
}
void* identity_005828d0(void* p,std::uint32_t,std::uint32_t) noexcept { return p; }
void* identity_00612970(void* p,std::uint32_t) noexcept { return p; }
void update_phase_widgets_006054c0(void* p,float phase,PhaseWidgetCalls& calls) {
    put(p,0x20,phase);
    for (std::size_t slot=4;slot<=8;slot+=4) {
        void* const widget=get<void*>(p,slot);
        if (!widget) continue;
        // First arm consumes the incoming stack value, second reloads +20h after
        // both callbacks. Reload every other object field at its native use.
        const float source=slot==4 ? phase : get<float>(p,0x20);
        const double quotient=static_cast<double>(source)/get<float>(p,slot==4 ? 0x18 : 0x1c);
        const double doubled=quotient+quotient;
        const double product=doubled*3.1415927410125732421875;
        float angle=static_cast<float>(product+get<float>(p,0x14));
        calls.wrap_angle_00605070(&angle);
        if (get<std::uint8_t>(p,0)!=0) angle=-0.0f-angle;
        calls.widget_set_float_44(widget,angle);
    }
}
} // namespace bsp::oct18b_nA2
