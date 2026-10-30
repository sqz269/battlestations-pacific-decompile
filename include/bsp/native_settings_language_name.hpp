#pragma once

namespace bsp {
// Original 008D4870, ECX actual BCh settings, EAX borrowed CString, RET.
// Read CURRENT index+4, DWORD-wrapped SHL5, CURRENT catalog pointer CELL,
// reached raw32 row+4, then NULL-only fallback to the actual process role.
// Caller proves the reached raw row/data are addressable and stable. Neither
// catalog count nor header length is read; no bounds/empty/English repair.
// Keep settings/catalog/string backing and fallback live through consuming
// reads, before mutation or world/pool/CRT cleanup. This owns no string/input.
// Ordinary Source interface, not Original register/ABI/hardware-fault proof.
const char* native_settings_language_name_008d4870(
    const void* actual_settings_bch,
    void* const volatile& actual_catalog_data_00f88974,
    const char* actual_empty_00f88a3c) noexcept;
} // namespace bsp
