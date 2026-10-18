// cc13_seg59b: segment 59 gun-bot bodies with no cc9 host (packet cc13-seg59A,
// docs/CC13_SEG59B.md). Names are hypotheses; see the header for the hooks. Each body was
// transcribed from its full export listing.

#include "bsp/cc13_seg59b.hpp"
#include "bsp/gun_bot_ticks.hpp"

#include <cmath>
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
    gun_bot_read_base_properties_008fe660(host, bot, archive);
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
    gun_bot_write_base_properties_008fe6d0(host, bot, archive);
    host.archive_close_section_vt08(archive);
    host.archive_write_field_vt0c(archive, key(kThinkTimeLeftKey), ArchiveArg{2u, at<std::uint32_t>(bot, 0x5C)});
    host.archive_write_field_vt0c(archive, key(kTargetAngleKey), ArchiveArg{2u, at<std::uint32_t>(bot, 0x60)});
}

namespace {

// The shared tail of the three ClearFireTarget overrides (008FDBA0, 008FE7A0, 008FEA60): the base
// clear 0072BD50, then the gun's vtable[1E8h](0) when the owner's slot [[bot+50h]+1ACh] is 8 or
// AI-held (00927F10). `gun_offset` is where the class keeps its gun pointer.
void clear_fire_target_then_release(Seg59bHost& host, void* bot, std::size_t gun_offset) {
    host.gun_bot_clear_fire_target_0072bd50(bot);
    const std::int32_t slot = at<std::int32_t>(ptr(bot, 0x50), 0x1AC);
    if (slot == 8 || host.party_slot_is_ai_held_00927f10(slot)) {
        host.gun_vt1e8(ptr(bot, gun_offset), 0);
    }
}

void read_float(Seg59bHost& host, void* archive, const char* name, void* bot, std::size_t off) {
    host.archive_read_field_vt10(archive, key(name), value_arg(2, add(bot, off)));
}

void write_float(Seg59bHost& host, void* archive, const char* name, void* bot, std::size_t off) {
    host.archive_write_field_vt0c(archive, key(name), ArchiveArg{2u, at<std::uint32_t>(bot, off)});
}

}  // namespace

// ---------------------------------------------------------------------------
// 008FBC80 BSP_GunBot_Attach. __thiscall(ECX = bot, parent), RET 4; base vtable slot +04h.
// Evidence: 008FBC8B 00876020(ECX = parent, bot); 008FBC90/008FBC95 +50h = [parent+28h];
// 008FBC93 null -> 008FBCC3; 008FBCA1 unit->vt[5Ch](5) false -> 008FBCC3; 008FBCB4
// [bot+50h]->vt[12Ch]() then 008FBCBC bot->vt[18h](that); 008FBCCC bot->vt[18h](1).
// The bot vtable word is loaded (008FBCB2) before the vt[12Ch] call; nothing here rewrites it.
// ---------------------------------------------------------------------------
void gun_bot_attach_008fbc80(Seg59bHost& host, void* bot, void* parent) {
    host.tick_subnode_reparent_00876020(parent, bot);
    void* const unit = ptr(parent, 0x28);
    at<void*>(bot, 0x50) = unit;
    if (unit != nullptr && host.entity_is_kind_of_vt5c(unit, bsp::kEntityKindUnit)) {
        host.bot_set_skill_vt18(bot, host.unit_skill_level_vt12c(ptr(bot, 0x50)));
        return;
    }
    host.bot_set_skill_vt18(bot, 1);
}

// ---------------------------------------------------------------------------
// 008FE170 BSP_GunBot_ReadTargetTypeProperties. __thiscall(ECX = record = bot+38h, archive),
// RET 4. Evidence: 008FE192 vt[04h]("trgType" 00D18074); 008FE1A0..008FE1CE for i = 0, 1:
// vt[10h]({1, i}, {3, record+i}); 008FE1D7 vt[08h]; 008FE1D9 [record] != 0: 008FE200
// vt[10h]("eId" 00D18070, {1, &temp}) where temp is the archive argument's own stack slot
// (008FE1EA LEA EDI,[ESP+18h] = [entry ESP+4]), 008FE202/008FE207 word +2 = low word of temp;
// 008FE235 vt[10h]("e" 00D1806C, {4, record+4}); 008FE237 [record+1] != 0: 008FE267
// vt[10h]("pos" 00CF8FA4, {5, record+8}); 008FE293 vt[10h]("range" 00D18064, {2, record+14h}).
// ---------------------------------------------------------------------------
void gun_bot_read_target_type_properties_008fe170(Seg59bHost& host, void* record, void* archive) {
    host.archive_open_section_vt04(archive, key(kTargetTypeSectionKey));
    for (std::uint32_t i = 0; i < 2; ++i) {
        host.archive_read_field_vt10(archive, ArchiveArg{1u, i}, value_arg(3, add(record, i)));
    }
    host.archive_close_section_vt08(archive);
    if (at<std::uint8_t>(record, 0x00) != 0) {
        // The slot still holds the archive pointer; an archive that does not write leaves it.
        std::uint32_t temp = static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(archive));
        host.archive_read_field_vt10(archive, key(kEntityIdKey), value_arg(1, &temp));
        at<std::uint16_t>(record, 0x02) = static_cast<std::uint16_t>(temp);
        host.archive_read_field_vt10(archive, key(kEntityKey), value_arg(4, add(record, 0x04)));
    }
    if (at<std::uint8_t>(record, 0x01) != 0) {
        host.archive_read_field_vt10(archive, key(kPositionKey), value_arg(5, add(record, 0x08)));
    }
    read_float(host, archive, kRangeKey, record, 0x14);
}

// ---------------------------------------------------------------------------
// 008FE2A0 BSP_GunBot_WriteTargetTypeProperties. __thiscall(ECX = record, archive), RET 4.
// Evidence: 008FE2C2 vt[04h]("trgType"); 008FE2C6..008FE2F4 for i = 0, 1: vt[0Ch]({1, i},
// {3, byte record[i]}) with only the low byte stored (008FE2CE MOV [EAX+4],CL); 008FE2FD vt[08h];
// [record] != 0: 008FE329 vt[0Ch]("eId", {1, MOVZX word +2}), 008FE355 vt[0Ch]("e", {4, dword
// +4}); [record+1] != 0: 008FE387 vt[0Ch]("pos", {5, record+8}); 008FE3B7 vt[0Ch]("range", {2,
// bits of +14h}).
// ---------------------------------------------------------------------------
void gun_bot_write_target_type_properties_008fe2a0(Seg59bHost& host, void* record, void* archive) {
    host.archive_open_section_vt04(archive, key(kTargetTypeSectionKey));
    for (std::uint32_t i = 0; i < 2; ++i) {
        host.archive_write_field_vt0c(archive, ArchiveArg{1u, i},
                                      ArchiveArg{3u, at<std::uint8_t>(record, i)});
    }
    host.archive_close_section_vt08(archive);
    if (at<std::uint8_t>(record, 0x00) != 0) {
        host.archive_write_field_vt0c(archive, key(kEntityIdKey),
                                      ArchiveArg{1u, at<std::uint16_t>(record, 0x02)});
        host.archive_write_field_vt0c(archive, key(kEntityKey),
                                      ArchiveArg{4u, at<std::uint32_t>(record, 0x04)});
    }
    if (at<std::uint8_t>(record, 0x01) != 0) {
        host.archive_write_field_vt0c(archive, key(kPositionKey), value_arg(5, add(record, 0x08)));
    }
    write_float(host, archive, kRangeKey, record, 0x14);
}

// ---------------------------------------------------------------------------
// 008FE660 BSP_GunBot_ReadBaseProperties. __thiscall(ECX = bot, archive), RET 4.
// Evidence: 008FE682 vt[04h]("_process" 00D1807C); 008FE687 008746F0(ECX = bot, archive);
// 008FE693 vt[08h]; 008FE6B7 vt[10h]("level" 00CFE9B8, {1, &temp}) with temp the archive
// argument's stack slot (008FE6A1 LEA EBX,[ESP+18h]); 008FE6B9/008FE6C1 +34h = temp;
// 008FE6C4 008FE170(ECX = bot+38h, archive).
// ---------------------------------------------------------------------------
void gun_bot_read_base_properties_008fe660(Seg59bHost& host, void* bot, void* archive) {
    host.archive_open_section_vt04(archive, key(kProcessSectionKey));
    host.tick_node_read_properties_008746f0(bot, archive);
    host.archive_close_section_vt08(archive);
    std::uint32_t temp = static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(archive));
    host.archive_read_field_vt10(archive, key(kLevelKey), value_arg(1, &temp));
    at<std::uint32_t>(bot, 0x34) = temp;
    gun_bot_read_target_type_properties_008fe170(host, add(bot, 0x38), archive);
}

// ---------------------------------------------------------------------------
// 008FE6D0 BSP_GunBot_WriteBaseProperties. __thiscall(ECX = bot, archive), RET 4.
// Evidence: 008FE6F2 vt[04h]("_process"); 008FE6F7 00874700(ECX = bot, archive); 008FE703
// vt[08h]; 008FE726 vt[0Ch]("level", {1, [bot+34h]}); 008FE72C 008FE2A0(ECX = bot+38h, archive).
// ---------------------------------------------------------------------------
void gun_bot_write_base_properties_008fe6d0(Seg59bHost& host, void* bot, void* archive) {
    host.archive_open_section_vt04(archive, key(kProcessSectionKey));
    host.tick_node_write_properties_00874700(bot, archive);
    host.archive_close_section_vt08(archive);
    host.archive_write_field_vt0c(archive, key(kLevelKey), ArchiveArg{1u, at<std::uint32_t>(bot, 0x34)});
    gun_bot_write_target_type_properties_008fe2a0(host, add(bot, 0x38), archive);
}

// ---------------------------------------------------------------------------
// 008FE740 BSP_GunBot_ConstructLeadClass (AAGunnerBot). __thiscall(ECX = bot), RET; EAX = bot.
// Evidence: 008FE749 0072BBD0(bot, [00E19998]); 008FE74E [bot] = 00D180A0; 008FE754 +1Ch =
// 00D18088; 008FE75B +5Ch = 0.
// ---------------------------------------------------------------------------
void* aa_gunner_bot_construct_008fe740(Seg59bHost& host, void* bot) {
    host.gun_bot_construct_base_0072bbd0(bot, host.image_dword(kAAGunnerDescriptorGlobal));
    at<std::uint32_t>(bot, 0x00) = kAAGunnerBotVtable;
    at<std::uint32_t>(bot, 0x1C) = kAAGunnerBotObserverVtable;
    at<std::uint32_t>(bot, 0x5C) = 0;
    return bot;
}

// ---------------------------------------------------------------------------
// 008FBDC0 BSP_GunBot_AttachLeadClass. __thiscall(ECX = bot, parent), RET 4; slot +04h.
// Evidence: 008FBDC8 008FBC80(bot, parent); 008FBDCD/008FBDD3 +5Ch = +50h; 008FBDD0 XORPS,
// 008FBDD6..008FBDEA +60h, +64h, +68h, +6Ch, +70h = 0.0f.
// ---------------------------------------------------------------------------
void aa_gunner_bot_attach_008fbdc0(Seg59bHost& host, void* bot, void* parent) {
    gun_bot_attach_008fbc80(host, bot, parent);
    at<void*>(bot, bsp::kAAGunnerBotOffGun) = ptr(bot, 0x50);
    for (std::size_t off = 0x60; off <= 0x70; off += 4) at<std::uint32_t>(bot, off) = 0;
}

// ---------------------------------------------------------------------------
// 008FE7A0 BSP_GunBot_ClearFireTargetLeadClass. __fastcall(ECX = bot), RET; slot +30h.
// Evidence: 008FE7A3 0072BD50; 008FE7AB [[bot+50h]+1ACh]; 008FE7B1 CMP 8 JZ; 008FE7B7 00927F10;
// 008FE7C0..008FE7CD [bot+5Ch]->vt[1E8h](0).
// ---------------------------------------------------------------------------
void aa_gunner_bot_clear_fire_target_008fe7a0(Seg59bHost& host, void* bot) {
    clear_fire_target_then_release(host, bot, bsp::kAAGunnerBotOffGun);
}

// ---------------------------------------------------------------------------
// 008FE800 BSP_GunBot_ReadPropertiesLeadClass. __thiscall(bot, archive), RET 4; slot +48h.
// Evidence: 008FE821 vt[04h]("_bot"); 008FE826 008FE660; 008FE832 vt[08h]; vt[10h] at 008FE85E
// ("calcTick" 00D18118, {2, +58h}), 008FE88A ("angleErrTick" 00D18108, {2, +60h}), 008FE8B6
// ("actAngleErr" 00D180FC, {6, +64h}), 008FE8E2 ("dAngleErr" 00D180F0, {6, +6Ch}); 008FE8E7
// +5Ch = +50h.
// ---------------------------------------------------------------------------
void aa_gunner_bot_read_properties_008fe800(Seg59bHost& host, void* bot, void* archive) {
    host.archive_open_section_vt04(archive, key(kBotSectionKey));
    gun_bot_read_base_properties_008fe660(host, bot, archive);
    host.archive_close_section_vt08(archive);
    read_float(host, archive, kCalcTickKey, bot, 0x58);
    read_float(host, archive, kAngleErrTickKey, bot, 0x60);
    host.archive_read_field_vt10(archive, key(kActAngleErrKey), value_arg(6, add(bot, 0x64)));
    host.archive_read_field_vt10(archive, key(kDAngleErrKey), value_arg(6, add(bot, 0x6C)));
    at<void*>(bot, bsp::kAAGunnerBotOffGun) = ptr(bot, 0x50);
}

// ---------------------------------------------------------------------------
// 008FE8F0 BSP_GunBot_WritePropertiesLeadClass. __thiscall(bot, archive), RET 4; slot +4Ch.
// Evidence: 008FE8F0..008FE91B section "_bot", 008FE6D0, close; vt[0Ch] "calcTick" {2, bits
// +58h} (008FE924), "angleErrTick" {2, bits +60h} (008FE954), "actAngleErr" {6, +64h}
// (008FE989), "dAngleErr" {6, +6Ch} (008FE9B5).
// ---------------------------------------------------------------------------
void aa_gunner_bot_write_properties_008fe8f0(Seg59bHost& host, void* bot, void* archive) {
    host.archive_open_section_vt04(archive, key(kBotSectionKey));
    gun_bot_write_base_properties_008fe6d0(host, bot, archive);
    host.archive_close_section_vt08(archive);
    write_float(host, archive, kCalcTickKey, bot, 0x58);
    write_float(host, archive, kAngleErrTickKey, bot, 0x60);
    host.archive_write_field_vt0c(archive, key(kActAngleErrKey), value_arg(6, add(bot, 0x64)));
    host.archive_write_field_vt0c(archive, key(kDAngleErrKey), value_arg(6, add(bot, 0x6C)));
}

// ---------------------------------------------------------------------------
// 008FE9F0 BSP_GunBot_ConstructTurretClass (TailGunnerBot). __thiscall(ECX = bot), RET.
// Evidence: 008FE9F9 0072BBD0(bot, [00E199A0]); 008FEA00 byte +58h = 0; 008FEA03 +68h = 0;
// 008FEA06 [bot] = 00D18140; 008FEA0C +1Ch = 00D18124.
// ---------------------------------------------------------------------------
void* tail_gunner_bot_construct_008fe9f0(Seg59bHost& host, void* bot) {
    host.gun_bot_construct_base_0072bbd0(bot, host.image_dword(kTailGunnerDescriptorGlobal));
    at<std::uint8_t>(bot, 0x58) = 0;
    at<std::uint32_t>(bot, 0x68) = 0;
    at<std::uint32_t>(bot, 0x00) = kTailGunnerBotVtable;
    at<std::uint32_t>(bot, 0x1C) = kTailGunnerBotObserverVtable;
    return bot;
}

// ---------------------------------------------------------------------------
// 008FBEC0 BSP_GunBot_AttachTurretClass. __thiscall(ECX = bot, parent), RET 4; slot +04h.
// Evidence: 008FBEC8 008FBC80; 008FBED3 +68h = +50h; 008FBED6 byte +58h = 0; 008FBEDA/008FBEDF
// +70h, +74h = 0.0f.
// ---------------------------------------------------------------------------
void tail_gunner_bot_attach_008fbec0(Seg59bHost& host, void* bot, void* parent) {
    gun_bot_attach_008fbc80(host, bot, parent);
    at<void*>(bot, bsp::kTailGunnerBotOffGun) = ptr(bot, 0x50);
    at<std::uint8_t>(bot, 0x58) = 0;
    at<std::uint32_t>(bot, 0x70) = 0;
    at<std::uint32_t>(bot, 0x74) = 0;
}

// ---------------------------------------------------------------------------
// 008FEA60 BSP_GunBot_ClearFireTargetTurretClass. __fastcall(ECX = bot), RET; slot +30h.
// Evidence: 008FEA63 0072BD50; 008FEA71 CMP 8; 008FEA77 00927F10; 008FEA80..008FEA8D
// [bot+68h]->vt[1E8h](0).
// ---------------------------------------------------------------------------
void tail_gunner_bot_clear_fire_target_008fea60(Seg59bHost& host, void* bot) {
    clear_fire_target_then_release(host, bot, bsp::kTailGunnerBotOffGun);
}

// ---------------------------------------------------------------------------
// 008FEAC0 BSP_GunBot_ReadPropertiesTurretClass. __thiscall(bot, archive), RET 4; slot +48h.
// Evidence: section "_bot" (008FEAE1), 008FE660 (008FEAE6), close (008FEAF2); vt[10h] in order
// "actFireState" {3, +58h}, "nextFireState" {3, +59h}, "fireDelay" {2, +5Ch},
// "horzAngleError" {2, +60h}, "vertAngleError" {2, +64h}, "aimTime" {2, +6Ch}, "tr" {6, +70h},
// "angleError" {2, +84h}, "aimPeriodMin" {2, +88h}, "aimPeriodMax" {2, +8Ch}, "shootRange"
// {2, +90h} (008FEAF9..008FECBA); 008FECE4 +68h = +50h.
// ---------------------------------------------------------------------------
void tail_gunner_bot_read_properties_008feac0(Seg59bHost& host, void* bot, void* archive) {
    host.archive_open_section_vt04(archive, key(kBotSectionKey));
    gun_bot_read_base_properties_008fe660(host, bot, archive);
    host.archive_close_section_vt08(archive);
    host.archive_read_field_vt10(archive, key(kActFireStateKey), value_arg(3, add(bot, 0x58)));
    host.archive_read_field_vt10(archive, key(kNextFireStateKey), value_arg(3, add(bot, 0x59)));
    read_float(host, archive, kFireDelayKey, bot, 0x5C);
    read_float(host, archive, kHorzAngleErrorKey, bot, 0x60);
    read_float(host, archive, kVertAngleErrorKey, bot, 0x64);
    read_float(host, archive, kAimTimeKey, bot, 0x6C);
    host.archive_read_field_vt10(archive, key(kTrKey), value_arg(6, add(bot, 0x70)));
    read_float(host, archive, kAngleErrorKey, bot, 0x84);
    read_float(host, archive, kAimPeriodMinKey, bot, 0x88);
    read_float(host, archive, kAimPeriodMaxKey, bot, 0x8C);
    read_float(host, archive, kShootRangeKey, bot, 0x90);
    at<void*>(bot, 0x68) = ptr(bot, 0x50);
}

