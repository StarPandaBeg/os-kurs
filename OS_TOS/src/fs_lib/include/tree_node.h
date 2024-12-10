#pragma once

#include <map>
#include <memory>
#include <vector>

namespace fs {
	namespace internal {
        template<typename Key, typename Value>
        class TreeNode {
        public:
            Key key;
            std::unique_ptr<Value> value;
            std::map<Key, std::unique_ptr<TreeNode<Key, Value>>> children;

            TreeNode(const Key& key_);

            bool insert(const std::vector<Key>& path, size_t index, const Value& val);
            TreeNode<Key, Value>* search(const std::vector<Key>& path, size_t index) const;
            bool remove(const std::vector<Key>& path, size_t index);
            void clear();
        };

        template<typename Key, typename Value>
        inline TreeNode<Key, Value>::TreeNode(const Key& key_) : key(key_), value(nullptr) {}

        template<typename Key, typename Value>
        inline bool TreeNode<Key, Value>::insert(const std::vector<Key>& path, size_t index, const Value& val)
        {
            if (index >= path.size()) {
                if (!value) {
                    value.reset(new Value(val));
                    return true;
                }
                return false;
            }
            const Key& currentKey = path[index];
            if (children.find(currentKey) == children.end()) {
                children[currentKey] = std::unique_ptr<TreeNode<Key, Value>>(new TreeNode<Key, Value>(currentKey));
            }
            return children[currentKey]->insert(path, index + 1, val);
        }

        template<typename Key, typename Value>
        inline TreeNode<Key, Value>* TreeNode<Key, Value>::search(const std::vector<Key>& path, size_t index) const
        {
            if (index >= path.size()) {
                return const_cast<TreeNode<Key, Value>*>(this);
            }
            const Key& currentKey = path[index];
            typename std::map<Key, std::unique_ptr<TreeNode<Key, Value>>>::const_iterator it = children.find(currentKey);
            if (it == children.end()) {
                return nullptr;
            }
            return it->second->search(path, index + 1);
        }

        template<typename Key, typename Value>
        inline bool TreeNode<Key, Value>::remove(const std::vector<Key>& path, size_t index)
        {
            if (index >= path.size()) {
                if (value) {
                    value.reset();
                    return children.empty();
                }
                return false;
            }
            const Key& currentKey = path[index];
            typename std::map<Key, std::unique_ptr<TreeNode<Key, Value>>>::iterator it = children.find(currentKey);
            if (it == children.end()) {
                return false;
            }
            bool shouldDeleteChild = it->second->remove(path, index + 1);
            if (shouldDeleteChild) {
                children.erase(it);
                return !value && children.empty();
            }
            return false;
        }

        template<typename Key, typename Value>
        inline void TreeNode<Key, Value>::clear()
        {
            children.clear();
            value.reset();
        }
	}
}