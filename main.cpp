#include <iostream>
#include <fstream>
#include <string>
#include <chrono>

#include "keywordTrie.hpp"
typedef std::chrono::high_resolution_clock::time_point timer;

int main(int argc, char** argv)
{
	keywordTrie::trie Trie;
	timer begin, end;


	begin   = std::chrono::high_resolution_clock::now();
	Trie.addString("AACGTTCA");
	end     = std::chrono::high_resolution_clock::now();
	std::cout << "Keyword trie construction took " << std::chrono::duration_cast<std::chrono::microseconds>( end - begin ).count() << " ms\n";

	std::string querry, line;

	std::ifstream myfile ("mgGenome.fasta");
	if (myfile.is_open())
	{
		getline (myfile,line);
		while ( getline (myfile,line) )
		{
			querry += line;
		}
		myfile.close();
	}

	begin   = std::chrono::high_resolution_clock::now();
	auto results = Trie.parseText(querry);
	end     = std::chrono::high_resolution_clock::now();
	std::cout << "Search took " << std::chrono::duration_cast<std::chrono::milliseconds>( end - begin ).count() << " ms\n";
	std::cout << results.size() << std::endl;

	const std::set<std::string> patterns {
		"he",
		"she",
		"help",
		"hey",
		"we",
		"her",
		"hers"
	};

	querry = "ushershe";
	keywordTrie::trie Trie2;
	Trie2.addString(patterns);
	results = Trie2.parseText(querry);
	std::cout << "Results: " << results.size() << std::endl;
	for (auto res : results) {
		std::cout << "Key: " << res.keyword <<std::endl;
		std::cout << "KeyID: " << res.id <<std::endl;
		std::cout << "Position: ushershe"<< std::endl;
		std::cout << std::string(10+res.start, ' ')
				  << std::string(res.keyword.size(), '^') <<std::endl;
	}

	return 0;
}
