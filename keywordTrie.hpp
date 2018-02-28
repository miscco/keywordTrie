/*
* Copyright (C) 2016 Michael Schellenberger Costa.
*
* This code is based on a C-implementation of the keyword trie construction,
* preprocessing and text search by Bernhard Haubold.
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
#include <memory>
#include <queue>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>

namespace keywordTrie {

/**
 * @brief The node struct containing the information of a trie node.
 */
template<typename CharType>
struct Node {
    using node = Node<CharType>;
    using nodePtr = std::weak_ptr<node>;
    
    int             id		= -1;	/**< Keyword index */
    const unsigned  depth= 0;	    /**< Depth in the trie*/
    const CharType  c	= '\0';	    /**< Character labelling the incoming edge */
    const nodePtr   parent;		    /**< Parent node */
    nodePtr         failure;		/**< Failure link */
    nodePtr         output;		    /**< Output link */
    std::vector<nodePtr> children;  /**< Child nodes */

    explicit Node ()
        : parent(this), failure(this), output(this) {}
    explicit Node (const unsigned d, const CharType &character, const nodePtr par, const nodePtr root)
        : depth(d), c(character), parent(par), failure(root), output(root) {}
};

/**
 * @brief The result struct containing the information about matches during a
 * search.
 */
template<typename CharType>
struct Result {
    using string_type = std::basic_string<CharType>;
    using result = Result<CharType>;

    const string_type	keyword;    /**< The found keyword */
    const unsigned      id;         /**< The index of the keyword in the keyword list*/
    unsigned            start;	    /**< The starting position of the match */
    unsigned            end;	    /**< The end position of the match */

    explicit Result (const string_type &key, const unsigned id)
        : keyword(key), id(id) {}
    explicit Result (const result &res, const unsigned endPos)
        : keyword(res.keyword), id(res.id), start(endPos-res.keyword.size()+1),
          end(endPos) {}
};

/**
 * @brief The trie class representing the keyword trie.
 */
template<typename CharType>
class basic_trie
{
public:
    using node = Node<CharType>;
    using result = Result<CharType>;
    using string_type = std::basic_string<CharType>;
    using nodePtr = std::weak_ptr<node>;
    using trieNode = std::shared_ptr<node>;

private:
    nodePtr root;                     /**< The root node */
    std::vector<trieNode> trieNodes;  /**< Container of the node pointers */
    std::vector<result> keywords;     /**< Container of the result stubs */
    bool caseSensitive = true;	      /**< Flag for case sensitivity */

public:
    /**
     * @brief trie Initializes the trie structure with its root node.
     */
    basic_trie() {
        trieNodes.emplace_back();
        root = trieNodes.back();
    }

    /**
     * @brief addString Insert a new keyword into the keyword trie.
     * @param key The new keyword to be inserted.
     * @param addFailure Flag to signal whether the failure links should
     * immediately be updated.
     */
    void addString (const string_type &key, bool addFailure = true) {
        if (key.empty()) {
            return;
        }
        nodePtr current = root;
        for (const CharType &character : key) {
            current = addChild(current, caseSensitive ? character :
                                                        std::tolower(character));
        }
        if (current->id != -1) {
            throw std::runtime_error(
                        "Attempted to add two identical strings to the keyword tree.");
        }
        current->id = keywords.size();
        keywords.emplace_back(key, keywords.size());

        if (addFailure) {
            addFailureLinks();
        }
    }

    /**
     * @brief addString Wrapper around addString(string_type, bool) to add a
     * set of strings.
     * @param keyList The set containing the keys.
     */
    void addString(const std::set<string_type>& keyList) {
        for (const string_type &key : keyList) {
            addString(key, false);
        }
        addFailureLinks();
    }

    /**
     * @brief addString Wrapper around addString(string_type, bool) to add a
     * vector of strings.
     * @param keyList The vector containing the keys.
     */
    void addString(const std::vector<string_type>& keyList) {
        for (const string_type &key : keyList) {
            addString(key, false);
        }
        addFailureLinks();
    }

