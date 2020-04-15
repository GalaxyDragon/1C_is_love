
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <deque>
#include <iostream>
#include <map>
#include <memory>
#include <queue>
#include <sstream>
#include <unordered_map>
#include <vector>

struct AhoCorasickNode {
    // Stores ids_ of strings_ which are ended at this node_.
    std::vector<size_t> terminal_ids_;
    // Stores tree structure of nodes.
    std::map<char, AhoCorasickNode> trie_transitions_;

    AhoCorasickNode *AddSon(char ch) {
        auto son = AhoCorasickNode();
        son.parent_ = this;
        son.in_char_ = ch;
        trie_transitions_.insert(std::make_pair(ch, std::move(son)));
        automaton_transitions_cache_.insert(
            std::make_pair(ch, &trie_transitions_[ch]));
        return automaton_transitions_cache_[ch];
    };

    std::unordered_map<char, AhoCorasickNode *> automaton_transitions_cache_;

    char in_char_ = '\0';
    AhoCorasickNode *parent_ = nullptr;
    AhoCorasickNode *suffix_link_ = nullptr;
    AhoCorasickNode *terminal_link_ = nullptr;
};

AhoCorasickNode *GetAutomatonTransition(AhoCorasickNode *node, char ch) {
    if (node->automaton_transitions_cache_.find(ch) ==
        node->automaton_transitions_cache_.end()) {
        auto char_transition_iterator = node->trie_transitions_.find(ch);
        if (char_transition_iterator != node->trie_transitions_.end()) {
            auto char_find_iterator = node->automaton_transitions_cache_.find(ch);
            char_find_iterator->second = &(char_transition_iterator->second);
            return char_find_iterator->second;
        }
        if (node == node->parent_) {
            node->automaton_transitions_cache_[ch] = node;
            return node;
        }
        auto total = GetAutomatonTransition(node->suffix_link_, ch);
        node->automaton_transitions_cache_[ch] = total;
        return total;
    }
    return node->automaton_transitions_cache_[ch];
};

class NodeReference {
 public:
    NodeReference() = default;

    explicit NodeReference(AhoCorasickNode *node) : node_(node) {}

    NodeReference Next(char ch) const;

    template <class Callback>
    void ForEachMatch(Callback cb) const;

    bool operator==(NodeReference &other) const { return (node_ == other.node_); }

    std::vector<size_t> GetPatternEnds() { return node_->terminal_ids_; };

 private:
    AhoCorasickNode *node_ = nullptr;
};

NodeReference NodeReference::Next(char ch) const {
    auto probably_node = GetAutomatonTransition(node_, ch);
    if (probably_node == nullptr) {
        return NodeReference();
    }
    return NodeReference(probably_node);
}

class AhoCorasick {
 public:
    AhoCorasick() = default;

    AhoCorasick(const AhoCorasick &) = delete;

    AhoCorasick &operator=(const AhoCorasick &) = delete;

    AhoCorasick(AhoCorasick &&) = delete;

    AhoCorasick &operator=(AhoCorasick &&) = delete;

    NodeReference Root() { return NodeReference(&root_); };

 private:
    friend class AhoCorasickBuilder;

    AhoCorasickNode root_;
};

class AhoCorasickBuilder {
 public:
    void AddString(std::string string, size_t id) {
        strings_.push_back(std::move(string));
        ids_.push_back(id);
    }

    std::unique_ptr<AhoCorasick> Build() {
        auto automaton = std::make_unique<AhoCorasick>();
        for (size_t i = 0; i < strings_.size(); ++i) {
            AddString(&automaton->root_, strings_[i], ids_[i]);
        }
        automaton->root_.parent_ = &automaton->root_;
        automaton->root_.suffix_link_ = &automaton->root_;
        automaton->root_.terminal_link_ = &automaton->root_;
        CalculateLinks(&automaton->root_);
        return automaton;
    }

 private:
    static void AddString(AhoCorasickNode *root, const std::string &string,
                          size_t id);

    static void CalculateLinks(AhoCorasickNode *root);

    std::vector<std::string> strings_;
    std::vector<size_t> ids_;
};

void AhoCorasickBuilder::AddString(AhoCorasickNode *root,
                                   const std::string &string, size_t id) {
    auto curr_node = root;
    for (char ch : string) {
        AhoCorasickNode *next_node = nullptr;
        auto find_ch_in_curr_node = curr_node->trie_transitions_.find(ch);
        if (find_ch_in_curr_node == curr_node->trie_transitions_.end()) {
            next_node = curr_node->AddSon(ch);
        } else {
            next_node = &(curr_node->trie_transitions_.find(ch)->second);
        }
        curr_node = next_node;
    }
    curr_node->terminal_ids_.push_back(id);
}

