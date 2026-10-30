#!/usr/bin/env python3
"""Run a byte-identical x86 retail copy inside a restricted, owned job.

This is a launch-time safety guard, not a binary replacement. A restored entry
breakpoint and SHGetSpecialFolderPathA's in-memory IAT slot establish isolation.
A private request marker optionally arms a restored one-shot motion breakpoint.
Default low mode uses the kernel's mandatory-integrity write barrier. Optional
medium-restricted mode uses full restricting SIDs and a fresh, exhaustive source
DACL denial audit, including parent DELETE_CHILD and ACL/owner rights. Both
barriers apply before entry (including loader/TLS execution).
The optional medium-write-locked mode retains normal reads and audits all
mutation rights except directory DELETE_CHILD. Independent parent read/list
opens without delete sharing cover that gap for every protected object and
ancestor; protected inherited copies back up their lifetime for this unchanged
trusted retail. That mode rejects every additional executable process.

prepare must run on an EMPTY private local/ directory before ordinary copying.
launch never detaches: it services debug events until exit or the bounded
deadline, then terminates its own job. Matching bootstrap children receive the
same guard; an unexpected child fails closed. No external process is killed.
"""
from __future__ import annotations

import argparse
import ctypes as C
from ctypes import wintypes as W
import hashlib
import json
import math
import os
from pathlib import Path
import struct
import stat
import subprocess
import sys
import time

if os.name != "nt" or struct.calcsize("P") != 8:
    raise SystemExit("Requires 64-bit Python on Windows (target is WOW64 x86).")

K = C.WinDLL("kernel32", use_last_error=True)
A = C.WinDLL("advapi32", use_last_error=True)
S = C.WinDLL("shell32", use_last_error=True)
N = C.WinDLL("ntdll", use_last_error=True)
PVOID, SIZE = C.c_void_p, C.c_size_t
DWORD, HANDLE = W.DWORD, W.HANDLE
LOW, MEDIUM = 0x1000, 0x2000
TOKEN_QUERY, TOKEN_ALL_ACCESS = 8, 0xF01FF
RETAIL = Path("I:/SteamLibrary/steamapps/common/Battlestations Pacific/battlestationspacific.exe")
KNOWN_XLIVE_SHA256 = "71b50b3e91b5e17603f1d8fd44f1fc1da53c305f191dd126fb5059e5d3c841b2"


class SID_AND_ATTRIBUTES(C.Structure):
    _fields_ = [("Sid", PVOID), ("Attributes", DWORD)]


class GENERIC_MAPPING(C.Structure):
    _fields_ = [("read", DWORD), ("write", DWORD), ("execute", DWORD), ("all", DWORD)]


class STARTUPINFO(C.Structure):
    _fields_ = [("cb", DWORD), ("lpReserved", W.LPWSTR), ("lpDesktop", W.LPWSTR),
                ("lpTitle", W.LPWSTR), ("dwX", DWORD), ("dwY", DWORD),
                ("dwXSize", DWORD), ("dwYSize", DWORD), ("dwXCountChars", DWORD),
                ("dwYCountChars", DWORD), ("dwFillAttribute", DWORD),
                ("dwFlags", DWORD), ("wShowWindow", W.WORD),
                ("cbReserved2", W.WORD), ("lpReserved2", PVOID),
                ("hStdInput", HANDLE), ("hStdOutput", HANDLE), ("hStdError", HANDLE)]


class STARTUPINFOEX(C.Structure):
    _fields_ = [("startup", STARTUPINFO), ("attributes", PVOID)]


class FILE_INFORMATION(C.Structure):
    _fields_ = [("attributes", DWORD), ("created", W.FILETIME), ("accessed", W.FILETIME),
        ("written", W.FILETIME), ("volume", DWORD), ("size_high", DWORD), ("size_low", DWORD),
        ("links", DWORD), ("index_high", DWORD), ("index_low", DWORD)]


class JOB_ACCOUNTING(C.Structure):
    _fields_ = [("user", C.c_longlong), ("kernel", C.c_longlong), ("period_user", C.c_longlong),
        ("period_kernel", C.c_longlong), ("faults", DWORD), ("total", DWORD),
        ("active", DWORD), ("terminated", DWORD)]


class PROCESS_INFORMATION(C.Structure):
    _fields_ = [("hProcess", HANDLE), ("hThread", HANDLE),
                ("dwProcessId", DWORD), ("dwThreadId", DWORD)]


class EXCEPTION_RECORD(C.Structure):
    _fields_ = [("code", DWORD), ("flags", DWORD), ("record", PVOID),
                ("address", PVOID), ("count", DWORD), ("parameters", SIZE * 15)]


class EXCEPTION_INFO(C.Structure):
    _fields_ = [("record", EXCEPTION_RECORD), ("first_chance", DWORD)]


class CREATE_PROCESS_INFO(C.Structure):
    _fields_ = [("hFile", HANDLE), ("hProcess", HANDLE), ("hThread", HANDLE),
                ("base", PVOID), ("debug_offset", DWORD), ("debug_size", DWORD),
                ("tls", PVOID), ("start", PVOID), ("name", PVOID), ("unicode", W.WORD)]


class LOAD_DLL_INFO(C.Structure):
    _fields_ = [("hFile", HANDLE), ("base", PVOID), ("debug_offset", DWORD),
                ("debug_size", DWORD), ("name", PVOID), ("unicode", W.WORD)]


class DEBUG_UNION(C.Union):
    _fields_ = [("exception", EXCEPTION_INFO), ("create", CREATE_PROCESS_INFO),
                ("dll", LOAD_DLL_INFO), ("unload_base", PVOID),
                ("exit_code", DWORD), ("padding", C.c_byte * 160)]


class DEBUG_EVENT(C.Structure):
    _fields_ = [("code", DWORD), ("pid", DWORD), ("tid", DWORD), ("u", DEBUG_UNION)]


class FLOAT_SAVE(C.Structure):
    _fields_ = [("control", DWORD), ("status", DWORD), ("tag", DWORD),
                ("error_offset", DWORD), ("error_selector", DWORD),
                ("data_offset", DWORD), ("data_selector", DWORD),
                ("registers", C.c_byte * 80), ("cr0", DWORD)]


class WOW64_CONTEXT(C.Structure):
    _fields_ = [("flags", DWORD)] + [(n, DWORD) for n in
        ("Dr0", "Dr1", "Dr2", "Dr3", "Dr6", "Dr7")] + [("float", FLOAT_SAVE)] + [
        (n, DWORD) for n in ("SegGs", "SegFs", "SegEs", "SegDs", "Edi", "Esi",
        "Ebx", "Edx", "Ecx", "Eax", "Ebp", "Eip", "SegCs", "EFlags", "Esp", "SegSs")] + [
        ("extended", C.c_byte * 512)]


class JOB_BASIC(C.Structure):
    _fields_ = [("process_time", C.c_longlong), ("job_time", C.c_longlong),
                ("flags", DWORD), ("minimum", SIZE), ("maximum", SIZE),
                ("active_limit", DWORD), ("affinity", SIZE),
                ("priority", DWORD), ("scheduling", DWORD)]


class IO_COUNTERS(C.Structure):
    _fields_ = [(n, C.c_ulonglong) for n in ("read_ops", "write_ops", "other_ops",
                 "read_bytes", "write_bytes", "other_bytes")]


class JOB_EXTENDED(C.Structure):
    _fields_ = [("basic", JOB_BASIC), ("io", IO_COUNTERS), ("process_memory", SIZE),
                ("job_memory", SIZE), ("peak_process", SIZE), ("peak_job", SIZE)]


def api(dll, name, result, *args):
    f = getattr(dll, name)
    f.restype, f.argtypes = result, args
    return f


api(K, "GetCurrentProcess", HANDLE)
api(K, "CloseHandle", W.BOOL, HANDLE)
api(K, "LocalFree", PVOID, PVOID)
api(A, "GetLengthSid", DWORD, PVOID)
api(A, "GetSidSubAuthorityCount", C.POINTER(W.BYTE), PVOID)
api(A, "GetSidSubAuthority", C.POINTER(DWORD), PVOID, DWORD)
api(A, "OpenProcessToken", W.BOOL, HANDLE, DWORD, C.POINTER(HANDLE))
api(A, "GetTokenInformation", W.BOOL, HANDLE, C.c_int, PVOID, DWORD, C.POINTER(DWORD))
api(A, "SetTokenInformation", W.BOOL, HANDLE, C.c_int, PVOID, DWORD)
api(A, "CreateRestrictedToken", W.BOOL, HANDLE, DWORD, DWORD, PVOID, DWORD, PVOID,
    DWORD, PVOID, C.POINTER(HANDLE))
api(A, "ConvertStringSidToSidW", W.BOOL, W.LPCWSTR, C.POINTER(PVOID))
api(A, "ConvertSidToStringSidW", W.BOOL, PVOID, C.POINTER(PVOID))
api(A, "GetNamedSecurityInfoW", DWORD, W.LPWSTR, C.c_int, DWORD, PVOID, PVOID,
    PVOID, C.POINTER(PVOID), C.POINTER(PVOID))
api(A, "GetAce", W.BOOL, PVOID, DWORD, C.POINTER(PVOID))
api(A, "DuplicateTokenEx", W.BOOL, HANDLE, DWORD, PVOID, C.c_int, C.c_int, C.POINTER(HANDLE))
api(A, "GetSecurityDescriptorLength", DWORD, PVOID)
api(A, "AccessCheck", W.BOOL, PVOID, HANDLE, DWORD, C.POINTER(GENERIC_MAPPING),
    PVOID, C.POINTER(DWORD), C.POINTER(DWORD), C.POINTER(W.BOOL))
api(A, "InitializeAcl", W.BOOL, PVOID, DWORD, DWORD)
api(A, "AddAce", W.BOOL, PVOID, DWORD, DWORD, PVOID, DWORD)
api(A, "AddAccessAllowedAceEx", W.BOOL, PVOID, DWORD, DWORD, DWORD, PVOID)
api(A, "CreateProcessAsUserW", W.BOOL, HANDLE, W.LPCWSTR, W.LPWSTR, PVOID, PVOID,
    W.BOOL, DWORD, PVOID, W.LPCWSTR, C.POINTER(STARTUPINFO), C.POINTER(PROCESS_INFORMATION))
api(K, "CreateJobObjectW", HANDLE, PVOID, W.LPCWSTR)
api(K, "SetInformationJobObject", W.BOOL, HANDLE, C.c_int, PVOID, DWORD)
api(K, "QueryInformationJobObject", W.BOOL, HANDLE, C.c_int, PVOID, DWORD, C.POINTER(DWORD))
api(K, "AssignProcessToJobObject", W.BOOL, HANDLE, HANDLE)
api(K, "TerminateJobObject", W.BOOL, HANDLE, W.UINT)
api(K, "TerminateProcess", W.BOOL, HANDLE, W.UINT)
api(K, "ResumeThread", DWORD, HANDLE)
api(K, "WaitForSingleObject", DWORD, HANDLE, DWORD)
api(K, "GetExitCodeProcess", W.BOOL, HANDLE, C.POINTER(DWORD))
api(K, "DebugSetProcessKillOnExit", W.BOOL, W.BOOL)
api(K, "DebugBreakProcess", W.BOOL, HANDLE)
api(K, "WaitForDebugEvent", W.BOOL, C.POINTER(DEBUG_EVENT), DWORD)
api(K, "ContinueDebugEvent", W.BOOL, DWORD, DWORD, DWORD)
api(K, "OpenThread", HANDLE, DWORD, W.BOOL, DWORD)
api(K, "Wow64GetThreadContext", W.BOOL, HANDLE, C.POINTER(WOW64_CONTEXT))
api(K, "Wow64SetThreadContext", W.BOOL, HANDLE, C.POINTER(WOW64_CONTEXT))
api(K, "ReadProcessMemory", W.BOOL, HANDLE, PVOID, PVOID, SIZE, C.POINTER(SIZE))
api(K, "WriteProcessMemory", W.BOOL, HANDLE, PVOID, PVOID, SIZE, C.POINTER(SIZE))
api(K, "VirtualProtectEx", W.BOOL, HANDLE, PVOID, SIZE, DWORD, C.POINTER(DWORD))
api(K, "VirtualAllocEx", PVOID, HANDLE, PVOID, SIZE, DWORD, DWORD)
api(K, "FlushInstructionCache", W.BOOL, HANDLE, PVOID, SIZE)
api(K, "QueryFullProcessImageNameW", W.BOOL, HANDLE, DWORD, W.LPWSTR, C.POINTER(DWORD))
api(K, "GetFinalPathNameByHandleW", DWORD, HANDLE, W.LPWSTR, DWORD, DWORD)
api(K, "K32GetMappedFileNameW", DWORD, HANDLE, PVOID, W.LPWSTR, DWORD)
api(K, "DuplicateHandle", W.BOOL, HANDLE, HANDLE, HANDLE, C.POINTER(HANDLE), DWORD, W.BOOL, DWORD)
api(K, "IsWow64Process", W.BOOL, HANDLE, C.POINTER(W.BOOL))
api(K, "CreateFileW", HANDLE, W.LPCWSTR, DWORD, DWORD, PVOID, DWORD, DWORD, HANDLE)
api(K, "GetFileInformationByHandle", W.BOOL, HANDLE, C.POINTER(FILE_INFORMATION))
api(K, "SetHandleInformation", W.BOOL, HANDLE, DWORD, DWORD)
api(K, "GetHandleInformation", W.BOOL, HANDLE, C.POINTER(DWORD))
api(K, "GetProcessHandleCount", W.BOOL, HANDLE, C.POINTER(DWORD))
api(K, "InitializeProcThreadAttributeList", W.BOOL, PVOID, DWORD, DWORD, C.POINTER(SIZE))
api(K, "UpdateProcThreadAttribute", W.BOOL, PVOID, DWORD, SIZE, PVOID, SIZE, PVOID, C.POINTER(SIZE))
api(K, "DeleteProcThreadAttributeList", None, PVOID)
api(S, "SHGetSpecialFolderPathW", W.BOOL, W.HWND, W.LPWSTR, C.c_int, W.BOOL)
api(N, "NtQueryInformationProcess", W.LONG, HANDLE, DWORD, PVOID, DWORD, C.POINTER(DWORD))

