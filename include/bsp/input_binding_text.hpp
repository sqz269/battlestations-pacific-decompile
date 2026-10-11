#pragma once

#include <array>
#include <cstdint>
#include <functional>
#include <string>

#include "bsp/input_settings.hpp"

namespace bsp {

// 00CF7F48 (20 DirectInput key codes) and 00CF7F98 (the GetKeyNameTextA
// lParam used for each): the extended-key overrides 0069E300 consults before
// falling back to (code << 16) + 1.
struct KeyNameOverride {
    std::uint32_t key_code;
    std::int32_t lparam;
};
extern const std::array<KeyNameOverride, 20> kKeyNameOverrides;

// GetKeyNameTextA(lParam, buffer, size) as imported through 00CE2354.
using GetKeyNameTextFn = std::function<int(std::int32_t lparam, char* buffer, int size)>;

// Address: 0069E300 (body 0069E300..0069E4F9).
// Original ABI: __fastcall, ECX = out NativeString*, EDX = DirectInput key
// code, RET; returns ECX. Callers: 0069E339 (itself, for the Return key) and
// 0069ED16 in the binding display-string builder 0069E9E0.
//
// Returns "^." + the Windows key name, or "" when the name is empty. Key 0Eh
// (DIK_BACK) is special-cased: when the name of key 1Ch (DIK_RETURN) equals
// "^.INVIO" case-insensitively (the Italian layout), it returns "^.INDIETRO".
// Native passes an uninitialised 100h-byte stack buffer; the host zero-fills
// it so a failing GetKeyNameTextA yields "" deterministically.
// Evidence: docs/CC13_SETTINGS_SAVEDATA.md.
std::string keyboard_key_display_name_0069e300(std::uint32_t key_code,
                                               const GetKeyNameTextFn& get_key_name_text);

// The device side of 0069E9E0: 004BA6D0 on the input backend [00F8BBF4]
// with class 2 and the binding's device index, then that device's virtual
// +0Ch (name) and +3Ch (label of one key code). `device_present` stands for
// the non-null test; native calls 004BA6D0 twice and uses the second result.
struct BindingTextHooks {
    std::function<bool(std::int32_t device_index)> device_present;
    std::function<std::string(std::int32_t device_index)> device_name;
    std::function<std::string(std::int32_t device_index, std::int32_t key)> device_key_label;
    GetKeyNameTextFn get_key_name_text;
};

// Address: 0069E9E0 (body 0069E9E0..0069EE1A).
// Original ABI: __thiscall, ECX = input settings (0x540 object), [ESP+4] =
// out NativeString* (appended to), [ESP+8] = KeyboardInputBinding*, RET 8.
// Callers: 006A3492 and 006A9915.
//
// Appends ".<index+1>#|" when device_index != 0 (unsigned test). For a class-2
// binding whose device exists: the ControllerInputNames label for
// (device name, key) when present, else the device's own key label; then
// returns. Otherwise, for class 0 the keyboard name from 0069E300, and then,
// for any class, the name of every InputNames record whose value+0 equals
// device_type and value+0Ch equals key, in map order. That match identifies
// InputNameRecord::primary/secondary as (device type, key code).
void append_binding_display_text_0069e9e0(const InputSettings& settings,
                                          const KeyboardInputBinding& binding,
                                          std::string& out, const BindingTextHooks& hooks);

// Address: 006A9850 (body 006A9850..006A9A3F).
// Original ABI: __thiscall, ECX = input settings, [ESP+4] = out NativeString*
// (also returned in EAX), [ESP+8] = device name NativeString*, [ESP+0Ch] =
// input name NativeString*, [ESP+10h] = slot, RET 10h. Callers 0055C5A0,
// 0055D380, 00560430 (control binding screens).
//
// "(DEVINPUTS)" when settings+50h is set; "FE_pc.cs_na" for an unbound slot
// (device_type == -1); otherwise the 0069E9E0 text, followed for axes
// (class 1 with key >= 8, class 2 with key >= 3Ch) by "|.+"/"|.-" or, for
// class 1 key 0Ah, "|. |^globals.down"/"|. |^globals.up", chosen by the
// runtime reverse bit of the slot; an empty result becomes "FE_pc.cs_unknown".
// Native uses map operator[] for the device, the input and the reverse vector
// and invokes the checked-container failure on a short vector; the host
// throws std::out_of_range for any missing entry instead of inserting.
std::string binding_slot_text_006a9850(const InputSettings& settings, const std::string& device,
                                       const std::string& input, std::int32_t slot,
                                       const BindingTextHooks& hooks);

} // namespace bsp
