/*
* Copyright (C) 2019 Michael Schellenberger Costa.
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

#ifndef MISCCO_KEYWORDTRIE_HPP
#define MISCCO_KEYWORDTRIE_HPP
#include <cctype>
#include <memory>
#include <queue>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>

namespace miscco
{
/**
 * @brief The trie class representing the keyword trie.
 */
template <bool CaseSensitive = true>
class keyword_trie
{
  public:
    /**
     * @brief The Result struct containing the information about matches during a
     * search.
     */
    struct Result
    {
        const std::string keyword; /**< The found keyword */
        const std::size_t id;      /**< The index of the keyword in the keyword list*/
        std::size_t start;         /**< The starting position of the match */
        std::size_t end;           /**< The end position of the match */

        explicit Result(const std::string &key, const std::size_t id)
            : keyword(key), id(id)
        {
        }
        explicit Result(const Result &res, const std::size_t endPos)
            : keyword(res.keyword), id(res.id), start(endPos - res.keyword.size() + 1), end(endPos)
        {
        }
    };

  private:
    /**
     * @brief The Node struct containing the information of a trie Node.
     */
    struct Node
    {
        int id = -1;                  /**< Keyword index */
        const int depth = 0;          /**< Depth in the trie*/
        const char c = '\0';          /**< Character labelling the incoming edge */
        Node *parent;                 /**< Parent Node */
        Node *failure;                /**< Failure link */
        Node *output;                 /**< Output link */
        std::vector<Node *> children; /**< Child Nodes */

        explicit Node() = default;
        explicit Node(const int d, const char character, Node *par, Node *root)
            : depth(d), c(character), parent(par), failure(root), output(root)
        {
        }
    };

    Node *root;                                   /**< The root Node */
    std::vector<std::unique_ptr<Node>> trieNodes; /**< Container of the Node pointers */
    std::vector<Result> keywords;                 /**< Container of the Result stubs */
  public:
    /**
     * @brief trie Initializes the trie structure with its root Node.
     */
    keyword_trie()
    {
        trieNodes.emplace_back(std::make_unique<Node>());
        root = trieNodes.front().get();
        root->parent = root;
        root->failure = root;
        root->output = root;
    }

    /**
     * @brief addString Insert a new keyword into the keyword trie.
     * @param key The new keyword to be inserted.
     * @param addFailure Flag to signal whether the failure links should
     * immediately be updated.
     */
    void addString(const std::string &key, const bool addFailure = true)
    {
        if (key.empty())
        {
            return;
        }
        Node *current = root;
        for (const char character : key)
        {
            current = addChild(current, CaseSensitive ? character : std::tolower(character));
        }
        if (current->id != -1)
        {
            throw std::runtime_error(
                "Attempted to add two identical strings to the keyword tree.");
        }
        current->id = keywords.size();
        keywords.emplace_back(key, keywords.size());

        if (addFailure)
        {
            addFailureLinks();
        }
    }

    /**
     * @brief addString Wrapper around addString(std::string, bool) to add a
     * set of strings.
     * @param keyList The set containing the keys.
     */
    void addString(const std::set<std::string> &keyList)
    {
        for (const std::string &key : keyList)
        {
            addString(key, false);
        }
        addFailureLinks();
    }

    /**
     * @brief addString Wrapper around addString(std::string, bool) to add a
     * vector of strings.
     * @param keyList The vector containing the keys.
     */
    void addString(const std::vector<std::string> &keyList)
    {
        for (const std::string &key : keyList)
        {
            addString(key, false);
        }
        addFailureLinks();
    }

    /**
     * @brief parseText Parses a text with the trie.
     * @param text The text to be parsed.
     * @return Returns a vector with all matches.
     */
    std::vector<Result> parseText(const std::string &text) const
    {
        std::vector<Result> Results;
        if (text.empty())
        {
            return Results;
        }
        Node *current = root;
        for (size_t i = 0; i < text.size(); i++)
        {
            current = findChild(current, CaseSensitive ? text.at(i)
                                                       : std::tolower(text.at(i)));
            if (current->id != -1)
            {
                Results.emplace_back(keywords.at(current->id), i);
            }
            /* Process the output links for possible additional matches */
            Node *temp = current->output;
            while (temp != root)
            {
                Results.emplace_back(keywords.at(temp->id), i);
                temp = temp->output;
            }
        }
        return Results;
    }

  private:
    /**
     * @brief addChild Add a child Node to the trie.
     * @param parrent The pointer to the parrent Node of the new one.
     * @param character The character on the edge to the new Node.
     * @return The pointer to the newly created Node.
     */
    Node *addChild(Node *current, const char &character)
    {
        for (Node *child : current->children)
        {
            if (child->c == character)
            {
                return child;
            }
        }
        trieNodes.emplace_back(std::make_unique<Node>(current->depth + 1,
                                                      character,
                                                      current,
                                                      root));
        current->children.emplace_back(trieNodes.back().get());
        return trieNodes.back().get();
    }

    /**
     * @brief addFailureLinks Utilize a breadth first search to generate the
     * failure links.
     */
    void addFailureLinks()
    {
        std::queue<Node *> q;
        q.push(root);
        while (!q.empty())
        {
            Node *temp = q.front();
            for (Node *child : temp->children)
            {
                q.push(child);
            }
            /* A failure link with just one less charater is the optimum and will
             * never change.
             */
            if (temp->failure->depth < temp->depth - 1)
            {
                for (Node *failchild : temp->parent->failure->children)
                {
                    if (failchild->c == temp->c)
                    {
                        temp->failure = failchild;
                    }
                }
            }

            /* Process the failure links for possible additional matches */
            Node *out = temp->failure;
            while (out != root)
            {
                if (out->id != -1)
                {
                    break;
                }
                out = out->failure;
            }
            temp->output = out;
            q.pop();
        }
    }

    /**
     * @brief findChild Searches for a child Node with given character or adds one.
     * @param current The pointer to the current Node.
     * @param character The character that is searched.
     * @param addWord Flag sign to decide whether a new Node should be added.
     * @return The pointer to the matching Node (possibly after failure links),
     * root or the newly created one.
     */
    Node *findChild(Node *current, const char &character) const
    {
        for (Node *child : current->children)
        {
            if (child->c == character)
            {
                return child;
            }
        }
        return traverseFail(current, character);
    }

    /**
     * @brief traverseFail Traverse the failure links during a search.
     * @param current The original Node.
     * @param character The character that is beeing searched.
     * @return The pointer to the matching Node after a failure link or root->
     */
    Node *traverseFail(Node *current, const char &character) const
    {
        Node *temp = current->failure;
        while (temp != root)
        {
            for (Node *failchild : temp->children)
            {
                if (failchild->c == character)
                {
                    return failchild;
                }
            }
            temp = temp->failure;
        }
        for (Node *rootchild : root->children)
        {
            if (rootchild->c == character)
            {
                return rootchild;
            }
        }
        return root;
    }
}; // class keyword_trie

} // namespace miscco
#endif // MISCCO_KEYWORDTRIE_HPP
