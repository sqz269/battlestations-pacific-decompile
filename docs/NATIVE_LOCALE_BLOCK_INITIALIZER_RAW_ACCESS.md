# Locale block initializer raw access

The existing host-free `string_map_node_pool_init_block_00a9e970` now writes
fenced raw bytes into the actual 7804h slab. It stores WORD count400h first,
then for each index0..3FFh stores WORD(3FFh-index) at7000h+2*index before the
DWORD slab identity at18h+1Ch*index. No payload, string header, link or padding
is initialized. The Source API and its void result are unchanged.

The old `at<T>&` stores required primitive field objects in freshly allocated
slab bytes; the direct allocator cannot establish that assumption by a caller
precondition. Only this function and its comment/includes change. The shared
access template and unrelated functions remain exact. Compiler fences provide
access ordering, not thread synchronization or Native fault/ABI compatibility.

This closes the identified raw-store prerequisite for new slab initialization,
not allocation success, payload ownership, quiescence, loading or startup.
Current heap/section/frame/failure-custody proofs remain separate. No new
test/probe or vendor/native-body credit; common build is recorded separately.
