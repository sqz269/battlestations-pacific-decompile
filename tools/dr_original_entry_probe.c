/* PRIVATE FIXTURE ONLY. Application-IAT redirect, never a shell32-wide hook. */
#include "dr_api.h"
#include "drmgr.h"
#include <string.h>

static app_pc entry;
static app_pc expected_base;
static app_pc entry_call_target;
static uint entry_rva, iat_rva;
static byte entry_bytes[16];
static char image_path[MAXIMUM_PATH], documents[MAXIMUM_PATH], log_path[MAXIMUM_PATH], cov_dir[MAXIMUM_PATH];
static void *gate_lock;
static bool admitted, force_failure;
static byte *stub;
static byte entry_fp[DR_FPSTATE_BUF_SIZE];
static bool fp_verified;
static bool call_verified;

static bool same_x87(const byte *left, const byte *right) {
    size_t i;
    if (memcmp(left, right, 5) || memcmp(left + 24, right + 24, 4)) return false; /* CW, SW, tag, MXCSR. */
    for (i = 0; i < 8; ++i) if (memcmp(left + 32 + i * 16, right + 32 + i * 16, 10)) return false;
    return true;
}

static void record(const char *event) {
    file_t file = dr_open_file(log_path, DR_FILE_WRITE_APPEND);
    if (file != INVALID_FILE) {
        dr_fprintf(file, "%s pid=%d\n", event, dr_get_process_id()); dr_close_file(file);
    }
}
static void reject(const char *reason) {
    file_t file = dr_open_file(log_path, DR_FILE_WRITE_APPEND);
    if (file != INVALID_FILE) {
        dr_fprintf(file, "admission_rejected reason=%s pid=%d\n", reason, dr_get_process_id());
        dr_close_file(file);
    }
    dr_exit_process(0xe0000030);
}
static void check(bool ok, const char *reason) { if (!ok) reject(reason); }
static void copy_arg(char *out, const char *in) {
    size_t size = strlen(in); check(size > 0 && size < MAXIMUM_PATH, "argument_length");
    memcpy(out, in, size + 1);
}
static uint hex(const char *text) {
    uint value = 0; int digit;
    check(*text != 0, "empty_hex");
    for (; *text; ++text) {
        digit = *text >= '0' && *text <= '9' ? *text - '0' :
            (*text >= 'a' && *text <= 'f' ? *text - 'a' + 10 : -1);
        check(digit >= 0, "invalid_hex"); value = value * 16 + digit;
    }
    return value;
}
static bool same_path(const char *left, const char *right) {
    while (*left && *right) {
        char a = *left++, b = *right++;
        if (a >= 'A' && a <= 'Z') a += 'a' - 'A';
        if (b >= 'A' && b <= 'Z') b += 'a' - 'A';
        if (a == '/') a = '\\'; if (b == '/') b = '\\';
        if (a != b) return false;
    }
    return *left == *right;
}
static void put32(byte *where, uint value) { memcpy(where, &value, 4); }