// ---------------------------------------------------------------------------
// 008FECF0 BSP_GunBot_WritePropertiesTurretClass. __thiscall(bot, archive), RET 4; slot +4Ch.
// Evidence: section "_bot", 008FE6D0 (008FED16), close; vt[0Ch] in the reader's order with the
// two state bytes as {3, low byte} (008FED24 MOV CL,[EDI+58h]; MOV [EAX+4],CL), the floats as
// {2, MOVSS bits} and "tr" as {6, +70h} (008FEE41..008FEE66).
// ---------------------------------------------------------------------------
void tail_gunner_bot_write_properties_008fecf0(Seg59bHost& host, void* bot, void* archive) {
    host.archive_open_section_vt04(archive, key(kBotSectionKey));
    gun_bot_write_base_properties_008fe6d0(host, bot, archive);
    host.archive_close_section_vt08(archive);
    host.archive_write_field_vt0c(archive, key(kActFireStateKey), ArchiveArg{3u, at<std::uint8_t>(bot, 0x58)});
    host.archive_write_field_vt0c(archive, key(kNextFireStateKey), ArchiveArg{3u, at<std::uint8_t>(bot, 0x59)});
    write_float(host, archive, kFireDelayKey, bot, 0x5C);
    write_float(host, archive, kHorzAngleErrorKey, bot, 0x60);
    write_float(host, archive, kVertAngleErrorKey, bot, 0x64);
    write_float(host, archive, kAimTimeKey, bot, 0x6C);
    host.archive_write_field_vt0c(archive, key(kTrKey), value_arg(6, add(bot, 0x70)));
    write_float(host, archive, kAngleErrorKey, bot, 0x84);
    write_float(host, archive, kAimPeriodMinKey, bot, 0x88);
    write_float(host, archive, kAimPeriodMaxKey, bot, 0x8C);
    write_float(host, archive, kShootRangeKey, bot, 0x90);
}

// ---------------------------------------------------------------------------
// 008FEFD0 BSP_GunBot_ConstructBallisticClass (AAFlakBot). __thiscall(ECX = bot), RET.
// Evidence: 008FEFD9 0072BBD0(bot, [00E1999C]); 008FEFE1 [bot] = 00D18238; 008FEFE7 +1Ch =
// 00D18220; 008FEFDE/008FEFEE XORPS then MOVSS +64h = +0.0f; 008FEFF3 +68h = 0.
// ---------------------------------------------------------------------------
void* aa_flak_bot_construct_008fefd0(Seg59bHost& host, void* bot) {
    host.gun_bot_construct_base_0072bbd0(bot, host.image_dword(kAAFlakDescriptorGlobal));
    at<std::uint32_t>(bot, 0x00) = kAAFlakBotVtable;
    at<std::uint32_t>(bot, 0x1C) = kAAFlakBotObserverVtable;
    at<std::uint32_t>(bot, 0x64) = 0;
    at<std::uint32_t>(bot, 0x68) = 0;
    return bot;
}

// ---------------------------------------------------------------------------
// 008FDBA0 BSP_GunBot_ClearFireTargetBallisticClass. __fastcall(ECX = bot), RET; slot +30h.
// Evidence: 008FDBA3 0072BD50; 008FDBB1 CMP 8; 008FDBB7 00927F10; 008FDBC0..008FDBCD
// [bot+68h]->vt[1E8h](0).
// ---------------------------------------------------------------------------
void aa_flak_bot_clear_fire_target_008fdba0(Seg59bHost& host, void* bot) {
    clear_fire_target_then_release(host, bot, 0x68);
}

// ---------------------------------------------------------------------------
// 008FF060 BSP_GunBot_ReadPropertiesBallisticClass. __thiscall(bot, archive), RET 4; +48h.
// Evidence: section "_bot", 008FE660 (008FF086), close; vt[10h] "horzAngleError" {2, +58h}
// (008FF0C3), "vertAngleError" {2, +5Ch} (008FF0EF), "distError" 00D18288 {2, +60h}
// (008FF116); 008FF118 +68h = +50h.
// ---------------------------------------------------------------------------
void aa_flak_bot_read_properties_008ff060(Seg59bHost& host, void* bot, void* archive) {
    host.archive_open_section_vt04(archive, key(kBotSectionKey));
    gun_bot_read_base_properties_008fe660(host, bot, archive);
    host.archive_close_section_vt08(archive);
    read_float(host, archive, kHorzAngleErrorKey, bot, 0x58);
    read_float(host, archive, kVertAngleErrorKey, bot, 0x5C);
    read_float(host, archive, kDistErrorKey, bot, 0x60);
    at<void*>(bot, 0x68) = ptr(bot, 0x50);
}

// ---------------------------------------------------------------------------
// 008FF130 BSP_GunBot_WritePropertiesBallisticClass. __thiscall(bot, archive), RET 4; +4Ch.
// Evidence: section "_bot", 008FE6D0 (008FF156), close; vt[0Ch] "horzAngleError" {2, bits +58h},
// "vertAngleError" {2, bits +5Ch}, "distError" {2, bits +60h} (008FF164..008FF1F2).
// ---------------------------------------------------------------------------
void aa_flak_bot_write_properties_008ff130(Seg59bHost& host, void* bot, void* archive) {
    host.archive_open_section_vt04(archive, key(kBotSectionKey));
    gun_bot_write_base_properties_008fe6d0(host, bot, archive);
    host.archive_close_section_vt08(archive);
    write_float(host, archive, kHorzAngleErrorKey, bot, 0x58);
    write_float(host, archive, kVertAngleErrorKey, bot, 0x5C);
    write_float(host, archive, kDistErrorKey, bot, 0x60);
}

// ---------------------------------------------------------------------------
// 008FF4A0 BSP_GunBot_ConstructDepthChargeClass. __thiscall(ECX = bot), RET; EAX = bot.
// Evidence: 008FF4A9 0072BBD0(bot, [00E19988]); 008FF4B6 [bot] = 00D18338; 008FF4BC +1Ch =
// 00D1831C; 008FF4C3 +58h = 0; 008FF4AE/008FF4CA MOVSS +60h = [00D7A260] (00 00 80 BF = -1.0f).
// ---------------------------------------------------------------------------
void* depth_charge_bot_construct_008ff4a0(Seg59bHost& host, void* bot) {
    host.gun_bot_construct_base_0072bbd0(bot, host.image_dword(kDepthChargeDescriptorGlobal));
    at<std::uint32_t>(bot, 0x00) = kDepthChargeBotVtable;
    at<std::uint32_t>(bot, 0x1C) = kDepthChargeBotObserverVtable;
    at<std::uint32_t>(bot, 0x58) = 0;
    at<std::uint32_t>(bot, 0x60) = kMinusOneBits;
    return bot;
}

// ---------------------------------------------------------------------------
// 008FF560 BSP_GunBot_ReadPropertiesDepthChargeClass. __thiscall(bot, archive), RET 4; +48h.
// Evidence: section "_bot", 008FE660 (008FF586), close; 008FF5BE vt[10h]("thinkTimeLeft",
// {2, +5Ch}); 008FF5C0/008FF5C3 +58h = +50h.
// ---------------------------------------------------------------------------
void depth_charge_bot_read_properties_008ff560(Seg59bHost& host, void* bot, void* archive) {
    host.archive_open_section_vt04(archive, key(kBotSectionKey));
    gun_bot_read_base_properties_008fe660(host, bot, archive);
    host.archive_close_section_vt08(archive);
    read_float(host, archive, kThinkTimeLeftKey, bot, 0x5C);
    at<void*>(bot, 0x58) = ptr(bot, 0x50);
}

// ---------------------------------------------------------------------------
// 008FF5D0 BSP_GunBot_WritePropertiesDepthChargeClass. __thiscall(bot, archive), RET 4; +4Ch.
// Evidence: section "_bot", 008FE6D0 (008FF5F6), close; 008FF632 vt[0Ch]("thinkTimeLeft",
// {2, bits +5Ch}).
// ---------------------------------------------------------------------------
void depth_charge_bot_write_properties_008ff5d0(Seg59bHost& host, void* bot, void* archive) {
    host.archive_open_section_vt04(archive, key(kBotSectionKey));
    gun_bot_write_base_properties_008fe6d0(host, bot, archive);
    host.archive_close_section_vt08(archive);
    write_float(host, archive, kThinkTimeLeftKey, bot, 0x5C);
}

// ---------------------------------------------------------------------------
// 008FB770 BSP_RobotConfig_ReadParametersBase. Base descriptor vtable 00D17DC0 slot +04h,
// __thiscall(descriptor, level_table, level), RET 8. Evidence: the whole body is 008FB770 RET 8.
// ---------------------------------------------------------------------------
void robot_config_read_parameters_base_008fb770(void* descriptor, void* level_table, std::int32_t level) {
    (void)descriptor;
    (void)level_table;
    (void)level;
}

// ---------------------------------------------------------------------------
// 008FE120 BSP_Entity_AsClass05OrNull. __fastcall(ECX = entity), RET; EAX = entity or null.
// Evidence: 008FE123 TEST ESI,ESI JZ -> null; 008FE12D entity->vt[5Ch](5); 008FE130 false -> null.
// Caller 009F1BC0 (ship AI approach frame state).
// ---------------------------------------------------------------------------
void* entity_as_kind05_or_null_008fe120(Seg59bHost& host, void* entity) {
    if (entity == nullptr) return nullptr;
    return host.entity_is_kind_of_vt5c(entity, 5) ? entity : nullptr;
}

// ---------------------------------------------------------------------------
// 008FE140 BSP_Entity_AsShipOrNull. __fastcall(ECX = entity), RET; EAX = entity or null.
// Evidence: 008FE143 null test; 008FE14D entity->vt[5Ch](6); 008FE150 false -> null. Caller
// 008FFF20 (the torpedo bot's tick; gun_bot_remainder.hpp names the address only).
// ---------------------------------------------------------------------------
void* entity_as_kind06_or_null_008fe140(Seg59bHost& host, void* entity) {
    if (entity == nullptr) return nullptr;
    return host.entity_is_kind_of_vt5c(entity, 6) ? entity : nullptr;
}

// ---------------------------------------------------------------------------
// 008EF7C0 BSP_SpawnSpec_SetField0C_Provisional. __thiscall(ECX = property, value), RET 4;
// EAX = property. Evidence: 008EF7C0..008EF7C9 MOV [EAX+0Ch],ECX. Callers (cc13_large61) use it
// on scene property records ("OwnerPlayer", the skill), so +0Ch is the record's value word.
// ---------------------------------------------------------------------------
void* scene_property_set_field0c_008ef7c0(void* property, std::uint32_t value) {
    at<std::uint32_t>(property, 0x0C) = value;
    return property;
}

namespace {

// The float compare the image repeats (008EF86B, 008EF927, 008EF93B, 008EF94F): FLD other; FLD
// this; FUCOMIP; LAHF; TEST AH,44h; JP -> not equal. Only an ordered equal passes (NaN fails).
bool float_field_equal(void* a, void* b, std::size_t off) {
    return at<float>(a, off) == at<float>(b, off);
}

}  // namespace

// ---------------------------------------------------------------------------
// 008EF820 BSP_SceneProperty_ValueEquals. __thiscall(ECX = property, other), RET 4; AL.
// Evidence: 008EF82A types differ -> false (008EF920); 008EF832 type > 0Bh -> false; table
// 008EF9FC: 0 int +0Ch (008EF885), 1 float +0Ch (008EF86B), 2 inline strcmp of the +0Ch strings
// (008EF8DB), 3 byte +0Ch (008EF842), 4 enum: declaration +28h and value +0Ch (008EF854),
// 5 reference (008EF895): both objects +18h null or both set, then pointer equality when set,
// else 00438E10(+1Ch, other+1Ch) == 0 (008EF8BE); 6 false (008EF920); 7 vec3 +0Ch/+10h/+14h
// (008EF927..008EF94F); 8..0Bh arrays: byte size +24h equal, then the inline memcmp of the +20h
// blocks (008EF95A..008EF9E6).
// ---------------------------------------------------------------------------
bool scene_property_value_equals_008ef820(Seg59bHost& host, void* property, void* other) {
    const std::uint32_t type = at<std::uint32_t>(property, 0x04);
    if (type != at<std::uint32_t>(other, 0x04) || type > 0x0B) return false;
    switch (type) {
    case 0:
        return at<std::uint32_t>(property, 0x0C) == at<std::uint32_t>(other, 0x0C);
    case 1:
        return float_field_equal(property, other, 0x0C);
    case 2:
        return std::strcmp(at<const char*>(property, 0x0C), at<const char*>(other, 0x0C)) == 0;
    case 3:
        return at<std::uint8_t>(property, 0x0C) == at<std::uint8_t>(other, 0x0C);
    case 4:
        return ptr(property, 0x28) == ptr(other, 0x28)
               && at<std::uint32_t>(property, 0x0C) == at<std::uint32_t>(other, 0x0C);
    case 5: {
        void* const mine = ptr(property, 0x18);
        void* const theirs = ptr(other, 0x18);
        if ((mine != nullptr) != (theirs != nullptr)) return false;
        if (mine != nullptr) return mine == theirs;
        return host.compare_insensitive_00438e10(at<const char*>(property, 0x1C),
                                                 at<const char*>(other, 0x1C)) == 0;
    }
    case 6:
        return false;
    case 7:
        return float_field_equal(property, other, 0x0C) && float_field_equal(property, other, 0x10)
               && float_field_equal(property, other, 0x14);
    default: {  // 8..0Bh
        const std::uint32_t size = at<std::uint32_t>(property, 0x24);
        if (size != at<std::uint32_t>(other, 0x24)) return false;
        return std::memcmp(ptr(property, 0x20), ptr(other, 0x20), size) == 0;
    }
    }
}

// ---------------------------------------------------------------------------
// 008F0B00 BSP_SceneEnumTable_FindNameByValue. __thiscall(ECX = table, value), RET 4.
// Evidence: 008F0B06/008F0B0A buckets at table+8, 40h of them (008F0B22..008F0B2A); each chain
// follows node+0Ch (008F0B1B); 008F0B16 [node+8] == value -> 008F0B33 name = [node+4], returned
// when non-null, else 00F89450 (008F0B3A); no match -> null (008F0B2D).
// ---------------------------------------------------------------------------
const char* scene_enum_table_find_name_008f0b00(Seg59bHost& host, void* table, std::uint32_t value) {
    for (std::uint32_t bucket = 0; bucket < 0x40; ++bucket) {
        for (void* node = ptr(table, 0x08 + bucket * 4); node != nullptr; node = ptr(node, 0x0C)) {
            if (at<std::uint32_t>(node, 0x08) == value) {
                const char* const name = at<const char*>(node, 0x04);
                return name != nullptr ? name : static_cast<const char*>(host.image_pointer(kEnumNameEmpty));
            }
        }
    }
    return nullptr;
}

// ---------------------------------------------------------------------------
// 008F0D80 BSP_SceneEnumDecl_FindNameByValue. __thiscall(ECX = declaration, value), RET 4.
// Evidence: 008F0D86/008F0D89 iterator.table = decl+4; 008F0D8E 008EEB10(&iterator); 008F0D93
// node null -> null (008F0DB6); 008F0D9F [node+8] == value -> 008F0DBF name = [node+4] or
// 00F89450 when null (008F0DC6); else 008F0DA9 008EEB60(&iterator) and loop while node != 0.
// ---------------------------------------------------------------------------
const char* scene_enum_decl_find_name_008f0d80(Seg59bHost& host, void* decl, std::uint32_t value) {
    Hash64Iterator iterator{add(decl, 0x04), nullptr, 0};
    host.hash64_iterator_begin_008eeb10(&iterator);
    while (iterator.node != nullptr) {
        if (at<std::uint32_t>(iterator.node, 0x08) == value) {
            const char* const name = at<const char*>(iterator.node, 0x04);
            return name != nullptr ? name : static_cast<const char*>(host.image_pointer(kEnumNameEmpty));
        }
        host.hash64_iterator_next_008eeb60(&iterator);
    }
    return nullptr;
}

// ---------------------------------------------------------------------------
// 008F0DF0 BSP_SceneProperty_GetStringValue. __fastcall(ECX = property), RET; EAX = text.
// Evidence: 008F0DF3 type 2 -> +0Ch (008F0DF8); 008F0DFC type 4 -> 008F0D80(+28h, +0Ch)
// (008F0E08); otherwise +1Ch when non-null (008F0E0E..008F0E1C), else "" 00CE3A0C (008F0E17).
// ---------------------------------------------------------------------------
const char* scene_property_string_value_008f0df0(Seg59bHost& host, void* property) {
    const std::uint32_t type = at<std::uint32_t>(property, 0x04);
    if (type == 2) return at<const char*>(property, 0x0C);
    if (type == 4) return scene_enum_decl_find_name_008f0d80(host, ptr(property, 0x28), at<std::uint32_t>(property, 0x0C));
    const char* const name = at<const char*>(property, 0x1C);
    return name != nullptr ? name : static_cast<const char*>(host.image_pointer(kEmptyString));
}

// ---------------------------------------------------------------------------
// 008F0E20 BSP_SceneProperty_GetValueText. __fastcall(ECX = property), RET; EAX = text.
// Evidence: type 2 -> +0Ch (008F0E25 -> 008F0F2A); type 4 -> 008F0D80 (008F0E2C -> 008F0F3A);
// type 5 -> +1Ch when non-null and non-empty, else "(none)" 00D16350 (008F0E3A..008F0E4C);
// type > 0Bh -> the buffer 00F89458 unwritten (008F0E52 -> 008F0F1F); table 008F0F50: 0 "%i"
// of +0Ch (008F0E62); 1 "%.2f" of +0Ch as a double (008F0E70); 3 [00E0C980] when byte +0Ch is
// non-zero else [00E0C97C] (008F0E91); 6 buffer[0] = 0 (008F0ED5); 7 "%.2f, %.2f, %.2f" of
// +0Ch/+10h/+14h (008F0EA6); 8 "(bin %u bytes)" of +24h (008F0EE2); 9/0Ah/0Bh
// "(floatarray|intarray|vec3array count=%u)" of 008EF7F0 (008F0EED/008F0EFA/008F0F07). Every
// format goes through 004B7F70 into 00F89458; the floats are FLD float / FSTP double (exact).
// ---------------------------------------------------------------------------
const char* scene_property_value_text_008f0e20(Seg59bHost& host, void* property) {
    const std::uint32_t type = at<std::uint32_t>(property, 0x04);
    if (type == 2) return at<const char*>(property, 0x0C);
    if (type == 4) return scene_enum_decl_find_name_008f0d80(host, ptr(property, 0x28), at<std::uint32_t>(property, 0x0C));
    if (type == 5) {
        const char* const name = at<const char*>(property, 0x1C);
        if (name != nullptr && name[0] != 0) return name;
        return static_cast<const char*>(host.image_pointer(kNoneText));
    }
    char* const buffer = static_cast<char*>(host.image_pointer(kValueTextBuffer));
    switch (type) {
    case 0:
        host.format_string_512_004b7f70_u32(buffer, "%i", at<std::uint32_t>(property, 0x0C));  // 00CE4BF8
        return buffer;
    case 1: {
        const double value = at<float>(property, 0x0C);
        host.format_string_512_004b7f70_f64(buffer, "%.2f", &value, 1);  // 00D16348
        return buffer;
    }
    case 3: {
        const std::uint32_t global = at<std::uint8_t>(property, 0x0C) != 0 ? kTrueTextGlobal : kFalseTextGlobal;
        return static_cast<const char*>(host.image_pointer(host.image_dword(global)));
    }
    case 6:
        buffer[0] = 0;
        return buffer;
    case 7: {
        const double values[3] = {at<float>(property, 0x0C), at<float>(property, 0x10), at<float>(property, 0x14)};
        host.format_string_512_004b7f70_f64(buffer, "%.2f, %.2f, %.2f", values, 3);  // 00D16334
        return buffer;
    }
    case 8:
        host.format_string_512_004b7f70_u32(buffer, "(bin %u bytes)", at<std::uint32_t>(property, 0x24));  // 00D16324
        return buffer;
    case 9:
        host.format_string_512_004b7f70_u32(buffer, "(floatarray count=%u)", host.scene_property_array_count_008ef7f0(property));  // 00D1630C
        return buffer;
    case 10:
        host.format_string_512_004b7f70_u32(buffer, "(intarray count=%u)", host.scene_property_array_count_008ef7f0(property));  // 00D162F8
        return buffer;
    case 11:
        host.format_string_512_004b7f70_u32(buffer, "(vec3array count=%u)", host.scene_property_array_count_008ef7f0(property));  // 00D162E0
        return buffer;
    default:
        return buffer;
    }
}

