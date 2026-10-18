// cc13_seg59b: segment 59 gun-bot bodies with no cc9 host (packet cc13-seg59A,
// docs/CC13_SEG59B.md). Names are hypotheses; see the header for the hooks. Each body was
// transcribed from its full export listing.

#include "bsp/cc13_seg59b.hpp"

#include <cstring>

namespace bsp::cc13_seg59b {

namespace {

ArchiveArg value_arg(std::uint32_t kind, void* address) noexcept {
    return ArchiveArg{kind, reinterpret_cast<std::uintptr_t>(address)};
}

}  // namespace

// ---------------------------------------------------------------------------
// 008FF260 BSP_GunBot_ConstructTorpedoClass. __thiscall(ECX = bot), RET; EAX = bot.
// Evidence: 008FF260 MOV EAX,[00E1998C]; 008FF269 CALL 0072BBD0 (one push, the descriptor);
// 008FF276 [bot] = 00D182B0; 008FF27C [bot+1Ch] = 00D18294; 008FF26E/008FF283 MOVSS copy of
// 00CE3D64 (bytes 00 40 1C 46 = 10000.0f) to +68h. operator new(70h) is the caller's (0072C866).
// ---------------------------------------------------------------------------
void* torpedo_bot_construct_008ff260(Seg59bHost& host, void* bot) {
    host.gun_bot_construct_base_0072bbd0(bot, host.image_dword(kTorpedoDescriptorGlobal));
    at<std::uint32_t>(bot, 0x00) = kTorpedoBotVtable;
    at<std::uint32_t>(bot, 0x1C) = kTorpedoBotObserverVtable;
    at<std::uint32_t>(bot, 0x68) = kTorpedoBotInitialRangeBits;
    return bot;
}

// ---------------------------------------------------------------------------
// 008FF360 BSP_GunBot_ReadPropertiesTorpedoClass. __thiscall(bot, archive), RET 4; vtable
// 00D182B0 slot +48h. Evidence: 008FF381 archive vt[04h]("_bot"); 008FF386 008FE660(bot,
// archive); 008FF392 vt[08h]; 008FF3BE vt[10h]("thinkTimeLeft", {2, bot+5Ch}); 008FF3EA
// vt[10h]("targetAngle", {2, bot+60h}); 008FF3EC..008FF3EF +58h = +50h.
// ---------------------------------------------------------------------------
void torpedo_bot_read_properties_008ff360(Seg59bHost& host, void* bot, void* archive) {
    host.archive_open_section_vt04(archive, key(kBotSectionKey));
    host.gun_bot_read_base_properties_008fe660(bot, archive);
    host.archive_close_section_vt08(archive);
    host.archive_read_field_vt10(archive, key(kThinkTimeLeftKey), value_arg(2, add(bot, 0x5C)));
    host.archive_read_field_vt10(archive, key(kTargetAngleKey), value_arg(2, add(bot, 0x60)));
    at<void*>(bot, 0x58) = ptr(bot, 0x50);
}

// ---------------------------------------------------------------------------
// 008FF400 BSP_GunBot_WritePropertiesTorpedoClass. __thiscall(bot, archive), RET 4; slot +4Ch.
// Evidence: 008FF421 vt[04h]("_bot"); 008FF426 008FE6D0; 008FF432 vt[08h]; 008FF434..008FF462
// vt[0Ch]("thinkTimeLeft", {2, MOVSS bits of +5Ch}); 008FF464..008FF492 vt[0Ch]("targetAngle",
// {2, bits of +60h}). MOVSS is a bit copy, so the float value is passed unchanged.
// ---------------------------------------------------------------------------
void torpedo_bot_write_properties_008ff400(Seg59bHost& host, void* bot, void* archive) {
    host.archive_open_section_vt04(archive, key(kBotSectionKey));
    host.gun_bot_write_base_properties_008fe6d0(bot, archive);
    host.archive_close_section_vt08(archive);
    host.archive_write_field_vt0c(archive, key(kThinkTimeLeftKey), ArchiveArg{2u, at<std::uint32_t>(bot, 0x5C)});
    host.archive_write_field_vt0c(archive, key(kTargetAngleKey), ArchiveArg{2u, at<std::uint32_t>(bot, 0x60)});
}

}  // namespace bsp::cc13_seg59b