static void admission(void) {
    byte fp_storage[DR_FPSTATE_BUF_SIZE + DR_FPSTATE_ALIGN];
    byte *fp = (byte *)(((ptr_uint_t)fp_storage + DR_FPSTATE_ALIGN - 1) & ~(DR_FPSTATE_ALIGN - 1));
    size_t fp_size = proc_save_fpstate(fp); /* First call; automatic x87 save is disabled. */
    module_data_t *main_module, *shell;
    app_pc original, current;
    byte observed[16]; uint old_prot, stub_prot; size_t page_size = dr_page_size();
    byte *iat = expected_base + iat_rva;
    byte *page = (byte *)((ptr_uint_t)iat & ~(page_size - 1));
    /* Same stdcall16 stub as original_baseline_guard.hook_bytes: ESI/EDI saved,
     * CSIDL masked to low byte, NULL personal buffer FALSE, other folders tailcall. */
    byte code[] = {0x8b,0x44,0x24,0x0c,0x25,0xff,0,0,0,0x83,0xf8,5,0x75,40,
        0x8b,0x54,0x24,8,0x85,0xd2,0x74,27,0x56,0x57,0x89,0xd7,0xbe,0,0,0,0,
        0xb9,0,0,0,0,0xfc,0xf3,0xa4,0x5f,0x5e,0xb8,1,0,0,0,0xc2,0x10,0,
        0x33,0xc0,0xc2,0x10,0,0xb8,0,0,0,0,0xff,0xe0};
    dr_mutex_lock(gate_lock);
    check(fp_size == DR_FPSTATE_BUF_SIZE, "unsupported_fp_format");
    if (admitted) {
        check(dr_safe_read(iat, 4, &current, NULL) && current == stub, "hook_removed");
        if (!fp_verified) {
            file_t file;
            check(same_x87(fp, entry_fp), "x87_redirect_changed_state"); fp_verified = true;
            file = dr_open_file(log_path, DR_FILE_WRITE_APPEND);
            if (file != INVALID_FILE) { dr_fprintf(file, "fp_preservation_verified cw=%04x mxcsr=%08x\n", *(ushort *)fp, *(uint *)(fp + 24)); dr_close_file(file); }
        }
        dr_mutex_unlock(gate_lock); proc_restore_fpstate(fp); return;
    }
    if (force_failure) reject("forced_fixture_gate_failure");
    main_module = dr_get_main_module();
    check(main_module && main_module->start == expected_base && main_module->entry_point == entry &&
        same_path(main_module->full_path, image_path) &&
        iat_rva + 4 <= (uint)(main_module->end - main_module->start), "image_identity");
    dr_free_module_data(main_module);
    check(dr_safe_read(entry, sizeof(observed), observed, NULL) && observed[0] == 0xeb && observed[1] == 0xfe &&
        memcmp(observed + 2, entry_bytes + 2, sizeof(observed) - 2) == 0, "independent_entry_barrier");
    {
        char path[MAXIMUM_PATH];
        int length = dr_snprintf(path, sizeof(path), "%s/drcov.dr_root_probe.exe.%05d.0000.proc.log", cov_dir, dr_get_process_id());
        check(length > 0 && length < MAXIMUM_PATH && dr_get_client_base(1), "tracer_not_loaded");
        /* drcov retains an exclusive writer. Query existence without opening
         * its file; this directory was empty/new and the PID is this process. */
        check(dr_file_exists(path), "tracer_not_initialized"); record("tracer_ready");
    }
    check(dr_safe_read(iat, 4, &original, NULL) && original, "unresolved_iat");
    shell = dr_lookup_module_by_name("shell32.dll");
    check(shell && original == (app_pc)dr_get_proc_address((module_handle_t)shell->start,
        "SHGetSpecialFolderPathA"), "unexpected_iat_target");
    dr_free_module_data(shell);
    check(dr_query_memory(iat, NULL, NULL, &old_prot), "iat_memory_query");
    stub = dr_custom_alloc(NULL, DR_ALLOC_NON_HEAP | DR_ALLOC_NON_DR, page_size,
        DR_MEMPROT_READ | DR_MEMPROT_WRITE, NULL);
    check(stub && !dr_memory_is_dr_internal(stub), "application_stub_allocation");
    put32(code + 27, (uint)(ptr_uint_t)(stub + sizeof(code)));
    put32(code + 32, (uint)strlen(documents) + 1);
    put32(code + 55, (uint)(ptr_uint_t)original);
    memcpy(stub, code, sizeof(code)); memcpy(stub + sizeof(code), documents, strlen(documents) + 1);
    check(dr_memory_protect(stub, page_size, DR_MEMPROT_READ | DR_MEMPROT_EXEC), "stub_rx");
    {
        /* Call only our pure copy/return stub here, never application or shell32
         * code from client context. The fallback is exercised by the app. */
        typedef int (__stdcall *personal_stub_t)(void *, char *, int, int);
        char result[MAXIMUM_PATH]; personal_stub_t personal = (personal_stub_t)stub;
        check(personal(NULL, result, 0x8005, 1) == 1 && !strcmp(result, documents) &&
            personal(NULL, NULL, 5, 0) == 0, "personal_stub_selftest");
        record("personal_stub_selftest_passed");
    }
    check(dr_memory_protect(page, page_size, old_prot | DR_MEMPROT_WRITE), "iat_writable");
    check(dr_safe_write(iat, 4, &stub, NULL), "iat_patch");
    check(dr_memory_protect(page, page_size, old_prot), "iat_protection_restore");
    check(dr_safe_read(iat, 4, &current, NULL) && current == stub &&
        dr_query_memory(stub, NULL, NULL, &stub_prot) &&
        (stub_prot & (DR_MEMPROT_READ | DR_MEMPROT_EXEC)) == (DR_MEMPROT_READ | DR_MEMPROT_EXEC) &&
        !(stub_prot & DR_MEMPROT_WRITE), "hook_verification");
    {
        file_t file = dr_open_file(log_path, DR_FILE_WRITE_APPEND); size_t i;
        check(file != INVALID_FILE, "hook_receipt_open");
        dr_fprintf(file, "hook_installed base=" PFX " entry=" PFX " iat=" PFX
            " original=" PFX " stub=" PFX " stub_protection=%u documents=%s bytes=",
            expected_base, entry, iat, original, stub, stub_prot, documents);
        for (i = 0; i < sizeof(code); ++i) dr_fprintf(file, "%02x", code[i]);
        dr_fprintf(file, "\n"); dr_close_file(file);
    }
    memcpy(entry_fp, fp, sizeof(entry_fp)); admitted = true; record("entry_admitted");
    dr_mutex_unlock(gate_lock); proc_restore_fpstate(fp);
}
/* Keep EBFE in application memory. Only this verified client can reconstruct
 * the original instruction in the cache; missing clients retain the spin. */
