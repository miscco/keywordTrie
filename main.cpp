#include <iostream>
#include <fstream>
#include <string>
#include <chrono>

#include "keywordTrie.hpp"
typedef std::chrono::high_resolution_clock::time_point timer;

int main(int argc, char** argv)
{
	keywordTrie::trie Trie, Trie2, Trie3;
	timer begin, end;


	begin   = std::chrono::high_resolution_clock::now();
	Trie.addString("AACGTTCA");
	end     = std::chrono::high_resolution_clock::now();
	std::cout << "Keyword trie construction took "
			  << std::chrono::duration_cast<std::chrono::microseconds>( end - begin ).count()
			  << " \u03BCs\n";

	std::string querry, line;

	std::ifstream myfile ("mgGenome.fasta");
	if (myfile.is_open()) {
		getline (myfile,line);
		while ( getline (myfile,line) ) {
			querry += line;
		}
		myfile.close();
	} else  {
		throw std::runtime_error("Cannot find mgGenome.fasta!");
	}

	begin   = std::chrono::high_resolution_clock::now();
	auto results = Trie.parseText(querry);
	end     = std::chrono::high_resolution_clock::now();
	std::cout << "Search took "
			  << std::chrono::duration_cast<std::chrono::milliseconds>( end - begin ).count()
			  << " ms\n";
	for (auto res : results) {
		std::cout << "Key: " << res.keyword
				  << "\t Position: " << res.start <<std::endl;
	}
	std::cout << std::endl;

	const std::set<std::string> patterns {
		"he",
		"she",
		"Her",
		"hers",
		"Help",
		"we"
	};

	querry = "ushershe";
	Trie2.addString(patterns);
	results = Trie2.parseText(querry);
	std::cout << "Results: " << results.size() << std::endl;
	for (auto res : results) {
		std::cout << "Key: " << res.keyword
				  << "\t Position: " << res.start <<std::endl;
	}

	Trie3.setCaseSensitivity(false);
	Trie3.addString(patterns);
	results = Trie3.parseText(querry);
	std::cout << "Results: " << results.size() << std::endl;
	for (auto res : results) {
		std::cout << "Key: " << res.keyword
				  << "\t Position: " << res.start <<std::endl;
	}

	return 0;
}