assert C.sizeof(WOW64_CONTEXT) == 716 and C.sizeof(DEBUG_EVENT) == 176


def checked(ok, action):
    if not ok:
        raise OSError(C.get_last_error(), action, None, C.get_last_error())
    return ok


def close(h):
    if h:
        K.CloseHandle(h)


def token_data(token, kind):
    size = DWORD()
    A.GetTokenInformation(token, kind, None, 0, C.byref(size))
    if not size.value:
        checked(False, "GetTokenInformation size")
    buf = C.create_string_buffer(size.value)
    checked(A.GetTokenInformation(token, kind, buf, size, C.byref(size)), "GetTokenInformation")
    return buf


def sid_rid(sid):
    count = A.GetSidSubAuthorityCount(sid)[0]
    if not count:
        raise RuntimeError("Invalid integrity SID")
    return A.GetSidSubAuthority(sid, count - 1)[0]


def token_summary(token):
    label = token_data(token, 25)  # TokenIntegrityLevel
    sid = C.cast(label, C.POINTER(SID_AND_ATTRIBUTES)).contents.Sid
    scalar = lambda kind: int.from_bytes(token_data(token, kind).raw, "little")
    return {"integrity_rid": sid_rid(sid), "mandatory_policy": scalar(27),
            "token_type": scalar(8), "has_restrictions": bool(scalar(21)),
            "ui_access": bool(scalar(26)), "restricting_sids": token_sids(token, 11)}


def sid_string(sid):
    value = PVOID()
    checked(A.ConvertSidToStringSidW(sid, C.byref(value)), "ConvertSidToStringSidW")
    try:
        return C.wstring_at(value)
    finally:
        K.LocalFree(value)


def token_sids(token, kind, required_attributes=0):
    data = token_data(token, kind)
    count = DWORD.from_buffer(data).value
    if not count:
        return []
    alignment = C.alignment(SID_AND_ATTRIBUTES)
    offset = (C.sizeof(DWORD) + alignment - 1) & ~(alignment - 1)
    entries = (SID_AND_ATTRIBUTES * count).from_buffer(data, offset)
    return [sid_string(entry.Sid) for entry in entries
            if entry.Attributes & required_attributes == required_attributes]


def expected_restricting_sids(include_world=True):
    current = HANDLE()
    checked(A.OpenProcessToken(K.GetCurrentProcess(), TOKEN_QUERY, C.byref(current)), "OpenProcessToken logon SID")
    try:
        logons = token_sids(current, 2, 0xC0000000)
        if len(logons) != 1:
            raise RuntimeError(f"Expected exactly one current logon SID, found {logons}")
        return ["S-1-5-32-545", logons[0], *(["S-1-1-0"] if include_world else [])]
    finally:
        close(current)


def verify_low(token):
    info = token_summary(token)
    if (info["integrity_rid"] != LOW or not info["mandatory_policy"] & 1
            or info["token_type"] != 1 or info["ui_access"] or not info["has_restrictions"]):
        raise RuntimeError(f"Unsafe token: {info}")
    return info


def verify_token(token, mode="low"):
    if mode == "low":
        return verify_low(token)
    info = token_summary(token)
    if (mode not in ("medium-restricted", "medium-write-locked") or info["integrity_rid"] != MEDIUM
            or not info["mandatory_policy"] & 1 or info["token_type"] != 1
            or info["ui_access"] or not info["has_restrictions"]
            or sorted(info["restricting_sids"]) != sorted(expected_restricting_sids())):
        raise RuntimeError(f"Unsafe medium restricted token: {info}")
    info["default_dacl"] = default_dacl_summary(token)
    for sid in expected_restricting_sids(False):
        if not any(a.get("sid") == sid and a.get("mask") == 0x10000000
                   for a in info["default_dacl"]["aces"] if a["type"] == 0):
            raise RuntimeError(f"Token default DACL lacks required Users/LOGON grant: {info}")
    if any(a.get("sid") == "S-1-1-0" and a.get("mask", 0) & 0x500D0156
           for a in info["default_dacl"]["aces"] if a["type"] == 0):
        raise RuntimeError("Refuse a token default DACL granting World writes")
    return info


def default_dacl_summary(token):
    data = token_data(token, 6)
    acl = PVOID.from_buffer(data).value
    if not acl:
        raise RuntimeError("Null token default DACL refused")
    size, count = C.c_ushort.from_address(acl + 2).value, C.c_ushort.from_address(acl + 4).value
    if count > 128:
        raise RuntimeError("Token default DACL ACE limit exceeded")
    aces = []
    for i in range(count):
        ace = PVOID()
        checked(A.GetAce(acl, i, C.byref(ace)), "GetAce token default DACL")
        kind = C.c_ubyte.from_address(ace.value).value
        item = {"type": kind, "flags": C.c_ubyte.from_address(ace.value + 1).value}
        if kind in (0, 1):
            item.update(mask=DWORD.from_address(ace.value + 4).value, sid=sid_string(ace.value + 8))
        aces.append(item)
    return {"sha256": hashlib.sha256(C.string_at(acl, size)).hexdigest(), "aces": aces}


def configure_token_default_dacl(token):
    """Only the new token's future objects; never a shared/existing ACL."""
    old = token_data(token, 6)
    acl = PVOID.from_buffer(old).value
    if not acl:
        raise RuntimeError("Null token default DACL refused")
    revision = C.c_ubyte.from_address(acl).value
    old_size, count = C.c_ushort.from_address(acl + 2).value, C.c_ushort.from_address(acl + 4).value
    allocated = []
    try:
        for value in expected_restricting_sids(False):  # Never World GENERIC_ALL.
            sid = PVOID()
            checked(A.ConvertStringSidToSidW(value, C.byref(sid)), "Convert default DACL SID")
            allocated.append(sid)
        size = old_size + sum(8 + A.GetLengthSid(sid) for sid in allocated)
        if size > 65535:
            raise RuntimeError("Token default DACL size limit exceeded")
        new = C.create_string_buffer(size)
        checked(A.InitializeAcl(new, size, revision), "InitializeAcl token default DACL")
        for i in range(count):
            ace = PVOID()
            checked(A.GetAce(acl, i, C.byref(ace)), "Read original token default ACE")
            length = C.c_ushort.from_address(ace.value + 2).value
            checked(A.AddAce(new, revision, 0xFFFFFFFF, ace, length), "Preserve token default ACE")
        for sid in allocated:
            checked(A.AddAccessAllowedAceEx(new, revision, 0, 0x10000000, sid), "Grant future token objects Users/LOGON")
        pointer = PVOID(C.addressof(new))
        checked(A.SetTokenInformation(token, 6, C.byref(pointer), C.sizeof(pointer)), "Set token-only default DACL")
    finally:
        for pointer in allocated:
            K.LocalFree(pointer)


def restricted_token(mode="low"):
    current, result, sid = HANDLE(), HANDLE(), PVOID()
    allocated = []
    checked(A.OpenProcessToken(K.GetCurrentProcess(), TOKEN_ALL_ACCESS, C.byref(current)), "OpenProcessToken")
    try:
        restrict = None
        if mode in ("medium-restricted", "medium-write-locked"):
            entries = []
            for value in expected_restricting_sids():
                pointer = PVOID()
                checked(A.ConvertStringSidToSidW(value, C.byref(pointer)), "Convert restricting SID")
                allocated.append(pointer)
                entries.append(SID_AND_ATTRIBUTES(pointer, 0))
            restrict = (SID_AND_ATTRIBUTES * len(entries))(*entries)
        elif mode != "low":
            raise RuntimeError(f"Unknown token mode: {mode}")
        # Strip privileges; no foreign token, credential, or elevation fallback.
        # WRITE_RESTRICTED is allowed only by the source-lock launch contract.
        # Its DELETE_CHILD gap must be covered by every object's no-delete open.
        checked(A.CreateRestrictedToken(current, 9 if mode == "medium-write-locked" else 1,
                                        0, None, 0, None, len(restrict) if restrict is not None else 0, restrict,
                                        C.byref(result)), "CreateRestrictedToken")
        checked(A.ConvertStringSidToSidW("S-1-16-8192" if restrict is not None else "S-1-16-4096", C.byref(sid)), "ConvertStringSidToSidW")
        label = SID_AND_ATTRIBUTES(sid, 0x20)
        checked(A.SetTokenInformation(result, 25, C.byref(label),
                                     C.sizeof(label) + A.GetLengthSid(sid)), "SetTokenInformation integrity level")
        # Changing TokenMandatoryPolicy requires SeTcbPrivilege on this host.
        # Inherit it from the caller and require NO_WRITE_UP; never elevate.
        if mode in ("medium-restricted", "medium-write-locked"):
            configure_token_default_dacl(result)
        verify_token(result, mode)
        return result
    except BaseException:
        close(result)
        raise
    finally:
        close(current)
        if sid:
            K.LocalFree(sid)
        for pointer in allocated:
            K.LocalFree(pointer)


WRITE_RIGHTS = {"write_data_or_add_file": 0x2, "append_or_add_subdirectory": 0x4,
    "write_ea": 0x10, "delete_child": 0x40, "write_attributes": 0x100,
    "delete": 0x10000, "write_dacl": 0x40000, "write_owner": 0x80000}


def access_descriptor(path):
    descriptor = PVOID()
    # AccessCheck requires OWNER and GROUP as well as the DACL. Include LABEL
    # for receipt/cache identity; source medium labels are verified separately.
    err = A.GetNamedSecurityInfoW(str(path), 1, 0x17, None, None, None, None, C.byref(descriptor))
    if err:
        raise OSError(err, f"Read-only security descriptor query: {path}")
    return descriptor


def access_write_rights(path, impersonation, cache=None):
    descriptor = access_descriptor(path)
    try:
        raw = C.string_at(descriptor, A.GetSecurityDescriptorLength(descriptor))
        key = hashlib.sha256(raw).hexdigest()
        directory = Path(path).is_dir()
        cache_key = (key, directory)
        if cache is not None and cache_key in cache:
            return key, cache[cache_key]
        mapping = GENERIC_MAPPING(0x120089, 0x120116, 0x1200A0, 0x1F01FF)
        allowed = []
        for name, right in WRITE_RIGHTS.items():
            if name == "delete_child" and not directory:
                continue  # Directory-only right, not a meaningful file access.
            privileges = C.create_string_buffer(1024)
            size, granted, status = DWORD(len(privileges)), DWORD(), W.BOOL()
            checked(A.AccessCheck(descriptor, impersonation, right, C.byref(mapping), privileges,
                C.byref(size), C.byref(granted), C.byref(status)), "AccessCheck source write right")
            if status.value or granted.value & right:
                allowed.append(name)
        if cache is not None:
            cache[cache_key] = allowed
        return key, allowed
    finally:
        K.LocalFree(descriptor)


def protected_paths(roots):
    seen = set()
    for root in roots:
        root = ordinary(root, False)
        candidates = list(reversed(root.parents))
        if root.exists():
            candidates.append(root)
        for path in candidates:
            if path not in seen:
                seen.add(path)
                # ordinary(root) already checked this complete ancestor chain.
                yield path
        if root.is_dir():
            for base, dirs, files in os.walk(root, followlinks=False, onerror=walk_error):
                dirs.sort()
                for name in sorted([*dirs, *files]):
                    path = Path(base) / name
                    if path.lstat().st_file_attributes & 0x400:
                        raise RuntimeError(f"Protected reparse point refused: {path}")
                    if path not in seen:
                        seen.add(path)
                        yield path


def path_digest_update(digest, path):
    digest.update((json.dumps(str(path).casefold(), ensure_ascii=True) + "\n").encode("utf-8"))


def audit_source_writes(token, roots, receipt=None, token_mode="medium-restricted"):
    """No original file opens for writing; evaluate the actual token's rights."""
    impersonation = HANDLE()
    checked(A.DuplicateTokenEx(token, TOKEN_QUERY, None, 2, 2, C.byref(impersonation)), "DuplicateTokenEx audit impersonation")
    cache, violations, paths_digest = {}, [], hashlib.sha256()
    result = {"method": "AccessCheck actual restricted token duplicated at SecurityImpersonation",
        "token": verify_token(token, token_mode),
        "rights_checked_individually": WRITE_RIGHTS, "roots": [str(r) for r in roots],
        "objects_checked": 0, "unique_descriptors": 0, "violations": violations,
        "complete": False, "passed": False, "delete_child_requires_no_delete_handles": token_mode == "medium-write-locked",
        "delete_child_allowed_directories": 0}
    def inspect(path):
        # protected_paths checked ancestors once and each descendant itself.
        path_digest_update(paths_digest, path)
        descriptor_hash, allowed = access_write_rights(path, impersonation, cache)
        result["objects_checked"] += 1
        if token_mode == "medium-write-locked" and "delete_child" in allowed:
            result["delete_child_allowed_directories"] += 1
            allowed = [a for a in allowed if a != "delete_child"]
        if allowed:
            if len(violations) < 32:
                violations.append({"path": str(path), "allowed": allowed, "descriptor_sha256": descriptor_hash})
            result["violation_count"] = result.get("violation_count", 0) + 1
        if result["objects_checked"] > 200000:
            raise RuntimeError("Source effective-access audit limit exceeded")
    try:
        for path in protected_paths(roots):
            inspect(path)
        result.update(complete=True, passed=not violations, unique_descriptors=len(cache),
                      paths_sha256=paths_digest.hexdigest())
        return result
    finally:
        close(impersonation)
        result["unique_descriptors"] = len(cache)
        if receipt:
            write_receipt(receipt, result)