// ---------------------------------------------------------------------------
// The spawn reference frame: +0 vtable 00D16AA4, +4/+8/+0Ch dwords, byte +10h, +14h the observed
// entity (registered with 00694A60, the frame as the member), +18h a 4x4 float matrix.
// ---------------------------------------------------------------------------

// 008F84A0 BSP_SpawnRefFrame_Assign. __thiscall(ECX = frame, other), RET 4; EAX = frame.
// Evidence: 008F84AA +14h = other+14h; non-null -> 008F84B4 00694A60(ECX = entity, EDX = frame);
// 008F84C0 004134F0(frame+18h, other+18h). The previous entity is not unregistered.
void* spawn_ref_frame_assign_008f84a0(Seg59bHost& host, void* frame, void* other) {
    void* const entity = ptr(other, 0x14);
    at<void*>(frame, 0x14) = entity;
    if (entity != nullptr) host.observer_register_pair_00694a60(entity, frame);
    host.matrix_copy_004134f0(add(frame, 0x18), add(other, 0x18));
    return frame;
}

// 008F84D0 BSP_SpawnRefFrame_ConstructFromMatrix. __thiscall(ECX = frame, matrix), RET 4.
// Evidence: 008F84D5..008F84E0 +4/+8/+0Ch = 0, byte +10h = 0, +14h = 0; 008F84EC vtable
// 00D16AA4; 008F84F2 004134F0(frame+18h, matrix).
void* spawn_ref_frame_construct_from_matrix_008f84d0(Seg59bHost& host, void* frame, const void* matrix) {
    at<std::uint32_t>(frame, 0x04) = 0;
    at<std::uint32_t>(frame, 0x08) = 0;
    at<std::uint32_t>(frame, 0x0C) = 0;
    at<std::uint8_t>(frame, 0x10) = 0;
    at<std::uint32_t>(frame, 0x14) = 0;
    at<std::uint32_t>(frame, 0x00) = kSpawnRefFrameVtable;
    host.matrix_copy_004134f0(add(frame, 0x18), matrix);
    return frame;
}

// 008F8530 BSP_SpawnRefFrame_ConstructFromEntity. __thiscall(ECX = frame, entity), RET 4.
// Evidence: zero +4/+8/+0Ch and byte +10h, vtable 00D16AA4, +14h = entity (008F8569); non-null
// -> 008F8574 00694A60(entity, frame); 008F8579..008F85EC an identity matrix on the stack
// (1.0f = 00D7A24C at +0, +14h, +28h, +3Ch, the rest 0.0f) copied by 004134F0 into +18h. The SEH
// frame (state 0) only covers unwinding.
void* spawn_ref_frame_construct_from_entity_008f8530(Seg59bHost& host, void* frame, void* entity) {
    at<std::uint32_t>(frame, 0x04) = 0;
    at<std::uint32_t>(frame, 0x08) = 0;
    at<std::uint32_t>(frame, 0x0C) = 0;
    at<std::uint8_t>(frame, 0x10) = 0;
    at<std::uint32_t>(frame, 0x00) = kSpawnRefFrameVtable;
    at<void*>(frame, 0x14) = entity;
    if (entity != nullptr) host.observer_register_pair_00694a60(entity, frame);
    float identity[16] = {};
    identity[0] = identity[5] = identity[10] = identity[15] = 1.0f;
    host.matrix_copy_004134f0(add(frame, 0x18), identity);
    return frame;
}

// 008F8610 BSP_SpawnRefFrame_CopyConstruct. __thiscall(ECX = frame, other), RET 4.
// Evidence: zero +4/+8/+0Ch and byte +10h, vtable 00D16AA4 (008F8641), +14h = other+14h
// (008F864E); non-null -> 008F8656 00694A60; 008F8662 004134F0(frame+18h, other+18h).
void* spawn_ref_frame_copy_construct_008f8610(Seg59bHost& host, void* frame, void* other) {
    at<std::uint32_t>(frame, 0x04) = 0;
    at<std::uint32_t>(frame, 0x08) = 0;
    at<std::uint32_t>(frame, 0x0C) = 0;
    at<std::uint8_t>(frame, 0x10) = 0;
    at<std::uint32_t>(frame, 0x00) = kSpawnRefFrameVtable;
    void* const entity = ptr(other, 0x14);
    at<void*>(frame, 0x14) = entity;
    if (entity != nullptr) host.observer_register_pair_00694a60(entity, frame);
    host.matrix_copy_004134f0(add(frame, 0x18), add(other, 0x18));
    return frame;
}

// 008F86B0 BSP_SpawnRefFrame_FreezeToMatrix. __fastcall(ECX = frame), RET.
// Evidence: entity = +14h (008F86B4); non-null: byte [entity+C8h] == 0 -> 008F86C6
// 00414DB0(entity); 008F86D5 004134F0(frame+18h, entity+CCh); null: 008F86EB 004134F0(frame+18h,
// frame+18h); both arms 008F86DA/008F86F0 +14h = 0. No unregister here.
void spawn_ref_frame_freeze_to_matrix_008f86b0(Seg59bHost& host, void* frame) {
    void* const entity = ptr(frame, 0x14);
    if (entity != nullptr) {
        if (at<std::uint8_t>(entity, 0xC8) == 0) host.entity_pose_refresh_world_00414db0(entity);
        host.matrix_copy_004134f0(add(frame, 0x18), add(entity, 0xCC));
    } else {
        host.matrix_copy_004134f0(add(frame, 0x18), add(frame, 0x18));
    }
    at<std::uint32_t>(frame, 0x14) = 0;
}

// 008F8700 BSP_SpawnRefFrame_OnObservedReleased. __thiscall(ECX = frame, entity), RET 4; vtable
// 00D16AA4 slot +04h. Evidence: 008F8704 entity == +14h -> 008F8709 008F86B0(frame).
void spawn_ref_frame_on_observed_released_008f8700(Seg59bHost& host, void* frame, void* entity) {
    if (entity == ptr(frame, 0x14)) spawn_ref_frame_freeze_to_matrix_008f86b0(host, frame);
}

// ---------------------------------------------------------------------------
// 00903C60 BSP_MatrixInterpolator_ReadArchive (hypothesis). __thiscall(ECX = element, archive),
// RET 4. Evidence: vt[10h] in order "entity" 00CE5818 {4, element} (00903C8F), "translation"
// 00D184A4 {5, +4} (00903CBB), "rotation" 00D18498 {5, +10h} (00903CE7), "time" 00CE37A8 {2, +1Ch}
// (00903D13), "initialMatrix" 00D18488 {7, +20h} (00903D3F), "initialTime" 00D1847C {2, +60h}
// (00903D6B).
// ---------------------------------------------------------------------------
void matrix_interpolator_read_archive_00903c60(Seg59bHost& host, void* element, void* archive) {
    host.archive_read_field_vt10(archive, key(kEntityFieldKey), value_arg(4, element));
    host.archive_read_field_vt10(archive, key(kTranslationKey), value_arg(5, add(element, 0x04)));
    host.archive_read_field_vt10(archive, key(kRotationKey), value_arg(5, add(element, 0x10)));
    read_float(host, archive, kTimeKey, element, 0x1C);
    host.archive_read_field_vt10(archive, key(kInitialMatrixKey), value_arg(7, add(element, 0x20)));
    read_float(host, archive, kInitialTimeKey, element, 0x60);
}

// ---------------------------------------------------------------------------
// 00903D80 BSP_MatrixInterpolator_WriteArchive (hypothesis). __thiscall(ECX = element, archive),
// RET 4. Evidence: vt[0Ch] "entity" {4, [element]} (00903D8A/00903DB6), "translation" {5, +4},
// "rotation" {5, +10h}, "time" {2, MOVSS bits +1Ch} (00903E0B), "initialMatrix" {7, +20h},
// "initialTime" {2, bits +60h} (00903E67..00903E95).
// ---------------------------------------------------------------------------
void matrix_interpolator_write_archive_00903d80(Seg59bHost& host, void* element, void* archive) {
    host.archive_write_field_vt0c(archive, key(kEntityFieldKey), ArchiveArg{4u, at<std::uint32_t>(element, 0x00)});
    host.archive_write_field_vt0c(archive, key(kTranslationKey), value_arg(5, add(element, 0x04)));
    host.archive_write_field_vt0c(archive, key(kRotationKey), value_arg(5, add(element, 0x10)));
    write_float(host, archive, kTimeKey, element, 0x1C);
    host.archive_write_field_vt0c(archive, key(kInitialMatrixKey), value_arg(7, add(element, 0x20)));
    write_float(host, archive, kInitialTimeKey, element, 0x60);
}

// ---------------------------------------------------------------------------
// 00904F70 BSP_World_ReadArchive (hypothesis). __thiscall(ECX = world, archive), RET 4.
// Evidence: 00904F98 vt[04h]("matrixInterpolators" 00D184C0); index = 0 in the archive
// argument's slot (00904F9A); loop: 00904FB9 vt[14h]({1, index}) false -> done; 00904FDD
// 009045C0(ECX = list world+4B0h, head, head->prev, &stack element); 00904FE7 00904E50(list, 1);
// 00904FED..00904FF5 head->prev = node, node->prev->next = node; 00904FFF/00905009 00BF6713
// when the last node is the head (the checked --end()); 0090501E vt[04h]({1, index}); 00905026
// 00903C60(node+8, archive); 00905032 vt[08h]; index + 1. Then 0090504C vt[08h], 00905062
// vt[04h]("stationaries" 00D184B0), 00905066 0084F080(archive), 00905072 vt[08h].
// ---------------------------------------------------------------------------
void world_read_archive_00904f70(Seg59bHost& host, void* world, void* archive) {
    host.archive_open_section_vt04(archive, key(kMatrixInterpolatorsKey));
    for (std::uint32_t index = 0;; ++index) {
        if (!host.archive_has_section_vt14(archive, ArchiveArg{1u, index})) break;
        void* const list = add(world, 0x4B0);
        void* const head = ptr(world, 0x4B4);
        // The image copies a 64h-byte stack element it never initialized; every byte of it is
        // then read back below, so the zeroed stand-in leaves no trace once the reads succeed.
        std::uint8_t element[0x64] = {};
        void* const node = host.list_buy_node_009045c0(list, head, ptr(head, 0x04), element);
        host.list_inc_size_00904e50(list, 1);
        at<void*>(head, 0x04) = node;
        at<void*>(ptr(node, 0x04), 0x00) = node;
        void* const head_now = ptr(list, 0x04);
        void* const last = ptr(head_now, 0x04);
        if (last == head_now) host.checked_iterator_failure_00bf6713();
        if (last == ptr(list, 0x04)) host.checked_iterator_failure_00bf6713();
        host.archive_open_section_vt04(archive, ArchiveArg{1u, index});
        matrix_interpolator_read_archive_00903c60(host, add(last, 0x08), archive);
        host.archive_close_section_vt08(archive);
    }
    host.archive_close_section_vt08(archive);
    host.archive_open_section_vt04(archive, key(kStationariesKey));
    host.stationaries_read_archive_0084f080(archive);
    host.archive_close_section_vt08(archive);
}

// ---------------------------------------------------------------------------
// 009042B0 BSP_World_WriteArchive (hypothesis). __thiscall(ECX = world, archive), RET 4.
// Evidence: 009042D6 vt[04h]("matrixInterpolators"); node = head->next (009042D8..009042DE);
// loop while node != head (re-read each pass, 00904300): 0090430B 00BF6713 when node is the head
// (unreachable after that test, kept), 0090431E vt[04h]({1, index}), 00904324 00903D80(node+8,
// archive), 00904330 vt[08h], 00904337 00BF6713 again for the checked ++, node = node->next,
// index + 1. The 009042F0 CMP EBX,EBX container check always passes. Then 0090434B vt[08h],
// 00904365 vt[04h]("stationaries"), 00904369 0084F1A0(archive), 00904375 vt[08h].
// ---------------------------------------------------------------------------
void world_write_archive_009042b0(Seg59bHost& host, void* world, void* archive) {
    host.archive_open_section_vt04(archive, key(kMatrixInterpolatorsKey));
    void* const list = add(world, 0x4B0);
    void* node = ptr(ptr(list, 0x04), 0x00);
    for (std::uint32_t index = 0; node != ptr(list, 0x04); ++index) {
        if (node == ptr(list, 0x04)) host.checked_iterator_failure_00bf6713();
        host.archive_open_section_vt04(archive, ArchiveArg{1u, index});
        matrix_interpolator_write_archive_00903d80(host, add(node, 0x08), archive);
        host.archive_close_section_vt08(archive);
        if (node == ptr(list, 0x04)) host.checked_iterator_failure_00bf6713();
        node = ptr(node, 0x00);
    }
    host.archive_close_section_vt08(archive);
    host.archive_open_section_vt04(archive, key(kStationariesKey));
    host.stationaries_write_archive_0084f1a0(archive);
    host.archive_close_section_vt08(archive);
}

// ---------------------------------------------------------------------------
// 009052E0 BSP_ScoreRecord_AddField18 (hypothesis). __thiscall(ECX = scoring, slot, delta),
// RET 8; EAX = &record+18h. Evidence: 009052E4 IMUL slot,284h; 009052EA FLD [rec+18h];
// 009052EE FADD float [ESP+8]; 009052F2 FSTP back; 009052F6 LEA EAX. Caller 005B45C0
// (HUD map update). x87: one add of two floats rounded to float; the same at 24- and 53-bit
// precision control: double rounding through binary64 is innocuous for a binary32 addition
// (53 >= 2*24 + 2), so the 53-bit result rounds to the same float.
// ---------------------------------------------------------------------------
float* score_record_add_field18_009052e0(void* scoring, std::int32_t slot, float delta) {
    float* const field = &at<float>(scoring, static_cast<std::size_t>(slot * 0x284) + 0x18);
    *field = *field + delta;
    return field;
}

// ---------------------------------------------------------------------------
// 008F37E0 BSP_ScenePropertyBag_AddVector3PropertyXYZ. __thiscall(ECX = bag, key, x, y, z),
// RET 10h; EAX = 008F33F0's result. Evidence: 008F37E3..008F3806 MOVSS bit copies of x, y, z to
// a local; 008F380B 00BF681B(38h); non-null: +0Ch = x, +10h = y, [+0] = 00CE89D4, +4 = 7,
// +14h = z, +18h/+1Ch/+20h/+24h/+30h/+34h = 0, byte +2Ch = 1 (008F3819..008F3851, in that
// order); 008F3859 008F33F0(ECX = bag, key, record). Null: 008F386F 008F33F0(bag, key, 0).
// ---------------------------------------------------------------------------
void* scene_property_bag_add_vec3_008f37e0(Seg59bHost& host, void* bag, const char* key,
                                           std::uint32_t x_bits, std::uint32_t y_bits,
                                           std::uint32_t z_bits) {
    void* const record = host.operator_new_00bf681b(0x38);
    if (record == nullptr) return host.scene_property_bag_insert_008f33f0(bag, key, nullptr);
    at<std::uint32_t>(record, 0x0C) = x_bits;
    at<std::uint32_t>(record, 0x10) = y_bits;
    at<std::uint32_t>(record, 0x00) = kScenePropertyRecordVtable;
    at<std::uint32_t>(record, 0x04) = 7;
    at<std::uint32_t>(record, 0x14) = z_bits;
    at<std::uint32_t>(record, 0x18) = 0;
    at<std::uint32_t>(record, 0x1C) = 0;
    at<std::uint32_t>(record, 0x20) = 0;
    at<std::uint32_t>(record, 0x24) = 0;
    at<std::uint32_t>(record, 0x30) = 0;
    at<std::uint32_t>(record, 0x34) = 0;
    at<std::uint8_t>(record, 0x2C) = 1;
    return host.scene_property_bag_insert_008f33f0(bag, key, record);
}

// ---------------------------------------------------------------------------
// 008F3880 BSP_ScenePropertyBag_AddMatrixAsFloatArray. __thiscall(ECX = bag, key, matrix by
// value), RET 44h. Evidence: 008F3884/008F3886 pushes 1 and 10h; 008F3888 LEA of the by-value
// matrix; 008F388E 008F3510(ECX = bag, key, matrix, 10h, 1); 008F3893 RET 44h with its EAX.
// ---------------------------------------------------------------------------
void* scene_property_bag_add_matrix_008f3880(Seg59bHost& host, void* bag, const char* key,
                                             const float (&matrix)[16]) {
    return host.scene_property_bag_add_float_array_008f3510(bag, key, matrix, 0x10, 1);
}

// ---------------------------------------------------------------------------
// 008EEBD0 BSP_DwordArray_ConstructCapacity. __thiscall(ECX = array, capacity), RET 4.
// Evidence: 008EEBD9/008EEBDC +0Ch = +8 = capacity; 008EEBE1..008EEBED capacity*4 with MUL /
// SETO / NEG / OR (an overflow asks for FFFFFFFFh); 008EEBEE 00BF55BE; 008EEBF3 +0 = block;
// 008EEBF8 +4 = 0.
// ---------------------------------------------------------------------------
void* dword_array_construct_capacity_008eebd0(Seg59bHost& host, void* array, std::uint32_t capacity) {
    at<std::uint32_t>(array, 0x0C) = capacity;
    at<std::uint32_t>(array, 0x08) = capacity;
    const std::uint64_t bytes = static_cast<std::uint64_t>(capacity) * 4u;
    const std::uint32_t request = bytes > 0xFFFFFFFFull ? 0xFFFFFFFFu : static_cast<std::uint32_t>(bytes);
    at<void*>(array, 0x00) = host.operator_new_00bf55be(request);
    at<std::uint32_t>(array, 0x04) = 0;
    return array;
}

// ---------------------------------------------------------------------------
// 008EEC10 / 008EEC30 / 008EF630 / 008EFC30 BSP_OwnedBuffer_Free_*. __thiscall(ECX = slot), RET.
// Evidence (each): [slot] non-null -> _free([slot]) (00BF65AC at 008EEC1A; 00BF6989 at 008EEC3A,
// 008EF63A, 008EFC3A), then [slot] = 0 inside that arm.
// ---------------------------------------------------------------------------
void owned_buffer_free_008eec10(Seg59bHost& host, void* slot) {
    if (ptr(slot, 0) != nullptr) {
        host.free_00bf65ac(ptr(slot, 0));
        at<void*>(slot, 0) = nullptr;
    }
}

void owned_buffer_free_008eec30(Seg59bHost& host, void* slot) {
    if (ptr(slot, 0) != nullptr) {
        host.free_00bf6989(ptr(slot, 0));
        at<void*>(slot, 0) = nullptr;
    }
}

void owned_buffer_free_008ef630(Seg59bHost& host, void* slot) {
    if (ptr(slot, 0) != nullptr) {
        host.free_00bf6989(ptr(slot, 0));
        at<void*>(slot, 0) = nullptr;
    }
}

void owned_buffer_free_008efc30(Seg59bHost& host, void* slot) {
    if (ptr(slot, 0) != nullptr) {
        host.free_00bf6989(ptr(slot, 0));
        at<void*>(slot, 0) = nullptr;
    }
}

