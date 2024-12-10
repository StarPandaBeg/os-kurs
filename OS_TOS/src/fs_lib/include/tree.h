#include <memory>
#include <vector>
#include <sstream>
#include <string>

#include "path.h"
#include "tree_node.h"

#define PATH_SEPARATOR '/'

namespace fs {
	namespace internal {
        template<typename Value>
        class Tree {
        public:
            Tree();

            bool insert(const Path& path, const Value& val);
            bool search(const Path& path, Value& outVal) const;
            bool remove(const Path& path);
            void clear();
        private:
            std::unique_ptr<TreeNode<std::string, Value>> root;
        };

        template<typename Value>
        inline Tree<Value>::Tree() : root(new TreeNode<std::string, Value>(std::string())) {}

        template<typename Value>
        inline bool Tree<Value>::insert(const Path& path, const Value& val)
        {
            auto components = path.components();
            return root->insert(components, 0, val);
        }

        template<typename Value>
        inline bool Tree<Value>::search(const Path& path, Value& outVal) const {
            auto components = path.components();
            TreeNode<std::string, Value>* node = root->search(components, 0);
            if (node && node->value) {
                outVal = *(node->value);
                return true;
            }
            return false;
        }

        template<typename Value>
        inline bool Tree<Value>::remove(const Path& path) {
            auto components = path.components();
            return root->remove(components, 0);
        }

        template<typename Value>
        inline void Tree<Value>::clear() {
            root->clear();
        }
    }
}