class SourceReadLocks:
    """Independent parent handles plus protected inherited lifetime backups."""
    def __init__(self, roots, audit, manifest, check_cancel=None):
        self.handles, self.attributes, self.handle_array = [], None, None
        self.attributes_initialized = False
        self.receipt = {"desired_access": "FILE_READ_DATA / FILE_LIST_DIRECTORY (0x1)",
            "share_mode": "FILE_SHARE_READ | FILE_SHARE_WRITE (3); no FILE_SHARE_DELETE",
            "parent_keeps_independent_handles_until_job_exit": True,
            "child_can_explicitly_clear_own_protect_flag": True, "scope": "unchanged trusted retail",
            "manifest": str(private(manifest, exists=False)), "complete": False}
        digest, paths_digest = hashlib.sha256(), hashlib.sha256()
        try:
            with open(manifest, "x", encoding="utf-8", newline="\n") as output:
                for path in protected_paths(roots):
                    if check_cancel and len(self.handles) % 256 == 0:
                        check_cancel()
                    before = path.stat()
                    handle = K.CreateFileW("\\\\?\\" + str(path), 1, 3, None, 3, 0x02200000, None)
                    if handle == PVOID(-1).value:
                        checked(False, f"Source no-delete read/list open {path}")
                    self.handles.append(handle)
                    if not handle or handle > 0xFFFFFFFF or handle == 0x400000:
                        raise RuntimeError(f"Unsafe source handle value: {handle}")
                    info = FILE_INFORMATION()
                    checked(K.GetFileInformationByHandle(handle, C.byref(info)), "Source handle identity")
                    canonical = C.create_unicode_buffer(32768)
                    length = K.GetFinalPathNameByHandleW(handle, canonical, len(canonical), 0)
                    if not length or length >= len(canonical):
                        raise RuntimeError(f"Cannot resolve source handle canonical path: {path}")
                    resolved = Path(canonical.value[4:] if canonical.value.startswith("\\\\?\\") else canonical.value)
                    after = path.stat()
                    file_index = (info.index_high << 32) | info.index_low
                    if (resolved != path or info.attributes & 0x400 or before.st_ino != file_index
                            or after.st_ino != file_index or before.st_dev != after.st_dev
                            or bool(info.attributes & 0x10) != stat.S_ISDIR(before.st_mode)):
                        raise RuntimeError(f"Source path/reparse/identity mismatch: {path}")
                    checked(K.SetHandleInformation(handle, 3, 3), "Source inherit/protect flags")
                    flags = DWORD()
                    checked(K.GetHandleInformation(handle, C.byref(flags)), "Verify source handle flags")
                    if flags.value != 3:
                        raise RuntimeError("Source handle did not retain inherit/protect flags")
                    record = {"path": str(path), "handle": handle, "flags": flags.value,
                        "volume": info.volume, "file_index": file_index, "directory": bool(info.attributes & 0x10)}
                    line = json.dumps(record, ensure_ascii=True, sort_keys=True) + "\n"
                    output.write(line)
                    digest.update(line.encode("utf-8"))
                    path_digest_update(paths_digest, path)
                    if len(self.handles) > 200000:
                        raise RuntimeError("Source handle limit exceeded")
            if len(self.handles) != audit["objects_checked"] or paths_digest.hexdigest() != audit["paths_sha256"]:
                raise RuntimeError("Audited source paths and acquired source-lock paths differ")
            self.receipt.update(complete=True, handles=len(self.handles), manifest_sha256=digest.hexdigest(),
                paths_sha256=paths_digest.hexdigest(), inherited_flags=3, main_module_handle_collision=False)
        except BaseException:
            self.close()
            raise

    def startup(self):
        size = SIZE()
        K.InitializeProcThreadAttributeList(None, 1, 0, C.byref(size))
        if not size.value:
            checked(False, "Inherited attribute list size")
        self.attributes = C.create_string_buffer(size.value)
        checked(K.InitializeProcThreadAttributeList(self.attributes, 1, 0, C.byref(size)), "Inherited attribute list init")
        self.attributes_initialized = True
        self.handle_array = (HANDLE * len(self.handles))(*self.handles)
        checked(K.UpdateProcThreadAttribute(self.attributes, 0, 0x20002, self.handle_array,
            C.sizeof(self.handle_array), None, None), "Explicit source handle list")
        startup = STARTUPINFOEX()
        startup.startup.cb = C.sizeof(startup)
        startup.startup.lpDesktop = "winsta0\\default"
        startup.attributes = C.addressof(self.attributes)
        self.receipt["explicit_handle_list_bytes"] = C.sizeof(self.handle_array)
        return startup

    def close(self):
        if self.attributes_initialized:
            K.DeleteProcThreadAttributeList(self.attributes)
            self.attributes = None
            self.attributes_initialized = False
        for handle in self.handles:
            checked(K.SetHandleInformation(handle, 3, 0), "Clear own source handle flags after job exit")
            close(handle)
        self.handles = []
        self.receipt["parent_handles_closed_unix"] = time.time()


def label(path):
    sacl, descriptor = PVOID(), PVOID()
    err = A.GetNamedSecurityInfoW(str(path), 1, 0x10, None, None, None,
                                  C.byref(sacl), C.byref(descriptor))
    if err:
        raise OSError(err, f"GetNamedSecurityInfo LABEL {path}")
    try:
        if not sacl:
            return {"rid": MEDIUM, "policy": 1, "explicit": False}
        count = C.c_ushort.from_address(sacl.value + 4).value
        found = []
        for i in range(count):
            ace = PVOID()
            checked(A.GetAce(sacl, i, C.byref(ace)), "GetAce")
            if C.c_ubyte.from_address(ace.value).value == 0x11:
                flags = C.c_ubyte.from_address(ace.value + 1).value
                if not flags & 8:  # INHERIT_ONLY_ACE does not label this object.
                    found.append({"rid": sid_rid(ace.value + 8),
                                  "policy": DWORD.from_address(ace.value + 4).value,
                                  "explicit": True})
        if len(found) > 1:
            raise RuntimeError(f"Ambiguous mandatory label: {path}")
        return found[0] if found else {"rid": MEDIUM, "policy": 1, "explicit": False}
    finally:
        if descriptor:
            K.LocalFree(descriptor)


def sha(path):
    result = hashlib.sha256()
    with open(path, "rb") as f:
        for block in iter(lambda: f.read(1024 * 1024), b""):
            result.update(block)
    return result.hexdigest()


def ordinary(path, exists=True):
    p = Path(os.path.abspath(path))
    for candidate in [p, *p.parents]:
        if not candidate.exists():
            if exists and candidate == p:
                raise RuntimeError(f"Missing path: {p}")
            continue
        if candidate.lstat().st_file_attributes & 0x400:
            raise RuntimeError(f"Reparse point refused: {candidate}")
    return p


def local_roots():
    repo = Path(__file__).absolute().parent.parent
    # The integrator can call a worker's reviewed script while a large copy is
    # staged in another registered worktree. Git registration, rather than an
    # arbitrary --allow-root switch, defines this repository's private roots.
    inventory = subprocess.check_output(["git", "worktree", "list", "--porcelain"], cwd=repo, text=True)
    return [ordinary(Path(line[9:]) / "local", False)
            for line in inventory.splitlines() if line.startswith("worktree ")]


def private(path, stage=None, exists=True):
    p = ordinary(path, exists)
    roots = [stage] if stage else local_roots()
    if not any((p != r or stage is not None) and p.is_relative_to(r) for r in roots):
        raise RuntimeError(f"Path is not strictly inside private local root: {p}")
    return p


def write_receipt(path, data):
    path = private(path, exists=False)
    if path.exists() and path.stat().st_nlink != 1:
        raise RuntimeError("Receipt hardlink refused")
    path.parent.mkdir(parents=True, exist_ok=True)
    temporary = path.with_name(path.name + f".{os.getpid()}.tmp")
    with open(temporary, "x", encoding="utf-8") as f:
        json.dump(data, f, indent=2, sort_keys=True)
        f.write("\n")
    os.replace(temporary, path)


def current_user_sid():
    token, text = HANDLE(), PVOID()
    checked(A.OpenProcessToken(K.GetCurrentProcess(), TOKEN_QUERY, C.byref(token)), "OpenProcessToken")
    try:
        data = token_data(token, 1)
        sid = C.cast(data, C.POINTER(SID_AND_ATTRIBUTES)).contents.Sid
        checked(A.ConvertSidToStringSidW(sid, C.byref(text)), "ConvertSidToStringSidW")
        return C.wstring_at(text)
    finally:
        if text:
            K.LocalFree(text)
        close(token)


def prepare(stage, receipt):
    stage = private(stage, exists=False)
    stage.mkdir(parents=True, exist_ok=True)
    if any(stage.iterdir()):
        raise RuntimeError("prepare requires an empty directory; copy only AFTER prepare")
    # User owns the new root, but workspace inheritance grants Modify rather
    # than WRITE_OWNER. Grant only this private empty root, never /T or originals.
    operations = []
    for args in (["/grant:r", f"*{current_user_sid()}:(OI)(CI)F"],
                 ["/setintegritylevel", "(OI)(CI)L"]):
        done = subprocess.run(["icacls.exe", str(stage), *args], capture_output=True, text=True)
        operations.append({"arguments": args, "exit_code": done.returncode,
                           "stdout": done.stdout.strip(), "stderr": done.stderr.strip()})
        if done.returncode:
            raise RuntimeError(f"Private root ACL/IL setup failed: {operations[-1]}")
    info = label(stage)
    if info["rid"] != LOW or not info["policy"] & 1:
        raise RuntimeError(f"Private root is not low-integrity/no-write-up: {info}")
    data = {"schema": "bsp.original-baseline.guard.prepare.v1",
            "phase": "prepared", "stage": str(stage), "stage_root": str(stage), "label": info,
            "live_launch_allowed": False, "acl_scope": "empty private stage root only",
            "operations": operations, "prepared_unix": time.time()}
    write_receipt(receipt, data)
    return data


def audit_private_writes(token, stage):
    impersonation = HANDLE()
    checked(A.DuplicateTokenEx(token, TOKEN_QUERY, None, 2, 2, C.byref(impersonation)), "DuplicateTokenEx private audit")
    cache, missing, count = {}, [], 0
    try:
        for base, dirs, files in os.walk(stage, followlinks=False, onerror=walk_error):
            for path in [Path(base), *(Path(base) / n for n in files)]:
                ordinary(path)
                _, allowed = access_write_rights(path, impersonation, cache)
                wanted = set(WRITE_RIGHTS) - (set() if path.is_dir() else {"delete_child"})
                absent = sorted(wanted - set(allowed))
                if absent and len(missing) < 32:
                    missing.append({"path": str(path), "denied_required_private_rights": absent})
                count += 1
                if count > 200000:
                    raise RuntimeError("Private access audit limit exceeded")
        return {"objects_checked": count, "unique_descriptors": len(cache),
                "complete": True, "passed": not missing, "missing": missing}
    finally:
        close(impersonation)


def prepare_medium(stage, receipt, executable=None, original_exe=RETAIL, protected_roots=None, probe=False):
    stage = private(stage)
    executable = private(executable or stage / "game/battlestationspacific.exe", stage)
    receipt = private(receipt, exists=False)
    original_exe = ordinary(original_exe)
    protected_documents = folder(5) / "Battlestations-Pacific"
    protected_roots = protected_roots or [original_exe.parent, protected_documents]
    original_hash, copied_hash = sha(original_exe), sha(executable)
    if original_hash != copied_hash:
        raise RuntimeError("Medium preparation requires a byte-identical private executable")
    before = documents_snapshot(protected_documents)
    data = {"schema": "bsp.original-baseline.guard.medium.prepare.v1", "phase": "checking",
        "stage_root": str(stage), "executable": str(executable), "original_exe": str(original_exe),
        "sha256": copied_hash, "token_mode": "medium-restricted", "creation_flags": 1,
        "live_launch_allowed": False, "probe": probe, "operations": [], "started_unix": time.time()}
    token = None
    try:
        # Inspect every copy before inheritable ACL changes. No junctions or
        # multiply-linked files may transmit a private mutation to originals.
        current_label = label(executable)["rid"]
        if current_label not in (LOW, MEDIUM):
            raise RuntimeError("Unexpected private executable label")
        data["sandbox_scan_before"] = scan_tree(stage, True,
            medium_executable=executable if current_label == MEDIUM else None)
        token = restricted_token("medium-restricted")
        data["token"] = verify_token(token, "medium-restricted")
        data["source_write_audit"] = audit_source_writes(token, protected_roots)
        if not data["source_write_audit"]["complete"] or not data["source_write_audit"]["passed"]:
            raise RuntimeError("Original source effective-access audit permits mutation; no private permissions changed")
        # Only verified private ordinary copies. This inherited grant deliberately
        # permits the restricting Users SID to access the private clone.
        for target, args in ((stage, ["/grant:r", "*S-1-5-32-545:(OI)(CI)F"]),
                             (executable, ["/setintegritylevel", "M"])):
            done = subprocess.run(["icacls.exe", str(target), *args], capture_output=True, text=True)
            data["operations"].append({"path": str(target), "arguments": args,
                "exit_code": done.returncode, "stdout": done.stdout.strip(), "stderr": done.stderr.strip()})
            if done.returncode:
                raise RuntimeError(f"Private medium preparation failed: {data['operations'][-1]}")
        data["sandbox_scan_after"] = scan_tree(stage, True, medium_executable=executable)
        data["private_write_audit"] = audit_private_writes(token, stage)
        if not data["private_write_audit"]["passed"]:
            raise RuntimeError("Private copy did not inherit the required Users access")
        data["phase"] = "prepared_medium"
    except BaseException as exc:
        data.update(phase="failed", error=f"{type(exc).__name__}: {exc}")
        raise
    finally:
        close(token)
        data["original_documents_unchanged"] = documents_snapshot(protected_documents) == before
        data["original_executable_unchanged"] = sha(original_exe) == original_hash
        data["copied_executable_bytes_unchanged"] = sha(executable) == copied_hash
        data["ended_unix"] = time.time()
        if not all(data[k] for k in ("original_documents_unchanged", "original_executable_unchanged", "copied_executable_bytes_unchanged")):
            data.update(phase="failed", error="Medium preparation preservation check failed")
        write_receipt(receipt, data)
        if data["phase"] == "prepared_medium":
            write_receipt(stage / ".guard-medium.json", data)
    if data["phase"] != "prepared_medium":
        raise RuntimeError(data["error"])
    return data