    /**
     * @brief parseText Parses a text with the trie.
     * @param text The text to be parsed.
     * @return Returns a vector with all matches.
     */
    std::vector<result> parseText (const string_type& text) const {
        std::vector<result> results;
        if (text.empty()) {
            return results;
        }
        nodePtr current = root;
        for (size_t i=0; i < text.size(); i++) {
            current = findChild(current, caseSensitive ? text.at(i)
                                                       : std::tolower(text.at(i)));
            if (current->id != -1) {
                results.emplace_back(keywords.at(current->id), i);
            }
            /* Process the output links for possible additional matches */
            nodePtr temp = current->output;
            while (temp != root) {
                results.emplace_back(keywords.at(temp->id), i);
                temp = temp->output;
            }
        }
        return results;
    }

    /**
     * @brief setCaseSensitivity Set the case sensitivity flag.
     * @param flag The new flag.
     */
    void setCaseSensitivity (bool flag) {
        caseSensitive = flag;
        if (!caseSensitive && !keywords.empty()) {
            throw std::runtime_error("Switching case sensitivity with existing "
                                     "trie might lead to invalid results");
        }
    }

private:
    /**
     * @brief addChild Add a child node to the trie.
     * @param parrent The pointer to the parrent node of the new one.
     * @param character The character on the edge to the new node.
     * @return The pointer to the newly created node.
     */
    nodePtr addChild (nodePtr current, const CharType &character) {
        for (auto&& child : current->children) {
            if (child->c == character) {
                return child;
            }
        }
        trieNodes.emplace_back(current->depth+1,
                               character,
                               current,
                               root);
        current->children.emplace_back(trieNodes.back());
        return trieNodes.back();
    }

    /**
     * @brief addFailureLinks Utilize a breadth first search to generate the
     * failure links.
     */
    void addFailureLinks() {
        std::queue<nodePtr> q;
        q.push(root);
        while (!q.empty()) {
            nodePtr temp = q.front();
            for (auto&& child : temp->children) {
                q.push(child);
            }
            /* A failure link with just one less charater is the optimum and will
             * never change.
             */
            if (temp->failure->depth < temp->depth - 1) {
                for (auto&& failchild : temp->parent->failure->children) {
                    if (failchild->c == temp->c) {
                        temp->failure = failchild;
                    }
                }
            }

            /* Process the failure links for possible additional matches */
            nodePtr out = temp->failure;
            while (out != root) {
                if (out->id != -1) {
                    break;
                }
                out = out->failure;
            }
            temp->output = out;
            q.pop();
        }
    }

    /**
     * @brief findChild Searches for a child node with given character or adds one.
     * @param current The pointer to the current node.
     * @param character The character that is searched.
     * @param addWord Flag sign to decide whether a new node should be added.
     * @return The pointer to the matching node (possibly after failure links),
     * root or the newly created one.
     */
    nodePtr findChild (nodePtr current, const CharType &character) const {
        for (auto&& child : current->children) {
            if (child->c == character) {
                return child;
            }
        }
        return traverseFail(current, character);
    }

    /**
     * @brief traverseFail Traverse the failure links during a search.
     * @param current The original node.
     * @param character The character that is beeing searched.
     * @return The pointer to the matching node after a failure link or root->
     */
    nodePtr traverseFail (nodePtr current, const CharType &character) const {
        nodePtr temp = current->failure;
        while (temp != root) {
            for (auto&& failchild : temp->children) {
                if (failchild->c == character) {
                    return failchild;
                }
            }
            temp = temp->failure;
        }
        for (auto&& rootchild : root->children) {
            if (rootchild->c == character) {
                return rootchild;
            }
        }
        return root;
    }
};

typedef basic_trie<char>     trie;
typedef basic_trie<wchar_t> wtrie;

} // namespace keywordTrie
#endif // KEYWORDTRIE_HPP
