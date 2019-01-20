# A simple keyword trie based text search

This library implements a simple trie based search algorithm for multiple keywords in a given text.  Due to failure and output links, the algorithm is very efficient, only traversing the text once.

The code is inspired by a C-based implementation from Bernhard Haubold (http://guanine.evolbio.mpg.de/homePage/index.html) and an implementation of the Aho-Corasick algorithm of Christopher Gilbert (https://github.com/blockchaindev/aho_corasick).

As an example the following code will create a simple keyword trie and use it to parse a given text.
```cpp
miscco::keyword_trie trie;
trie.addString("hers");
trie.addString("his");
trie.addString("she");
trie.addString("he");
trie.addString("heR");
auto results = trie.parseText("usheRs");
```

The output structure features the following information.
- The string of the found keyword
- The ID of the keyword based on its addition to the keyword trie
- The start and end position of the match

Similarly a case insensitive search can be performed.
```cpp
miscco::keyword_trie trie<false>;
trie.addString("hers");
trie.addString("his");
trie.addString("she");
trie.addString("he");
trie.addString("Her");
auto results = trie.parseText("usheRs");
```