def scan_tree(root, low, medium_executable=None):
    root = ordinary(root)
    count = 0
    for base, dirs, files in os.walk(root, followlinks=False, onerror=walk_error):
        for p in [Path(base), *(Path(base) / n for n in files)]:
            ordinary(p)
            info = label(p)
            expected = MEDIUM if medium_executable and p == medium_executable else LOW
            if (low and (info["rid"] != expected or not info["policy"] & 1)) or (
                    not low and (info["rid"] < MEDIUM or not info["policy"] & 1)):
                raise RuntimeError(f"Unsafe {'sandbox' if low else 'protected'} label {p}: {info}")
            if low and p.is_file() and p.stat().st_nlink != 1:
                raise RuntimeError(f"Sandbox hardlink refused: {p}")
            count += 1
            if count > 200000:
                raise RuntimeError("Integrity scan limit exceeded")
        for name in dirs:
            ordinary(Path(base) / name)
    return {"root": str(root), "objects_checked": count,
            "required_integrity": "low except private medium executable" if medium_executable else "low" if low else "medium-or-higher", "no_write_up": True}


def documents_snapshot(root):
    if not root.exists():
        return {"exists": False, "files": {}}
    ordinary(root)
    result = {}
    for base, dirs, files in os.walk(root, followlinks=False, onerror=walk_error):
        for name in [*dirs, *files]:
            p = ordinary(Path(base) / name)
            if p.is_file():
                result[str(p.relative_to(root))] = {"sha256": sha(p), "size": p.stat().st_size}
            if len(result) > 10000:
                raise RuntimeError("Protected Documents snapshot limit exceeded")
    return {"exists": True, "files": result}


def walk_error(error):
    raise error


def folder(csidl):
    path = C.create_unicode_buffer(260)
    checked(S.SHGetSpecialFolderPathW(None, path, csidl, False), "SHGetSpecialFolderPathW")
    return ordinary(path.value)


def pe_info(path):
    import pefile
    pe = pefile.PE(str(path), fast_load=False)
    try:
        if pe.FILE_HEADER.Machine != 0x14C or pe.OPTIONAL_HEADER.Magic != 0x10B:
            raise RuntimeError("Only native x86 PE32 targets are accepted")
        imports = [i.address - pe.OPTIONAL_HEADER.ImageBase
                   for d in pe.DIRECTORY_ENTRY_IMPORT if d.dll.lower() == b"shell32.dll"
                   for i in d.imports if i.name == b"SHGetSpecialFolderPathA"]
        if len(imports) != 1:
            raise RuntimeError("Expected exactly one SHGetSpecialFolderPathA import")
        entry = pe.OPTIONAL_HEADER.AddressOfEntryPoint
        motion_rva, controlled_rva = 0x425F20, 0xA188D8
        exports = {s.name: s.address for s in getattr(getattr(pe, "DIRECTORY_ENTRY_EXPORT", None), "symbols", [])}
        # The one safety fixture exports two test anchors; the retail layout is
        # separately fixed and verified before launch, never inferred from these.
        if b"_motion_probe_entry@4" in exports and b"motion_probe_controlled" in exports:
            motion_rva = exports[b"_motion_probe_entry@4"]
            controlled_rva = exports[b"motion_probe_controlled"]
        return {"image_base": pe.OPTIONAL_HEADER.ImageBase, "entry_rva": entry,
                "iat_rva": imports[0], "entry_byte": pe.get_data(entry, 1).hex(),
                "image_size": pe.OPTIONAL_HEADER.SizeOfImage,
                "imports_set_handle_information": any(i.name == b"SetHandleInformation"
                    for d in pe.DIRECTORY_ENTRY_IMPORT for i in d.imports),
                "motion_rva": motion_rva, "controlled_rva": controlled_rva,
                "directinput_entry_bytes": pe.get_data(0x698340, 2).hex()
                    if pe.OPTIONAL_HEADER.SizeOfImage > 0x698341 else None,
                "motion_entry_byte": pe.get_data(motion_rva, 1).hex()
                    if pe.OPTIONAL_HEADER.SizeOfImage > motion_rva else None}
    finally:
        pe.close()


def read_remote(process, address, size):
    buf, transferred = C.create_string_buffer(size), SIZE()
    checked(K.ReadProcessMemory(process, address, buf, size, C.byref(transferred)), "ReadProcessMemory")
    if transferred.value != size:
        raise RuntimeError("Short remote read")
    return buf.raw


def write_remote(process, address, content):
    buf, transferred = C.create_string_buffer(content), SIZE()
    checked(K.WriteProcessMemory(process, address, buf, len(content), C.byref(transferred)), "WriteProcessMemory")
    if transferred.value != len(content):
        raise RuntimeError("Short remote write")
    checked(K.FlushInstructionCache(process, address, len(content)), "FlushInstructionCache")


def replace_remote(process, address, content):
    old, ignored = DWORD(), DWORD()
    checked(K.VirtualProtectEx(process, address, len(content), 0x40, C.byref(old)), "VirtualProtectEx writable")
    try:
        write_remote(process, address, content)
    finally:
        checked(K.VirtualProtectEx(process, address, len(content), old, C.byref(ignored)), "VirtualProtectEx restore")


def hook_bytes(remote, original, document_bytes):
    # x86 stdcall(HWND, char*, int, BOOL). Preserve ESI/EDI and stack; EAX,
    # ECX/EDX are caller-saved. All CSIDL flag bits occupy the upper bytes.
    code = bytearray(b"\x8b\x44\x24\x0c\x25\xff\x00\x00\x00\x83\xf8\x05\x75\x00")
    code += b"\x8b\x54\x24\x08\x85\xd2\x74\x00\x56\x57\x89\xd7\xbe" + b"\0" * 4
    path_operand = len(code) - 4
    code += b"\xb9" + struct.pack("<I", len(document_bytes) + 1)
    code += b"\xfc\xf3\xa4\x5f\x5e\xb8\x01\x00\x00\x00\xc2\x10\x00"
    failure = len(code)
    code += b"\x33\xc0\xc2\x10\x00"
    fallback = len(code)
    code += b"\xb8" + struct.pack("<I", original) + b"\xff\xe0"
    code[13] = fallback - 14
    code[21] = failure - 22
    code[path_operand:path_operand + 4] = struct.pack("<I", remote + len(code))
    return bytes(code) + document_bytes + b"\0"


def entry_heap_state(process):
    """Read-only WOW64 PEB observation; never alter debugger/heap settings."""
    peb, returned = SIZE(), DWORD()
    status = N.NtQueryInformationProcess(process, 26, C.byref(peb), C.sizeof(peb), C.byref(returned))
    result = {"query_class": "ProcessWow64Information(26)", "ntstatus": f"{status & 0xFFFFFFFF:08x}",
              "peb32": peb.value, "nt_global_flag_offset": 0x68}
    if status < 0 or not 0 < peb.value <= 0xFFFFFFFF:
        result["diagnostic_error"] = "WOW64 PEB query did not return a valid 32-bit address"
        return result
    try:
        flags = struct.unpack("<I", read_remote(process, peb.value + 0x68, 4))[0]
        result.update(nt_global_flag=flags, nt_global_flag_hex=f"{flags:08x}",
                      heap_debug_bits=flags & 0x70, heap_debug_bits_hex=f"{flags & 0x70:02x}")
    except OSError as exc:
        result["diagnostic_error"] = str(exc)
    return result


def establish_hook(process, thread_id, state, document_bytes):
    original = struct.unpack("<I", read_remote(process, state["iat"], 4))[0]
    if not original or state["base"] <= original < state["base"] + state["size"]:
        raise RuntimeError(f"Import is not resolved outside the image: {original:08x}")
    # Readable instruction bytes establish a real resolved target before tailcall.
    original_head = read_remote(process, original, 8).hex()
    remote = K.VirtualAllocEx(process, None, 4096, 0x3000, 4)
    if not remote or remote > 0xFFFFFFFF:
        raise RuntimeError("No 32-bit remote allocation")
    content = hook_bytes(remote, original, document_bytes)
    write_remote(process, remote, content)
    old = DWORD()
    checked(K.VirtualProtectEx(process, remote, len(content), 0x20, C.byref(old)), "VirtualProtectEx stub RX")
    replace_remote(process, state["iat"], struct.pack("<I", remote))
    if read_remote(process, state["iat"], 4) != struct.pack("<I", remote):
        raise RuntimeError("IAT verification failed")
    replace_remote(process, state["entry"], bytes.fromhex(state["entry_byte"]))
    thread = K.OpenThread(0x18, False, thread_id)
    checked(thread, "OpenThread context")
    try:
        context = WOW64_CONTEXT()
        context.flags = 0x10001  # WOW64_CONTEXT_CONTROL
        checked(K.Wow64GetThreadContext(thread, C.byref(context)), "Wow64GetThreadContext")
        if context.Eip != state["entry"] + 1:
            raise RuntimeError(f"Unexpected entry EIP: {context.Eip:08x}")
        context.Eip = state["entry"]
        checked(K.Wow64SetThreadContext(thread, C.byref(context)), "Wow64SetThreadContext")
    finally:
        close(thread)
    state.update(hooked=True, stub=f"{remote:08x}", original_import=f"{original:08x}",
                 original_import_head=original_head, hook_bytes=content.hex(),
                 entry_restored=True, hook_time_unix=time.time())
    state["entry_heap"] = entry_heap_state(process)


def image_path(process):
    buf, count = C.create_unicode_buffer(32768), DWORD(32768)
    checked(K.QueryFullProcessImageNameW(process, 0, buf, C.byref(count)), "QueryFullProcessImageNameW")
    return ordinary(buf.value)


def duplicate(handle):
    out, current = HANDLE(), K.GetCurrentProcess()
    checked(K.DuplicateHandle(current, handle, current, C.byref(out), 0, False, 2), "DuplicateHandle")
    return out


def process_token_info(process, mode="low"):
    token = HANDLE()
    checked(A.OpenProcessToken(process, TOKEN_QUERY, C.byref(token)), "OpenProcessToken child")
    try:
        return verify_token(token, mode)
    finally:
        close(token)


def module_record(process, base, file_handle=None, known_path=None, known_size=None):
    """Bounded read-only evidence; missing optional loader metadata is retained."""
    result = {"base": base, "base_hex": f"{base:08x}", "loaded": True}
    try:
        if known_path:
            result["path"] = str(known_path)
        else:
            name = C.create_unicode_buffer(32768)
            length = K.GetFinalPathNameByHandleW(file_handle, name, len(name), 0) if file_handle else 0
            if length and length < len(name):
                result["path"] = name.value[4:] if name.value.startswith("\\\\?\\") else name.value
            else:
                length = K.K32GetMappedFileNameW(process, base, name, len(name))
                if length:
                    result["path"] = name.value
        if known_size:
            result["size"] = known_size
        else:
            dos = read_remote(process, base, 64)
            pe_offset = struct.unpack_from("<I", dos, 0x3C)[0]
            if dos[:2] != b"MZ" or not 64 <= pe_offset <= 0x100000:
                raise RuntimeError("Invalid module DOS header")
            header = read_remote(process, base + pe_offset, 0x60)
            if header[:4] != b"PE\0\0":
                raise RuntimeError("Invalid module PE signature")
            result["size"] = struct.unpack_from("<I", header, 0x50)[0]
        if result.get("path") and Path(result["path"]).name.lower() == "xlive.dll":
            result["sha256"] = sha(Path(result["path"]))
    except BaseException as exc:
        result["diagnostic_error"] = f"{type(exc).__name__}: {exc}"
    return result


def address_module(state, address):
    for module in reversed(state.get("modules", [])):
        if module.get("loaded") and module["base"] <= address < module["base"] + module.get("size", 0):
            return {"path": module.get("path"), "base": module["base"],
                    "rva": address - module["base"]}
    return None


def exception_diagnostic(process, thread_id, state, exception):
    record = exception.record
    result = {"unix": time.time(), "thread_id": thread_id,
        "code": f"{record.code:08x}", "first_chance": bool(exception.first_chance),
        "known_folder_hook_established": state["hooked"],
        "exception_address": record.address or 0,
        "exception_module": address_module(state, record.address or 0),
        "parameters": list(record.parameters[:min(record.count, 15)])}
    thread = K.OpenThread(0x8, False, thread_id)
    try:
        checked(thread, "OpenThread exception diagnostic")
        context = WOW64_CONTEXT()
        context.flags = 0x1000B
        checked(K.Wow64GetThreadContext(thread, C.byref(context)), "Wow64GetThreadContext exception")
        result["registers"] = {n: getattr(context, n) for n in
            ("Eip", "Esp", "Ebp", "Eax", "Ebx", "Ecx", "Edx", "Esi", "Edi", "EFlags")}
        result["eip_module"] = address_module(state, context.Eip)
        result["x87_control_word"] = context.float.control & 0xFFFF
        try:
            result["instruction_bytes"] = read_remote(process, context.Eip, 32).hex()
        except OSError as exc:
            result["instruction_error"] = str(exc)
        try:
            words = list(struct.unpack("<64I", read_remote(process, context.Esp, 256)))
            result["stack_address"], result["stack_words"] = context.Esp, words
            result["stack_code_pointers"] = [
                {"offset": i * 4, "value": v, "module": address_module(state, v)}
                for i, v in enumerate(words) if address_module(state, v)][:24]
        except OSError as exc:
            result["stack_error"] = str(exc)
        frames, frame = [], context.Ebp
        for _ in range(8):
            if not 0x10000 <= frame <= 0xFFFFFFF7:
                break
            try:
                next_frame, return_address = struct.unpack("<II", read_remote(process, frame, 8))
            except OSError:
                break
            frames.append({"ebp": frame, "next_ebp": next_frame, "return_address": return_address,
                           "module": address_module(state, return_address)})
            if not frame < next_frame <= frame + 0x100000:
                break
            frame = next_frame
        result["ebp_frames"] = frames
    except BaseException as exc:
        result["diagnostic_error"] = f"{type(exc).__name__}: {exc}"
    finally:
        close(thread)
    return result


