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

	auto results = Trie.parseText("ushers");
	std::cout << "Results: " << results.size() << std::endl;
	for (auto res : results) {
		std::cout << "Key: " << res.keyword <<std::endl;
		std::cout << "KeyID: " << res.keyID <<std::endl;
		std::cout << "Position: ushers"<< std::endl;
		std::cout << std::string(10+res.position, ' ')
				  << std::string(res.keyword.size(), '^') <<std::endl;
	}

	//Trie.printTrie();

	return 0;
}