// ---------------------------------------------------------------------------
// 008EEFD0 BSP_SceneTokenizer_PeekFlag. __thiscall(ECX = tokenizer), RET; AL. Evidence: 008EEFD3
// 008D8A70(tokenizer); 008EEFD8 AL = byte +4.
// ---------------------------------------------------------------------------
std::uint8_t scene_tokenizer_peek_flag_008eefd0(Seg59bHost& host, void* tokenizer) {
    host.scene_tokenizer_peek_token_008d8a70(tokenizer);
    return at<std::uint8_t>(tokenizer, 0x04);
}

// ---------------------------------------------------------------------------
// 008EEFE0 BSP_SceneProperty_WriteBoolText. __thiscall(ECX = writer, value), RET 4 (by the
// tail-jump). Evidence: 008EEFE0 byte arg != 0 -> "true" 00CE4378, else "false" 00CE4370
// (008EEFEC); 008EEFF1 the argument slot is overwritten with the text; 008EEFF5 JMP 0094FF10.
// ---------------------------------------------------------------------------
void scene_property_write_bool_text_008eefe0(Seg59bHost& host, void* writer, bool value) {
    const char* const text = static_cast<const char*>(host.image_pointer(value ? 0x00CE4378u : 0x00CE4370u));
    host.scene_text_write_0094ff10(writer, text);
}

// ---------------------------------------------------------------------------
// 008EF030 BSP_ScenePropertyReader_Destruct. __thiscall(ECX = reader), RET. Evidence: 008EF04F
// [reader] = 00D162CC; 008EF05B 00950320(reader) under EH state 0; 008EF063 [reader] = 00CE4DFC.
// Reached only from the unwind funclet 00CA4700.
// ---------------------------------------------------------------------------
void scene_property_reader_destruct_008ef030(Seg59bHost& host, void* reader) {
    at<std::uint32_t>(reader, 0x00) = 0x00D162CCu;
    host.scene_text_chunk_list_clear_00950320(reader);
    at<std::uint32_t>(reader, 0x00) = 0x00CE4DFCu;
}

// ---------------------------------------------------------------------------
// 008EF730 BSP_ByteBuffer_Assign. __thiscall(ECX = buffer, data, size, copy), RET 0Ch.
// Evidence: old block non-null -> 008EF73B 00BF6989 and [buffer] = 0 (008EF743); 008EF753
// +4 = size; copy: 008EF758 00BF55BE(size), [buffer] = block (008EF765), 008EF766 00BF7680(block,
// data, size); else 008EF777 [buffer] = data (borrowed).
// ---------------------------------------------------------------------------
void byte_buffer_assign_008ef730(Seg59bHost& host, void* buffer, void* data, std::uint32_t size, bool copy) {
    if (ptr(buffer, 0) != nullptr) {
        host.free_00bf6989(ptr(buffer, 0));
        at<void*>(buffer, 0) = nullptr;
    }
    at<std::uint32_t>(buffer, 0x04) = size;
    if (copy) {
        void* const block = host.operator_new_00bf55be(size);
        at<void*>(buffer, 0) = block;
        host.memcpy_00bf7680(block, data, size);
        return;
    }
    at<void*>(buffer, 0) = data;
}

// ---------------------------------------------------------------------------
// 008EFBC0 BSP_ScenePropertyPtr_CompareOrder. __cdecl(a, b), RET; EAX. Evidence: 008EFBC0..
// 008EFBD2 [[a]+34h] - [[b]+34h]. No code xrefs (a qsort-style comparator by address).
// ---------------------------------------------------------------------------
std::int32_t scene_property_ptr_compare_order_008efbc0(const void* a, const void* b) {
    void* const left = *static_cast<void* const*>(a);
    void* const right = *static_cast<void* const*>(b);
    return static_cast<std::int32_t>(at<std::uint32_t>(left, 0x34) - at<std::uint32_t>(right, 0x34));
}

// ---------------------------------------------------------------------------
// 008F0010 BSP_PointerArray_PushBackGrow. __thiscall(ECX = array {data, count, capacity, grow},
// value*), RET 4; EAX = count. Evidence: 008F0016 count == capacity -> capacity += grow
// (008F001E..008F0021); 008F0035 00BF55BE(capacity*4, saturated on overflow); 008F0048
// 00BF7680(new, old, count*4); old non-null -> 008F0057 00BF6989, [array] = 0; 008F0065
// [array] = new; 008F0070 data[count] = *value; 008F0073 count + 1.
// ---------------------------------------------------------------------------
std::uint32_t pointer_array_push_back_grow_008f0010(Seg59bHost& host, void* array, const std::uint32_t* value) {
    if (at<std::uint32_t>(array, 0x04) == at<std::uint32_t>(array, 0x08)) {
        const std::uint32_t capacity = at<std::uint32_t>(array, 0x08) + at<std::uint32_t>(array, 0x0C);
        at<std::uint32_t>(array, 0x08) = capacity;
        const std::uint64_t bytes = static_cast<std::uint64_t>(capacity) * 4u;
        const std::uint32_t request = bytes > 0xFFFFFFFFull ? 0xFFFFFFFFu : static_cast<std::uint32_t>(bytes);
        void* const block = host.operator_new_00bf55be(request);
        host.memcpy_00bf7680(block, ptr(array, 0), at<std::uint32_t>(array, 0x04) * 4u);
        if (ptr(array, 0) != nullptr) {
            host.free_00bf6989(ptr(array, 0));
            at<void*>(array, 0) = nullptr;
        }
        at<void*>(array, 0) = block;
    }
    const std::uint32_t count = at<std::uint32_t>(array, 0x04);
    static_cast<std::uint32_t*>(ptr(array, 0))[count] = *value;
    at<std::uint32_t>(array, 0x04) = count + 1;
    return count + 1;
}

// ---------------------------------------------------------------------------
// 008F0600 BSP_OwnedPolymorphic_Reset. __thiscall(ECX = slot), RET. Evidence: [slot] non-null
// -> 008F060F [slot]->vtable[0](1); 008F0611 [slot] = 0 (that arm only).
// ---------------------------------------------------------------------------
void owned_polymorphic_reset_008f0600(Seg59bHost& host, void* slot) {
    if (ptr(slot, 0) != nullptr) {
        host.object_scalar_delete_vt00(ptr(slot, 0), 1);
        at<void*>(slot, 0) = nullptr;
    }
}

// ---------------------------------------------------------------------------
// 008F1010 BSP_PropertyHash64_ForEach. __thiscall(ECX = owner), RET. Evidence: iterator.table =
// owner+4 (008F1013); 008F101C 008EF650(&iterator); while node != 0 (008F1021/008F1030):
// 008F102B 008EEA80(&iterator). The per-node body is empty in the image.
// ---------------------------------------------------------------------------
void property_hash64_for_each_008f1010(Seg59bHost& host, void* owner) {
    Hash64Iterator iterator{add(owner, 0x04), nullptr, 0};
    host.hash64_iterator_begin_008ef650(&iterator);
    while (iterator.node != nullptr) host.hash64_iterator_next_008eea80(&iterator);
}

namespace {

// The native string release used throughout: data (+4) non-null -> 00419CC0, then
// 00BD1510(pool; data, size (+0) + 1, 1). The header itself is left as it is.
void release_native_string(Seg59bHost& host, void* string) {
    if (ptr(string, 0x04) != nullptr) {
        void* const pool = host.sized_pool_singleton_00419cc0();
        host.sized_pool_return_block_00bd1510(pool, ptr(string, 0x04), at<std::uint32_t>(string, 0x00) + 1u, 1u);
    }
}

// The bucket unlink shared by 008F18B0 and 008F4030: a head hit stores node->next (+0Ch) into
// the bucket; otherwise the chain is walked to the node's predecessor (no end test, as in the
// image) and the predecessor's link takes node->next.
void property_map_unlink_node(void* map, std::uint32_t bucket, void* node) {
    void* const head = ptr(map, 0x08 + bucket * 4u);
    if (head != node) {
        void* previous = nullptr;
        void* cursor = head;
        do {
            previous = cursor;
            cursor = ptr(cursor, 0x0C);
        } while (cursor != node);
        if (previous != nullptr) {
            at<void*>(previous, 0x0C) = ptr(node, 0x0C);
            return;
        }
    }
    at<void*>(map, 0x08 + bucket * 4u) = ptr(node, 0x0C);
}

}  // namespace

// ---------------------------------------------------------------------------
// 008EFA30 BSP_SceneText_WriteMultiline. __stdcall(text, writer), RET 8 (ECX unused).
// Evidence: null text -> return (008EFA3E). Per piece: 008EFA4F _strchr(text, 0Ah); length =
// newline - text + 1 (008EFA5D) or strlen (inline, 008EFA70); piece 1 -> writer+8 += 1
// (008EFA86) then 0094FFA0(writer, 0); later pieces -> 0094FFA0(writer, 0) only (008EFA94);
// 008EFAA4 0094FF10(writer, "\"" 00CE436C); 008EFAAF 008EE5F0(text, length); 008EFAB7
// 0094FF10(that writer, escaped); 008EFAC3 0094FF10(result, "\""); count += 1 (008EFAC8);
// stop without a newline, else text = newline + 1 while non-null (008EFAD1). After: count > 1
// (signed, 008EFADC) and writer+8 != 0 -> writer+8 -= 1 (008EFAF0).
// ---------------------------------------------------------------------------
void scene_text_write_multiline_008efa30(Seg59bHost& host, const char* text, void* writer) {
    std::int32_t count = 0;
    if (text == nullptr) return;
    for (;;) {
        const char* const newline = host.strchr_00bf86f0(text, 0x0A);
        const std::uint32_t length = newline != nullptr
            ? static_cast<std::uint32_t>(newline - text) + 1u
            : static_cast<std::uint32_t>(std::strlen(text));
        if (count == 1) {
            at<std::uint32_t>(writer, 0x08) += 1u;
            host.scene_text_writer_manipulator_0094ffa0(writer, 0);
        } else if (count != 0) {
            host.scene_text_writer_manipulator_0094ffa0(writer, 0);
        }
        const char* const quote = static_cast<const char*>(host.image_pointer(0x00CE436Cu));
        void* const chained = host.scene_text_write_0094ff10(writer, quote);
        const char* const escaped = host.scene_text_escape_008ee5f0(text, length);
        void* const after = host.scene_text_write_0094ff10(chained, escaped);
        host.scene_text_write_0094ff10(after, quote);
        ++count;
        if (newline == nullptr) break;
        text = newline + 1;
        if (text == nullptr) break;
    }
    if (count > 1 && at<std::uint32_t>(writer, 0x08) != 0) at<std::uint32_t>(writer, 0x08) -= 1u;
}

// ---------------------------------------------------------------------------
// 008F08E0 BSP_ScenePropertyBag_ResolveReferences. __thiscall(ECX = bag, callback, context),
// RET 8; AL. Evidence: iterator {bag+4, node, bucket} (008F08E3); 008F08EE 00480690 begin;
// result = 1 (008F08F9). Per node, record = [node+8]: type 5 with +18h == 0 -> while the result
// is still 1, 008F0929 callback(ECX = [record+1Ch], EDX = context), +18h = EAX, result = EAX !=
// 0; once 0, later unresolved references are skipped (008F0937). Type 6 -> 008F0945 recursion
// on [record+0Ch] (result ignored). 008F094E 0047E480 next while node != 0.
// ---------------------------------------------------------------------------
bool scene_property_bag_resolve_references_008f08e0(Seg59bHost& host, void* bag, std::uintptr_t callback,
                                                    void* context) {
    Hash64Iterator iterator{add(bag, 0x04), nullptr, 0};
    host.scene_property_bag_iterator_begin_00480690(&iterator);
    bool result = true;
    while (iterator.node != nullptr) {
        void* const record = ptr(iterator.node, 0x08);
        const std::uint32_t type = at<std::uint32_t>(record, 0x04);
        if (type == 5) {
            if (ptr(record, 0x18) == nullptr) {
                if (result) {
                    void* const resolved = host.call_reference_resolver(callback, ptr(record, 0x1C), context);
                    at<void*>(record, 0x18) = resolved;
                    result = resolved != nullptr;
                } else {
                    result = false;
                }
            }
        } else if (type == 6) {
            scene_property_bag_resolve_references_008f08e0(host, ptr(record, 0x0C), callback, context);
        }
        host.scene_property_bag_iterator_next_0047e480(&iterator);
    }
    return result;
}

// ---------------------------------------------------------------------------
// 008F18B0 BSP_PropertyMap_RenameKey. __thiscall(ECX = map, old key, new key), RET 8; AL.
// Evidence: 008F18C0 00435C40(old, new) equal -> 1; 008F18D1 0043B8B0(map, new, &bucket)
// found -> 0; 008F18EA 0043B8B0(map, old, &bucket) none -> 0; unlink from bucket (008F18F5..
// 008F1919); 008F1920 0043B760(map, new); node+0Ch = bucket head, head = node (008F192B/
// 008F1931); 008F1933 00425F40(node, new); 1.
// ---------------------------------------------------------------------------
bool property_map_rename_key_008f18b0(Seg59bHost& host, void* map, void* old_key, void* new_key) {
    if (host.native_string_equals_insensitive_00435c40(old_key, new_key)) return true;
    std::uint32_t bucket = 0;
    if (host.property_map_find_node_0043b8b0(map, new_key, &bucket) != nullptr) return false;
    void* const node = host.property_map_find_node_0043b8b0(map, old_key, &bucket);
    if (node == nullptr) return false;
    property_map_unlink_node(map, bucket, node);
    const std::uint32_t target = host.property_key_compute_bucket_0043b760(map, new_key);
    at<void*>(node, 0x0C) = ptr(map, 0x08 + target * 4u);
    at<void*>(map, 0x08 + target * 4u) = node;
    host.native_string_assign_header_00425f40(node, new_key);
    return true;
}

// ---------------------------------------------------------------------------
// 008F2AA0 / 008F2AE0 BSP_SceneNameNode_ScalarDeletingDtor(Copy). __thiscall(ECX = node, flags),
// RET 4; EAX = node. Evidence: name data +4 non-null -> 00419CC0, 00BD1510(data, size+1, 1)
// (008F2AB8 / 008F2AF8); flags & 1 -> 0043B0A0(ECX = 00E17578 / 00E17540, node) (008F2AC6 /
// 008F2B06).
// ---------------------------------------------------------------------------
void* scene_name_node_scalar_deleting_dtor_008f2aa0(Seg59bHost& host, void* node, std::uint32_t flags) {
    release_native_string(host, node);
    if ((flags & 1u) != 0) host.enum_node_pool_return_0043b0a0(host.image_pointer(0x00E17578u), node);
    return node;
}

void* scene_name_node_scalar_deleting_dtor_008f2ae0(Seg59bHost& host, void* node, std::uint32_t flags) {
    release_native_string(host, node);
    if ((flags & 1u) != 0) host.enum_node_pool_return_0043b0a0(host.image_pointer(0x00E17540u), node);
    return node;
}

// ---------------------------------------------------------------------------
// 008F2B20 BSP_SceneSymbol_CreateFromName. __thiscall(ECX = list, name), RET 4; EAX.
// Evidence: 008F2B3B 00BF681B(8); non-null -> 008F2B5A 0041E870(block, name), else 0;
// 008F2B70 0094F270(list, header, 1). The SEH state only frees the block on a throw.
// ---------------------------------------------------------------------------
std::uint32_t scene_symbol_create_from_name_008f2b20(Seg59bHost& host, void* list, const char* name) {
    void* const block = host.operator_new_00bf681b(8);
    void* const header = block != nullptr ? host.native_string_assign_0041e870(block, name) : nullptr;
    return host.string_key_list_append_owned_header_0094f270(list, header, 1);
}

// ---------------------------------------------------------------------------
// 008F2CF0 BSP_PropertyMap_RenameKeyCString. __thiscall(ECX = owner, old name, new name), RET 8.
// Evidence: 008F2D14 0041E870(local A, new name); 008F2D2A 0041E870(local B, old name);
// 008F2D41 008F18B0(ECX = owner+4, B, A) (result unused); release B (008F2D63), then A
// (008F2D8A), each through 00419CC0 / 00BD1510.
// ---------------------------------------------------------------------------
void property_map_rename_key_cstring_008f2cf0(Seg59bHost& host, void* owner, const char* old_name,
                                              const char* new_name) {
    alignas(4) std::uint8_t new_key[8] = {};
    alignas(4) std::uint8_t old_key[8] = {};
    host.native_string_assign_0041e870(new_key, new_name);
    host.native_string_assign_0041e870(old_key, old_name);
    property_map_rename_key_008f18b0(host, add(owner, 0x04), old_key, new_key);
    release_native_string(host, old_key);
    release_native_string(host, new_key);
}

// ---------------------------------------------------------------------------
// 008F4030 BSP_PropertyMap_UnlinkKey. __thiscall(ECX = map, key), RET 4; AL. Evidence:
// 008F403F 0043B8B0(map, key, &bucket); none -> 0 (008F4095); unlink from bucket (008F4048..
// 008F407B); 008F406B / 008F4085 0043D2D0(map, node); count +4 -= 1; 1.
// ---------------------------------------------------------------------------
bool property_map_unlink_key_008f4030(Seg59bHost& host, void* map, const void* key) {
    std::uint32_t bucket = 0;
    void* const node = host.property_map_find_node_0043b8b0(map, key, &bucket);
    if (node == nullptr) return false;
    property_map_unlink_node(map, bucket, node);
    host.property_map_node_destroy_0043d2d0(map, node);
    at<std::uint32_t>(map, 0x04) -= 1u;
    return true;
}

// ---------------------------------------------------------------------------
// 008F7CF0 BSP_RandomThreadsScope_Construct. __thiscall(ECX = scope), RET; EAX = scope.
// Evidence: 008F7CF3 00BD2FE0 with ECX = scope unchanged; 008F7CF8 EAX = scope. No code xrefs.
// ---------------------------------------------------------------------------
void* random_threads_scope_construct_008f7cf0(Seg59bHost& host, void* scope) {
    host.random_threads_register_current_00bd2fe0(scope);
    return scope;
}

// ---------------------------------------------------------------------------
// 008F7D20 BSP_SingletonLifetime_DestroyGlobal01090AA0. No arguments, RET. Evidence: [01090AA0]
// non-null -> 008F7D2D 00BD0400(ECX = object), 008F7D33 _free 00BF65AC(object), 008F7D3B
// [01090AA0] = 0 (that arm only). No code xrefs (an atexit-style entry).
// ---------------------------------------------------------------------------
void singleton_lifetime_destroy_global_008f7d20(Seg59bHost& host) {
    void** const slot = static_cast<void**>(host.image_pointer(0x01090AA0u));
    void* const object = *slot;
    if (object == nullptr) return;
    host.singleton_lifetime_destroy_00bd0400(object);
    host.free_00bf65ac(object);
    *slot = nullptr;
}

// ---------------------------------------------------------------------------
// 008F7D80 BSP_RandomThreadsScope_Release. __thiscall(ECX = slot), RET. Evidence: [slot] non-null
// -> 008F7D8A 00BD3050 with ECX = slot unchanged, 008F7D90 _free 00BF65AC([slot]), 008F7D98
// [slot] = 0. No code xrefs.
// ---------------------------------------------------------------------------
void random_threads_scope_release_008f7d80(Seg59bHost& host, void* slot) {
    void* const object = ptr(slot, 0);
    if (object == nullptr) return;
    host.random_threads_unregister_current_00bd3050(slot);
    host.free_00bf65ac(object);
    at<void*>(slot, 0) = nullptr;
}

namespace {

void push_dword(std::uint32_t* args, std::uint32_t& count, std::uint32_t value) { args[count++] = value; }

void push_pointer(std::uint32_t* args, std::uint32_t& count, const void* value) {
    args[count++] = static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(value));
}

