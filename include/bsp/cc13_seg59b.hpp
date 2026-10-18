// Segment 59 gun bots (008FB000..00905300), reconstruction pass (packet cc13-seg59A).
//
// The torpedo bot, the gunner bots (lead / turret / ballistic / torpedo / depth-charge gun-bot
// classes) and the robot-config descriptor rows that still had no body or ledger record.
// docs/CC13_SEG59B.md carries the evidence and the status of every routine, including the ones
// recorded ledger-only because a cc9 gameplay host (or an earlier cc13 file) already implements
// them.
//
// Model: every routine works on the original object as raw bytes (Win32 layout, offsets as in the
// image) and reaches the engine through Seg59bHost, whose methods name the image routine or the
// virtual slot they stand for. New C++ interfaces, not drop-in binary replacements; nothing here is
// bound into bsp_game. Names are hypotheses, not recovered symbols. Marker: cc13_seg59b.

#ifndef BSP_CC13_SEG59B_HPP
#define BSP_CC13_SEG59B_HPP

#include <cstddef>
#include <cstdint>

namespace bsp::cc13_seg59b {

template <class T>
inline T& at(void* base, std::size_t off) noexcept {
    return *reinterpret_cast<T*>(static_cast<std::uint8_t*>(base) + off);
}
inline void* ptr(void* base, std::size_t off) noexcept { return at<void*>(base, off); }
inline void* add(void* base, std::size_t off) noexcept {
    return static_cast<std::uint8_t*>(base) + off;
}

// The property archive's tagged by-value argument (8 bytes on the stack): kind 0 = C string key,
// 2 = float (value bits when written, float* when read).
struct ArchiveArg {
    std::uint32_t kind;
    std::uintptr_t value;
};
inline ArchiveArg key(const char* name) noexcept {
    return ArchiveArg{0u, reinterpret_cast<std::uintptr_t>(name)};
}

// ---------------------------------------------------------------------------
// Image constants.
// ---------------------------------------------------------------------------
inline constexpr std::uint32_t kTorpedoBotVtable = 0x00D182B0;          // primary
inline constexpr std::uint32_t kTorpedoBotObserverVtable = 0x00D18294;  // +1Ch
inline constexpr std::uint32_t kTorpedoBotInitialRangeBits = 0x461C4000; // 00CE3D64, 10000.0f
inline constexpr std::uint32_t kTorpedoDescriptorGlobal = 0x00E1998C;   // TorpedoBot descriptor
inline constexpr const char* kBotSectionKey = "_bot";                   // 00CF904C
inline constexpr const char* kThinkTimeLeftKey = "thinkTimeLeft";       // 00D1830C
inline constexpr const char* kTargetAngleKey = "targetAngle";           // 00D18300

// ---------------------------------------------------------------------------
// Engine boundary. Each method stands for one image routine or one virtual slot, named by
// address; the comment gives the original register ABI.
// ---------------------------------------------------------------------------
struct Seg59bHost {
    virtual ~Seg59bHost() = default;

    // The dword at an image global (the robot descriptor aliases at 00E19988..00E199A0).
    virtual std::uint32_t image_dword(std::uint32_t address) = 0;

    // 0072BBD0, __thiscall(bot, descriptor), RET 4: the gun-bot base constructor.
    virtual void gun_bot_construct_base_0072bbd0(void* bot, std::uint32_t descriptor) = 0;
    // 008FE660 / 008FE6D0, __thiscall(bot, archive), RET 4: the base property visitors.
    virtual void gun_bot_read_base_properties_008fe660(void* bot, void* archive) = 0;
    virtual void gun_bot_write_base_properties_008fe6d0(void* bot, void* archive) = 0;

    // The property archive's virtual slots, __thiscall(archive, ...): +04h open section(key)
    // RET 8, +08h close section RET, +0Ch write field(key, value) RET 10h, +10h read field(key,
    // dest) RET 10h.
    virtual void archive_open_section_vt04(void* archive, ArchiveArg key) = 0;
    virtual void archive_close_section_vt08(void* archive) = 0;
    virtual void archive_write_field_vt0c(void* archive, ArchiveArg key, ArchiveArg value) = 0;
    virtual void archive_read_field_vt10(void* archive, ArchiveArg key, ArchiveArg dest) = 0;
};

// --- Torpedo bot (vtable 00D182B0) ---------------------------------------------------------
// 008FF260, __thiscall(ECX = bot), RET; EAX = bot. Base 0072BBD0([00E1998C]), vtables, +68h.
void* torpedo_bot_construct_008ff260(Seg59bHost& host, void* bot);
// 008FF360, __thiscall(bot, archive), RET 4; slot +48h. Save/load reader.
void torpedo_bot_read_properties_008ff360(Seg59bHost& host, void* bot, void* archive);
// 008FF400, __thiscall(bot, archive), RET 4; slot +4Ch. Save/load writer.
void torpedo_bot_write_properties_008ff400(Seg59bHost& host, void* bot, void* archive);

}  // namespace bsp::cc13_seg59b

#endif  // BSP_CC13_SEG59B_HPP