def known_xlive_debug_close(process, stage, state, diagnostic, enabled):
    """Opt-in for ONE proven debugger-only CloseHandle(HMODULE) exception.

    Microsoft documents the debugger-specific exception for invalid handles:
    https://learn.microsoft.com/windows/win32/api/handleapi/nf-handleapi-closehandle
    Installed xlive bytes (not a whole-source-build identity claim) show
    GetModuleHandleA(NULL) -> VirtualProtect -> CloseHandle at RVA 17C26A.
    A real live return+argument pair and the exact loaded DLL hash are required.
    """
    evidence = {"enabled": enabled, "continued": False}
    if (not enabled or diagnostic["code"] != "c0000008" or not diagnostic["first_chance"]
            or state["hooked"] or state.get("known_xlive_debug_close_continued")):
        evidence["reason"] = "policy disabled, wrong exception phase, guarded entry already reached, or already handled once"
        return evidence
    modules = [m for m in state.get("modules", []) if m.get("loaded")
        and m.get("sha256") == KNOWN_XLIVE_SHA256
        and Path(m.get("path", "")).name.lower() == "xlive.dll"
        and Path(m.get("path", "")) == stage / "game/xlive.dll"]
    words = diagnostic.get("stack_words", [])
    for module in modules:
        return_address = module["base"] + 0x17C270
        pairs = [i for i in range(len(words) - 1)
                 if words[i] == return_address and words[i + 1] == state["base"]]
        if not pairs:
            continue
        expected_call = b"\xff\x15" + struct.pack("<I", module["base"] + 0x4DD0D8)
        try:
            actual_call = read_remote(process, module["base"] + 0x17C26A, 6)
        except OSError as exc:
            evidence["call_read_error"] = str(exc)
            continue
        if actual_call != expected_call:
            continue
        evidence.update(continued=True, reason="exact loaded xlive SHA/callsite/live return+main-HMODULE argument matched",
            module=module, return_address=return_address, invalid_handle=state["base"],
            stack_pair_offset=pairs[0] * 4, call_instruction=actual_call.hex())
        state["known_xlive_debug_close_continued"] = True
        return evidence
    evidence["reason"] = "exact loaded DLL, call instruction, and live CloseHandle return/argument pair did not all match"
    return evidence


def motion_request_options(body):
    """Legacy text remains valid; JSON provides bounded diagnostic controls."""
    stripped = body.strip().lstrip("\ufeff").strip()
    options = json.loads(stripped) if stripped.startswith("{") else {"hits": int(stripped)} if stripped.isdecimal() else {}
    if not isinstance(options, dict) or set(options) - {"hits", "interval_ms", "timeout_seconds"}:
        raise ValueError("Motion marker must be legacy text, an integer, or JSON with hits/interval_ms/timeout_seconds")
    result = {"hits": options.get("hits", 20), "interval_ms": options.get("interval_ms", 50),
              "timeout_seconds": options.get("timeout_seconds", 30)}
    for name, low, high in (("hits", 1, 100), ("interval_ms", 10, 1000), ("timeout_seconds", 1, 120)):
        if type(result[name]) is not int or not low <= result[name] <= high:
            raise ValueError(f"Motion {name} must be an integer in [{low}, {high}]")
    return result


def finish_motion_request(state, status):
    session = state.get("motion_session")
    if session and session["status"] == "active":
        session.update(status=status, ended_unix=time.time())
    state.update(motion_pending=False, motion_next_arm_monotonic=None)
    if state.get("motion_armed"):
        state["motion_restore_pending"] = True


def start_motion_request(state, request, options):
    previous = state.get("motion_session")
    if previous and previous["status"] == "active":
        raise ValueError("A motion request is active; wait for completion or expiry before rearming")
    if state.get("motion_armed") or state.get("motion_restore_pending"):
        raise ValueError("Previous motion breakpoint restoration is pending")
    sessions = state.setdefault("motion_requests", [])
    if len(sessions) >= 32 or len(state.get("motion_captures", [])) + options["hits"] > 1000:
        raise ValueError("Bounded motion receipt budget exhausted")
    session = {"sequence": len(sessions) + 1, "request": request, **options,
        "status": "active", "captured_hits": 0, "trap_events": 0, "arm_attempts": 0,
        "arm_deferrals": 0, "histogram": [], "unit_counts": {}, "unique_units": 0}
    sessions.append(session)
    state.pop("motion_request_rejected", None)
    state.update(motion_request=request, motion_session=session, motion_pending=True,
        motion_next_arm_monotonic=time.monotonic(),
        motion_request_deadline_monotonic=time.monotonic() + options["timeout_seconds"])


def arm_motion(process, state, thread_ids):
    """Called only at a debug stop; never single-step or rewrite running code."""
    address = state["base"] + state["motion_rva"]
    session = state["motion_session"]
    session["arm_attempts"] += 1
    blocked = []
    for thread_id in sorted(thread_ids):
        thread = K.OpenThread(0x8, False, thread_id)
        try:
            checked(thread, "OpenThread motion rearm boundary")
            context = WOW64_CONTEXT()
            context.flags = 0x10001  # CONTROL; all target threads are debug-stopped.
            checked(K.Wow64GetThreadContext(thread, C.byref(context)), "Motion rearm boundary context")
            if context.Eip in (address, address + 1):
                blocked.append({"thread_id": thread_id, "eip": context.Eip})
        except OSError as exc:
            blocked.append({"thread_id": thread_id, "context_error": str(exc)})
        finally:
            close(thread)
    if not thread_ids or blocked:
        session["arm_deferrals"] += 1
        session["last_arm_blockers"] = blocked[:8]
        state["motion_next_arm_monotonic"] = time.monotonic() + session["interval_ms"] / 1000
        return False
    if read_remote(process, address, 1).hex() != state["motion_entry_byte"]:
        raise RuntimeError("Motion entry byte differs from original image")
    replace_remote(process, address, b"\xcc")
    state.update(motion_armed=True, motion_pending=False, motion_address=address, motion_entry_restored=False)
    session["last_arm_scanned_threads"] = len(thread_ids)
    return True


def motion_capture(process, thread_id, state, capture=True):
    thread = K.OpenThread(0x18, False, thread_id)
    checked(thread, "OpenThread motion context")
    try:
        context = WOW64_CONTEXT()
        context.flags = 0x1000B  # CONTROL | INTEGER | FLOATING_POINT
        checked(K.Wow64GetThreadContext(thread, C.byref(context)), "Wow64GetThreadContext motion")
        if context.Eip != state["motion_address"] + 1:
            raise RuntimeError("Unexpected motion-breakpoint EIP")
        session = state.get("motion_session")
        if session:
            session["trap_events"] += 1
        capture = (capture and session and session["status"] == "active"
            and time.monotonic() < state["motion_request_deadline_monotonic"]
            and session["captured_hits"] < session["hits"])
        if capture:
            dt_bytes = read_remote(process, context.Esp + 4, 4)
            dt = struct.unpack("<f", dt_bytes)[0]
            controlled = struct.unpack("<I", read_remote(process, state["base"] + state["controlled_rva"], 4))[0]
            unit = (context.Ecx - 0x310) & 0xFFFFFFFF
            main_thread_id = state["main_thread_id"]
            cw = context.float.control & 0xFFFF
            state.setdefault("motion_captures", []).append({
                "pid": state["pid"], "thread_id": thread_id, "unix": time.time(),
                "event_thread_id": thread_id, "initial_thread_id": state["initial_thread_id"],
                "original_entry_thread_id": state["original_entry_thread_id"],
                "main_thread_id": main_thread_id, "is_main_thread": thread_id == main_thread_id,
                "request_sequence": session["sequence"], "hit_index": session["captured_hits"] + 1,
                "breakpoint_armed_at_dispatch": bool(state.get("motion_armed")),
                "request": state["motion_request"], "ecx": context.Ecx, "esp": context.Esp,
                "unit": unit, "unit_hex": f"{unit:08x}", "controlled": controlled,
                "controlled_hex": f"{controlled:08x}", "matches_controlled_pointer": bool(unit and unit == controlled),
                "engine_dt": dt if math.isfinite(dt) else None, "engine_dt_bytes": dt_bytes.hex(),
                "x87_control_word": cw, "x87_control_word_hex": f"{cw:04x}",
                "interpretation": "unit hypothesis ECX-0x310; pointer equality only, no ship/team classification"})
            session["captured_hits"] += 1
            key = f"{unit:08x}"
            session["unit_counts"][key] = session["unit_counts"].get(key, 0) + 1
            session["unique_units"] = len(session["unit_counts"])
            bucket = next((b for b in session["histogram"] if b["thread_id"] == thread_id
                           and b["x87_control_word"] == cw), None)
            if bucket is None:
                bucket = {"thread_id": thread_id, "main_thread_id": main_thread_id,
                    "is_main_thread": thread_id == main_thread_id, "x87_control_word": cw,
                    "x87_control_word_hex": f"{cw:04x}", "hits": 0}
                session["histogram"].append(bucket)
            bucket["hits"] += 1
        current_byte = read_remote(process, state["motion_address"], 1)
        if current_byte == b"\xcc":
            replace_remote(process, state["motion_address"], bytes.fromhex(state["motion_entry_byte"]))
        elif current_byte.hex() != state["motion_entry_byte"]:
            raise RuntimeError("Motion byte changed outside this guard")
        state.update(motion_armed=False, motion_entry_restored=True)
        context.Eip = state["motion_address"]
        checked(K.Wow64SetThreadContext(thread, C.byref(context)), "Wow64SetThreadContext motion")
        if session and session["status"] == "active":
            if session["captured_hits"] >= session["hits"]:
                finish_motion_request(state, "complete")
            elif time.monotonic() >= state["motion_request_deadline_monotonic"]:
                finish_motion_request(state, "expired")
            else:
                # Small deterministic staggering avoids locking sampling to one
                # fixed frame phase. Already queued real traps are also retained.
                delay_ms = session["interval_ms"] + (session["trap_events"] * 17 % 31)
                state.update(motion_pending=True, motion_next_arm_monotonic=time.monotonic() + delay_ms / 1000)
    finally:
        close(thread)


def arm_directinput_observation(process, state):
    address = state["base"] + 0x698340
    if read_remote(process, address, 2) != b"\x8b\x07":
        raise RuntimeError("DirectInput observation site is not original MOV EAX,[EDI]")
    replace_remote(process, address, b"\xcc")
    state.update(directinput_address=address, directinput_armed=True)


def directinput_observation(process, thread_id, state, capture=True):
    thread = K.OpenThread(0x18, False, thread_id)
    checked(thread, "OpenThread DirectInput observation")
    try:
        context = WOW64_CONTEXT()
        context.flags = 0x10003  # CONTROL | INTEGER
        checked(K.Wow64GetThreadContext(thread, C.byref(context)), "Wow64GetThreadContext DirectInput")
        if context.Eip != state["directinput_address"] + 1:
            raise RuntimeError("Unexpected DirectInput breakpoint EIP")
        if capture:
            result = {"unix": time.time(), "pid": state["pid"], "thread_id": thread_id,
                "address": state["directinput_address"], "eax_hresult": context.Eax,
                "eax_hresult_hex": f"{context.Eax:08x}", "eax_hresult_signed": C.c_int32(context.Eax).value,
                "hresult_failed": bool(context.Eax & 0x80000000), "output_slot_edi": context.Edi,
                "esp": context.Esp, "observed_before": "MOV EAX,[EDI] after DirectInput8Create thunk"}
            try:
                output = struct.unpack("<I", read_remote(process, context.Edi, 4))[0]
                result.update(output_pointer=output, output_pointer_hex=f"{output:08x}", interface_null=not output)
            except OSError as exc:
                result["output_read_error"] = str(exc)
            state["directinput_observation"] = result
            replace_remote(process, state["directinput_address"], b"\x8b")
            state.update(directinput_armed=False, directinput_entry_restored=True)
        context.Eip = state["directinput_address"]
        checked(K.Wow64SetThreadContext(thread, C.byref(context)), "Wow64SetThreadContext DirectInput")
    finally:
        close(thread)