// FSTP double [ESP]: the low dword sits first on the stack.
void push_double_from_float(std::uint32_t* args, std::uint32_t& count, float value) {
    const double widened = static_cast<double>(value);
    std::uint64_t bits = 0;
    std::memcpy(&bits, &widened, sizeof bits);
    args[count++] = static_cast<std::uint32_t>(bits);
    args[count++] = static_cast<std::uint32_t>(bits >> 32);
}

void trace(Seg59bHost& host, std::uint32_t format, const std::uint32_t* args, std::uint32_t count) {
    host.trace_printf_004254b0(static_cast<const char*>(host.image_pointer(format)), args, count);
}

}  // namespace

// ---------------------------------------------------------------------------
// 008F2540 BSP_ScenePropertyBag_WriteText. __thiscall(ECX = bag, depth), RET 4. A debug dump:
// every line goes to 004254B0, a trivial body in this build.
// Evidence: iterator {bag+4} and 008F256B 00480690 begin first; indent string {0, 0} resized by
// 008F2591 0041DD40(depth*4+4, 1); data non-null -> 008F25A6 memset(data, ' ', size); 008F25BE
// "PropTree {\n" D164D8. Per node: record = [node+8]; "%s%s [%i]= " D164CC with indent (or
// F89450), [node+4] (or F89450), [record+34h] (008F25F7); table 008F27F0 on [record+4]:
// 0 "%i;\n", 1 "%f;\n" (float widened), 2 "\"%s\";\n", 3 byte +0Ch "true;\n" D164C4 /
// "false;\n" D164BC, 4 "%i, %s:%s;\n" (value, decl+11Ch, 008F0D80(decl, value)), 5 "ref [%s];\n"
// (+1Ch or CE3A0C), 6 008F270E recursion on +0Ch with depth+1, 7 "(%.2f, %.2f, %.2f);\n",
// 8 "binary %X (%u bytes)\n" (+20h, +24h), 9 "floatarray %X (count=%u)\n" (+24h >> 2),
// 0Ah "intarray ..." (+24h >> 2), 0Bh "vec3array ..." (+24h / 12), else "unknown type\n";
// 008F2790 0047E480 next. Then "%s}\n\n" D163F4 with the indent (or F89450); release the indent
// string through 00419CC0 / 00BD1510 (008F27D8).
// ---------------------------------------------------------------------------
void scene_property_bag_write_text_008f2540(Seg59bHost& host, void* bag, std::int32_t depth) {
    Hash64Iterator iterator{add(bag, 0x04), nullptr, 0};
    host.scene_property_bag_iterator_begin_00480690(&iterator);
    alignas(4) std::uint8_t indent[8] = {};
    host.native_string_resize_0041dd40(indent, static_cast<std::uint32_t>(depth) * 4u + 4u, 1);
    char* const indent_text = static_cast<char*>(ptr(indent, 0x04));
    if (indent_text != nullptr) host.memset_00bf79f0(indent_text, 0x20, at<std::uint32_t>(indent, 0x00));
    const void* const empty_name = host.image_pointer(kEnumNameEmpty);
    std::uint32_t args[8];
    std::uint32_t count = 0;
    trace(host, 0x00D164D8u, args, 0);
    while (iterator.node != nullptr) {
        void* const record = ptr(iterator.node, 0x08);
        const void* const key = ptr(iterator.node, 0x04);
        count = 0;
        push_pointer(args, count, indent_text != nullptr ? static_cast<const void*>(indent_text) : empty_name);
        push_pointer(args, count, key != nullptr ? key : empty_name);
        push_dword(args, count, at<std::uint32_t>(record, 0x34));
        trace(host, 0x00D164CCu, args, count);
        count = 0;
        switch (at<std::uint32_t>(record, 0x04)) {
        case 0:
            push_dword(args, count, at<std::uint32_t>(record, 0x0C));
            trace(host, 0x00D164A0u, args, count);
            break;
        case 1:
            push_double_from_float(args, count, at<float>(record, 0x0C));
            trace(host, 0x00D164A8u, args, count);
            break;
        case 2:
            push_dword(args, count, at<std::uint32_t>(record, 0x0C));
            trace(host, 0x00D1648Cu, args, count);
            break;
        case 3:
            trace(host, at<std::uint8_t>(record, 0x0C) != 0 ? 0x00D164C4u : 0x00D164BCu, args, 0);
            break;
        case 4: {
            void* const decl = ptr(record, 0x28);
            const std::uint32_t value = at<std::uint32_t>(record, 0x0C);
            const char* const name = scene_enum_decl_find_name_008f0d80(host, decl, value);
            push_dword(args, count, value);
            push_pointer(args, count, add(decl, 0x11C));
            push_pointer(args, count, name);
            trace(host, 0x00D164B0u, args, count);
            break;
        }
        case 5: {
            const void* const name = ptr(record, 0x1C);
            push_pointer(args, count, name != nullptr ? name : host.image_pointer(kEmptyString));
            trace(host, 0x00D16494u, args, count);
            break;
        }
        case 6:
            scene_property_bag_write_text_008f2540(host, ptr(record, 0x0C), depth + 1);
            break;
        case 7:
            push_double_from_float(args, count, at<float>(record, 0x0C));
            push_double_from_float(args, count, at<float>(record, 0x10));
            push_double_from_float(args, count, at<float>(record, 0x14));
            trace(host, 0x00D16474u, args, count);
            break;
        case 8:
            push_dword(args, count, at<std::uint32_t>(record, 0x20));
            push_dword(args, count, at<std::uint32_t>(record, 0x24));
            trace(host, 0x00D1645Cu, args, count);
            break;
        case 9:
            push_dword(args, count, at<std::uint32_t>(record, 0x20));
            push_dword(args, count, at<std::uint32_t>(record, 0x24) >> 2);
            trace(host, 0x00D16440u, args, count);
            break;
        case 0x0A:
            push_dword(args, count, at<std::uint32_t>(record, 0x20));
            push_dword(args, count, at<std::uint32_t>(record, 0x24) >> 2);
            trace(host, 0x00D16428u, args, count);
            break;
        case 0x0B:
            push_dword(args, count, at<std::uint32_t>(record, 0x20));
            push_dword(args, count, at<std::uint32_t>(record, 0x24) / 12u);
            trace(host, 0x00D1640Cu, args, count);
            break;
        default:
            trace(host, 0x00D163FCu, args, 0);
            break;
        }
        host.scene_property_bag_iterator_next_0047e480(&iterator);
    }
    count = 0;
    push_pointer(args, count, indent_text != nullptr ? static_cast<const void*>(indent_text) : empty_name);
    trace(host, 0x00D163F4u, args, count);
    release_native_string(host, indent);
}

namespace {

// The 008F1CA0 array tail: a newline (0094FFA0(writer, 0)) and writer+8 += 1 before the first
// element, and a newline every `per_line` elements; each element text from 00439AC0 goes out
// through 0094FF10(writer, text).
template <typename Element>
void write_array_elements(Seg59bHost& host, void* writer, std::uint32_t per_line, Element element,
                          std::uint32_t (*count_of)(Seg59bHost&, void*), void* record) {
    std::uint32_t left_on_line = per_line;
    for (std::uint32_t index = 0; index < count_of(host, record); ++index) {
        if (left_on_line == 0) {
            host.scene_text_writer_manipulator_0094ffa0(writer, 0);
            left_on_line = per_line;
        }
        host.scene_text_write_0094ff10(writer, element(index));
        --left_on_line;
    }
}

std::uint32_t array_count_008ef7f0(Seg59bHost& host, void* record) {
    return host.scene_property_array_count_008ef7f0(record);
}

}  // namespace

// ---------------------------------------------------------------------------
// 008F1CA0 BSP_SceneProperty_WriteText. __thiscall(ECX = never read, name, record, writer),
// RET 0Ch. Callers 008F71E0, 008F7550 (scene property file writers).
// Evidence: 008F1CCC 0094FF10(writer, name), 008F1CD8 " = " CF8284; table 008F2224 on
// [record+4]:
//  0 "I " D163E8, 008F1E0E 00950000(int); 1 "F " D163EC, 008F1DF2 00950130(float);
//  2 "S " D163E4, 008F1E2E 008EFA30(008F0DF0(record), writer);
//  3 "B " D163F0, 008F1D0D 008EEFE0(byte +0Ch);
//  4 quote buffers q1 = q2 = {20h, 0}; 008F1D30 _strchr(decl+11Ch, ' ') -> q1[0] = '"';
//    008F1D4A _strchr(008F0DF0(record), ' ') -> q2[0] = '"'; then letter [decl+118h] (or F89450),
//    q1, decl+11Ch, q1, ":" CF00B0, q2, 008F0DF0(record), q2, chained;
//  5 local string {0, 0}; 008F1EA4 008F1960(record, &local); its data (or F89450), " \"" D163DC;
//    [record+18h] != 0 -> 0094FF10(writer, F89450) then "\"" with a temporary destroyed by
//    0041DD20 (008F1EFF); else 008F0DF0(record) then "\""; 008F1F2B 0041DD20(&local);
//  7 "V3 " D163E0, 00950130 x, " " CE3A90, y, " ", z, chained;
//  8 "BIN " D163D4, 00950050(+24h); 16 per line "%.2hX " D163CC of each byte;
//  9 "FA " D163C8, 00950050(008EF7F0), 4 per line "%.7f " D163C0 (float widened);
//  0Ah "IA " D163BC, 4 per line "%11i " D163B4; 0Bh "V3A " D163AC, 2 per line
//    "(%.7f, %.7f, %.7f) " D16398 over 12-byte elements;
//  6 and > 0Bh write nothing here.
// Arrays with a non-zero count (re-read through 008EF7F0 each pass) first send 0094FFA0(writer,
// 0) and writer+8 += 1, and at 008F21EC writer+8 -= 1 when non-zero. Every case ends with
// 0094FF10(writer, " ;" D16394) then 0094FFA0(that, 0) (008F21FE/008F2207).
// ---------------------------------------------------------------------------
void scene_property_write_text_008f1ca0(Seg59bHost& host, const char* name, void* record, void* writer) {
    auto text = [&host](std::uint32_t address) { return static_cast<const char*>(host.image_pointer(address)); };
    const char* const empty = text(kEnumNameEmpty);
    host.scene_text_write_0094ff10(host.scene_text_write_0094ff10(writer, name), text(0x00CF8284u));
    bool array_tail = false;
    switch (at<std::uint32_t>(record, 0x04)) {
    case 0:
        host.scene_text_write_int_00950000(host.scene_text_write_0094ff10(writer, text(0x00D163E8u)),
                                           at<std::int32_t>(record, 0x0C));
        break;
    case 1: {
        const float value = at<float>(record, 0x0C);
        host.scene_text_write_float_00950130(host.scene_text_write_0094ff10(writer, text(0x00D163ECu)), value);
        break;
    }
    case 2:
        host.scene_text_write_0094ff10(writer, text(0x00D163E4u));
        scene_text_write_multiline_008efa30(host, scene_property_string_value_008f0df0(host, record), writer);
        break;
    case 3: {
        const std::uint8_t value = at<std::uint8_t>(record, 0x0C);
        void* const chained = host.scene_text_write_0094ff10(writer, text(0x00D163F0u));
        scene_property_write_bool_text_008eefe0(host, chained, value != 0);
        break;
    }
    case 4: {
        void* const decl = ptr(record, 0x28);
        char open[2] = {0x20, 0x00};
        char value_quote[2] = {0x20, 0x00};
        if (host.strchr_00bf86f0(static_cast<const char*>(add(decl, 0x11C)), 0x20) != nullptr) open[0] = 0x22;
        if (host.strchr_00bf86f0(scene_property_string_value_008f0df0(host, record), 0x20) != nullptr) {
            value_quote[0] = 0x22;
        }
        const char* const letter = static_cast<const char*>(ptr(decl, 0x118));
        void* w = host.scene_text_write_0094ff10(writer, letter != nullptr ? letter : empty);
        w = host.scene_text_write_0094ff10(w, open);
        w = host.scene_text_write_0094ff10(w, static_cast<const char*>(add(decl, 0x11C)));
        w = host.scene_text_write_0094ff10(w, open);
        w = host.scene_text_write_0094ff10(w, text(0x00CF00B0u));
        w = host.scene_text_write_0094ff10(w, value_quote);
        w = host.scene_text_write_0094ff10(w, scene_property_string_value_008f0df0(host, record));
        host.scene_text_write_0094ff10(w, value_quote);
        break;
    }
    case 5: {
        alignas(4) std::uint8_t letter[8] = {};
        host.scene_reference_get_type_letter_008f1960(record, letter);
        const char* const letter_text = static_cast<const char*>(ptr(letter, 0x04));
        host.scene_text_write_0094ff10(host.scene_text_write_0094ff10(writer, letter_text != nullptr ? letter_text : empty),
                                       text(0x00D163DCu));
        if (ptr(record, 0x18) != nullptr) {
            alignas(4) std::uint8_t temporary[8] = {};
            host.scene_text_write_0094ff10(host.scene_text_write_0094ff10(writer, empty), text(0x00CE436Cu));
            host.native_string_destroy_storage_0041dd20(temporary);
        } else {
            host.scene_text_write_0094ff10(host.scene_text_write_0094ff10(writer, scene_property_string_value_008f0df0(host, record)),
                                           text(0x00CE436Cu));
        }
        host.native_string_destroy_storage_0041dd20(letter);
        break;
    }
    case 7: {
        void* w = host.scene_text_write_0094ff10(writer, text(0x00D163E0u));
        w = host.scene_text_write_float_00950130(w, at<float>(record, 0x0C));
        w = host.scene_text_write_0094ff10(w, text(0x00CE3A90u));
        w = host.scene_text_write_float_00950130(w, at<float>(record, 0x10));
        w = host.scene_text_write_0094ff10(w, text(0x00CE3A90u));
        host.scene_text_write_float_00950130(w, at<float>(record, 0x14));
        break;
    }
    case 8: {
        host.scene_text_write_unsigned_00950050(host.scene_text_write_0094ff10(writer, text(0x00D163D4u)),
                                                at<std::uint32_t>(record, 0x24));
        if (at<std::uint32_t>(record, 0x24) == 0) break;
        host.scene_text_writer_manipulator_0094ffa0(writer, 0);
        at<std::uint32_t>(writer, 0x08) += 1u;
        const std::uint8_t* const bytes = static_cast<const std::uint8_t*>(ptr(record, 0x20));
        std::uint32_t left_on_line = 0x10;
        for (std::uint32_t index = 0; index < at<std::uint32_t>(record, 0x24); ++index) {
            if (left_on_line == 0) {
                host.scene_text_writer_manipulator_0094ffa0(writer, 0);
                left_on_line = 0x10;
            }
            const std::uint32_t args[1] = {bytes[index]};
            host.scene_text_write_0094ff10(writer, host.string_format_static_00439ac0(text(0x00D163CCu), args, 1));
            --left_on_line;
        }
        array_tail = true;
        break;
    }
    case 9:
    case 0x0A:
    case 0x0B: {
        const std::uint32_t kind = at<std::uint32_t>(record, 0x04);
        const std::uint32_t label = kind == 9 ? 0x00D163C8u : kind == 0x0A ? 0x00D163BCu : 0x00D163ACu;
        void* const chained = host.scene_text_write_0094ff10(writer, text(label));
        host.scene_text_write_unsigned_00950050(chained, host.scene_property_array_count_008ef7f0(record));
        if (host.scene_property_array_count_008ef7f0(record) == 0) break;
        host.scene_text_writer_manipulator_0094ffa0(writer, 0);
        at<std::uint32_t>(writer, 0x08) += 1u;
        const std::uint8_t* const data = static_cast<const std::uint8_t*>(ptr(record, 0x20));
        if (kind == 9) {
            write_array_elements(host, writer, 4, [&](std::uint32_t index) {
                std::uint32_t args[2];
                std::uint32_t count = 0;
                float value;
                std::memcpy(&value, data + index * 4u, sizeof value);
                push_double_from_float(args, count, value);
                return host.string_format_static_00439ac0(text(0x00D163C0u), args, count);
            }, array_count_008ef7f0, record);
        } else if (kind == 0x0A) {
            write_array_elements(host, writer, 4, [&](std::uint32_t index) {
                std::uint32_t args[1];
                std::memcpy(&args[0], data + index * 4u, sizeof args[0]);
                return host.string_format_static_00439ac0(text(0x00D163B4u), args, 1);
            }, array_count_008ef7f0, record);
        } else {
            write_array_elements(host, writer, 2, [&](std::uint32_t index) {
                std::uint32_t args[6];
                std::uint32_t count = 0;
                float xyz[3];
                std::memcpy(xyz, data + index * 12u, sizeof xyz);
                push_double_from_float(args, count, xyz[0]);
                push_double_from_float(args, count, xyz[1]);
                push_double_from_float(args, count, xyz[2]);
                return host.string_format_static_00439ac0(text(0x00D16398u), args, count);
            }, array_count_008ef7f0, record);
        }
        array_tail = true;
        break;
    }
    default:
        break;
    }
    if (array_tail && at<std::uint32_t>(writer, 0x08) != 0) at<std::uint32_t>(writer, 0x08) -= 1u;
    host.scene_text_writer_manipulator_0094ffa0(host.scene_text_write_0094ff10(writer, text(0x00D16394u)), 0);
}

// ---------------------------------------------------------------------------
// 00903790 / 009037B0 BSP_OwnedBuffer_Free_*. __thiscall(ECX = slot), RET. Evidence (each):
// [slot] non-null -> _free 00BF65AC (00903799 / 009037B9), [slot] = 0 in that arm. No code
// xrefs (reached from unwind or vtable slots).
// ---------------------------------------------------------------------------
void owned_buffer_free_00903790(Seg59bHost& host, void* slot) {
    if (ptr(slot, 0) != nullptr) {
        host.free_00bf65ac(ptr(slot, 0));
        at<void*>(slot, 0) = nullptr;
    }
}

void owned_buffer_free_009037b0(Seg59bHost& host, void* slot) {
    if (ptr(slot, 0) != nullptr) {
        host.free_00bf65ac(ptr(slot, 0));
        at<void*>(slot, 0) = nullptr;
    }
}

// ---------------------------------------------------------------------------
// 00905300 BSP_SlotCounter_Increment. __thiscall(ECX = scoring, slot), RET 4; EAX = &counter.
// Evidence: 00905304 IMUL slot,284h; 0090530A ADD [rec+170h],1; 00905312 LEA EAX. Callers
// 0077C8D0, 0077C980, 0077CA60 (formation join / release / snapshot; include/bsp/
// lua_binding_navigator.hpp:180 keeps only the stride and offset).
// ---------------------------------------------------------------------------
std::uint32_t* score_record_increment_field170_00905300(void* scoring, std::int32_t slot) {
    std::uint32_t& counter = at<std::uint32_t>(scoring, static_cast<std::uint32_t>(slot) * 0x284u + 0x170u);
    counter += 1u;
    return &counter;
}

// ---------------------------------------------------------------------------
// 00906430 BSP_MissionResult_CallNamedScript. __thiscall(ECX = owner), RET. Evidence: 00906430
// [owner+147Ch] (the name's length, unsigned) > 0 -> 00906453 00887E50(ECX = [[00E188A8]+
// 1A08h]; 0, owner+147Ch, 0, 0, -1). Caller 00920A20 (debrief bring-up, 00920BEB, when not
// aborted).
// ---------------------------------------------------------------------------
void mission_result_call_named_script_00906430(Seg59bHost& host, void* owner) {
    if (at<std::uint32_t>(owner, 0x147C) == 0) return;
    void* const game = *static_cast<void**>(host.image_pointer(0x00E188A8u));
    host.lua_call_named_00887e50(ptr(game, 0x1A08), 0, add(owner, 0x147C), nullptr, 0, -1);
}

