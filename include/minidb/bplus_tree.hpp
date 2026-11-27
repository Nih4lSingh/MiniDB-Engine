#pragma once

#include <cstddef>
#include <memory>
#include <vector>

namespace minidb {

    class BPlusTree {
    public:
        using Key = int;
        using RowId = std::size_t;

        BPlusTree();

        void insert(Key key, RowId rowId);

        std::vector<RowId> findEqual(Key key) const;

    private:
        struct Node {
            explicit Node(bool isLeafNode)
                : isLeaf(isLeafNode) {}

            bool isLeaf;
            std::vector<Key> keys;

            std::vector<std::vector<RowId>> rowIds;

            std::vector<std::unique_ptr<Node>> children;

            Node* nextLeaf = nullptr;
        };

        static constexpr std::size_t maxKeys = 3;

        std::unique_ptr<Node> root_;
    };
}