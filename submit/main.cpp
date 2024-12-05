#include <iostream>
#include <stack>
using namespace std;

class BTreeNode {
public:
    int* keys;
    BTreeNode** children;
    int size;
    bool isLeaf;

    BTreeNode(int maxKeys, bool leaf) {
        keys = new int[maxKeys];
        children = new BTreeNode * [maxKeys + 1];
        size = 0;
        isLeaf = leaf;
        for (int i = 0; i < maxKeys + 1; i++) {
            children[i] = nullptr;
        }
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
    int m; // B-트리의 차수 (최대 자식 수)

    void splitChild(BTreeNode* parent, int index) {
        int t = m / 2;
        BTreeNode* child = parent->children[index];
        BTreeNode* newChild = new BTreeNode(m - 1, child->isLeaf);
        newChild->size = t - 1;

        // 새로운 자식에게 키를 복사
        for (int i = 0; i < t - 1; i++) {
            newChild->keys[i] = child->keys[i + t];
        }

        // 자식이 리프가 아니면 자식 포인터도 복사
        if (!child->isLeaf) {
            for (int i = 0; i < t; i++) {
                newChild->children[i] = child->children[i + t];
            }
        }

        child->size = t - 1;

        // 부모에 새로운 자식 삽입
        for (int i = parent->size; i >= index + 1; i--) {
            parent->children[i + 1] = parent->children[i];
        }
        parent->children[index + 1] = newChild;

        // 부모에 새로운 키 삽입
        for (int i = parent->size - 1; i >= index; i--) {
            parent->keys[i + 1] = parent->keys[i];
        }
        parent->keys[index] = child->keys[t - 1];
        parent->size++;
    }

    void insertNonFull(BTreeNode* node, int key) {
        int i = node->size - 1;

        if (node->isLeaf) {
            // 키를 삽입할 위치 찾기
            while (i >= 0 && node->keys[i] > key) {
                node->keys[i + 1] = node->keys[i];
                i--;
            }
            // 키 삽입
            node->keys[i + 1] = key;
            node->size++;
        } else {
            // 자식 노드로 내려감
            while (i >= 0 && node->keys[i] > key) {
                i--;
            }
            i++;
            // 자식 노드가 가득 찼다면 분할
            if (node->children[i]->size == m - 1) {
                splitChild(node, i);
                if (node->keys[i] < key) {
                    i++;
                }
            }
            insertNonFull(node->children[i], key);
        }
    }

    // 트리를 중위 순회하여 출력
    void inorder(BTreeNode* node, ostream& os) const {
        os << "<";

        for (int i = 0; i < node->size; i++) {
            // 자식이 리프가 아닐 경우 자식 노드를 먼저 순회
            if (!node->isLeaf) {
                inorder(node->children[i], os);
            }
            // 현재 키를 출력
            os << " " << node->keys[i];
            if (i == node->size - 1 || !node->isLeaf)
                os << " ";
        }

        // 마지막 자식 노드를 순회
        if (!node->isLeaf) {
            inorder(node->children[node->size], os);
        }

        os << ">";
    }


    void remove(BTreeNode* node, int key) {
        int idx = findKey(node, key);

        if (idx < node->size && node->keys[idx] == key) {
            if (node->isLeaf) {
                removeFromLeaf(node, idx);
            } else {
                removeFromNonLeaf(node, idx);
            }
        } else {
            if (node->isLeaf) {
                cout << "d " << key << ": The key does not exist" << endl;
                return;
            }

            bool flag = ((idx == node->size) ? true : false);

            if (node->children[idx]->size < (m + 1) / 2) {
                fill(node, idx);
            }

            if (flag && idx > node->size) {
                remove(node->children[idx - 1], key);
            } else {
                remove(node->children[idx], key);
            }
        }
    }

    int findKey(BTreeNode* node, int key) {
        int idx = 0;
        while (idx < node->size && node->keys[idx] < key) {
            idx++;
        }
        return idx;
    }

    void removeFromLeaf(BTreeNode* node, int idx) {
        for (int i = idx + 1; i < node->size; i++) {
            node->keys[i - 1] = node->keys[i];
        }
        node->size--;
    }

    void removeFromNonLeaf(BTreeNode* node, int idx) {
        int key = node->keys[idx];
        if (node->children[idx]->size >= (m + 1) / 2) {
            int pred = getPred(node, idx);
            node->keys[idx] = pred;
            remove(node->children[idx], pred);
        } else if (node->children[idx + 1]->size >= (m + 1) / 2) {
            int succ = getSucc(node, idx);
            node->keys[idx] = succ;
            remove(node->children[idx + 1], succ);
        } else {
            merge(node, idx);
            remove(node->children[idx], key);
        }
    }

    int getPred(BTreeNode* node, int idx) {
        BTreeNode* cur = node->children[idx];
        while (!cur->isLeaf) {
            cur = cur->children[cur->size];
        }
        return cur->keys[cur->size - 1];
    }

    int getSucc(BTreeNode* node, int idx) {
        BTreeNode* cur = node->children[idx + 1];
        while (!cur->isLeaf) {
            cur = cur->children[0];
        }
        return cur->keys[0];
    }

    void fill(BTreeNode* node, int idx) {
        if (idx != 0 && node->children[idx - 1]->size >= (m + 1) / 2) {
            borrowFromPrev(node, idx);
        } else if (idx != node->size && node->children[idx + 1]->size >= (m + 1) / 2) {
            borrowFromNext(node, idx);
        } else {
            if (idx != node->size) {
                merge(node, idx);
            } else {
                merge(node, idx - 1);
            }
        }
    }

    void borrowFromPrev(BTreeNode* node, int idx) {
        BTreeNode* child = node->children[idx];
        BTreeNode* sibling = node->children[idx - 1];

        for (int i = child->size - 1; i >= 0; --i) {
            child->keys[i + 1] = child->keys[i];
        }

        if (!child->isLeaf) {
            for (int i = child->size; i >= 0; --i) {
                child->children[i + 1] = child->children[i];
            }
        }

        child->keys[0] = node->keys[idx - 1];

        if (!child->isLeaf) {
            child->children[0] = sibling->children[sibling->size];
        }

        node->keys[idx - 1] = sibling->keys[sibling->size - 1];

        child->size += 1;
        sibling->size -= 1;
    }

    void borrowFromNext(BTreeNode* node, int idx) {
        BTreeNode* child = node->children[idx];
        BTreeNode* sibling = node->children[idx + 1];

        child->keys[child->size] = node->keys[idx];

        if (!child->isLeaf) {
            child->children[child->size + 1] = sibling->children[0];
        }

        node->keys[idx] = sibling->keys[0];

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

    void merge(BTreeNode* node, int idx) {
        BTreeNode* child = node->children[idx];
        BTreeNode* sibling = node->children[idx + 1];

        child->keys[(m - 1) / 2] = node->keys[idx];

        for (int i = 0; i < sibling->size; ++i) {
            child->keys[i + (m + 1) / 2] = sibling->keys[i];
        }

        if (!child->isLeaf) {
            for (int i = 0; i <= sibling->size; ++i) {
                child->children[i + (m + 1) / 2] = sibling->children[i];
            }
        }

        for (int i = idx + 1; i < node->size; ++i) {
            node->keys[i - 1] = node->keys[i];
        }

        for (int i = idx + 2; i <= node->size; ++i) {
            node->children[i - 1] = node->children[i];
        }

        child->size += sibling->size + 1;
        node->size--;

        delete sibling;
    }

public:
    BTree(int _m) : m(_m), root(nullptr) {}

    bool insertBT(int key) {
        if (!root) {
            root = new BTreeNode(m - 1, true);
            root->keys[0] = key;
            root->size = 1;
            return true;
        }

        if (searchBT(key)) {
            cout << "i " << key << ": The key already exists" << endl;
            return false;
        }

        if (root->size == m - 1) {
            BTreeNode* newRoot = new BTreeNode(m - 1, false);
            newRoot->children[0] = root;
            splitChild(newRoot, 0);
            int i = 0;
            if (newRoot->keys[0] < key) {
                i++;
            }
            insertNonFull(newRoot->children[i], key);
            root = newRoot;
        } else {
            insertNonFull(root, key);
        }
        return true;
    }

    bool deleteBT(int key) {
        if (!root) {
            cout << "d " << key << ": The key does not exist" << endl;
            return false;
        }

        remove(root, key);

        if (root->size == 0) {
            BTreeNode* temp = root;
            if (root->isLeaf) {
                root = nullptr;
            } else {
                root = root->children[0];
            }
            delete temp;
        }
        return true;
    }

    bool searchBT(int key) {
        BTreeNode* current = root;
        while (current) {
            int i = 0;
            while (i < current->size && key > current->keys[i]) {
                i++;
            }
            if (i < current->size && key == current->keys[i]) {
                return true;
            }
            if (current->isLeaf) {
                return false;
            } else {
                current = current->children[i];
            }
        }
        return false;
    }

    friend ostream& operator<<(ostream& os, const BTree& tree) {
        if (tree.root) {
            tree.inorder(tree.root, os);
        } else {
            os << "";
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
    BTree tree(4); // B-트리의 차수 4
    char command;
    int key;

    while (cin >> command >> key) {
        switch (command) {
        case 'i':
            tree.insertBT(key);
            cout << tree << endl;
            break;
        case 'd':
            tree.deleteBT(key);
            cout << tree << endl;
            break;
        default:
            cout << "Invalid command! Use 'i' for insert or 'd' for delete." << endl;
        }
    }
    tree.clear();
    return 0;
}