def run_guard(stage, executable, cwd, documents, receipt, seconds, original_exe=None,
              arguments=(), protected_roots=(), probe=False, motion_discovery=True,
              handle_known_xlive_debug_close=False, observe_directinput_startup=False,
              token_mode="low"):
    drain = Path(os.environ["USERPROFILE"]) / ".bsp/DRAIN"
    def check_drain():
        if drain.exists() and os.environ.get("BSP_IGNORE_DRAIN") != "1":
            raise RuntimeError(f"DRAIN is set: {drain}; terminating only the owned job")
    check_drain()
    stage = private(stage)
    executable, cwd, documents = [private(p, stage) for p in (executable, cwd, documents)]
    if token_mode not in ("low", "medium-restricted", "medium-write-locked"):
        raise RuntimeError("Unsupported token mode")
    locked_mode = token_mode == "medium-write-locked"
    if token_mode != "low":
        marker = private(stage / ".guard-medium.json", stage)
        prepared = json.loads(marker.read_text(encoding="utf-8"))
        if (prepared.get("schema") != "bsp.original-baseline.guard.medium.prepare.v1"
                or prepared.get("phase") != "prepared_medium" or Path(prepared.get("stage_root", "")) != stage
                or Path(prepared.get("executable", "")) != executable or prepared.get("sha256") != sha(executable)):
            raise RuntimeError("Private medium preparation receipt does not match this executable/stage")
    receipt = private(receipt, exists=False)
    if receipt.is_relative_to(cwd) and receipt == executable:
        raise RuntimeError("Receipt collides with executable")
    if not 5 <= seconds <= 3600:
        raise RuntimeError("Bounded deadline must be between 5 and 3600 seconds")
    document_bytes = str(documents).encode("mbcs", "strict")
    if len(document_bytes) >= 260 or b"\0" in document_bytes:
        raise RuntimeError("Private Documents path is not a valid ANSI MAX_PATH")
    for p in (stage / "profile", stage / "profile/AppData/Roaming", stage / "profile/AppData/Local", stage / "Temp"):
        p.mkdir(parents=True, exist_ok=True)
    info, expected_hash = pe_info(executable), sha(executable)
    if observe_directinput_startup and (probe or info["directinput_entry_bytes"] != "8b07"):
        raise RuntimeError("DirectInput startup observation requires the verified original MOV EAX,[EDI] site")
    discovery_enabled = motion_discovery and bool(info["motion_entry_byte"])
    protected_documents = folder(5) / "Battlestations-Pacific"
    original_hash = sha(original_exe) if original_exe else expected_hash
    if original_hash != expected_hash:
        raise RuntimeError("Staged executable is not byte-identical to the original")
    if not probe and (info["image_base"] != 0x400000 or info["entry_rva"] != 0x7FD2BD
                      or info["iat_rva"] != 0x8E22F0):
        raise RuntimeError(f"Unexpected retail PE layout: {info}")
    data = {"schema": "bsp.original-baseline.guard.launch.v1", "phase": "checking", "guard_pid": os.getpid(), "stage": str(stage),
            "executable": str(executable), "cwd": str(cwd), "documents": str(documents),
            "arguments": list(arguments),
            "env_NO_DEBUG_HEAP": os.environ.get("_NO_DEBUG_HEAP"),
            "protected_documents": str(protected_documents), "seconds": seconds,
            "sha256": expected_hash, "original_exe": str(original_exe) if original_exe else None,
            "pe": info, "processes": [], "live_launch_allowed": False,
            "token_mode": token_mode, "creation_flags": 9 if locked_mode else 1,
            "isolation": "write-restricted token with exhaustive source rights audit plus independent parent/protected inherited no-delete read handles" if locked_mode else "full restricted Users+LOGON+World token, fresh source DACL denial audit, owned kill-on-close job" if token_mode == "medium-restricted" else "restricted primary token, low mandatory IL/no-write-up, owned kill-on-close job",
            "known_folder": "entry-time SHGetSpecialFolderPathA IAT hook; CSIDL_PERSONAL flags masked",
            "child_policy": "DEBUG_PROCESS; root process only; reject every additional executable before continuation" if locked_mode else "DEBUG_PROCESS; only byte-identical images under stage; otherwise terminate owned job",
            "motion_discovery": "bounded private discover-motion.request sampling; each hit restores entry byte/EIP; safe stopped-thread rearm" if discovery_enabled else "disabled",
            "known_xlive_debug_close_policy": "opt-in exact SHA+live return/argument+callsite, first chance before OEP, once" if handle_known_xlive_debug_close else "disabled; exceptions are not swallowed",
            "directinput_startup_observation": "restored one-shot at original 00A98340" if observe_directinput_startup else "disabled",
            "disk_binary_modified": False, "started_unix": time.time(), "probe": probe}
    write_receipt(receipt, data)
    try:
        data["sandbox_scan"] = scan_tree(stage, True,
            medium_executable=executable if token_mode != "low" else None)
        data["protected_scans"] = [scan_tree(r, False) for r in protected_roots] if token_mode == "low" else []
        before = documents_snapshot(protected_documents)
    except BaseException as exc:
        data.update(phase="failed", error=f"Preflight {type(exc).__name__}: {exc}")
        write_receipt(receipt, data)
        raise
    data["protected_documents_before"] = before
    token, job, pi, source_locks = None, None, PROCESS_INFORMATION(), None
    owned, live, live_threads = {}, set(), {}
    failed, exit_seen, suspended_created = None, False, False
    try:
        token = restricted_token(token_mode)
        data["token"] = verify_token(token, token_mode)
        if token_mode != "low":
            if not protected_roots:
                raise RuntimeError("Medium mode requires protected source audit roots")
            data["source_write_audit"] = audit_source_writes(token, protected_roots, token_mode=token_mode)
            write_receipt(receipt, data)
            if not data["source_write_audit"]["complete"] or not data["source_write_audit"]["passed"]:
                raise RuntimeError("Original source permits restricted-token mutation; launch refused")
        if locked_mode:
            manifest = receipt.with_name(f"{receipt.stem}-source-locks-{os.getpid()}-{time.time_ns()}.jsonl")
            source_locks = SourceReadLocks(protected_roots, data["source_write_audit"], manifest, check_cancel=check_drain)
            data["source_locks"] = source_locks.receipt
            write_receipt(receipt, data)
        job = K.CreateJobObjectW(None, None)
        checked(job, "CreateJobObjectW")
        limits = JOB_EXTENDED()
        limits.basic.flags = 0x2000  # JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE, no breakaway.
        checked(K.SetInformationJobObject(job, 9, C.byref(limits), C.sizeof(limits)), "SetInformationJobObject")
        env = dict(os.environ)
        profile = stage / "profile"
        env.update(USERPROFILE=str(profile), APPDATA=str(profile / "AppData/Roaming"),
                   LOCALAPPDATA=str(profile / "AppData/Local"), TEMP=str(stage / "Temp"),
                   TMP=str(stage / "Temp"), HOMEDRIVE=profile.drive,
                   HOMEPATH=str(profile)[len(profile.drive):], __COMPAT_LAYER="RunAsInvoker")
        env_block = C.create_unicode_buffer("\0".join(f"{k}={v}" for k, v in sorted(env.items(), key=lambda kv: kv[0].upper())) + "\0\0")
        if source_locks:
            si = source_locks.startup()
            startup_pointer = C.cast(C.byref(si), C.POINTER(STARTUPINFO))
        else:
            si = STARTUPINFO()
            si.cb = C.sizeof(si)
            si.lpDesktop = "winsta0\\default"
            startup_pointer = C.byref(si)
        command = C.create_unicode_buffer(subprocess.list2cmdline([str(executable), *arguments]))
        # CREATE_SUSPENDED | DEBUG_PROCESS | CREATE_UNICODE_ENVIRONMENT |
        # NORMAL_PRIORITY_CLASS. Never retry with an unrestricted token.
        check_drain()
        checked(A.CreateProcessAsUserW(token, str(executable), command, None, None, bool(source_locks),
                                      0x4 | 0x1 | 0x400 | 0x20 | (0x80000 if source_locks else 0), env_block, str(cwd),
                                      startup_pointer, C.byref(pi)), "CreateProcessAsUserW restricted suspended")
        suspended_created = True
        data["root_pid"] = pi.dwProcessId
        data["root_initial_thread_id"] = pi.dwThreadId
        process_token_info(pi.hProcess, token_mode)
        wow64 = W.BOOL()
        checked(K.IsWow64Process(pi.hProcess, C.byref(wow64)), "IsWow64Process")
        if not wow64:
            raise RuntimeError("Target is not WOW64")
        checked(K.AssignProcessToJobObject(job, pi.hProcess), "AssignProcessToJobObject")
        checked(K.DebugSetProcessKillOnExit(True), "DebugSetProcessKillOnExit")
        if K.ResumeThread(pi.hThread) == 0xFFFFFFFF:
            checked(False, "ResumeThread")
        data["phase"] = "awaiting_entry"
        write_receipt(receipt, data)
        deadline, entry_deadline = time.monotonic() + seconds, time.monotonic() + min(seconds, 90)
        while True:
            check_drain()
            marker = stage / "discover-motion.request"
            if discovery_enabled and marker.exists() and live and all(owned[p][1]["hooked"] for p in live):
                ordinary(marker)
                if marker.stat().st_nlink != 1 or marker.stat().st_size > 512:
                    raise RuntimeError("Invalid motion request marker")
                request = {"sha256": sha(marker), "mtime_ns": marker.stat().st_mtime_ns,
                           "accepted_unix": time.time()}
                try:
                    options = motion_request_options(marker.read_text(encoding="utf-8"))
                    parse_error = None
                except (ValueError, UnicodeError) as exc:
                    options, parse_error = None, str(exc)
                ack = stage / "discover-motion.ack"
                ordinary(ack, False)
                if ack.exists() and (not ack.is_file() or ack.stat().st_nlink != 1):
                    raise RuntimeError("Invalid motion acknowledgement path")
                marker.replace(ack)
                for pid in live:
                    _, state = owned[pid]
                    try:
                        if parse_error:
                            raise ValueError(parse_error)
                        start_motion_request(state, request, options)
                    except ValueError as exc:
                        state["motion_request_rejected"] = {"request": request, "reason": str(exc)}
                write_receipt(receipt, data)
            for pid in live:
                process, state = owned[pid]
                session = state.get("motion_session")
                if session and session["status"] == "active" and time.monotonic() >= state["motion_request_deadline_monotonic"]:
                    finish_motion_request(state, "expired")
                    write_receipt(receipt, data)
                helper_alive = state.get("motion_helper_thread_id") in live_threads.get(pid, set())
                due = state.get("motion_next_arm_monotonic")
                need_stop = state.get("motion_restore_pending") or (
                    state.get("motion_pending") and not state.get("motion_armed") and due is not None and time.monotonic() >= due)
                if need_stop and not state.get("motion_break_requested") and not helper_alive:
                    state["motion_break_requested"] = True
                    checked(K.DebugBreakProcess(process), "DebugBreakProcess bounded motion rearm")
            if time.monotonic() >= deadline:
                data["phase"] = "timeout"
                break
            if live and not all(owned[pid][1]["hooked"] for pid in live) and time.monotonic() >= entry_deadline:
                raise RuntimeError("Target did not reach a guarded entry within the startup deadline")
            event = DEBUG_EVENT()
            wait_ms = 200
            for pid in live:
                due = owned[pid][1].get("motion_next_arm_monotonic")
                if due is not None and due > time.monotonic():
                    wait_ms = min(wait_ms, max(1, int((due - time.monotonic()) * 1000)))
            if not K.WaitForDebugEvent(C.byref(event), wait_ms):
                if C.get_last_error() == 121:  # ERROR_SEM_TIMEOUT
                    continue
                checked(False, "WaitForDebugEvent")
            status, stop = 0x10002, False  # DBG_CONTINUE
            try:
                if event.code == 3:  # CREATE_PROCESS_DEBUG_EVENT, before execution.
                    creation = event.u.create
                    try:
                        if locked_mode and event.pid != pi.dwProcessId:
                            raise RuntimeError("Additional executable forbidden by root-only source-lock policy")
                        path = image_path(creation.hProcess)
                        private(path, stage)
                        if sha(path) != expected_hash:
                            raise RuntimeError(f"Unexpected child executable: {path}")
                        child_info = process_token_info(creation.hProcess, token_mode)
                        handle = duplicate(creation.hProcess)
                        base = creation.base
                        state = {"pid": event.pid, "path": str(path), "base": base,
                                 "entry": base + info["entry_rva"], "iat": base + info["iat_rva"],
                                 "entry_byte": info["entry_byte"], "size": info["image_size"],
                                 "motion_entry_byte": info["motion_entry_byte"],
                                 "motion_rva": info["motion_rva"], "controlled_rva": info["controlled_rva"],
                                 "initial_thread_id": event.tid,
                                 "token": child_info, "hooked": False, "loader_breakpoints": 0}
                        state["modules"] = [module_record(handle, base, known_path=path, known_size=info["image_size"])]
                        owned[event.pid] = (handle, state)
                        live.add(event.pid)
                        live_threads[event.pid] = {event.tid}
                        data["processes"].append(state)
                        if read_remote(handle, state["entry"], 1).hex() != info["entry_byte"]:
                            raise RuntimeError("Entry byte does not match the original image")
                        replace_remote(handle, state["entry"], b"\xcc")
                        write_receipt(receipt, data)
                    finally:
                        close(creation.hFile)
                elif event.code == 2:  # CREATE_THREAD_DEBUG_EVENT
                    live_threads.setdefault(event.pid, set()).add(event.tid)
                elif event.code == 4:  # EXIT_THREAD_DEBUG_EVENT
                    live_threads.get(event.pid, set()).discard(event.tid)
                    if event.pid in owned and owned[event.pid][1].get("motion_helper_thread_id") == event.tid:
                        owned[event.pid][1]["motion_helper_thread_id"] = None
                elif event.code == 6:  # LOAD_DLL_DEBUG_EVENT
                    try:
                        if event.pid in owned:
                            handle, state = owned[event.pid]
                            if len(state["modules"]) < 256:
                                state["modules"].append(module_record(handle, event.u.dll.base, event.u.dll.hFile))
                            else:
                                state["module_records_dropped"] = state.get("module_records_dropped", 0) + 1
                    finally:
                        close(event.u.dll.hFile)
                elif event.code == 7:  # UNLOAD_DLL_DEBUG_EVENT
                    if event.pid in owned:
                        for module in reversed(owned[event.pid][1]["modules"]):
                            if module["base"] == event.u.unload_base and module.get("loaded"):
                                module["loaded"] = False
                                break
                elif event.code == 1:
                    exception = event.u.exception
                    record = exception.record
                    handle, state = owned[event.pid]
                    if record.code in (0x80000003, 0x4000001F):
                        if record.address == state.get("directinput_address"):
                            directinput_observation(handle, event.tid, state, state.get("directinput_armed", False))
                            write_receipt(receipt, data)
                        elif record.address == state.get("motion_address"):
                            # Queued traps are genuine calls from other stopped
                            # threads. Retain them while the request has capacity;
                            # always rewind, even after the requested count ends.
                            motion_capture(handle, event.tid, state)
                            write_receipt(receipt, data)
                        elif state.get("motion_break_requested"):
                            state["motion_break_requested"] = False
                            state["motion_helper_thread_id"] = event.tid
                        elif record.address == state["entry"] and not state["hooked"]:
                            state.update(original_entry_thread_id=event.tid, main_thread_id=event.tid,
                                entry_thread_matches_initial=event.tid == state["initial_thread_id"])
                            establish_hook(handle, event.tid, state, document_bytes)
                            if source_locks:
                                count = DWORD()
                                checked(K.GetProcessHandleCount(handle, C.byref(count)), "Inherited source handle count at entry")
                                if count.value < len(source_locks.handles):
                                    raise RuntimeError("Owned child handle count is below inherited source-lock count")
                                state["source_lock_handles_at_entry"] = count.value
                            if observe_directinput_startup:
                                arm_directinput_observation(handle, state)
                            if all(owned[pid][1]["hooked"] for pid in live):
                                data.update(phase="running", live_launch_allowed=True)
                            write_receipt(receipt, data)
                        elif not state["hooked"] and state["loader_breakpoints"] < 2:
                            state["loader_breakpoints"] += 1
                        else:
                            status = 0x80010001  # DBG_EXCEPTION_NOT_HANDLED
                    else:
                        status = 0x80010001
                        diagnostics = state.setdefault("exception_diagnostics", [])
                        important = record.code in (0xC0000008, 0xC0000005, 0xC000001D, 0xC00000FD) or not exception.first_chance
                        if len(diagnostics) < 8 or important:
                            diagnostic = exception_diagnostic(handle, event.tid, state, exception)
                            if record.code == 0xC0000008:
                                diagnostic["known_xlive_debug_close"] = known_xlive_debug_close(
                                    handle, stage, state, diagnostic, handle_known_xlive_debug_close)
                                if diagnostic["known_xlive_debug_close"]["continued"]:
                                    status = 0x10002
                            if len(diagnostics) >= 24:
                                diagnostics[-1] = diagnostic
                                state["exception_records_dropped"] = state.get("exception_records_dropped", 0) + 1
                            else:
                                diagnostics.append(diagnostic)
                            # Persist first/second chance evidence BEFORE replying
                            # to Windows, including the default NOT_HANDLED path.
                            write_receipt(receipt, data)
                        if not exception.first_chance:
                            data["last_unhandled_exception"] = {"pid": event.pid,
                                "code": f"{record.code:08x}", "address": f"{record.address or 0:08x}"}
                elif event.code == 5:  # EXIT_PROCESS_DEBUG_EVENT
                    if event.pid in owned:
                        owned[event.pid][1]["exit_code"] = event.u.exit_code
                        owned[event.pid][1]["exit_unix"] = time.time()
                    live.discard(event.pid)
                    if event.pid in owned:
                        finish_motion_request(owned[event.pid][1], "process_exited")
                    if not live:
                        data["phase"], exit_seen, stop = "exited", True, True
                if event.pid in live:
                    process, state = owned[event.pid]
                    if state.get("motion_restore_pending"):
                        replace_remote(process, state["motion_address"], bytes.fromhex(state["motion_entry_byte"]))
                        state.update(motion_restore_pending=False, motion_armed=False, motion_entry_restored=True)
                        write_receipt(receipt, data)
                    elif (state.get("motion_pending") and not state.get("motion_armed")
                            and state.get("motion_session", {}).get("status") == "active"
                            and time.monotonic() >= state["motion_next_arm_monotonic"]):
                        arm_motion(process, state, live_threads.get(event.pid, set()))
                        write_receipt(receipt, data)
            except BaseException:
                # Do not resume an unknown/unverified child or a partially
                # established hook. Mark the owned job terminating FIRST.
                K.TerminateJobObject(job, 0xE0000001)
                K.TerminateProcess(pi.hProcess, 0xE0000001)
                raise
            finally:
                checked(K.ContinueDebugEvent(event.pid, event.tid, status), "ContinueDebugEvent")
            if stop:
                break
        if not all(state["hooked"] for _, state in owned.values()) or not owned:
            raise RuntimeError("Process exited without an established known-folder guard")
        if exit_seen:
            nonzero = [{"pid": state["pid"], "exit_code": state["exit_code"],
                        "exit_code_hex": f"{state['exit_code']:08x}"}
                       for _, state in owned.values() if state.get("exit_code", 0) != 0]
            if nonzero:
                failed = RuntimeError("Owned child exited with a nonzero status: " +
                    ", ".join(f"PID {p['pid']} 0x{p['exit_code_hex']}" for p in nonzero))
                data.update(phase="crashed" if any(p["exit_code"] >= 0x80000000 for p in nonzero) else "exit_failed",
                            error=str(failed), child_exit_failures=nonzero)
    except BaseException as exc:
        failed = exc
        data.update(phase="failed", error=f"{type(exc).__name__}: {exc}")
    finally:
        if suspended_created and not exit_seen:
            # TerminateProcess covers failure before job assignment; the only
            # target handle is the process just created by this invocation.
            if job:
                K.TerminateJobObject(job, 0xE0000001)
            if pi.hProcess:
                K.TerminateProcess(pi.hProcess, 0xE0000001)
            # Drain debug exit events so termination completes while we are the
            # debugger. No process is resumed after failure.
            drain_until = time.monotonic() + 10
            while time.monotonic() < drain_until:
                event = DEBUG_EVENT()
                if K.WaitForDebugEvent(C.byref(event), 100):
                    if event.code == 3:
                        close(event.u.create.hFile)
                    elif event.code == 6:
                        close(event.u.dll.hFile)
                    if event.code == 5 and event.pid in owned:
                        owned[event.pid][1]["exit_code"] = event.u.exit_code
                        live.discard(event.pid)
                    K.ContinueDebugEvent(event.pid, event.tid, 0x10002)
                    if not live and K.WaitForSingleObject(pi.hProcess, 0) == 0:
                        break
                elif K.WaitForSingleObject(pi.hProcess, 0) == 0:
                    break
        for handle, state in owned.values():
            code = DWORD()
            if K.GetExitCodeProcess(handle, C.byref(code)):
                state["final_exit_code"] = code.value
            close(handle)
        if source_locks and pi.hProcess:
            # Keep every independent parent lock while servicing termination.
            # The execution deadline is bounded; safe lock release additionally
            # requires the owned job to be empty. Child copies are lifetime
            # backups if the guard process itself is terminated.
            last_report = time.monotonic()
            while True:
                accounting, returned = JOB_ACCOUNTING(), DWORD()
                checked(K.QueryInformationJobObject(job, 1, C.byref(accounting), C.sizeof(accounting), C.byref(returned)), "Owned job exit before source-lock release")
                if accounting.active == 0 and K.WaitForSingleObject(pi.hProcess, 0) == 0:
                    data["source_locks"]["owned_job_exited_before_parent_release"] = True
                    break
                K.TerminateJobObject(job, 0xE0000001)
                event = DEBUG_EVENT()
                if K.WaitForDebugEvent(C.byref(event), 200):
                    if event.code == 3:
                        close(event.u.create.hFile)
                    elif event.code == 6:
                        close(event.u.dll.hFile)
                    checked(K.ContinueDebugEvent(event.pid, event.tid, 0x10002), "Drain owned job before source-lock release")
                if time.monotonic() - last_report >= 10:
                    data["source_locks"]["cleanup_waiting_for_job_exit"] = True
                    write_receipt(receipt, data)
                    last_report = time.monotonic()
        close(pi.hThread)
        close(pi.hProcess)
        close(job)
        close(token)
        if source_locks:
            source_locks.close()
        data["ended_unix"] = time.time()
        data["live_launch_allowed"] = False
        data["protected_documents_after"] = documents_snapshot(protected_documents)
        data["protected_documents_unchanged"] = data["protected_documents_after"] == before
        data["disk_executable_unchanged"] = sha(executable) == expected_hash
        data["original_executable_unchanged"] = not original_exe or sha(original_exe) == original_hash
        if not data["protected_documents_unchanged"] or not data["disk_executable_unchanged"] or not data["original_executable_unchanged"]:
            failed = RuntimeError("Post-launch preservation check failed")
            data.update(phase="failed", error=str(failed))
        write_receipt(receipt, data)
    if failed:
        raise RuntimeError(f"{data['error']}; receipt: {receipt}") from failed
    return data


