# Locale onexit first frame helper: primary review

Accepted worker `1ab57ea8033c543975836a75fb8d77e028f5278a` for the complete `00C07C00..00C07C44` body: 69 bytes and 21 instructions. Saved entry name `__SEH_prolog4` and prototype remain metadata, not recovered Original SEH identity. Native epoch 98 and function count 64,730 stayed unchanged.

Root verified actual frozen and delivery Git objects, seven finite input blobs, two owned outputs and four physical helpers. Both core and delivery replay passed with only their supplied ZIP readable and imports, directories, processes, network and mutation denied. Worker Source tree IDs describe its frozen revision; current Root Source differs after the accepted retirement changes. No Source contents census or current equality is asserted.

Root independently decoded every opcode, ModRM/SIB byte, FS override and operand and compared them with Capstone 5.0.7 and both context-zero saved text captures. Both fresh 69-response operand passes equal the already reviewed metadata. The retained failed atomic-save attempt is separate evidence of persistence failure; it does not replace an accepted capture.

Conditionally let entry ESP be S and EBP be B. The two pushes precede loading allocation A from then-current SS:[S+8]. The helper stores B there and sets EBP to F=S+8. It subtracts A, saves EBX/ESI/EDI, reads the unqueried absolute word at E15590, performs the two XORs, saves a cookie word and records ESP=S-A-24 at SS:[F-24]. It then pushes the then-current SS:[F-8], reloads SS:[F-4], writes FFFFFFFE at F-4 and that reloaded word at F-8, sets EAX=F-16, writes FS:[0] and executes RET. Pre-RET ESP is S-A-28 and the consumed current SS word is unknown; post-RET ESP is S-A-24 if execution completes. Every read follows preceding writes, with possible segment/storage aliasing retained.

For the previously accepted BF6FB9 two pushes and first CALL, the additional unchanged-SS-slot condition gives S=W-12, A=12 and F=W-4. This arithmetic does not prove the RET target BF6FC5. Neither literal C07C90 nor the memory operand E15590 was followed or read. No stack contents, callback/status, original CRT/SEH ABI, runtime or gameplay claim follows.

See `reports/cc12_locale_onexit_prolog_primary.json` and retained `local/cc12_locale_onexit_prolog_primary/` evidence.
