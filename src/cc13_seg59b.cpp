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
    if (unit != nullptr && host.entity_is_kind_of_vt5c(unit, 5)) {
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
    at<void*>(bot, 0x5C) = ptr(bot, 0x50);
    for (std::size_t off = 0x60; off <= 0x70; off += 4) at<std::uint32_t>(bot, off) = 0;
}

// ---------------------------------------------------------------------------
// 008FE7A0 BSP_GunBot_ClearFireTargetLeadClass. __fastcall(ECX = bot), RET; slot +30h.
// Evidence: 008FE7A3 0072BD50; 008FE7AB [[bot+50h]+1ACh]; 008FE7B1 CMP 8 JZ; 008FE7B7 00927F10;
// 008FE7C0..008FE7CD [bot+5Ch]->vt[1E8h](0).
// ---------------------------------------------------------------------------
void aa_gunner_bot_clear_fire_target_008fe7a0(Seg59bHost& host, void* bot) {
    clear_fire_target_then_release(host, bot, 0x5C);
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
    at<void*>(bot, 0x5C) = ptr(bot, 0x50);
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
    at<void*>(bot, 0x68) = ptr(bot, 0x50);
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
    clear_fire_target_then_release(host, bot, 0x68);
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
// 009043A0 BSP_SpatialIndex_QuerySegmentExcluding (hypothesis). __stdcall(from, to, record,
// entity, kind_filter), RET 14h; EAX = 0098ADD0's result. Evidence: entity = arg 4 (009043A1);
// non-null: 009043AA 0042E630; kind_filter and record are pushed first (009043BB/009043C2) and
// stay on the stack as 0098ADD0's arguments 5 and 4 across 009043C5 entity->vt[B0h]() (no stack
// arguments); 009043D4 0098ADD0(ECX = index; from, to, exclude = that result, record,
// kind_filter). Null: 009043DE 0042E630, the frame is rewritten in place (009043E8/009043EC) and
// 009043F6 tail-jumps 0098ADD0(from, to, 0, record, kind_filter). Callers: HUD unit pick
// 00526A40, free-camera effects 0053F2D0, 005220C0.
// ---------------------------------------------------------------------------
std::uint32_t spatial_query_segment_excluding_009043a0(Seg59bHost& host, void* from, void* to,
                                                       void* record, void* entity,
                                                       std::uint32_t kind_filter) {
    if (entity != nullptr) {
        void* const index = host.spatial_index_singleton_0042e630();
        void* const exclude = host.entity_query_exclude_vtb0(entity);
        return host.spatial_index_query_segment_0098add0(index, from, to, exclude, record, kind_filter);
    }
    void* const index = host.spatial_index_singleton_0042e630();
    return host.spatial_index_query_segment_0098add0(index, from, to, nullptr, record, kind_filter);
}

// ---------------------------------------------------------------------------
// 009052E0 BSP_ScoreRecord_AddField18 (hypothesis). __thiscall(ECX = scoring, slot, delta),
// RET 8; EAX = &record+18h. Evidence: 009052E4 IMUL slot,284h; 009052EA FLD [rec+18h];
// 009052EE FADD float [ESP+8]; 009052F2 FSTP back; 009052F6 LEA EAX. Caller 005B45C0
// (HUD map update). x87: one add of two floats rounded to float; the same at 24- and 53-bit
// precision control (a float sum fits double exactly before the final rounding).
// ---------------------------------------------------------------------------
float* score_record_add_field18_009052e0(void* scoring, std::int32_t slot, float delta) {
    float* const field = &at<float>(scoring, static_cast<std::size_t>(slot * 0x284) + 0x18);
    *field = *field + delta;
    return field;
}
}  // namespace bsp::cc13_seg59b
