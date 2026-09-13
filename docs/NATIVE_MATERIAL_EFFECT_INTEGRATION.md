# Native effect and Cursor integration

Addresses: `00B2EBB0`, `00B18F70`, `00B318B0`, `00B31090`, `00B2E940`, `00B301A0`, `00B2FFE0`, `00B2FD10`, `00B2FA10`, `00B31FF0`, `00B45EE0`, `00B46950`, `00B17DD0`, `00B5F160`, `00B5F6A0`, `00711370`, `00AB8910`.

This batch composes actual effect-cache records and aliases, filename rewriting and effect creation, retained shader-program operations, and the Cursor's native Model-prefix acquisition. The cache holds actual resource identities and the effect companion borrows the same native count. Descriptor deletion bindings live in the same canonical effect record through its terminal release. Cursor uses one persistent Model acquisition and one reused native temporary-name header.

Review corrected two lifetime details: the shader child's resolved-name argument remains stable on a retained host failure, and rewrite operands use captured pointers only on normal cleanup while FH3 cleanup uses their current headers. Saved Ghidra flow repairs restore the cache-record destructor through `B2FA87` and record reserve through `B300BC`; both direct tail calls now verify.

The combined Win32 build and both existing CTests pass. Exact promoted-commit validation and focused primary-library fixture results are pinned in `reports/native_material_effect_integration.json`. The shader pruning fixture compares original installed instructions with the actual production state-list implementation. Record and Model-prefix fixtures cover only their stated storage/lifetime behavior.

Actual texture loading at renderer `64h`, mutable VFS resolution, descriptor parsing, compilation and renderer state caches remain required dependencies. The constructor's callable renderer table and the cache's original numeric renderer profile still need a complete real binding. A cold effect load and full Text/gameplay behavior are not validated.

Follow-up packets: actual `B319B0`/`B30B40` texture loading and constructor binding; `B43B00` descriptor reading; `B3C3A0` compilation and state caches; actual `BDF4C0` candidate resolution; then Text/material caller migration with persistent inner acquisitions and installed-asset validation.
