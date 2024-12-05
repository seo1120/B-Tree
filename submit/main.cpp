#include <iostream>
using namespace std;

class BTreeNode {
public:
    int* keys;
    BTreeNode** children;
    int size;
    bool isLeaf;

    BTreeNode(int maxKeys, bool leaf) {
        keys = new int[maxKeys];
        children = new BTreeNode * [maxKeys + 1]{ nullptr };
        size = 0;
        isLeaf = leaf;
    }

    ~BTreeNode() {
        delete[] keys;
        delete[] children;
    }

    void clear() {
        if (!isLeaf) {
            for (int i = 0; i <= size; i++) {
                if (children[i]) {
                    children[i]->clear();
                    delete children[i];
                    children[i] = nullptr;
                }
            }
        }
    }
};

class BTree {
private:
    BTreeNode* root;
    int m; // B-Tree의 차수 (최대 자식 수)

    void insertNonFull(BTreeNode* node, int key) {
        int i = node->size - 1;

        if (node->isLeaf) {
            while (i >= 0 && node->keys[i] > key) {
                node->keys[i + 1] = node->keys[i];
                i--;
            }

            node->keys[i + 1] = key;
            node->size++;
        }
        else {
            while (i >= 0 && node->keys[i] > key) {
                i--;
            }
            i++;

            if (node->children[i]->size == m - 1) {
                splitChild(node, i);

                if (node->keys[i] < key) {
                    i++;
                }
            }
            insertNonFull(node->children[i], key);
        }
    }

    void splitChild(BTreeNode* parent, int index) {
        BTreeNode* child = parent->children[index];
        BTreeNode* newChild = new BTreeNode(m, child->isLeaf);
        int t = (m - 1) / 2;

        newChild->size = t;

        for (int i = 0; i < t; i++) {
            newChild->keys[i] = child->keys[i + t + 1];
        }

        if (!child->isLeaf) {
            for (int i = 0; i <= t; i++) {
                newChild->children[i] = child->children[i + t + 1];
            }
        }

        child->size = t;

        for (int i = parent->size; i >= index + 1; i--) {
            parent->children[i + 1] = parent->children[i];
        }
        parent->children[index + 1] = newChild;

        for (int i = parent->size - 1; i >= index; i--) {
            parent->keys[i + 1] = parent->keys[i];
        }
        parent->keys[index] = child->keys[t];
        parent->size++;
    }

    void inorder(BTreeNode* node, ostream& os) const {
        if (node->isLeaf) {
            for (int i = 0; i < node->size; i++) {
                os << " " << node->keys[i];
            }
        }
        else {
            os << "<";
            for (int i = 0; i < node->size; i++) {
                inorder(node->children[i], os);
                os << " " << node->keys[i];
            }
            inorder(node->children[node->size], os);
            os << ">";
        }
    }

    int getPredecessor(BTreeNode* node) {
        BTreeNode* current = node;
        while (!current->isLeaf) {
            current = current->children[current->size];
        }
        return current->keys[current->size - 1];
    }

    int getSuccessor(BTreeNode* node) {
        BTreeNode* current = node;
        while (!current->isLeaf) {
            current = current->children[0];
        }
        return current->keys[0];
    }

    void deleteKey(BTreeNode* node, int key, bool& success) {
        int idx = 0;
        while (idx < node->size && node->keys[idx] < key) {
            idx++;
        }

        if (idx < node->size && node->keys[idx] == key) {
            if (node->isLeaf) {
                for (int i = idx; i < node->size - 1; i++) {
                    node->keys[i] = node->keys[i + 1];
                }
                node->size--;
                success = true;
            }
            else {
                if (node->children[idx]->size >= (m + 1) / 2) {
                    int predKey = getPredecessor(node->children[idx]);
                    node->keys[idx] = predKey;
                    deleteKey(node->children[idx], predKey, success);
                }
                else if (node->children[idx + 1]->size >= (m + 1) / 2) {
                    int succKey = getSuccessor(node->children[idx + 1]);
                    node->keys[idx] = succKey;
                    deleteKey(node->children[idx + 1], succKey, success);
                }
                else {
                    mergeNode(node, idx);
                    deleteKey(node->children[idx], key, success);
                }
            }
        }
        else {
            if (node->isLeaf) {
                success = false;
                return;
            }

            bool flag = ((idx == node->size) ? true : false);

            if (node->children[idx]->size < (m + 1) / 2) {
                fill(node, idx);
            }

            if (flag && idx > node->size) {
                deleteKey(node->children[idx - 1], key, success);
            }
            else {
                deleteKey(node->children[idx], key, success);
            }
        }
    }

    void fill(BTreeNode* node, int idx) {
        if (idx != 0 && node->children[idx - 1]->size >= (m + 1) / 2) {
            borrowFromPrev(node, idx);
        }
        else if (idx != node->size && node->children[idx + 1]->size >= (m + 1) / 2) {
            borrowFromNext(node, idx);
        }
        else {
            if (idx != node->size) {
                mergeNode(node, idx);
            }
            else {
                mergeNode(node, idx - 1);
            }
        }
    }