// ---------------------------------------------------------------------------
// 0090D2F0 BSP_LuaObjectHolder_Destruct. __thiscall(ECX = holder), RET. Evidence: 0090D30E
// vtable 00D18784; 0090D31D 00441210(holder+4); 0090D326 vtable 00CE3784. Reached from the
// unwind funclet 00CA5738.
// ---------------------------------------------------------------------------
void lua_object_holder_destruct_0090d2f0(Seg59bHost& host, void* holder) {
    at<std::uint32_t>(holder, 0x00) = 0x00D18784u;
    host.lua_object_vector_tidy_00441210(add(holder, 0x04));
    at<std::uint32_t>(holder, 0x00) = 0x00CE3784u;
}

// ---------------------------------------------------------------------------
// 0090E920 BSP_LuaObjectHolder_Construct. __thiscall(ECX = holder, LuaObject by value), RET 14h;
// EAX = holder. Evidence: vtable 00D18784 (0090E946); vector +8/+0Ch/+10h = 0 (0090E94C..
// 0090E952); 0090E95F 00442220(holder+4, &argument); 0090E970 00B67700(&argument). Caller
// 0090EA00.
// ---------------------------------------------------------------------------
void* lua_object_holder_construct_0090e920(Seg59bHost& host, void* holder, void* object) {
    at<std::uint32_t>(holder, 0x00) = 0x00D18784u;
    at<std::uint32_t>(holder, 0x08) = 0;
    at<std::uint32_t>(holder, 0x0C) = 0;
    at<std::uint32_t>(holder, 0x10) = 0;
    host.lua_object_vector_push_back_00442220(add(holder, 0x04), object);
    host.lua_object_destruct_00b67700(object);
    return holder;
}

// ---------------------------------------------------------------------------
// 009221E0 BSP_WatchList_Construct. __thiscall(ECX = owner), RET; EAX = owner. Evidence:
// vtable 00D18C24 (009221E9); 009221F2 004C3020(owner+4); +8 = head (009221F7); +0Ch = 0.
// Caller 004DE610 (world construction, member game+21CCh per include/bsp/world_construct.hpp).
// ---------------------------------------------------------------------------
void* watch_list_construct_009221e0(Seg59bHost& host, void* owner) {
    at<std::uint32_t>(owner, 0x00) = 0x00D18C24u;
    void* const head = host.pooled_string_list_allocate_sentinel_004c3020(add(owner, 0x04));
    at<void*>(owner, 0x08) = head;
    at<std::uint32_t>(owner, 0x0C) = 0;
    return owner;
}

// ---------------------------------------------------------------------------
// 00922210 BSP_WatchList_Destruct. __thiscall(ECX = owner), RET. Evidence: vtable 00D18C24
// (00922214); 0092221C 004D05E0(owner+4); 00922225 _free([owner+8]); [owner+8] = 0.
// ---------------------------------------------------------------------------
void watch_list_destruct_00922210(Seg59bHost& host, void* owner) {
    at<std::uint32_t>(owner, 0x00) = 0x00D18C24u;
    host.pooled_string_list_clear_storage_004d05e0(add(owner, 0x04));
    host.free_00bf65ac(ptr(owner, 0x08));
    at<void*>(owner, 0x08) = nullptr;
}

// ---------------------------------------------------------------------------
// 00922240 BSP_WatchList_Add. __thiscall(ECX = owner, NativeString by value), RET 8. Evidence:
// 0092226A 005FE8B0(owner+4, &name) (remove equal names); head = [owner+8]; 0092227E
// 004CE6F0(owner+4, head, head->prev, &name); 00922289 004CE780(owner+4, 1); head->prev = node,
// node->prev->next = node (0092228E..00922294); release the argument through 00419CC0 /
// 00BD1510 (009222B8). Callers 0088D130 (Lua AddWatch), 00922350.
// ---------------------------------------------------------------------------
void watch_list_add_00922240(Seg59bHost& host, void* owner, void* name) {
    void* const list = add(owner, 0x04);
    host.string_list_remove_005fe8b0(list, name);
    void* const head = ptr(owner, 0x08);
    void* const node = host.alias_node_allocate_copy_004ce6f0(list, head, ptr(head, 0x04), name);
    host.alias_list_grow_count_004ce780(list, 1);
    at<void*>(head, 0x04) = node;
    at<void*>(ptr(node, 0x04), 0x00) = node;
    release_native_string(host, name);
}

namespace {

// One std::map / std::set clear as 00915760 inlines it: root = head->parent, the _Erase
// instantiation `callee`(tree, root), then head->parent = head, size = 0, head->left = head,
// head->right = head (tree = {allocator, head +4, size +8}).
void clear_tree_inline(Seg59bHost& host, void* record, std::uint32_t offset, std::uint32_t callee) {
    void* const tree = add(record, offset);
    host.stl_tree_erase_subtree(callee, tree, ptr(ptr(tree, 0x04), 0x04));
    void* const head = ptr(tree, 0x04);
    at<void*>(head, 0x04) = head;
    at<std::uint32_t>(tree, 0x08) = 0;
    at<void*>(head, 0x00) = head;
    at<void*>(head, 0x08) = head;
}

// vector<16-bit>::erase(begin, end) as 00915760 inlines it, with both checked-iterator tests.
void clear_short_vector_inline(Seg59bHost& host, void* record, std::uint32_t first_offset) {
    std::uint8_t* const last = static_cast<std::uint8_t*>(ptr(record, first_offset + 4));
    if (static_cast<std::uint8_t*>(ptr(record, first_offset)) > last) host.checked_iterator_failure_00bf6713();
    std::uint8_t* const first = static_cast<std::uint8_t*>(ptr(record, first_offset));
    if (first > static_cast<std::uint8_t*>(ptr(record, first_offset + 4))) host.checked_iterator_failure_00bf6713();
    if (first == last) return;
    const std::int32_t count =
        static_cast<std::int32_t>(static_cast<std::uint8_t*>(ptr(record, first_offset + 4)) - last) >> 1;
    const std::uint32_t bytes = static_cast<std::uint32_t>(count) * 2u;
    std::uint8_t* const new_last = first + bytes;
    if (count > 0) host.memmove_s_00bf67a7(first, bytes, last, bytes);
    at<void*>(record, first_offset + 4) = new_last;
}

// NativeString assign from another header as 00915760 inlines it: 0041DD40(dst, src.size, 1),
// then memcpy(dst.data, src.data, dst.size) when src.size != 0.
void assign_native_string_inline(Seg59bHost& host, void* dst, void* src) {
    host.native_string_resize_0041dd40(dst, at<std::uint32_t>(src, 0x00), 1);
    if (at<std::uint32_t>(src, 0x00) != 0) {
        host.memcpy_00bf7680(ptr(dst, 0x04), ptr(src, 0x04), at<std::uint32_t>(dst, 0x00));
    }
}

}  // namespace

// ---------------------------------------------------------------------------
// 00915760 BSP_MissionScoreRecord_Clear. __thiscall(ECX = record), RET. Callers 008BC540
// (Scoring_ClearPlayerScore, which saves and restores record+4 and the byte +24Ch around it)
// and 00916980 (BSP_MissionPlayerRecords_Reset).
// Evidence: 0091576C..00915801 dwords +8, +4, +0, floats +14h/+10h/+0Ch, then +1E0h, +1DCh,
// +1D8h, +1D4h, +1D0h, +1CCh, +1C8h, +17Ch, +178h, +1E8h, +1E4h, +250h, +18Ch, +188h, +184h,
// +180h, +16Ch, +168h, +174h, +170h, +190h, +194h = 0, in that order. 36 inline tree clears
// (the 00915813..00915E74 sequence below, each erase followed by the head reset); the 16-bit
// vectors at +130h (00915B64..00915BB6) and +140h (00915D5E..00915DB0) erased in place with
// 00BF6713 checks and 00BF67A7. Then byte +24Ch = 0 (00915E97); 00915E9D 0041DD40(+264h, 0,
// 0); data non-null -> 00915EB2 memcpy(data, "" CE3A0C, size); +25Ch = +264h (00915ECB,
// 00915EE0); +254h = +25Ch (00915EFA, 00915F0F).
// ---------------------------------------------------------------------------
void mission_score_record_clear_00915760(Seg59bHost& host, void* record) {
    at<std::uint32_t>(record, 0x08) = 0;
    at<std::uint32_t>(record, 0x04) = 0;
    at<std::uint32_t>(record, 0x00) = 0;
    at<float>(record, 0x14) = 0.0f;
    at<float>(record, 0x10) = 0.0f;
    at<float>(record, 0x0C) = 0.0f;
    static constexpr std::uint32_t kScalars[] = {0x1E0, 0x1DC, 0x1D8, 0x1D4, 0x1D0, 0x1CC, 0x1C8, 0x17C,
                                                 0x178, 0x1E8, 0x1E4, 0x250, 0x18C, 0x188, 0x184, 0x180,
                                                 0x16C, 0x168, 0x174, 0x170, 0x190, 0x194};
    for (const std::uint32_t offset : kScalars) at<std::uint32_t>(record, offset) = 0;
    struct TreeClear { std::uint32_t offset; std::uint32_t callee; };
    static constexpr TreeClear kFirst[] = {
        {0x018, 0x0058B520}, {0x024, 0x0058B520}, {0x030, 0x0058B520}, {0x03C, 0x0058B520},
        {0x048, 0x0058B520}, {0x054, 0x0058B520}, {0x060, 0x0058B520}, {0x204, 0x0058BCD0},
        {0x210, 0x0058BCD0}, {0x21C, 0x0058BCD0}, {0x228, 0x0058BCD0}, {0x234, 0x0058BCD0},
        {0x240, 0x0058BCD0}, {0x1EC, 0x0058B520}, {0x1F8, 0x0058B520}, {0x0CC, 0x005914B0},
        {0x0D8, 0x00582B00}, {0x09C, 0x00592A20}, {0x0B4, 0x00592EA0}, {0x0A8, 0x00592A20},
        {0x0C0, 0x00592EA0}};
    for (const TreeClear& clear : kFirst) clear_tree_inline(host, record, clear.offset, clear.callee);
    clear_short_vector_inline(host, record, 0x130);
    static constexpr TreeClear kSecond[] = {
        {0x15C, 0x00582C00}, {0x198, 0x00582C80}, {0x1A4, 0x00582C80}, {0x1BC, 0x00582D00},
        {0x1B0, 0x00582C80}, {0x078, 0x0058B520}, {0x120, 0x0058B520}, {0x06C, 0x0058B520},
        {0x084, 0x0058B520}, {0x090, 0x0058B520}};
    for (const TreeClear& clear : kSecond) clear_tree_inline(host, record, clear.offset, clear.callee);
    clear_short_vector_inline(host, record, 0x140);
    static constexpr TreeClear kThird[] = {
        {0x114, 0x00582B80}, {0x0E4, 0x00582B00}, {0x0F0, 0x00582B00}, {0x0FC, 0x00582B00},
        {0x108, 0x00582B00}};
    for (const TreeClear& clear : kThird) clear_tree_inline(host, record, clear.offset, clear.callee);
    at<std::uint8_t>(record, 0x24C) = 0;
    void* const text = add(record, 0x264);
    host.native_string_resize_0041dd40(text, 0, 0);
    if (ptr(text, 0x04) != nullptr) {
        host.memcpy_00bf7680(ptr(text, 0x04), host.image_pointer(kEmptyString), at<std::uint32_t>(text, 0x00));
    }
    assign_native_string_inline(host, add(record, 0x25C), text);
    assign_native_string_inline(host, add(record, 0x254), add(record, 0x25C));
}

namespace {

void* game_global(Seg59bHost& host) { return *static_cast<void**>(host.image_pointer(0x00E188A8u)); }

void* award_table(Seg59bHost& host) { return add(*static_cast<void**>(host.image_pointer(0x00E19900u)), 0x10); }

// The checked "is this 16-bit id already listed" walk over vector<u16> {first +4, last +8} used
// by 00913D80 (00914099..009140CF) and 00914100 (009141F5..00914255): a null first or an index
// past the end ends the walk; the per-element 00BF6713 test repeats the same bounds.
bool short_vector_checked_index(Seg59bHost& host, void* vector, std::uint32_t index) {
    const std::uint8_t* const first = static_cast<const std::uint8_t*>(ptr(vector, 0x04));
    if (first == nullptr) return false;
    const auto size = static_cast<std::uint32_t>(
        static_cast<std::int32_t>(static_cast<const std::uint8_t*>(ptr(vector, 0x08)) - first) >> 1);
    if (index >= size) return false;
    if (ptr(vector, 0x04) == nullptr ||
        index >= static_cast<std::uint32_t>(static_cast<std::int32_t>(
                     static_cast<const std::uint8_t*>(ptr(vector, 0x08)) -
                     static_cast<const std::uint8_t*>(ptr(vector, 0x04))) >> 1)) {
        host.checked_iterator_failure_00bf6713();
    }
    return true;
}

}  // namespace

// ---------------------------------------------------------------------------
// 00913D80 BSP_MissionScoring_RecordComponentFailure. __thiscall(ECX = scoring, unit, hit,
// owner_kind), RET 0Ch. Caller 0093BED0 (BSP_ShipSystems_RollComponentFailure) at 0093C0DC; the
// cc9 host records the call only (game_hosts_gunnery.cpp, "Failure::statistics_00913d80").
// Evidence: [[00E188A8]+1FE4h] == 2 -> return (00913DAB); attacker = [hit+4], null -> return;
// 00913DD1 record = attacker->vt[108h](). Record byte +21h clear and slot = +1Ch <= 7:
//  [record+ACh] non-null and [[record+ACh] + 00779A00([record+4])*4 + 1ACh] == slot and
//  [unit+54h] != [[game + that*4 + 18CCh]+28h] -> ++[scoring + slot*284h + 194h] (00913E30);
//  unit non-null, [unit+54h] != [attacker+54h], unit->vt[5Ch](6), [record+ACh] non-null with
//  vt[5Ch](12h), owner_kind == 8: "Counter_RUA_DBU" D187C4 counter in the map at scoring +
//  slot*284h + 70h incremented (005070C0, 00913EC4); 0050FC30("RUA_DBU" D188E0) record, its
//  +44h vector [0] (00481750) as the target; the counter re-read == target -> 0090EDE0(slot,
//  "RUA_DBU", 1, 0) (00913F9E); each key a 0041E870 temporary destroyed by 0041DD20; then byte
//  [record+21h] = 1 (00913FB4). Then (also when the flag was set or slot > 7): unit->vt[5Ch](6)
//  false -> return; a, b, c, d = 00939F70 / 00939F80 / 00939FC0 / 00939FB0 on unit+A20h; trace
//  "%s (%d) HP:%f bekap %f %f " D188C4 (name [unit+158h] or F899A0, word +174h, [unit+370h],
//  c*a, d*b) to 004254B0; c*a + d*b > [unit+370h] (FCOMIP, JBE also on unordered) and
//  slot = [unit+180h] <= 7 -> the unit id (word +174h) is pushed (007265E0) on the vector at
//  scoring + slot*284h + 140h unless already present.
// x87: the two products and the sum are modeled at 24-bit precision (main thread); at 53 bits
// they could differ in the last place. No shared RNG draws.
// ---------------------------------------------------------------------------
void scoring_record_component_failure_00913d80(Seg59bHost& host, void* scoring, void* unit, void* hit,
                                                std::int32_t owner_kind) {
    void* const game = game_global(host);
    if (at<std::uint32_t>(game, 0x1FE4) == 2) return;
    void* const attacker = ptr(hit, 0x04);
    if (attacker == nullptr) return;
    void* const record = host.entity_player_record_vt108(attacker);
    bool mark = false;
    if (at<std::uint8_t>(record, 0x21) == 0 && at<std::uint32_t>(record, 0x1C) <= 7) {
        mark = true;
        const std::uint32_t slot = at<std::uint32_t>(record, 0x1C);
        if (ptr(record, 0xAC) != nullptr) {
            const std::uint32_t category = host.ordnance_kind_to_category_00779a00(ptr(record, 0x04));
            if (at<std::uint32_t>(ptr(record, 0xAC), 0x1AC + category * 4u) == slot &&
                at<std::uint32_t>(unit, 0x54) != at<std::uint32_t>(ptr(game, 0x18CC + category * 4u), 0x28)) {
                at<std::uint32_t>(scoring, slot * 0x284u + 0x194u) += 1u;
            }
        }
        void* const crew_unit = ptr(record, 0xAC);
        if (unit != nullptr && at<std::uint32_t>(unit, 0x54) != at<std::uint32_t>(attacker, 0x54) &&
            host.entity_is_kind_of_vt5c(unit, 6) && crew_unit != nullptr &&
            host.entity_is_kind_of_vt5c(crew_unit, 0x12) && owner_kind == 8) {
            const char* const counter_key = static_cast<const char*>(host.image_pointer(0x00D187C4u));
            const char* const award_key = static_cast<const char*>(host.image_pointer(0x00D188E0u));
            alignas(4) std::uint8_t key[8] = {};
            host.native_string_assign_0041e870(key, counter_key);
            *host.string_int_map_index_005070c0(add(scoring, at<std::uint32_t>(record, 0x1C) * 0x284u + 0x70u), key) += 1;
            host.native_string_destroy_storage_0041dd20(key);
            host.native_string_assign_0041e870(key, award_key);
            void* const award = host.award_table_find_record_0050fc30(award_table(host), key);
            const std::int32_t target = *host.int_vector_at_00481750(add(award, 0x44), 0);
            host.native_string_destroy_storage_0041dd20(key);
            host.native_string_assign_0041e870(key, counter_key);
            const bool reached =
                *host.string_int_map_index_005070c0(add(scoring, at<std::uint32_t>(record, 0x1C) * 0x284u + 0x70u), key) == target;
            host.native_string_destroy_storage_0041dd20(key);
            if (reached) {
                host.native_string_assign_0041e870(key, award_key);
                host.scoring_grant_award_0090ede0(scoring, static_cast<std::int32_t>(at<std::uint32_t>(record, 0x1C)), key, 1, 0);
                host.native_string_destroy_storage_0041dd20(key);
            }
        }
    }
    if (mark) at<std::uint8_t>(record, 0x21) = 1;
    if (!host.entity_is_kind_of_vt5c(unit, 6)) return;
    void* const block = add(unit, 0xA20);
    const float a = host.unit_block_float_00939f70(block);
    const float b = host.unit_block_float_00939f80(block);
    const float c = host.unit_block_float_00939fc0(block);
    const float d = host.unit_block_float_00939fb0(block);
    const float health = at<float>(unit, 0x370);
    const void* name = ptr(unit, 0x158);
    if (name == nullptr) name = host.image_pointer(0x00F899A0u);
    const float db = d * b;
    const float ca = c * a;
    std::uint32_t args[8];
    std::uint32_t count = 0;
    push_pointer(args, count, name);
    push_dword(args, count, at<std::uint16_t>(unit, 0x174));
    push_double_from_float(args, count, health);
    push_double_from_float(args, count, ca);
    push_double_from_float(args, count, db);
    trace(host, 0x00D188C4u, args, count);
    const float sum = ca + db;
    if (!(sum > at<float>(unit, 0x370))) return;
    std::uint32_t id = at<std::uint16_t>(unit, 0x174);
    const std::uint32_t slot = at<std::uint32_t>(unit, 0x180);
    if (slot > 7) return;
    void* const vector = add(scoring, slot * 0x284u + 0x140u);
    for (std::uint32_t index = 0; short_vector_checked_index(host, vector, index); ++index) {
        if (static_cast<const std::uint16_t*>(ptr(vector, 0x04))[index] == static_cast<std::uint16_t>(id)) return;
    }
    host.short_vector_push_back_007265e0(vector, &id);
}