PROBE_SOURCE = r'''
#define WIN32_LEAN_AND_MEAN
#define DIRECTINPUT_VERSION 0x0800
#include <windows.h>
#include <shlobj.h>
#include <dinput.h>
#include <cstdio>
#include <cstring>
extern "C" __declspec(dllexport) DWORD motion_probe_controlled = 0x12345000;
extern "C" __declspec(dllexport) __declspec(naked) void __stdcall motion_probe_entry(float) {
    __asm { ret 4 }
}
volatile LONG probe_motion_stop = 0;
DWORD WINAPI motion_probe_worker(LPVOID) {
    unsigned short worker_cw = 0x027f;
    __asm { fldcw worker_cw }
    while (InterlockedCompareExchange(&probe_motion_stop, 0, 0) == 0) {
        __asm {
            mov ecx, 22345310h
            push 03c888889h
            call motion_probe_entry
        }
        Sleep(1);
    }
    return 0;
}
volatile LONG probe_invalid_handle_caught = 0;
void NTAPI guard_probe_tls(PVOID, DWORD reason, PVOID) {
    if (reason != DLL_PROCESS_ATTACH) return;
    __try { CloseHandle((HANDLE)GetModuleHandleA(NULL)); }
    __except(GetExceptionCode() == EXCEPTION_INVALID_HANDLE ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) {
        probe_invalid_handle_caught = 1;
    }
}
#pragma section(".CRT$XLB", long, read)
extern "C" __declspec(allocate(".CRT$XLB")) PIMAGE_TLS_CALLBACK guard_probe_tls_anchor = guard_probe_tls;
#pragma comment(linker, "/INCLUDE:__tls_used")
#pragma comment(linker, "/INCLUDE:_guard_probe_tls_anchor")
int main(int argc, char** argv) {
    if (argc == 2 && !strcmp(argv[1], "--fixture-crash-exit")) ExitProcess(0xC0000005);
    if (argc == 3 && !strcmp(argv[1], "--fixture-marker-child")) {
        HANDLE mark = CreateFileA(argv[2], GENERIC_WRITE, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
        if (mark != INVALID_HANDLE_VALUE) { DWORD n; WriteFile(mark, "child-ran", 9, &n, NULL); CloseHandle(mark); }
        return 0;
    }
    if (argc == 3 && !strcmp(argv[1], "--fixture-extra-process")) {
        char self[MAX_PATH], command[MAX_PATH * 2 + 80]; GetModuleFileNameA(NULL, self, MAX_PATH);
        sprintf_s(command, "\"%s\" --fixture-marker-child \"%s\"", self, argv[2]);
        STARTUPINFOA startup = {}; startup.cb = sizeof(startup); PROCESS_INFORMATION child = {};
        if (!CreateProcessA(NULL, command, NULL, NULL, FALSE, 0, NULL, NULL, &startup, &child)) return 94;
        Sleep(10000); CloseHandle(child.hThread); CloseHandle(child.hProcess); return 95;
    }
    if (argc != 6) return 90;
    const bool locked_mode = !strcmp(argv[5], "medium-write-locked");
    const bool generic_invalid_handle_caught = probe_invalid_handle_caught != 0;
    char a[MAX_PATH] = {}, b[MAX_PATH] = {}, c[MAX_PATH] = {}, desktop[MAX_PATH] = {};
    const bool personal = SHGetSpecialFolderPathA(NULL, a, CSIDL_PERSONAL, FALSE) && !_stricmp(a, argv[1]);
    const bool flags = SHGetSpecialFolderPathA(NULL, b, CSIDL_PERSONAL | CSIDL_FLAG_DONT_VERIFY, FALSE) &&
        SHGetSpecialFolderPathA(NULL, c, CSIDL_PERSONAL | CSIDL_FLAG_CREATE, TRUE) &&
        !_stricmp(b, argv[1]) && !_stricmp(c, argv[1]);
    const BOOL fallback_return = SHGetSpecialFolderPathA(NULL, desktop, CSIDL_WINDOWS, FALSE);
    const DWORD fallback_error = GetLastError();
    const bool fallback = fallback_return && !_stricmp(desktop, argv[2]);
    IDirectInput8A* directinput = NULL;
    const HRESULT directinput_hr = DirectInput8Create(GetModuleHandleA(NULL), 0x0800,
        IID_IDirectInput8A, (void**)&directinput, NULL);
    const bool directinput_ok = SUCCEEDED(directinput_hr) && directinput != NULL;
    if (directinput) directinput->Release();
    char target[MAX_PATH]; sprintf_s(target, "%s\\private-write.txt", argv[1]);
    HANDLE f = CreateFileA(target, GENERIC_WRITE, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
    const bool allowed = f != INVALID_HANDLE_VALUE;
    if (allowed) { DWORD n; WriteFile(f, "sandbox", 7, &n, NULL); CloseHandle(f); }
    f = CreateFileA(argv[3], GENERIC_WRITE, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
    const DWORD denial = GetLastError(); const bool denied = f == INVALID_HANDLE_VALUE && denial == ERROR_ACCESS_DENIED;
    if (f != INVALID_HANDLE_VALUE) CloseHandle(f);
    char parent[MAX_PATH], protected_child[MAX_PATH]; strcpy_s(parent, argv[3]);
    char* last = strrchr(parent, '\\'); if (!last) return 93; *last = 0;
    sprintf_s(protected_child, "%s\\protected-child.txt", parent);
    f = CreateFileA(parent, FILE_DELETE_CHILD, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL);
    const DWORD delete_child_error = GetLastError();
    const bool delete_child_denied = f == INVALID_HANDLE_VALUE && delete_child_error == ERROR_ACCESS_DENIED;
    const bool delete_child_open_allowed = f != INVALID_HANDLE_VALUE;
    if (f != INVALID_HANDLE_VALUE) CloseHandle(f);
    const BOOL deleted = DeleteFileA(protected_child); const DWORD delete_error = GetLastError();
    const bool protected_delete_denied = !deleted && delete_error == (locked_mode ? ERROR_SHARING_VIOLATION : ERROR_ACCESS_DENIED);
    bool passed = personal && flags && fallback && allowed && denied && generic_invalid_handle_caught
        && (locked_mode ? delete_child_open_allowed : delete_child_denied) && protected_delete_denied;
    unsigned short saved_cw, main_cw = 0x007f;
    __asm { fnstcw saved_cw }
    __asm { fldcw main_cw }
    DWORD worker_thread_id = 0;
    HANDLE worker = CreateThread(NULL, 0, motion_probe_worker, NULL, 0, &worker_thread_id);
    passed = passed && worker != NULL;
    FILE* report = NULL;
    if (fopen_s(&report, argv[4], "w") || !report) return 91;
    for (char* p = desktop; *p; ++p) if (*p == '\\') *p = '/';
    for (char* p = argv[2]; *p; ++p) if (*p == '\\') *p = '/';
    fprintf(report, "{\"personal_redirect\":%s,\"csidl_flags\":%s,\"fallback\":%s,\"fallback_return\":%d,\"fallback_error\":%lu,\"other_folder\":\"%s\",\"expected_other_folder\":\"%s\",\"private_write\":%s,\"medium_write_denied\":%s,\"denial_error\":%lu,\"generic_invalid_handle_caught\":%s,\"delete_child_denied\":%s,\"delete_child_open_allowed\":%s,\"protected_delete_denied\":%s,\"protected_delete_error\":%lu,\"directinput_hresult\":%lu,\"directinput_ok\":%s,\"probe_main_thread_id\":%lu,\"probe_worker_thread_id\":%lu,\"passed\":%s}\n",
        personal?"true":"false", flags?"true":"false", fallback?"true":"false", fallback_return, fallback_error, desktop, argv[2], allowed?"true":"false", denied?"true":"false", denial, generic_invalid_handle_caught?"true":"false", delete_child_denied?"true":"false", delete_child_open_allowed?"true":"false", protected_delete_denied?"true":"false", delete_error, (unsigned long)directinput_hr, directinput_ok?"true":"false", GetCurrentThreadId(), worker_thread_id, passed?"true":"false");
    fclose(report);
    const DWORD begin = GetTickCount();
    while (GetTickCount() - begin < 8000) {
        __asm {
            mov ecx, dword ptr [motion_probe_controlled]
            add ecx, 310h
            push 03c888889h
            call motion_probe_entry
        }
        Sleep(1);
    }
    InterlockedExchange(&probe_motion_stop, 1);
    if (worker) { WaitForSingleObject(worker, 5000); CloseHandle(worker); }
    // Rearm with no further motion calls: expiry must restore the byte itself.
    char scope[MAX_PATH], expiry_marker[MAX_PATH]; strcpy_s(scope, argv[4]);
    char* split = strrchr(scope, '\\'); if (!split) return 96; *split = 0;
    sprintf_s(expiry_marker, "%s\\discover-motion.request", scope);
    FILE* expiry = NULL;
    if (fopen_s(&expiry, expiry_marker, "w") || !expiry) return 97;
    fputs("{\"hits\":1,\"interval_ms\":30,\"timeout_seconds\":1}\n", expiry); fclose(expiry);
    Sleep(1800);
    __asm { fldcw saved_cw }
    return passed ? 0 : 92;
}
'''


