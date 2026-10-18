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

}  // namespace bsp::cc13_seg59b
