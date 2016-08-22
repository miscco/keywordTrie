#include "keywordTrie.h"

namespace keywordTrie {

/**
 * @brief trie::trie Initialize the trie structure with its root node.
 */
trie::trie() {
	root = new node();
	root->failure = root;
	root->parent = root;
	trieNodes.push_back(root);
}

/**
 * @brief trie::addChild Add a child node to the trie
 * @param parrent The parrent node of the new one.
 * @param character The character on the edge to the new node
 */
node* trie::addChild (node *parent, char &character) {
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
	node *temp;

	q.push(root);

	while (!q.empty()) {
		temp = q.front();
		for (node *child : temp->children) {
			q.push(child);
		}
		/* A failure link with just one less charater is the optimum and will
		 * never change.
		 */
		if (temp->failure->depth < temp->depth - 1) {
			/* Parse the children of the parrents failure link */
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
 */
void trie::addString (const std::string &key, bool addFailure = true) {
	if (key.empty()) {
		return;
	}

	keywords.push_back(key);
	node *current = root;
	for (char character : key) {
		current = findChild(current, character, true);
	}
	/* Check if the keyword was already found */
	if (current->id != -1) {
		throw std::runtime_error(
					"Attempted to add two identical strings to keyword tree.");
	}
	current->id = keywords.size()-1;

	if (addFailure)
		addFailureLinks();
}

/**
 * @brief trie::addString Wrapper around addString(std::string) to add a set of
 * strings
 * @param keyList The set containing with the keys.
 */
void trie::addStrings(const std::set<std::string> &keyList) {
	for (const std::string &key : keyList)
		addString(key, false);
	addFailureLinks();
}

/**
 * @brief trie::addString Wrapper around addString(std::string) to add a vector
 * of strings
 * @param keyList The vector containing with the keys.
 */
void trie::addStrings(const std::vector<std::string> &keyList) {
	for (const std::string &key : keyList)
		addString(key, false);
	addFailureLinks();
}

/**
 * @brief trie::findChild Searches for a child node with given character
 * @param current The current node
 * @param character The character that we are searching
 * @param addWord Flag sign to decide whether a new node should be added
 */
node* trie::findChild (node *current, char &character, bool addWord) {
	/* Traverse the children of the node to check for existing character. */
	for (node *child : current->children) {
		if (child->c == character) {
			return child;
		}
	}
	if (addWord) {
		return addChild(current, character);
	} else {
		return current->failure;
	}
}

/**
 * @brief trie::parseText Parses a text with the trie
 * @param text The text to be parsed
 * @return Returns a vector with all matches
 */
std::vector<result> trie::parseText (std::string &text) {
	std::vector<result> results;
	if (text.empty()) {
		return results;
	}
	node *current= root;
	node *temp;
	for (unsigned i=0; i < text.size(); i++) {
		findChild(current, text.at(i), false);
		if (current->id != -1) {
			results.push_back(result(keywords.at(current->id),
									 current->id,
									 i - current->depth + 1));
		}
		temp = temp->failure;
		/* Process the failure links for possible additional matches */
		while (temp->depth > 0) {
			if (temp->id != -1) {
				results.push_back(result(keywords.at(temp->id),
										 temp->id,
										 i - temp->depth + 1));
			}
			temp = temp->failure;
		}
	}
	return results;
}

/**
 * @brief trie::printTrie
 */
void trie::printTrie() {
	std::string printout;
	std::queue<node*> q;
	node *temp;
	unsigned thisnode = 0;
	unsigned nodecount = 1;

	printout += "digraph BST {\n";
	printout += "\t node [fontname=\"Arial\", label = \"\"];\n";
	q.push(root);
	printout += "node" + std::to_string(thisnode) + ";\n";
	while (!q.empty()) {
		temp = q.front();
		for (node *child : temp->children) {
			q.push(child);
			if (child->id != -1) {
				printout += "node" + std::to_string(nodecount);
				printout += "[label = " + keywords.at(child->id) + "];\n";
			} else {
			printout += "node" + std::to_string(nodecount) + ";\n";
			}
			printout += "node" + std::to_string(thisnode);
			printout += " -> node" + std::to_string(nodecount);
			printout += " [label = " + std::string(1, child->c) + ", labeldistance=2.5]\n";
			nodecount++;
		}
		q.pop();
		thisnode++;
	}
	printout += "}\n";

	FILE* png = fopen("trie.png", "w");

	GVC_t *gvc;
	gvc = gvContext();
	Agraph_t* g = agmemread(printout.c_str());
	gvLayout(gvc, g, "dot");
	gvRender(gvc, g, "png", png);
	gvFreeLayout(gvc, g);
	agclose(g);
	gvFreeContext(gvc);
	system("okular trie.png && rm trie.png");
}

} // namespace keywordTrie
