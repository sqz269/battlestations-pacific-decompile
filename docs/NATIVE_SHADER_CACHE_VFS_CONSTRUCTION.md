# Cache acquisition through actual VFS owners

Addresses: 00B3A600, 00B38A70, 00B35340, 00BEDA60, 00BE1890.

The source cache acquisition path now has runtime evidence through a constructed
native-layout VFS manager, its registered physical factory and a real mounted
physical provider. B3A600 calls B38A70, which captures D68D04/BDF310 and opens
`shaderfx/shaders.bin` read-only with flags2. B35340 then loads every record.
The resulting 1,628 records serialize to all 2,250,700 installed bytes, SHA256
`dcc65686e26e0b7b4f4179b45b153aaa353696b0b1b08c32251fbc7c97c0ecf8`.

The fixture verifies the actual cache publication, cursor, stream profile and
position, manager read count1 and byte count2250700. All owners share the same
raw singleton and string-pool publications. The existing read-only data service
validates the complete original PE and maps the required native table/literal
bands as data; no original code or code/table relocation is used in this run.

The two unchanged worker translation units and this focused fixture compile
with MSVC Win32 /MD /O2 /Oy- /W4 /WX /fp:strict. The link map retains both cache
constructor entrypoints and resolves eleven supporting provider/lifetime
implementations to the published root library. Fresh captures match the installed
PE across five bodies (903 bytes) and three profile prefixes (148 bytes).

After explicit fixture cache retirement, its sole stream reference is released
through the actual physical stream recycler. The shared raw singleton manager
drains while all service contexts remain alive, clearing the five observed
string/factory/stream-pool/batch publications. The actual physical-provider pool
then destroys its storage and unlinks from the allocator list. This cache
retirement is harness work, not a reconstructed whole-cache destructor.

This extends `NATIVE_SHADER_CACHE_PARENT_REVIEW.md`: its unused constructor
binding was insufficient and the linker removed those entrypoints. The new
fixture supplies the real derived manager and executes the constructor path.
The previous comparison and its frozen evidence remain unchanged.

The installed loose-file path is the tested domain. Archive startup, package
scans, alternate providers, source-mode skip, variants writes, CD9010/CRT pool
startup, original constructor/FH3/ABI behavior, full cache shutdown ordering,
canonical compiler/effect admission, drawing and gameplay remain unproved.
Production files remain unchanged and unregistered pending ownership/integration.
See `reports/native_shader_cache_vfs_construction.json` and the retained local
fixture at `local/native_shader_cache_vfs_construction/`.

The live verifier passed 18 direct calls. Seven indirect calls are separately
resolved; six belong to the exercised source path and the variants-write call
remains unexecuted. The immutable checkpoint is
`local/checkpoints/4a4a5172/shader-cache-vfs-construction/validation.json`, SHA256
`4a50a5e3591911466b5c4c2a7ed7290b8a95bdfcfea292c448fce32089fc8034`
(598 artifacts and 17 physical x86 modules).