static dr_emit_flags_t entry_app2app(void *context, void *tag, instrlist_t *list,
                                   bool trace, bool translating) {
    instr_t *spin, *original; byte *next;
    if (tag != entry) return DR_EMIT_DEFAULT;
    check(!trace && entry_bytes[0] == 0xe8, "unsupported_entry_shape");
    spin = instrlist_first_app(list);
    check(spin && spin == instrlist_last_app(list) && instr_is_ubr(spin) &&
        opnd_is_pc(instr_get_target(spin)) && opnd_get_pc(instr_get_target(spin)) == entry, "entry_ir_not_spin");
    original = instr_create(context); next = decode_from_copy(context, entry_bytes, entry, original);
    check(next == entry_bytes + 5 && instr_is_call_direct(original) &&
        opnd_is_pc(instr_get_target(original)) && opnd_get_pc(instr_get_target(original)) == entry_call_target,
        "original_entry_decode");
    instr_set_translation(original, entry); instrlist_replace(list, spin, original); instr_destroy(context, spin);
    check(instrlist_set_return_target(list, entry + 5), "entry_call_return_metadata");
    if (!translating) {
        file_t file = dr_open_file(log_path, DR_FILE_WRITE_APPEND);
        check(file != INVALID_FILE, "entry_ir_receipt");
        dr_fprintf(file, "entry_ir_substituted pc=" PFX " call_target=" PFX " return_pc=" PFX " native_barrier=ebfe\n",
            entry, entry_call_target, entry + 5); dr_close_file(file);
    }
    return DR_EMIT_STORE_TRANSLATIONS;
}
static void verify_entry_call(void) {
    dr_mcontext_t context = {sizeof(context), DR_MC_ALL}; app_pc return_pc;
    if (!admitted || call_verified) return;
    check(fp_verified && dr_get_mcontext(dr_get_current_drcontext(), &context) &&
        dr_safe_read((void *)context.xsp, sizeof(return_pc), &return_pc, NULL) && return_pc == entry + 5,
        "entry_call_runtime_return");
    call_verified = true;
    {
        file_t file = dr_open_file(log_path, DR_FILE_WRITE_APPEND);
        check(file != INVALID_FILE, "entry_call_receipt");
        dr_fprintf(file, "entry_call_target_and_return_verified target=" PFX " stack_return=" PFX "\n",
            entry_call_target, return_pc); dr_close_file(file);
    }
}
static dr_emit_flags_t bb(void *context, void *tag, instrlist_t *list, instr_t *instruction,
                         bool trace, bool translating, void *user_data) {
    if (!translating && tag == entry && instruction == instrlist_first_app(list)) {
        dr_insert_clean_call_ex(context, list, instruction, (void *)admission, DR_CLEANCALL_READS_APP_CONTEXT, 0);
        dr_insert_clean_call_ex(context, list, instruction, (void *)admission, DR_CLEANCALL_READS_APP_CONTEXT, 0);
    }
    if (!translating && tag == entry_call_target && instruction == instrlist_first_app(list))
        dr_insert_clean_call_ex(context, list, instruction, (void *)verify_entry_call,
            DR_CLEANCALL_READS_APP_CONTEXT | DR_CLEANCALL_SAVE_FLOAT, 0);
    return DR_EMIT_DEFAULT;
}
DR_EXPORT void dr_client_main(client_id_t id, int argc, const char *argv[]) {
    int i; (void)id;
    if (argc != 10) dr_exit_process(0xe0000030);
    copy_arg(log_path, argv[1]); copy_arg(documents, argv[2]); copy_arg(image_path, argv[3]);
    expected_base = (app_pc)(ptr_uint_t)hex(argv[4]); entry_rva = hex(argv[5]); iat_rva = hex(argv[6]);
    check(strlen(argv[7]) == 32, "entry_hex_length");
    for (i = 0; i < 16; ++i) { char pair[3] = {argv[7][i * 2], argv[7][i * 2 + 1], 0}; entry_bytes[i] = (byte)hex(pair); }
    force_failure = !strcmp(argv[8], "1"); entry = expected_base + entry_rva;
    copy_arg(cov_dir, argv[9]);
    {
        int displacement; check(entry_bytes[0] == 0xe8, "unsupported_entry_shape");
        memcpy(&displacement, entry_bytes + 1, 4); entry_call_target = entry + 5 + displacement;
    }
    gate_lock = dr_mutex_create(); check(gate_lock != NULL, "gate_lock");
    check(drmgr_init() && drmgr_register_bb_app2app_event(entry_app2app, NULL) &&
        drmgr_register_bb_instrumentation_event(NULL, bb, NULL), "bb_callback_registration");
    record("client_ready");
}
