#include <iostream>
#include <string>

#include "keywordTrie.h"

int main(int argc, char** argv)
{
	const std::set<std::string> strings {
		"he",
		"she",
		"help",
		"hey",
		"we",
		"her"
	};

	keywordTrie::trie Trie;

	std::string test = "Test";

	Trie.addString(test, true);
	Trie.addStrings(strings);

	Trie.printTrie();

	return 0;
}
