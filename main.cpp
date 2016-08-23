#include <iostream>
#include <string>

#include "keywordTrie.hpp"

int main(int argc, char** argv)
{
	const std::set<std::string> strings {
		"he",
		"she",
		"help",
		"hey",
		"we",
		"her",
		"hers"
	};

	keywordTrie::trie Trie;

	std::string test = "Test";

	Trie.addString(test, true);
	Trie.addStrings(strings);

	auto results = Trie.parseText("ushershe");
	std::cout << "Results: " << results.size() << std::endl;
	for (auto res : results) {
		std::cout << "Key: " << res.keyword <<std::endl;
		std::cout << "KeyID: " << res.id <<std::endl;
		std::cout << "Position: ushershe"<< std::endl;
		std::cout << std::string(10+res.start, ' ')
				  << std::string(res.keyword.size(), '^') <<std::endl;
	}

	//Trie.printTrie();

	return 0;
}