    void borrowFromPrev(BTreeNode* parent, int idx) {
        BTreeNode* child = parent->children[idx];
        BTreeNode* sibling = parent->children[idx - 1];

        for (int i = child->size - 1; i >= 0; --i) {
            child->keys[i + 1] = child->keys[i];
        }

        if (!child->isLeaf) {
            for (int i = child->size; i >= 0; --i) {
                child->children[i + 1] = child->children[i];
            }
        }

        child->keys[0] = parent->keys[idx - 1];

        if (!child->isLeaf) {
            child->children[0] = sibling->children[sibling->size];
        }

        parent->keys[idx - 1] = sibling->keys[sibling->size - 1];

        child->size += 1;
        sibling->size -= 1;
    }

    void borrowFromNext(BTreeNode* parent, int idx) {
        BTreeNode* child = parent->children[idx];
        BTreeNode* sibling = parent->children[idx + 1];

        child->keys[child->size] = parent->keys[idx];

        if (!(child->isLeaf)) {
            child->children[child->size + 1] = sibling->children[0];
        }

        parent->keys[idx] = sibling->keys[0];

        for (int i = 1; i < sibling->size; ++i) {
            sibling->keys[i - 1] = sibling->keys[i];
        }

        if (!sibling->isLeaf) {
            for (int i = 1; i <= sibling->size; ++i) {
                sibling->children[i - 1] = sibling->children[i];
            }
        }

        child->size += 1;
        sibling->size -= 1;
    }

    void mergeNode(BTreeNode* parent, int idx) {
        BTreeNode* child = parent->children[idx];
        BTreeNode* sibling = parent->children[idx + 1];

        // 부모의 키를 자식 노드로 가져옴
        child->keys[child->size] = parent->keys[idx];

        // 형제 노드의 키를 자식 노드로 복사
        for (int i = 0; i < sibling->size; ++i) {
            child->keys[child->size + 1 + i] = sibling->keys[i];
        }

        if (!child->isLeaf) {
            for (int i = 0; i <= sibling->size; ++i) {
                child->children[child->size + 1 + i] = sibling->children[i];
            }
        }

        child->size += sibling->size + 1;

        for (int i = idx + 1; i < parent->size; ++i) {
            parent->keys[i - 1] = parent->keys[i];
            parent->children[i] = parent->children[i + 1];
        }

        parent->size--;

        delete sibling;
    }

public:
    BTree(int _m) : root(nullptr), m(_m) {}

    bool insertBT(int key) {
        if (!root) {
            root = new BTreeNode(m, true);
            root->keys[0] = key;
            root->size = 1;
            return true;
        }
        else {
            if (searchKey(root, key)) {
                return false; // 이미 존재하는 키
            }

            if (root->size == m - 1) {
                BTreeNode* newRoot = new BTreeNode(m, false);
                newRoot->children[0] = root;
                splitChild(newRoot, 0);

                int i = 0;
                if (newRoot->keys[0] < key) {
                    i++;
                }
                insertNonFull(newRoot->children[i], key);

                root = newRoot;
            }
            else {
                insertNonFull(root, key);
            }
            return true;
        }
    }

    bool searchKey(BTreeNode* node, int key) {
        int i = 0;
        while (i < node->size && key > node->keys[i]) {
            i++;
        }

        if (i < node->size && node->keys[i] == key) {
            return true;
        }

        if (node->isLeaf) {
            return false;
        }
        else {
            return searchKey(node->children[i], key);
        }
    }

    bool deleteBT(int key) {
        if (!root) {
            return false;
        }

        bool success = false;
        deleteKey(root, key, success);

        if (root->size == 0) {
            BTreeNode* temp = root;
            if (root->isLeaf) {
                root = nullptr;
            }
            else {
                root = root->children[0];
            }
            delete temp;
        }

        return success;
    }

    friend ostream& operator<<(ostream& os, const BTree& tree) {
        if (tree.root) {
            tree.inorder(tree.root, os);
            os << endl;
        }
        else {
            os << endl;
        }
        return os;
    }

    void clear() {
        if (root) {
            root->clear();
            delete root;
            root = nullptr;
        }
    }

    ~BTree() {
        clear();
    }
};

int main() {
    BTree tree(4); // B-Tree 차수 4
    char command;
    int key;

    while (cin >> command >> key) {
        switch (command) {
            case 'i': {
                if (tree.insertBT(key)) {
                    cout << tree;
                }
                else {
                    cout << "i " << key << ": The key already exists" << endl;
                }
                break;
            }
            case 'd': {
                if (tree.deleteBT(key)) {
                    cout << tree;
                }
                else {
                    cout << "d " << key << ": The key does not exist" << endl;
                }
                break;
            }
            default:
                cout << "Invalid command! Use 'i' for insert or 'd' for delete." << endl;
        }
    }
    tree.clear();
    return 0;
}
