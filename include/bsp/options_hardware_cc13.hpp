#pragma once

#include "bsp/game_settings.hpp"
#include "bsp/native_game_settings_owner.hpp"
#include "bsp/profile_commit.hpp"

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace bsp {

// Host projections, not original layouts or binary replacement interfaces.
// Address, ABI, assembly evidence and uncertainty: docs/CC13_OPTIONS_AND_HARDWARE.md.

// ---------------------------------------------------------------------------
// Settings save to the profile store: 008D6C00 and its continuation 008D6B70.
// The write counterpart of restore_profile_settings_008d7a50. Both share the
// pending-callback slot 00F88958, so the save takes ProfileSettingsRestoreState.
// ---------------------------------------------------------------------------
struct ProfileSettingsSaveHost {
    virtual ~ProfileSettingsSaveHost() = default;
    // Byte +21h of the storage manager at [0109CECC].
    virtual bool storage_available_21() = 0;
    // 00A3E510 with ECX = [00F8ABE8]; the native does not null-check the global.
    virtual bool xenon_has_selected_user_00a3e510() = 0;
    // 007F8CA0 with ECX = game+650h (the profile object).
    virtual bool profile_has_save_name_007f8ca0() = 0;
    // 00425F40 copy of game+684h (profile+34h).
    virtual std::string copy_profile_save_name_684() = 0;
    // 004C5E00: ECX = storage manager, calls manager vtable +10h with an
    // out-string. On the PC vtable 00D68C10 that slot is 00BEB630, the newest
    // valid slot (newest_valid_storage_slot_00beb630 below).
    virtual std::string storage_fallback_name_004c5e00() = 0;
    // 00BD3E10 (ECX = manager, NativeString*, DWORD kind; RET 8).
    virtual void request_single_write_00bd3e10(std::string_view name, std::uint32_t kind) = 0;
    // 006ADB50 stores ECX in 00E198F8 and drives the storage manager. It may
    // invoke the continuation synchronously or retain it across a prompt.
    virtual void run_storage_operation_006adb50(ProfileCompletion continuation) = 0;
    // Storage manager +08h. The writer is filled only in state 2.
    virtual int storage_state_08() = 0;
    // The temporary 8h store, vtable 00CE4104 and word +4 = 0. The native
    // normal path stores no retiring vtable here; end_ releases the host writer.
    virtual SettingsWriter& begin_settings_writer_00ce4104() = 0;
    virtual void end_settings_writer(SettingsWriter&) noexcept = 0;
};

// 008D6C00: __thiscall with ECX = settings (00F88980 at all seven call sites)
// but ECX is never read; one stack callback, RET 4. Only an accepted write
// stores the callback in 00F88958; every rejection invokes it directly.
void save_profile_settings_008d6c00(const GameSettingsBlock& settings,
    ProfileSettingsRestoreState& state, ProfileSettingsSaveHost& host,
    ProfileCompletion completion);

// 008D6B70: no arguments, RET. No Ghidra function existed at this address
// before this packet; it is reached only as the 006ADB50 continuation literal
// pushed at 008D6D11. State 2 writes through 008D64A0 and hands the pending
// callback to the storage driver; any other state delivers it directly. The
// slot is cleared before either use.
void complete_profile_settings_save_008d6b70(const GameSettingsBlock& settings,
    ProfileSettingsRestoreState& state, ProfileSettingsSaveHost& host);

inline constexpr std::uint32_t kSettingsSaveStorageKind = 0;   // 008D6CFD push 0
inline constexpr int kSettingsSaveWriterState = 2;             // 008D6B8D cmp [eax+8],2

// One FindFirstFileA/FindNextFileA result of 00BEB630's walk over the pattern
// at manager +53Ch (root + "\*", 00D68C58), with the two per-entry queries.
struct StorageSlotCandidate {
    std::string name;
    bool has_valid_marker{false};   // vtable +20h = 00BEB1C0, root\name\valid exists
    std::uint64_t valid_write_time{0}; // 00BEB0C0: last-write FILETIME of root\name\valid, 0 on failure
};

// 00BEB630 selection rule: ECX = manager, out NativeString*, RET 4. Starts from
// the empty string and a zero time; an entry with the valid marker replaces the
// choice only when its time is strictly greater (high dword first, then low),
// so ties keep the earlier entry and a zero time never wins. FindClose is
// called on the handle even when FindFirstFileA failed. The Win32 walk itself
// is not projected here; PcStorageBackend does not implement this slot yet.
std::string newest_valid_storage_slot_00beb630(const std::vector<StorageSlotCandidate>& entries);

// ---------------------------------------------------------------------------
// Settings equality: 008D4AF0 and its content-vector helper 008D48E0.
// Neither has a caller in the image (rel32 and absolute-dword census, see the
// doc); they are recovered for the field set they define, not for a call path.
// ---------------------------------------------------------------------------

// 008D48E0: __thiscall(ECX = vector, [ESP+4] = other vector), RET 4, AL.
// Stride-8 NativeString entries (+0 length, +4 text). Counts must match; a
// zero-length entry equals only a zero-length entry; otherwise __stricmp
// (00BF7FBF) decides, with no length comparison first.
bool native_string_vector_equal_008d48e0(
    const std::vector<std::string>& self, const std::vector<std::string>& other);

// The pointer-bearing members are passed as projections; every other compared
// field is read from the native 0BCh byte image.
struct GameSettingsComparand {
    const NativeGameSettingsStorage& bytes;
    const std::vector<std::string>& downloaded_content_98; // +98h data / +9Ch count
    const std::string& clan_text_b4;                       // NativeString +B4h
};

enum class SettingsFieldWidth : std::uint8_t { Byte, Dword, Float };

struct SettingsComparedField {
    std::uint8_t offset;
    SettingsFieldWidth width;
};

// The scalar fields 008D4AF0 compares before the +98h vector call, in native
// order (008D4AF3..008D4D0D). Floats use FUCOMIP/LAHF/TEST AH,44h/JP, which is
// IEEE equality: NaN differs from itself and +0 equals -0.
inline constexpr SettingsComparedField kSettingsEqualityHead[] = {
    {0x14, SettingsFieldWidth::Dword}, {0x18, SettingsFieldWidth::Dword},
    {0x1E, SettingsFieldWidth::Byte},  {0x20, SettingsFieldWidth::Float},
    {0x24, SettingsFieldWidth::Byte},  {0x28, SettingsFieldWidth::Float},
    {0x2C, SettingsFieldWidth::Float}, {0x30, SettingsFieldWidth::Float},
    {0x34, SettingsFieldWidth::Float}, {0x38, SettingsFieldWidth::Byte},
    {0x3C, SettingsFieldWidth::Dword}, {0x4B, SettingsFieldWidth::Byte},
    {0x49, SettingsFieldWidth::Byte},  {0x4A, SettingsFieldWidth::Byte},
    {0x40, SettingsFieldWidth::Byte},  {0x4C, SettingsFieldWidth::Byte},
    {0x50, SettingsFieldWidth::Dword}, {0x41, SettingsFieldWidth::Byte},
    {0x42, SettingsFieldWidth::Byte},  {0x43, SettingsFieldWidth::Byte},
    {0x44, SettingsFieldWidth::Byte},  {0x54, SettingsFieldWidth::Dword},
    {0x90, SettingsFieldWidth::Dword}, {0x84, SettingsFieldWidth::Byte},
    {0x85, SettingsFieldWidth::Byte},  {0x5C, SettingsFieldWidth::Dword},
    {0x58, SettingsFieldWidth::Dword}, {0x60, SettingsFieldWidth::Byte},
    {0x64, SettingsFieldWidth::Float}, {0x68, SettingsFieldWidth::Dword},
    {0x6C, SettingsFieldWidth::Byte},  {0x70, SettingsFieldWidth::Dword},
    {0x74, SettingsFieldWidth::Byte},  {0x78, SettingsFieldWidth::Dword},
    {0x7C, SettingsFieldWidth::Byte},  {0x04, SettingsFieldWidth::Dword},
    {0x08, SettingsFieldWidth::Byte},  {0x09, SettingsFieldWidth::Byte},
    {0x10, SettingsFieldWidth::Byte},
};

// Between the vector call (008D4D1B) and the NativeString call (008D4DB1).
// +90h is compared a second time here; the native repeats it.
inline constexpr SettingsComparedField kSettingsEqualityMiddle[] = {
    {0x86, SettingsFieldWidth::Byte},  {0x94, SettingsFieldWidth::Byte},
    {0x88, SettingsFieldWidth::Dword}, {0x8C, SettingsFieldWidth::Byte},
    {0x8D, SettingsFieldWidth::Byte},  {0x90, SettingsFieldWidth::Dword},
    {0xB0, SettingsFieldWidth::Byte},  {0xB2, SettingsFieldWidth::Byte},
};

// After the NativeString call, ending at 008D4DD6.
inline constexpr SettingsComparedField kSettingsEqualityTail[] = {
    {0xB1, SettingsFieldWidth::Byte}, {0x1D, SettingsFieldWidth::Byte},
    {0x1C, SettingsFieldWidth::Byte},
};

// 008D4AF0: __thiscall(ECX = this, [ESP+4] = other), RET 4, EAX 1 or 0.
// The +B4h text uses 00435C40 (length equality, then __stricmp). Fields the
// body never reads, among them +0Ch hints, +80h marker alpha and +95h
// HardwareReported, do not affect the result.
bool game_settings_equal_008d4af0(
    const GameSettingsComparand& self, const GameSettingsComparand& other);

} // namespace bsp
