#ifndef KEYWORDTRIE_H
#define KEYWORDTRIE_H

#include <gvc.h>
#include <iostream>
#include <fstream>
#include <queue>
#include <set>
#include <stdexcept>
#include <stdlib.h>
#include <string>
#include <vector>

namespace keywordTrie {

struct node {
  int id		= -1;			/* Keyword id */
  int depth		= 0;			/* Depth in the trie*/
  char c		= '\0';			/* Character labelling incoming edge */
  node *parent  = nullptr;		/* Parent node */
  node *failure = nullptr;		/* Failure link */
  std::vector<node*> children;	/* Child nodes */
  explicit node (int d, char character, node *par, node *root)
	  : depth(d), c(character), parent(par), failure(root)
  {id = -1; children = std::vector<node*> ();}
  explicit node () {}
};

struct result {
	std::string keyword;
	int			keyID;
	int			position;

	explicit result (std::string &key, int id, int pos)
		:keyword(key), keyID(id), position(pos) {}
};

class trie
{
public:
	trie();
	~trie() {for (node* N : trieNodes) delete N;}

	void addString(const std::string &key, bool addFailure /*= true*/);
	void addStrings(const std::set<std::string> &keyList);
	void addStrings(const std::vector<std::string> &keyList);

	std::vector<result> parseText(std::string &text);
	void printTrie (void);

private:
	node *root;
	std::vector<node*>		 trieNodes;
	std::vector<std::string> keywords;

	node *addChild		(node *current, char &character);
	node *findChild		(node *current, char &character, bool addWord);
	void addFailureLinks();
};

} // namespace keywordTrie
#endif // KEYWORDTRIE_H