def safety_probe(root, token_mode="low"):
    root = private(root, exists=False)
    root.mkdir(parents=True, exist_ok=False)
    stage, medium = root / "stage", root / "medium"
    stage.mkdir()
    medium.mkdir()
    grant = subprocess.run(["icacls.exe", str(medium), "/grant:r", f"*{current_user_sid()}:(OI)(CI)F"],
                           capture_output=True, text=True)
    if grant.returncode:
        raise RuntimeError("Cannot prepare user-owned protected directory fixture")
    protected_child = medium / "protected-child.txt"
    protected_child.write_text("protected-kernel-fixture\n", encoding="utf-8")
    protected_hash = sha(protected_child)
    if label(medium)["rid"] < MEDIUM:
        raise RuntimeError("Probe medium fixture is not medium integrity")
    prepare(stage, stage / "guard-prepare.json")
    documents = stage / "Documents"
    documents.mkdir()
    source = root / "guard_probe.cpp"
    source.write_text(PROBE_SOURCE, encoding="utf-8")
    vswhere = Path(os.environ["ProgramFiles(x86)"]) / "Microsoft Visual Studio/Installer/vswhere.exe"
    vs = subprocess.check_output([str(vswhere), "-latest", "-products", "*", "-requires",
            "Microsoft.VisualStudio.Component.VC.Tools.x86.x64", "-property", "installationPath"], text=True).strip()
    vcvars = Path(vs) / "VC/Auxiliary/Build/vcvarsall.bat"
    if not vcvars.is_file():
        raise RuntimeError("MSVC x86 environment unavailable")
    exe = stage / "guard_probe.exe"
    batch = root / "compile_probe.cmd"
    batch.write_text('@echo off\ncall "' + str(vcvars) + '" x86 >nul\nif errorlevel 1 exit /b %errorlevel%\n' +
        subprocess.list2cmdline(["cl.exe", "/nologo", "/W4", "/O2", "/MT", str(source),
            "/Fe:" + str(exe), "/Fo:" + str(stage / "guard_probe.obj"), "/link", "/MANIFEST:EMBED",
            "shell32.lib", "dinput8.lib", "dxguid.lib"]) + "\n", encoding="utf-8")
    done = subprocess.run(["cmd.exe", "/d", "/c", str(batch)], cwd=root, capture_output=True, text=True)
    (root / "compile.log").write_text(done.stdout + done.stderr, encoding="utf-8")
    if done.returncode:
        raise RuntimeError(f"Probe compile failed ({done.returncode}): {root / 'compile.log'}")
    result = stage / "probe-result.json"
    if token_mode != "low":
        prepare_medium(stage, root / "probe-medium-prepare.json", executable=exe,
            original_exe=exe, protected_roots=[medium], probe=True)
    assert motion_request_options("legacy safety-probe\n")["hits"] == 20
    (stage / "discover-motion.request").write_text(json.dumps(
        {"hits": 6, "interval_ms": 30, "timeout_seconds": 10}) + "\n", encoding="utf-8")
    data = run_guard(stage, exe, stage, documents, root / "probe-launch.json", 30,
        arguments=[str(documents), str(folder(0x24)), str(medium / "must-not-exist.txt"), str(result), token_mode],
        protected_roots=[medium], probe=True, handle_known_xlive_debug_close=True, token_mode=token_mode)
    if not result.exists():
        raise RuntimeError(f"Probe did not produce a result; receipt: {root / 'probe-launch.json'}")
    evidence = json.loads(result.read_text(encoding="utf-8"))
    captures = [c for state in data["processes"] for c in state.get("motion_captures", [])]
    main_id, worker_id = evidence["probe_main_thread_id"], evidence["probe_worker_thread_id"]
    motion_ok = (len(captures) == 6 and {c["unit"] for c in captures} == {0x12345000, 0x22345000}
        and all(c["engine_dt_bytes"] == "8988883c" and c["main_thread_id"] == main_id
            and c["original_entry_thread_id"] == main_id and c["initial_thread_id"] == main_id
            and c["event_thread_id"] == c["thread_id"] and c["is_main_thread"] == (c["thread_id"] == main_id)
            and ((c["thread_id"], c["unit"], c["x87_control_word"]) in
                 ((main_id, 0x12345000, 0x007f), (worker_id, 0x22345000, 0x027f))) for c in captures)
        and all(s.get("motion_entry_restored") and not s.get("motion_armed")
            and s["motion_requests"][0]["status"] == "complete" and s["motion_requests"][0]["captured_hits"] == 6
            and s["motion_requests"][0]["unique_units"] == 2 for s in data["processes"]))
    evidence["one_shot_motion_capture"] = motion_ok
    expiry_ok = all(len(s["motion_requests"]) == 2 and s["motion_requests"][1]["status"] == "expired"
        and s["motion_requests"][1]["captured_hits"] == 0 and not s.get("motion_armed")
        and not s.get("motion_restore_pending") and s.get("motion_entry_restored") for s in data["processes"])
    evidence["bounded_thread_precision_capture"] = motion_ok
    evidence["idle_motion_request_expires_restored"] = expiry_ok
    diagnostics = [d for s in data["processes"] for d in s.get("exception_diagnostics", [])]
    invalid_handle = [d for d in diagnostics if d["code"] == "c0000008"]
    diagnostic_ok = (bool(invalid_handle) and all(d["first_chance"]
        and not d["known_folder_hook_established"]
        and not d["known_xlive_debug_close"]["continued"]
        and d.get("registers") and d.get("stack_words") and d.get("eip_module") for d in invalid_handle))
    evidence["generic_invalid_handle_not_swallowed"] = diagnostic_ok
    crash_receipt = root / "probe-crash-exit.json"
    crash_rejected = False
    try:
        run_guard(stage, exe, stage, documents, crash_receipt, 15,
            arguments=["--fixture-crash-exit"], protected_roots=[medium], probe=True,
            handle_known_xlive_debug_close=True, token_mode=token_mode)
    except RuntimeError:
        crashed = json.loads(crash_receipt.read_text(encoding="utf-8"))
        crash_rejected = (crashed["phase"] == "crashed" and
            crashed["child_exit_failures"][0]["exit_code"] == 0xC0000005 and
            crashed["protected_documents_unchanged"] and crashed["disk_executable_unchanged"])
    evidence["nonzero_child_exit_rejected"] = crash_rejected
    if token_mode == "medium-write-locked":
        child_marker, extra_receipt = stage / "Documents/child-must-not-run.txt", root / "probe-extra-process.json"
        rejected = False
        try:
            run_guard(stage, exe, stage, documents, extra_receipt, 15,
                arguments=["--fixture-extra-process", str(child_marker)], protected_roots=[medium],
                probe=True, handle_known_xlive_debug_close=True, token_mode=token_mode)
        except RuntimeError:
            extra = json.loads(extra_receipt.read_text(encoding="utf-8"))
            rejected = (extra["phase"] == "failed" and "Additional executable forbidden" in extra.get("error", "")
                and extra["source_locks"].get("owned_job_exited_before_parent_release") and not child_marker.exists())
        evidence["extra_process_rejected_before_execution"] = rejected
        evidence["passed"] = evidence["passed"] and rejected
    evidence["protected_child_unchanged"] = protected_child.exists() and sha(protected_child) == protected_hash
    evidence["passed"] = (evidence["passed"] and motion_ok and expiry_ok and diagnostic_ok and crash_rejected
        and evidence["protected_child_unchanged"] and (token_mode == "low" or evidence["directinput_ok"]))
    if not evidence.get("passed") or (medium / "must-not-exist.txt").exists():
        raise RuntimeError(f"Isolation probe failed: {evidence}")
    return {"phase": "probe_passed", "root": str(root), "token_mode": token_mode, "evidence": evidence,
            "receipt": str(root / "probe-launch.json"), "root_pid": data["root_pid"]}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    sub = parser.add_subparsers(dest="mode", required=True)
    prep = sub.add_parser("prepare")
    prep.add_argument("--stage", type=Path, required=True)
    prep.add_argument("--receipt", type=Path, required=True)
    med = sub.add_parser("prepare-medium")
    med.add_argument("--stage", type=Path, required=True)
    med.add_argument("--receipt", type=Path, required=True)
    med.add_argument("--exe", type=Path)
    med.add_argument("--original-exe", type=Path, default=RETAIL)
    launch = sub.add_parser("launch")
    launch.add_argument("--stage", type=Path, required=True)
    launch.add_argument("--receipt", type=Path, required=True)
    launch.add_argument("--exe", type=Path)
    launch.add_argument("--cwd", type=Path)
    launch.add_argument("--documents", type=Path)
    launch.add_argument("--original-exe", type=Path, default=RETAIL)
    launch.add_argument("--seconds", "--timeout", type=int, default=900)
    launch.add_argument("--token-mode", choices=("low", "medium-restricted", "medium-write-locked"), default="low")
    launch.add_argument("--no-motion-discovery", action="store_true")
    launch.add_argument("--handle-known-xlivedebugclose", action="store_true",
        help="Opt in to one first-chance invalid-HMODULE CloseHandle exception only with exact loaded xlive hash/call/live stack proof")
    launch.add_argument("--observe-directinput-startup", action="store_true",
        help="Record EAX HRESULT and [EDI] once after DirectInput8Create, restore the breakpoint and continue without changing results")
    launch.add_argument("--arg", action="append", dest="game_arguments", default=[],
        help="One native game argument; repeat --arg=VALUE (equals supports values starting with '-')")
    launch.add_argument("arguments", nargs=argparse.REMAINDER)
    probe = sub.add_parser("probe")
    probe.add_argument("--root", type=Path, required=True)
    probe.add_argument("--token-mode", choices=("low", "medium-restricted", "medium-write-locked"), default="low")
    args = parser.parse_args()
    try:
        if args.mode == "prepare":
            result = prepare(args.stage, args.receipt)
        elif args.mode == "prepare-medium":
            result = prepare_medium(args.stage, args.receipt, args.exe, args.original_exe)
        elif args.mode == "probe":
            result = safety_probe(args.root, args.token_mode)
        else:
            exe = args.exe or args.stage / "game/battlestationspacific.exe"
            real_documents = folder(5) / "Battlestations-Pacific"
            roots = [args.original_exe.parent, real_documents if real_documents.exists() else real_documents.parent]
            result = run_guard(args.stage, exe, args.cwd or exe.parent,
                args.documents or args.stage / "Documents", args.receipt, args.seconds,
                original_exe=ordinary(args.original_exe),
                arguments=[*args.game_arguments, *(args.arguments[1:] if args.arguments[:1] == ["--"] else args.arguments)],
                protected_roots=roots,
                motion_discovery=not args.no_motion_discovery,
                handle_known_xlive_debug_close=args.handle_known_xlivedebugclose,
                observe_directinput_startup=args.observe_directinput_startup, token_mode=args.token_mode)
        print(json.dumps({k: result[k] for k in ("phase", "stage", "root", "root_pid", "receipt", "evidence") if k in result}))
        return 0
    except BaseException as exc:
        print(json.dumps({"phase": "failed", "error": f"{type(exc).__name__}: {exc}"}), file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