// ---------------------------------------------------------------------------
// 00914100 BSP_MissionScoring_RecordRepairCompleted. __thiscall(ECX = scoring, unit), RET 4.
// Reached by the tail-jump at 0093CAF1 (BSP_RepairTask_Update 0093CA20); the cc9 host marks it
// unreached (game_hosts_gunnery.cpp, repair_completed_notify).
// Evidence: slot = [unit+180h] > 7 -> return (0091412D). Key "GA_PL" D1887C (0041E870,
// 0091413E); 0050FC30 award record (0091415E); its +48h int vector: null or empty -> 00BF6713
// (00914176); threshold = [0]. Flag = threshold >= 00923BE0(unit) * 100.0 (double 00D7A220;
// FCOMIP, JC also on unordered, 0091418D) and [unit+370h] > 0.0f (00D7A218; COMISS JA,
// 0091419E). The key is released (00419CC0 / 00BD1510, 009141D4). Flag set: for each u16 in
// the vector at scoring + slot*284h + 144h (+148h last) equal to word [unit+174h], push
// {u16 id, float [00F876A4]} on the vector at +150h through 0090E7C0 (00914250). The 8-byte
// element shares the key's stack slot, so its padding bytes 2..3 are the key's size bits.
// x87: the health product is modeled at 24-bit precision (main thread); 53-bit noted.
// ---------------------------------------------------------------------------
void scoring_record_repair_completed_00914100(Seg59bHost& host, void* scoring, void* unit) {
    const std::uint32_t slot = at<std::uint32_t>(unit, 0x180);
    if (slot > 7) return;
    alignas(4) std::uint8_t local[8] = {};
    host.native_string_assign_0041e870(local, static_cast<const char*>(host.image_pointer(0x00D1887Cu)));
    void* const award = host.award_table_find_record_0050fc30(award_table(host), local);
    if (ptr(award, 0x48) == nullptr ||
        (static_cast<std::int32_t>(static_cast<std::uint8_t*>(ptr(award, 0x4C)) -
                                   static_cast<std::uint8_t*>(ptr(award, 0x48))) >> 2) == 0) {
        host.checked_iterator_failure_00bf6713();
    }
    const std::int32_t threshold = *static_cast<const std::int32_t*>(ptr(award, 0x48));
    const float scaled = host.unit_get_health_00923be0(unit) * 100.0f;
    const bool flag = !(static_cast<double>(threshold) < static_cast<double>(scaled)) &&
                      !std::isnan(scaled) && at<float>(unit, 0x370) > 0.0f;
    release_native_string(host, local);
    if (!flag) return;
    void* const record = add(scoring, slot * 0x284u);
    const std::uint16_t id = at<std::uint16_t>(unit, 0x174);
    void* const ids = add(record, 0x140);
    for (std::uint32_t index = 0; short_vector_checked_index(host, ids, index); ++index) {
        if (static_cast<const std::uint16_t*>(ptr(ids, 0x04))[index] != id) continue;
        std::memcpy(local, &id, sizeof id);
        const float clock = *static_cast<const float*>(host.image_pointer(0x00F876A4u));
        std::memcpy(local + 4, &clock, sizeof clock);
        host.repair_event_vector_push_back_0090e7c0(add(record, 0x150), local);
    }
}

// ---------------------------------------------------------------------------
// 009150A0 BSP_MissionScoring_ForceRefresh. __thiscall(ECX = scoring, slot), RET 4. Caller
// 008B8E60 (Lua Scoring.ForceRefreshScoringTable; src/cc13_left55_seg56b.cpp calls it as
// scoring_force_refresh_009150a0 through its host).
// Evidence: record = scoring + slot*284h + 4 (009150C2); 009150CC 00914390(record); 009150D3
// 0090EA00(record); [[00E188A8]+1FE4h] == 0 -> return (009150E5). Else key "playerIndex"
// CF9D28 (0041E870, 009150F4); 00915112 00B67980(game+1A0Ch, &globals); 00915128
// 00B67800(globals, &table, "scoring" CF8268); 0091513A 00B67460(table, &key, slot); 00B67700
// on the table (00915148), then on the globals (00915156); the key released (0091517D).
// ---------------------------------------------------------------------------
void scoring_force_refresh_009150a0(Seg59bHost& host, void* scoring, std::int32_t slot) {
    void* const record = add(scoring, static_cast<std::uint32_t>(slot) * 0x284u + 4u);
    host.score_record_recompute_category_totals_00914390(record);
    host.score_record_publish_0090ea00(record);
    void* const game = game_global(host);
    if (at<std::uint32_t>(game, 0x1FE4) == 0) return;
    alignas(4) std::uint8_t key[8] = {};
    host.native_string_assign_0041e870(key, static_cast<const char*>(host.image_pointer(0x00CF9D28u)));
    alignas(4) std::uint8_t globals[0x14] = {};
    alignas(4) std::uint8_t table[0x14] = {};
    void* const globals_object = host.lua_owner_get_globals_00b67980(add(game, 0x1A0C), globals);
    void* const table_object = host.lua_object_get_by_name_00b67800(
        globals_object, table, static_cast<const char*>(host.image_pointer(0x00CF8268u)));
    host.lua_object_set_native_string_integer_00b67460(table_object, key, slot);
    host.lua_object_destruct_00b67700(table);
    host.lua_object_destruct_00b67700(globals);
    release_native_string(host, key);
}

// ---------------------------------------------------------------------------
// 008F3270 BSP_ScenePropertyBag_CollectKeyNames. __thiscall(ECX = bag, list, sort), RET 8. No
// code xrefs (vtable or dead).
// Evidence: 008F3293 0094F3E0(list); iterator {bag+4} with 008EEB10 begin (008F32A3); per node:
// name = [node+4] or F89450 (008F32C0); 008F32C7 00BF681B(8); non-null -> header {0, 0},
// 008F32F7 0041DD40(header, strlen(name), 1), data non-null -> 008F330B memcpy(data, name,
// size + 1); 008F3324 0094F270(list, header or 0, 1); 008F332D 008EEB60 next while node != 0.
// After: byte sort != 0 -> 008F3348 0094F1B0(list, 0).
// ---------------------------------------------------------------------------
void scene_property_bag_collect_key_names_008f3270(Seg59bHost& host, void* bag, void* list, bool sort) {
    host.string_key_list_clear_owned_headers_0094f3e0(list);
    Hash64Iterator iterator{add(bag, 0x04), nullptr, 0};
    host.hash64_iterator_begin_008eeb10(&iterator);
    while (iterator.node != nullptr) {
        const char* name = static_cast<const char*>(ptr(iterator.node, 0x04));
        if (name == nullptr) name = static_cast<const char*>(host.image_pointer(kEnumNameEmpty));
        void* header = host.operator_new_00bf681b(8);
        if (header != nullptr) {
            at<std::uint32_t>(header, 0x00) = 0;
            at<std::uint32_t>(header, 0x04) = 0;
            host.native_string_resize_0041dd40(header, static_cast<std::uint32_t>(std::strlen(name)), 1);
            if (ptr(header, 0x04) != nullptr) {
                host.memcpy_00bf7680(ptr(header, 0x04), name, at<std::uint32_t>(header, 0x00) + 1u);
            }
        }
        host.string_key_list_append_owned_header_0094f270(list, header, 1);
        host.hash64_iterator_next_008eeb60(&iterator);
    }
    if (sort) host.string_key_list_sort_0094f1b0(list, 0);
}

// ---------------------------------------------------------------------------
// 008F6E20 BSP_ScenePropertyBag_CollectRecordsSorted. __thiscall(ECX = bag, out), RET 4; EAX =
// count. Caller 008F7550.
// Evidence: a local std::vector<record*> {alloc, first, last, end} zeroed (008F6E3E..008F6E4C);
// iterator {bag+4} 00480690 begin (008F6E63). Per node: value = [node+8]; first non-null and
// size < capacity -> stored at last, last += 4 (008F6E8D); else first > last -> 00BF6713, then
// 008F6EB4 008F5900(vector, &ret, {vector, last}, &[node+8]); count += 1; 0047E480 next
// (008F6EC1). Then the two first <= last checks (00BF6713 at 008F6EE1 / 008F6EF2) and 008F6F08
// 008F3C90(first, last, size, 008EFBC0). out+4 = 0 (008F6F15); for i < count: checked index
// (00BF6713 at 008F6F35), 008F6F44 008F0010(out, &v[i]); _free(first) when non-null (008F6F59).
// ---------------------------------------------------------------------------
std::int32_t scene_property_bag_collect_records_sorted_008f6e20(Seg59bHost& host, void* bag, void* out) {
    alignas(4) std::uint8_t vector[0x10] = {};
    auto first = [&]() { return static_cast<std::uint8_t*>(ptr(vector, 0x04)); };
    auto last = [&]() { return static_cast<std::uint8_t*>(ptr(vector, 0x08)); };
    auto end = [&]() { return static_cast<std::uint8_t*>(ptr(vector, 0x0C)); };
    Hash64Iterator iterator{add(bag, 0x04), nullptr, 0};
    host.scene_property_bag_iterator_begin_00480690(&iterator);
    std::int32_t count = 0;
    while (iterator.node != nullptr) {
        void* const value = add(iterator.node, 0x08);
        if (first() != nullptr && ((last() - first()) >> 2) < ((end() - first()) >> 2)) {
            std::memcpy(last(), value, 4);
            at<std::uint8_t*>(vector, 0x08) = last() + 4;
        } else {
            if (first() > last()) host.checked_iterator_failure_00bf6713();
            alignas(4) std::uint8_t ret[8] = {};
            host.pointer_vector_insert_008f5900(vector, ret, vector, last(), value);
        }
        host.scene_property_bag_iterator_next_0047e480(&iterator);
        ++count;
    }
    std::uint8_t* const sort_last = last();
    if (first() > sort_last) {
        host.checked_iterator_failure_00bf6713();
        if (first() > last()) host.checked_iterator_failure_00bf6713();
    }
    host.sort_by_predicate_008f3c90(first(), sort_last, static_cast<std::int32_t>((sort_last - first()) >> 2),
                                    0x008EFBC0u);
    at<std::uint32_t>(out, 0x04) = 0;
    for (std::int32_t index = 0; index < count; ++index) {
        if (first() == nullptr || static_cast<std::uint32_t>(index) >= static_cast<std::uint32_t>((last() - first()) >> 2)) {
            host.checked_iterator_failure_00bf6713();
        }
        pointer_array_push_back_grow_008f0010(host, out,
                                              reinterpret_cast<const std::uint32_t*>(first() + index * 4));
    }
    if (first() != nullptr) host.free_00bf65ac(first());
    return count;
}

namespace {
// The key lookup shared by 008F4290 and 008F4420: a local NativeString {0, 0} sized by 0041DD40
// to strlen(name) and filled by memcpy(size + 1) when its data is non-null, 0043B8B0 on the
// destination map, then the key released (00419CC0 / 00BD1510). EAX = the found node's record.
void* find_record_by_iterator_key(Seg59bHost& host, Hash64Iterator* iterator, void* map, bool& found) {
    const char* const name = host.property_bag_iterator_key_00484d20(iterator);
    alignas(4) std::uint8_t key[8] = {};
    host.native_string_resize_0041dd40(key, static_cast<std::uint32_t>(std::strlen(name)), 1);
    if (ptr(key, 0x04) != nullptr) host.memcpy_00bf7680(ptr(key, 0x04), name, at<std::uint32_t>(key, 0x00) + 1u);
    std::uint32_t bucket = 0;
    void* const node = host.property_map_find_node_0043b8b0(map, key, &bucket);
    found = node != nullptr;
    void* const record = found ? ptr(node, 0x08) : nullptr;
    release_native_string(host, key);
    return record;
}
}  // namespace

// ---------------------------------------------------------------------------
// 008F4290 BSP_ScenePropertyBag_SubtractEqual. __thiscall(ECX = bag, other), RET 4. Only caller
// is its own recursion (008F43AC); no absolute-dword reference (CC13_BAND_0080_008F.md).
// Evidence: 008F42AF 00BF681B(0Ch) iterator, non-null -> [it] = other+4, 008F42CA 00480690 begin;
// a null allocation leaves 0 and the image reads [0+4]. While [it+4] != 0: 008F42FA 00484D20 key,
// key NativeString (0041DD40 at 008F4322, memcpy at 008F4339), 008F4353 0043B8B0(bag+4); found ->
// record = [node+8]; key released (008F437C/008F4383). Found: record type 6 -> the other record
// ([[it+4]+8]) must be type 6 too (008F43A3), recurse 008F43AC(record+0C, other+0C), remove when
// [[record+0C]+8] (count) == 0; other types -> 008F43CB 008EF820(record, other record) and remove
// on AL. Remove = 008F43D9 004F01F0(bag+4, record). 008F43E4 0047E480 next. Then 008F43F9 _free.
// ---------------------------------------------------------------------------
void scene_property_bag_subtract_equal_008f4290(Seg59bHost& host, void* bag, void* other) {
    auto* const iterator = static_cast<Hash64Iterator*>(host.operator_new_00bf681b(0x0C));
    if (iterator != nullptr) {
        iterator->table = add(other, 0x04);
        host.scene_property_bag_iterator_begin_00480690(iterator);
    }
    if (iterator->node != nullptr) {
        void* const map = add(bag, 0x04);
        do {
            bool found = false;
            void* const record = find_record_by_iterator_key(host, iterator, map, found);
            if (found) {
                void* const other_record = ptr(iterator->node, 0x08);
                bool remove = false;
                if (at<std::uint32_t>(record, 0x04) == 6) {
                    if (at<std::uint32_t>(other_record, 0x04) == 6) {
                        scene_property_bag_subtract_equal_008f4290(host, ptr(record, 0x0C), ptr(other_record, 0x0C));
                        remove = at<std::uint32_t>(ptr(record, 0x0C), 0x08) == 0;
                    }
                } else {
                    remove = scene_property_value_equals_008ef820(host, record, other_record);
                }
                if (remove) host.scene_property_bag_remove_record_004f01f0(map, record);
            }
            host.scene_property_bag_iterator_next_0047e480(iterator);
        } while (iterator->node != nullptr);
    }
    host.free_00bf65ac(iterator);
}

// ---------------------------------------------------------------------------
// 008F4420 BSP_ScenePropertyBag_SubtractSameType. __thiscall(ECX = bag, other), RET 4. Only
// caller is its own recursion (008F4536); no absolute-dword reference.
// Evidence: the 008F4290 shape without the value test: found and record type ([EDI+4]) equal to
// the other record's type (008F4525) -> type 6 recurses 008F4536(record+0C, other+0C) and removes
// when the sub-bag count is 0 (008F453E), other types remove directly; 008F4549 004F01F0(bag+4,
// record). The loop (008F454E..008F4556) re-tests [it+4] with no 0047E480 call: the iterator
// never advances, so a non-empty other spins on its first key. Reproduced as in the image.
// ---------------------------------------------------------------------------
void scene_property_bag_subtract_same_type_008f4420(Seg59bHost& host, void* bag, void* other) {
    auto* const iterator = static_cast<Hash64Iterator*>(host.operator_new_00bf681b(0x0C));
    if (iterator != nullptr) {
        iterator->table = add(other, 0x04);
        host.scene_property_bag_iterator_begin_00480690(iterator);
    }
    if (iterator->node != nullptr) {
        void* const map = add(bag, 0x04);
        do {
            bool found = false;
            void* const record = find_record_by_iterator_key(host, iterator, map, found);
            if (found) {
                void* const other_record = ptr(iterator->node, 0x08);
                const std::uint32_t type = at<std::uint32_t>(record, 0x04);
                if (type == at<std::uint32_t>(other_record, 0x04)) {
                    bool remove = true;
                    if (type == 6) {
                        scene_property_bag_subtract_same_type_008f4420(host, ptr(record, 0x0C),
                                                                       ptr(other_record, 0x0C));
                        remove = at<std::uint32_t>(ptr(record, 0x0C), 0x08) == 0;
                    }
                    if (remove) host.scene_property_bag_remove_record_004f01f0(map, record);
                }
            }
        } while (iterator->node != nullptr);
    }
    host.free_00bf65ac(iterator);
}

// ---------------------------------------------------------------------------
// 008F1A20 BSP_SceneObject_BuildDottedPath. __thiscall(ECX = object, out NativeString), RET 4.
// No code caller and no absolute-dword reference (CC13_BAND_0080_008F.md "Unreferenced").
// Evidence: owner = [object+30h]; 0 -> 008F1C69 0041DD40(out, 0, 0) and memcpy(data, "" CE3A0C,
// size) (008F1C7E). Else name = 008F0B00(owner+4, object) (008F1A52), out = 0041DD40(out,
// strlen or 0, 0) + memcpy(data, name, size) (008F1A77/008F1A88). cursor = owner+110h; while
// [cursor] != 0: "." (CE3A70) NativeString (0041DD40(1, 1), memcpy size+1 at 008F1AE3); parent =
// [cursor]; name = parent and [parent+30h] ? 008F0B00([parent+30h]+4, parent) : 0 (008F1B00), its
// NativeString (008F1B2A, memcpy 008F1B41); 008F1B5C 004261A0(name, &joined, &dot); 008F1B6E
// 004261A0(joined, &result, out); result != out -> 0041DD40(out, result.size, 1) and, size != 0,
// memcpy(out.data, result.data, out.size) (008F1B9A); result, joined, name and dot released in
// that order; cursor = [[cursor]+30h]+110h (008F1C3A).
// The ledger's FormatValue name is replaced: the routine joins owner names with ".", outermost
// first. A null name in the loop is passed to strlen, as in the image.
// ---------------------------------------------------------------------------
void scene_object_build_dotted_path_008f1a20(Seg59bHost& host, void* object, void* out) {
    void* const owner = ptr(object, 0x30);
    if (owner == nullptr) {
        host.native_string_resize_0041dd40(out, 0, 0);
        if (ptr(out, 0x04) != nullptr) {
            host.memcpy_00bf7680(ptr(out, 0x04), host.image_pointer(kEmptyString), at<std::uint32_t>(out, 0x00));
        }
        return;
    }
    const char* const name = scene_enum_table_find_name_008f0b00(
        host, add(owner, 0x04), static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(object)));
    host.native_string_resize_0041dd40(out, name != nullptr ? static_cast<std::uint32_t>(std::strlen(name)) : 0u, 0);
    if (ptr(out, 0x04) != nullptr) host.memcpy_00bf7680(ptr(out, 0x04), name, at<std::uint32_t>(out, 0x00));
    void* cursor = add(owner, 0x110);
    while (ptr(cursor, 0x00) != nullptr) {
        alignas(4) std::uint8_t dot[8] = {};
        host.native_string_resize_0041dd40(dot, 1, 1);
        if (ptr(dot, 0x04) != nullptr) {
            host.memcpy_00bf7680(ptr(dot, 0x04), host.image_pointer(0x00CE3A70), at<std::uint32_t>(dot, 0x00) + 1u);
        }
        void* const parent = ptr(cursor, 0x00);
        const char* parent_name = nullptr;
        if (parent != nullptr && ptr(parent, 0x30) != nullptr) {
            parent_name = scene_enum_table_find_name_008f0b00(
                host, add(ptr(parent, 0x30), 0x04), static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(parent)));
        }
        alignas(4) std::uint8_t name_string[8] = {};
        host.native_string_resize_0041dd40(name_string, static_cast<std::uint32_t>(std::strlen(parent_name)), 1);
        if (ptr(name_string, 0x04) != nullptr) {
            host.memcpy_00bf7680(ptr(name_string, 0x04), parent_name, at<std::uint32_t>(name_string, 0x00) + 1u);
        }
        alignas(4) std::uint8_t joined[8] = {};
        alignas(4) std::uint8_t result_storage[8] = {};
        host.native_string_concat_004261a0(name_string, joined, dot);
        void* const result = host.native_string_concat_004261a0(joined, result_storage, out);
        if (out != result) {
            host.native_string_resize_0041dd40(out, at<std::uint32_t>(result, 0x00), 1);
            if (at<std::uint32_t>(result, 0x00) != 0) {
                host.memcpy_00bf7680(ptr(out, 0x04), ptr(result, 0x04), at<std::uint32_t>(out, 0x00));
            }
        }
        release_native_string(host, result_storage);
        release_native_string(host, joined);
        release_native_string(host, name_string);
        release_native_string(host, dot);
        cursor = add(ptr(ptr(cursor, 0x00), 0x30), 0x110);
    }
}

