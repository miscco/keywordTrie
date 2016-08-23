/*
* Copyright (C) 2016 Michael Schellenberger Costa.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*/

#ifndef KEYWORDTRIE_HPP
#define KEYWORDTRIE_HPP
#include <queue>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>

namespace keywordTrie {

/**
 * @brief The node struct containing the information of a trie node.
 */
struct node {
	int id			= -1;			/**< Keyword id */
	int depth		= 0;			/**< Depth in the trie*/
	const char c	= '\0';			/**< Character labelling the incoming edge */
	node *parent	= nullptr;		/**< Parent node */
	node *failure	= nullptr;		/**< Failure link */
	std::vector<node*> children;	/**< Child nodes */

	explicit node () {}
	explicit node (int d, const char character, node *par, node *root)
		: depth(d), c(character), parent(par), failure(root) {}
};

/**
 * @brief The result struct containing the information about matches during a
 * search
 */
struct result {
	std::string keyword;			/**< The found keyword */
	int			id;					/**< The id of the keyword in the keyword list*/
	int			start;				/**< The starting position of the match */
	int			end;				/**< The end position of the match */

	explicit result (const std::string &key, int id)
		: keyword(key), id(id) {}
	explicit result (const result &res, int endPos)
		: keyword(res.keyword), id(res.id), start(endPos-res.keyword.size()+1),
		  end(endPos) {}
};

/**
 * @brief The trie class representing the keyword trie
 */
class trie
{
public:
	trie();
	~trie() {for (node* N : trieNodes) delete N;}

	void addString (const std::string &key) {addString(key, true);}
	void addString (const std::string &key, bool addFailure);
	void addStrings(const std::set<std::string> &keyList);
	void addStrings(const std::vector<std::string> &keyList);
	std::vector<result> parseText(std::string text);

private:
	node				*root = nullptr;	/**< The root node */
	std::vector<node*>	trieNodes;			/**< Container of the node pointers */
	std::vector<result> keywords;			/**< Container of the result stubs */

	node *addChild		(node *current, const char character);
	node *findChild		(node *current, const char character, bool addWord);
	node *traverseFail	(node *current, const char character);
	void addFailureLinks(void);
};

/**
 * @brief trie::trie Initializes the trie structure with its root node.
 */
trie::trie() {
	root = new node();
	root->failure = root;
	root->parent = root;
	trieNodes.push_back(root);
}

/**
 * @brief trie::addChild Add a child node to the trie
 * @param parrent The pointer to the parrent node of the new one.
 * @param character The character on the edge to the new node.
 * @return The pointer to the newly created node.
 */
node* trie::addChild (node *parent, const char character) {
	trieNodes.push_back(new node(parent->depth+1, character, parent, root));
	parent->children.push_back(trieNodes.back());
	return trieNodes.back();
}

/**
 * @brief trie::addFailureLinks Utilize a breadth first search to generate the
 * failure links.
 */
void trie::addFailureLinks() {
	std::queue<node*> q;
	q.push(root);
	while (!q.empty()) {
		node *temp = q.front();
		for (node *child : temp->children) {
			q.push(child);
		}
		/* A failure link with just one less charater is the optimum and will
		 * never change.
		 */
		if (temp->failure->depth < temp->depth - 1) {
			for (node *failchild : temp->parent->failure->children) {
				if (failchild->c == temp->c) {
					temp->failure = failchild;
				}
			}
		}
		q.pop();
	}
}

/**
 * @brief trie::addString Insert a new keyword into the keyword trie.
 * @param key The new keyword to be inserted.
 * @param addFailure Flag to signal whether the failure links should immediately
 * be updated
 */
void trie::addString (const std::string &key, bool addFailure) {
	if (key.empty()) {
		return;
	}
	node *current = root;
	for (const char character : key) {
		current = findChild(current, character, true);
	}
	if (current->id != -1) {
		throw std::runtime_error(
					"Attempted to add two identical strings to the keyword tree.");
	}
	current->id = keywords.size();
	keywords.push_back(result(key, keywords.size()));

	if (addFailure) {
		addFailureLinks();
	}
}

/**
 * @brief trie::addStrings Wrapper around addString(std::string) to add a set of
 * strings
 * @param keyList The set containing the keys.
 */
void trie::addStrings(const std::set<std::string> &keyList) {
	for (const std::string &key : keyList) {
		addString(key, false);
	}
	addFailureLinks();
}

/**
 * @brief trie::addStrings Wrapper around addString(std::string) to add a vector
 * of strings
 * @param keyList The vector containing the keys.
 */
void trie::addStrings(const std::vector<std::string> &keyList) {
	for (const std::string &key : keyList) {
		addString(key, false);
	}
	addFailureLinks();
}

/**
 * @brief trie::findChild Searches for a child node with given character
 * @param current The pointer to the current node
 * @param character The character that is searched
 * @param addWord Flag sign to decide whether a new node should be added
 * @return The pointer to the matching node (possibly after failure links), root
 * or the newly created one
 */
node* trie::findChild (node *current, const char character, bool addWord) {
	for (node *child : current->children) {
		if (child->c == character) {
			return child;
		}
	}
	if (addWord) {
		return addChild(current, character);
	} else {
		return traverseFail(current, character);
	}
}

/**
 * @brief trie::parseText Parses a text with the trie
 * @param text The text to be parsed
 * @return Returns a vector with all matches
 */
std::vector<result> trie::parseText (std::string text) {
	std::vector<result> results;
	if (text.empty()) {
		return results;
	}
	node *current= root;
	for (unsigned i=0; i < text.size(); i++) {
		current = findChild(current, text.at(i), false);
		if (current->id != -1) {
			results.push_back(result(keywords.at(current->id), i));
		}
		/* Process the failure links for possible additional matches */
		node *temp = current->failure;
		while (temp != root) {
			if (temp->id != -1) {
				results.push_back(result(keywords.at(temp->id), i));
			}
			temp = temp->failure;
		}
	}
	return results;
}

/**
 * @brief trie::traverseFail traverse the failure links during a search
 * @param current The original node
 * @param character The character that is beeing searched
 * @return The pointer to the matching node after a failure link or root
 */
node* trie::traverseFail (node *current, const char character) {
	node *temp = current->failure;
	while (temp != root) {
		for (node *failchild : temp->children) {
			if (failchild->c == character) {
				return failchild;
			}
		}
		temp = temp->failure;
	}
	return temp;
}
} // namespace keywordTrie
#endif // KEYWORDTRIE_HPP