void AhoCorasickBuilder::CalculateLinks(AhoCorasickNode *root) {
    std::queue<AhoCorasickNode *> q;
    q.push(root);
    while (!q.empty()) {
        AhoCorasickNode *curr_node = q.front();
        for (auto &i : curr_node->trie_transitions_) {
            q.push(&i.second);
        }
        q.pop();
        if (curr_node->parent_ == root || curr_node == root) {
            curr_node->suffix_link_ = root;
        } else {
            AhoCorasickNode *candidate = curr_node->parent_->suffix_link_;
            char way = curr_node->in_char_;
            while (candidate != root) {
                if (candidate->trie_transitions_.find(way) !=
                    candidate->trie_transitions_.end()) {
                    curr_node->suffix_link_ = &candidate->trie_transitions_[way];
                    break;
                }
                candidate = candidate->suffix_link_;
            }
            if (candidate == root) {
                auto find_way = candidate->trie_transitions_.find(way);
                if (find_way != candidate->trie_transitions_.end()) {
                    curr_node->suffix_link_ = &(find_way->second);
                } else {
                    curr_node->suffix_link_ = root;
                }
            }
        }
        if (!curr_node->suffix_link_->terminal_ids_.empty()) {
            curr_node->terminal_link_ = curr_node->suffix_link_;
        } else {
            curr_node->terminal_link_ = curr_node->suffix_link_->terminal_link_;
        }
        for (auto x : curr_node->terminal_link_->terminal_ids_) {
            curr_node->terminal_ids_.push_back(x);
        }
    }
}

std::vector<std::pair<std::string, size_t>> Split(const std::string &string,
                                                  char delimiter) {
    std::vector<std::pair<std::string, size_t>> answer(1, std::make_pair("", 0));
    int cnt = 0;
    for (char c : string) {
        if (c == delimiter) {
            if (!answer[answer.size() - 1].first.empty()) {
                answer[answer.size() - 1].second =
                    string.size() - static_cast<size_t>(cnt);
                answer.emplace_back(std::make_pair("", 0));
            }
        } else {
            answer[answer.size() - 1].first += c;
        }
        ++cnt;
    }
    if (answer[answer.size() - 1].first.empty()) {
        answer.pop_back();
    }
    return answer;
};

// Wildcard is a character that may be substituted
// for any possible character.
class WildcardMatcher {
 public:
    WildcardMatcher(const std::string &pattern, char wildcard) {
        auto splitted = Split(pattern, wildcard);
        number_of_words_ = splitted.size();
        pattern_length_ = pattern.size();
        AhoCorasickBuilder build;
        for (const auto &pair : splitted) {
            build.AddString(pair.first, pair.second);
        }
        automaton_ = build.Build();
        state_ = automaton_->Root();
        occurrences_by_offset_ = std::deque<size_t>(pattern_length_);
    };

    // Resets the matcher. Call allows to abandon all data which was already
    // scanned, a new stream can be scanned afterwards.
    void Reset();

    template <class Callback>
    void Scan(char character, Callback on_match);

 private:
    void UpdateWordOccurrencesCounters();

    void ShiftWordOccurrencesCounters() {
        occurrences_by_offset_.pop_front();
        occurrences_by_offset_.push_back(0);
    };

    // Storing only O(|pattern|) elements allows us
    // to consume only O(|pattern|) memory for the matcher.
    std::deque<size_t> occurrences_by_offset_;
    size_t number_of_words_;
    size_t pattern_length_;
    std::unique_ptr<AhoCorasick> automaton_;
    NodeReference state_;
    size_t position_ = 0;
};

template <class Callback>
void WildcardMatcher::Scan(char character, Callback on_match) {
    state_ = state_.Next(character);
    position_++;
    auto ids = state_.GetPatternEnds();
    for (size_t id : ids) {
        ++occurrences_by_offset_[id];
    }
    if (occurrences_by_offset_[0] == number_of_words_ &&
        position_ >= pattern_length_) {
        on_match(position_ - pattern_length_);
    }
    ShiftWordOccurrencesCounters();
}
//пример использования
/*
int main() {
    std::ios_base::sync_with_stdio(false);
    std::cin.tie(0);
    std::string pattern;
    std::string text;
    std::cin >> pattern >> text;
    WildcardMatcher matcher(pattern, '?');

    std::vector<size_t> match_ends;
    for (size_t i = 0; i < text.size(); ++i) {
        matcher.Scan(text[i], [](size_t p) { std::cout << p << " "; });
    }
}
*/