namespace {
const char* image_text(Seg59bHost& host, std::uint32_t address) {
    return static_cast<const char*>(host.image_pointer(address));
}

// The MSVC std::string the 008F71E0 / 008F7550 / 008F79A0 frames build inline (1Ch bytes):
// {+0 allocator, +4 16-byte buffer or pointer, +14h size, +18h capacity}; capacity 0Fh is the
// inline buffer. Construction and the post-destroy tidy both store capacity 0Fh, size 0 and a
// zero first byte; destruction frees the pointer (00BF65AC) when capacity >= 10h.
void std_string_tidy(void* string) {
    at<std::uint32_t>(string, 0x18) = 0x0F;
    at<std::uint32_t>(string, 0x14) = 0;
    at<std::uint8_t>(string, 0x04) = 0;
}

void std_string_release(Seg59bHost& host, void* string) {
    if (at<std::uint32_t>(string, 0x18) >= 0x10) host.free_00bf65ac(ptr(string, 0x04));
}

const char* std_string_c_str(void* string) {
    return static_cast<const char*>(at<std::uint32_t>(string, 0x18) >= 0x10 ? ptr(string, 0x04) : add(string, 0x04));
}

// One map<std::string, record*> insertion as both writers inline it: name -> std::string T
// (00408720), pair P1 = {T copy (00408120(P1, T, 0, -1)), record}, P2 = {P1 copy, record},
// 008F6BA0(map, &result, &P2); then P2 and P1 destroyed with their tidy, T destroyed.
void insert_named_record(Seg59bHost& host, void* map, void* result, const char* name, void* record) {
    alignas(4) std::uint8_t named[0x1C] = {};
    alignas(4) std::uint8_t first[0x20] = {};
    alignas(4) std::uint8_t second[0x20] = {};
    std_string_tidy(named);
    host.std_string_assign_counted_00408720(named, name, static_cast<std::uint32_t>(std::strlen(name)));
    std_string_tidy(first);
    host.std_string_assign_substring_00408120(first, named, 0, 0xFFFFFFFFu);
    at<void*>(first, 0x1C) = record;
    std_string_tidy(second);
    host.std_string_assign_substring_00408120(second, first, 0, 0xFFFFFFFFu);
    at<void*>(second, 0x1C) = at<void*>(first, 0x1C);
    host.property_tree_insert_unique_008f6ba0(map, result, second);
    std_string_release(host, second);
    std_string_tidy(second);
    std_string_release(host, first);
    std_string_tidy(first);
    std_string_release(host, named);
}

// The map {+0 comparator, +4 head, +8 size} set up inline: 008F0AB0 head, head isnil (+2Dh) = 1,
// head left/parent/right = head, size 0.
void init_named_record_map(Seg59bHost& host, void* map) {
    at<void*>(map, 0x04) = host.stl_tree_buy_head_node_008f0ab0(map);
    void* const head = ptr(map, 0x04);
    at<std::uint8_t>(head, 0x2D) = 1;
    at<void*>(ptr(map, 0x04), 0x04) = ptr(map, 0x04);
    at<void*>(ptr(map, 0x04), 0x00) = ptr(map, 0x04);
    at<void*>(ptr(map, 0x04), 0x08) = ptr(map, 0x04);
    at<std::uint32_t>(map, 0x08) = 0;
}

// The checked in-order walk prologue per step: iterator {container, node}. Returns false at end.
bool named_record_iterator_step(Seg59bHost& host, void* map, void* iterator, void*& node) {
    void* const end = ptr(map, 0x04);
    void* const container = ptr(iterator, 0x00);
    if (container == nullptr || container != map) host.checked_iterator_failure_00bf6713();
    node = ptr(iterator, 0x04);
    if (node == end) return false;
    if (container == nullptr) host.checked_iterator_failure_00bf6713();
    if (node == ptr(container, 0x04)) host.checked_iterator_failure_00bf6713();
    return true;
}

// The map teardown: 008F6D50(map, &result, {map, head->left}, {map, head}), _free(head), then
// head and size zeroed.
void destroy_named_record_map(Seg59bHost& host, void* map, void* result) {
    void* const head = ptr(map, 0x04);
    host.stl_tree_erase_range_008f6d50(map, result, map, ptr(head, 0x00), map, head);
    host.free_00bf65ac(ptr(map, 0x04));
    at<void*>(map, 0x04) = nullptr;
    at<std::uint32_t>(map, 0x08) = 0;
}
}  // namespace

// ---------------------------------------------------------------------------
// 008F71E0 BSP_ScenePropertyBag_WriteTextBlock. __thiscall(ECX = bag, writer), RET 4. Callers:
// itself (008F746C) and 008F7550 (008F7586, 008F78C5); no absolute-dword reference.
// Evidence: 008F7216 0094FF10(writer, " {" D16564), 008F721D 0094FFA0(.., 0). An unused pair
// {std::string, 0} constructed at frame+94h (008F7237..008F724C) and destroyed at the end. map at
// frame+10h set up through 008F0AB0 (008F725E). Walk of bag+4 (00480690 at 008F722D, 0047E480 at
// 008F73BC): name = [node+4] or F89450, record = [node+8], inserted as above (00408720 at
// 008F72D7, 00408120 at 008F72FF / 008F732C, 008F6BA0 at 008F7352). writer+8 += 1 (008F73D8).
// In order from head->left with the 00BF6713 checks (008F7400..008F7428): key = the node's
// c_str (+10h, or the pointer when [+24h] >= 10h); record = [node+28h]; type 6 -> 0094FF10 '"'
// (CE436C), key, '"', then 008F746C 008F71E0(record+0Ch, writer); else 008F7495 008F1CA0(bag,
// key, record, writer); 008EFF30 next. Then writer+8 -= 1 when non-zero (008F74B7), 0094FF10
// "}" (CE4CD4) + 0094FFA0(0), 008F74F2 008F6D50 erase, 008F74FC _free(head), the pair's string
// freed when its capacity >= 10h (never: it is not written).
// ---------------------------------------------------------------------------
void scene_property_bag_write_text_block_008f71e0(Seg59bHost& host, void* bag, void* writer) {
    host.scene_text_writer_manipulator_0094ffa0(host.scene_text_write_0094ff10(writer, image_text(host, 0x00D16564u)), 0);
    Hash64Iterator iterator{add(bag, 0x04), nullptr, 0};
    host.scene_property_bag_iterator_begin_00480690(&iterator);
    alignas(4) std::uint8_t unused_pair[0x20] = {};
    std_string_tidy(unused_pair);
    at<std::uint32_t>(unused_pair, 0x1C) = 0;
    alignas(4) std::uint8_t map[0x0C] = {};
    init_named_record_map(host, map);
    alignas(4) std::uint8_t result[0x0C] = {};
    alignas(4) std::uint8_t erased[0x08] = {};
    while (iterator.node != nullptr) {
        void* const record = ptr(iterator.node, 0x08);
        const char* name = static_cast<const char*>(ptr(iterator.node, 0x04));
        if (name == nullptr) name = image_text(host, kEnumNameEmpty);
        insert_named_record(host, map, result, name, record);
        host.scene_property_bag_iterator_next_0047e480(&iterator);
    }
    at<std::uint32_t>(writer, 0x08) += 1;
    alignas(4) std::uint8_t walk[0x08] = {};
    at<void*>(walk, 0x00) = map;
    at<void*>(walk, 0x04) = ptr(ptr(map, 0x04), 0x00);
    void* node = nullptr;
    while (named_record_iterator_step(host, map, walk, node)) {
        void* const record = ptr(node, 0x28);
        if (node == ptr(ptr(walk, 0x00), 0x04)) host.checked_iterator_failure_00bf6713();
        const char* const key = std_string_c_str(add(node, 0x0C));
        if (at<std::uint32_t>(record, 0x04) == 6) {
            void* const quote_writer = host.scene_text_write_0094ff10(writer, image_text(host, 0x00CE436Cu));
            host.scene_text_write_0094ff10(host.scene_text_write_0094ff10(quote_writer, key), image_text(host, 0x00CE436Cu));
            scene_property_bag_write_text_block_008f71e0(host, ptr(record, 0x0C), writer);
        } else {
            scene_property_write_text_008f1ca0(host, key, record, writer);
        }
        host.stl_tree_iterator_increment_008eff30(walk);
    }
    if (at<std::uint32_t>(writer, 0x08) != 0) at<std::uint32_t>(writer, 0x08) -= 1;
    host.scene_text_writer_manipulator_0094ffa0(host.scene_text_write_0094ff10(writer, image_text(host, 0x00CE4CD4u)), 0);
    destroy_named_record_map(host, map, erased);
    std_string_release(host, unused_pair);
}

// ---------------------------------------------------------------------------
// 008F7550 BSP_ScenePropertyBag_WriteTextDiff. __thiscall(ECX = bag, writer, base), RET 8.
// Caller: itself (008F78A2); no absolute-dword reference.
// Evidence: base == 0 -> 008F7586 008F71E0(bag, writer) and return. Else 0094FF10 " {" +
// 0094FFA0(0) (008F75A1/008F75A8); pointer array {00BF55BE(40h*4) (overflow-checked size,
// 008F75BB..008F75CD), 0, 40h, 40h}; 008F75EB 008F6E20(bag, &array); unused pair and map as in
// 008F71E0 (008F75F0..008F763D). For i < count (signed): record = data[i]; name = [record+30h] ?
// 008F0B00([record+30h]+4, record) : 0 (strlen of 0 faults, as in the image); inserted (00408720
// at 008F76A4, 00408120 at 008F76D1 / 008F7704, 008F6BA0 at 008F772A). writer+8 += 1. In order:
// other = 008F7823 008F2260(base, key); record type != 6 -> other non-null and 008F7839
// 008EF820(record, other) true -> skip; otherwise 008F7851 008F1CA0(bag, key, record, writer).
// Type 6 -> '"', key, '"'; other non-null with type 6 -> 008F78A2 008F7550(record+0Ch, writer,
// other+0Ch); else 008F78C5 008F71E0(record+0Ch, writer). Then "}" + 0094FFA0(0), 008F7923
// 008F6D50 erase, _free(head), the pair's string, array data non-null -> 008F797B 00BF6989.
// The ledger name WriteTextEntry is replaced: the routine writes only what differs from base.
// ---------------------------------------------------------------------------
void scene_property_bag_write_text_diff_008f7550(Seg59bHost& host, void* bag, void* writer, void* base) {
    if (base == nullptr) {
        scene_property_bag_write_text_block_008f71e0(host, bag, writer);
        return;
    }
    host.scene_text_writer_manipulator_0094ffa0(host.scene_text_write_0094ff10(writer, image_text(host, 0x00D16564u)), 0);
    alignas(4) std::uint8_t array[0x10] = {};
    at<std::uint32_t>(array, 0x0C) = 0x40;
    at<std::uint32_t>(array, 0x08) = 0x40;
    at<void*>(array, 0x00) = host.operator_new_00bf55be(0x40u * 4u);
    at<std::int32_t>(array, 0x04) = 0;
    scene_property_bag_collect_records_sorted_008f6e20(host, bag, array);
    alignas(4) std::uint8_t unused_pair[0x20] = {};
    std_string_tidy(unused_pair);
    at<std::uint32_t>(unused_pair, 0x1C) = 0;
    alignas(4) std::uint8_t map[0x0C] = {};
    init_named_record_map(host, map);
    alignas(4) std::uint8_t result[0x0C] = {};
    alignas(4) std::uint8_t erased[0x08] = {};
    for (std::int32_t index = 0; index < at<std::int32_t>(array, 0x04); ++index) {
        void* const record = static_cast<void**>(ptr(array, 0x00))[index];
        const char* name = nullptr;
        if (ptr(record, 0x30) != nullptr) {
            name = scene_enum_table_find_name_008f0b00(
                host, add(ptr(record, 0x30), 0x04), static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(record)));
        }
        insert_named_record(host, map, result, name, static_cast<void**>(ptr(array, 0x00))[index]);
    }
    at<std::uint32_t>(writer, 0x08) += 1;
    alignas(4) std::uint8_t walk[0x08] = {};
    at<void*>(walk, 0x00) = map;
    at<void*>(walk, 0x04) = ptr(ptr(map, 0x04), 0x00);
    void* node = nullptr;
    while (named_record_iterator_step(host, map, walk, node)) {
        void* const record = ptr(node, 0x28);
        if (node == ptr(ptr(walk, 0x00), 0x04)) host.checked_iterator_failure_00bf6713();
        const char* const key = std_string_c_str(add(node, 0x0C));
        void* const other = host.scene_property_bag_find_008f2260(base, key);
        if (at<std::uint32_t>(record, 0x04) == 6) {
            void* const quote_writer = host.scene_text_write_0094ff10(writer, image_text(host, 0x00CE436Cu));
            host.scene_text_write_0094ff10(host.scene_text_write_0094ff10(quote_writer, key), image_text(host, 0x00CE436Cu));
            if (other != nullptr && at<std::uint32_t>(other, 0x04) == 6) {
                scene_property_bag_write_text_diff_008f7550(host, ptr(record, 0x0C), writer, ptr(other, 0x0C));
            } else {
                scene_property_bag_write_text_block_008f71e0(host, ptr(record, 0x0C), writer);
            }
        } else if (other == nullptr || !scene_property_value_equals_008ef820(host, record, other)) {
            scene_property_write_text_008f1ca0(host, key, record, writer);
        }
        host.stl_tree_iterator_increment_008eff30(walk);
    }
    if (at<std::uint32_t>(writer, 0x08) != 0) at<std::uint32_t>(writer, 0x08) -= 1;
    host.scene_text_writer_manipulator_0094ffa0(host.scene_text_write_0094ff10(writer, image_text(host, 0x00CE4CD4u)), 0);
    destroy_named_record_map(host, map, erased);
    std_string_release(host, unused_pair);
    std_string_tidy(unused_pair);
    if (ptr(array, 0x00) != nullptr) host.free_00bf6989(ptr(array, 0x00));
}

// ---------------------------------------------------------------------------
// 008F79A0 BSP_ScenePropertyBag_RemoveMissing. __thiscall(ECX = bag, base, other), RET 8.
// Caller: itself (008F7A6B); no absolute-dword reference.
// Evidence: iterator {bag+4} begun (008F79CD); other != 0 -> a heap iterator 00BF681B(0Ch)
// {other+4} begun (008F79F3) and copied over the local one (never freed; a null allocation is
// read through, as in the image). vector<std::string> {+0, first, last, end} zeroed. Per node:
// name = [node+4] or F89450; found = 008F7A4F 008F2260(base, name); found and type 6 ->
// 008F7A6B 008F79A0(bag, found+0Ch, [[node+8]+0Ch]); found other type -> nothing; not found ->
// std::string (00408720 at 008F7AA5) pushed by 008F7AB8 00849170(vector, &string), then freed.
// 0047E480 next (008F7ADA). Then other != 0: for i < size (1Ch stride, signed /1Ch) while first
// != 0: 008F7B3D 0043D870(other, c_str); other == 0: a NativeString of the c_str (0041DD40 at
// 008F7BB2, memcpy at 008F7BC9), 008F7BDF 008F4030(bag+4, &key), released. A null first skips
// the teardown; otherwise 008F7C25 004CE520(ECX = first, EDX = last, &vector, junk) and
// 008F7C2F _free(first). The junk dword is arg 2's slot ([ESP+60h]): other, or the byte offset
// the other == 0 loop kept there.
// The ledger name WriteFiltered is replaced: the routine erases the names base does not have.
// ---------------------------------------------------------------------------
void scene_property_bag_remove_missing_008f79a0(Seg59bHost& host, void* bag, void* base, void* other) {
    Hash64Iterator iterator{add(bag, 0x04), nullptr, 0};
    host.scene_property_bag_iterator_begin_00480690(&iterator);
    if (other != nullptr) {
        auto* const heap_iterator = static_cast<Hash64Iterator*>(host.operator_new_00bf681b(0x0C));
        if (heap_iterator != nullptr) {
            heap_iterator->table = add(other, 0x04);
            host.scene_property_bag_iterator_begin_00480690(heap_iterator);
        }
        iterator = *heap_iterator;
    }
    alignas(4) std::uint8_t vector[0x10] = {};
    while (iterator.node != nullptr) {
        const char* name = static_cast<const char*>(ptr(iterator.node, 0x04));
        if (name == nullptr) name = image_text(host, kEnumNameEmpty);
        void* const found = host.scene_property_bag_find_008f2260(base, name);
        if (found != nullptr) {
            if (at<std::uint32_t>(found, 0x04) == 6) {
                scene_property_bag_remove_missing_008f79a0(host, bag, ptr(found, 0x0C),
                                                           ptr(ptr(iterator.node, 0x08), 0x0C));
            }
        } else {
            const char* missing = static_cast<const char*>(ptr(iterator.node, 0x04));
            if (missing == nullptr) missing = image_text(host, kEnumNameEmpty);
            alignas(4) std::uint8_t string[0x1C] = {};
            std_string_tidy(string);
            host.std_string_assign_counted_00408720(string, missing, static_cast<std::uint32_t>(std::strlen(missing)));
            host.string_vector_push_back_00849170(vector, string);
            std_string_release(host, string);
        }
        host.scene_property_bag_iterator_next_0047e480(&iterator);
    }
    auto size = [&]() {
        return static_cast<std::int32_t>(static_cast<std::uint8_t*>(ptr(vector, 0x08)) -
                                         static_cast<std::uint8_t*>(ptr(vector, 0x04))) / 0x1C;
    };
    std::uint32_t junk = static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(other));
    if (other != nullptr) {
        for (std::uint32_t index = 0;; ++index) {
            if (ptr(vector, 0x04) == nullptr) return;
            if (index >= static_cast<std::uint32_t>(size())) break;
            host.property_map_erase_by_name_0043d870(
                other, std_string_c_str(add(ptr(vector, 0x04), index * 0x1C)));
        }
    } else {
        junk = 0;
        for (std::uint32_t index = 0;; ++index) {
            if (ptr(vector, 0x04) == nullptr) return;
            if (index >= static_cast<std::uint32_t>(size())) break;
            const char* const text = std_string_c_str(add(ptr(vector, 0x04), junk));
            alignas(4) std::uint8_t key[8] = {};
            host.native_string_resize_0041dd40(key, static_cast<std::uint32_t>(std::strlen(text)), 1);
            if (ptr(key, 0x04) != nullptr) host.memcpy_00bf7680(ptr(key, 0x04), text, at<std::uint32_t>(key, 0x00) + 1u);
            property_map_unlink_key_008f4030(host, add(bag, 0x04), key);
            release_native_string(host, key);
            junk += 0x1C;
        }
    }
    host.string_range_destroy_004ce520(ptr(vector, 0x04), ptr(vector, 0x08), vector, junk);
    host.free_00bf65ac(ptr(vector, 0x04));
}
}  // namespace bsp::cc13_seg59b
