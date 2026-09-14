# Actual resource-manager tree storage BQ

Addresses: 00B7E000, 00B7E050, 00B7DF40, 00B7E740, 00B7CEF0

The five complete ordinary bodies span 432 native bytes. They prepare raw parser/cache head storage and implement parser lookup/predecessor on the actual 1Ch-node layout. This replaces no parser registry with a host map. Constructor/registration/destruction of the complete resource manager remains a separate dependency frontier.

`B7E000` and `B7E050` each allocate 1Ch through the established BF681B allocation service. Neither consumes incoming ECX. Both separately test each computed link address before zeroing it, write color1/nil0, and leave key/value/tail bytes untouched. The caller still must set nil1, self-links and tree count. The canonical allocator throws after exhausted new-handler retries; no new nullable allocation policy or cleanup is added.

The actual tree contains allocator storage at0, head at4, and count at8. Its head's parent at4 is the root. Nodes contain left/parent/right at0/4/8, native key length/data atC/10, borrowed parser at14, color at18, nil at19, and untouched tail bytes1A/1B.

`B7DF40` takes ECX tree and one stack name-header argument, returns a node in EAX and RET4. It walks the actual links, selecting the first key not less than the query. Stored length0 controls empty ordering independently of data pointer. Nonempty keys use CRT case-insensitive C-string comparison without a stored-length tie-break. Its inline gates reuse the proven source comparator 443D00, but its native direct call is DF70 to BF7FBF; no nonexistent native call edge to 443D00 is claimed.

`B7E740` takes ECX tree and stack output/name, returns the output and RET8. Lower-bound precedes its returning null-tree validation. It compares with the current head, tests reverse equivalence, reloads the head for the end iterator, captures owner/node, then writes output owner before node. The output may alias caller storage within the documented valid backing contract.

`B7CEF0` takes ECX actual owner/node iterator and RET, with no uniform EAX result. The null-owner diagnostic may return and repair fields before the node reload. A sentinel selects its maximum first; a still-nil maximum tail-calls the returning diagnostic. Normal traversal either walks rightmost within the left child or updates the current iterator while climbing ancestors. The final nil diagnostic is a tail return with no subsequent ancestor store.

The primary recovered the two source drafts left by the stopped BQ worker and verified them against the retained BP hashes. The complete five native listings and reused allocator/comparator contracts were reviewed. One ignored actual-storage fixture checks both head allocations, empty/case-insensitive lookup, the full predecessor walk, borrowed payloads, output aliasing, and returning-CRT repairs. Strict MSVC Win32 compilation and that fixture pass. The accompanying report records byte/membership checks, direct transfers, dependency and artifact hashes, and combined-build status separately.

The APIs use explicit source services and current MSVC CRT behavior. Original register/stack/CRT exception identity, original locale state, malformed storage, concurrent mutation, production manager construction and gameplay are not established. Descriptive names are hypotheses; the established STL/library names are preserved.
