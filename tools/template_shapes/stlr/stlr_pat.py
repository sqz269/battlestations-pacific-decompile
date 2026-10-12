"""Shared patterns (copied from stlr_feat / stlr_bucket so importing has no side effects)."""
import re
ELEM = re.compile(r'NativeString|SizedStoragePool|CharacterString|LegacySbo|StringRange|RefPtr|_CopyConstruct|_Destroy|_Destruct|_Assign|_Release|_AddRef|_Construct|DestroyActualStorage|DestroyStorage|_Copy(_|$)|_Less|_Equals|_Compare|_Hash')
TMPL = re.compile(r'^(STL_|stl_|TMPL_|MSVC_|LIBSTL_|std::|_)|(PushBack|PopFront|PopBack|LookupOrInsert|FindOrInsert|GetOrInsert\w*|Subscript|Registry_Index|Map_Index|Iterator(Previous|Next|Increment|Decrement)|(increment|decrement)_\w*iterator|EraseRange|InsertN|CopyConstruct)(_|$)|(Vector|Tree|List|Map|Set|Deque|Array|Range|Iterator|Heap|Queue|Stack|Table|Pool|Buffer|Container|Pointers?)_?'
                  r'(PushBack|PopBack|Push|Pop|Insert\w*|Erase\w*|Reserve|Resize|Clear|CopyConstruct|Copy\w*|Assign\w*|'
                  r'Increment\w*|Decrement\w*|Inc|Dec|Append\w*|Remove\w*|Find\w*|LowerBound|UpperBound|Lbound|Ubound|IndexOf|Tidy|'
                  r'Construct\w*|Destroy\w*|Destruct\w*|Grow|Buy\w*|Fill|Uninit\w*|Iterator\w*|Next|Prev|At|Subscript|Swap|Sort\w*|Unique|Splice|Move\w*)'
                  r'(_|$)|^(resize|insert|erase|push|copy|assign|destroy)_\w*(vector|tree|list|map)', re.I)
