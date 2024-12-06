#include <iostream>
using namespace std;

class BTreeNode {
public:
    int* keys;           
    BTreeNode** children; // 자식 노드 포인터 배열
    int size;               // 현재 키 개수
    bool isLeaf;         // 리프 노드 여부
    // 생성자
    BTreeNode(int maxKeys, bool leaf) {
        keys = new int[maxKeys];
        children = new BTreeNode * [maxKeys + 1];
        size = 0;
        isLeaf = leaf;
    }
    // 소멸자
    ~BTreeNode() {
        delete[] keys;  // keys 배열 해제
        delete[] children;  // children 배열 해제
    }

    // 재귀적으로 삭제
    void clear() {
        for (int i = 0; i <= size; i++) {
            if (children[i]) {
                children[i]->clear();
                delete children[i]; 
            }
        }
    }

};

class BTree {
private:
    BTreeNode* root;
    int m; // B-Tree의 차수
    bool searchPath(BTreeNode*& node, int key, int& idx) // 스택대신 idx씀 재귀기 때문에
    {
        while (idx >= 0 && key <= node->keys[idx]) {
            if (key == node->keys[idx]) {
                return false;
            }
            node->keys[idx + 1] = node->keys[idx];
            idx--;
        }
        node->keys[idx + 1] = key;
        node->size++;
        return true;
    }

    // 리프 노드나 내부 노드에 키를 삽입
    void insertKey(BTreeNode*& node, int key, bool& success) { 
        int i = node->size - 1;
        if (node->isLeaf) {

            if (!searchPath(node, key, i))
            {
                success = false;
                return;
            }
            if (node->size > m - 1) {
                // 노드가 꽉 찼을 경우 분할 처리
                if (node == root) {
                    BTreeNode* newNode = new BTreeNode(m, false);
                    newNode->children[0] = node;
                    splitNode(newNode, 0);
                    node = newNode;  // 이제 node가 변경되므로 root도 변경됨
                }
            }
        }
        else {
            // 내부 노드 처리
            while (i >= 0 && key < node->keys[i]) {
                i--;
            }
            if (key == node->keys[i]) {
                success = false; // 중복 키 발견 시
                return;
            }
            i++;
            if (node->children[i]->size == m - 1 && node->children[i]->isLeaf) {
                // 자식이 꽉 찼을 경우
                if (key == node->keys[i]) {
                    success = false; // 중복 키 발견 시
                    return;
                }
                 
                insertKey(node->children[i], key, success);
                if (!success) return;
                splitNode(node, i);
                if (key > node->keys[i]) {
                    i++;
                }
                return;
            }           

            insertKey(node->children[i], key, success);
            if (!success) return;
            if (node->children[i]->size > m-1)
            {
                splitNode(node, i);
            }

        }
    }

    void splitNode(BTreeNode* parent, int index) {
        BTreeNode* child = parent->children[index];
        int mid = m / 2; // 중간값 인덱스
        BTreeNode* newChild = new BTreeNode(m, child->isLeaf);

        newChild->size = child->size - mid - 1;
        for (int i = 0; i < newChild->size; i++) {
            newChild->keys[i] = child->keys[mid + 1 + i];
        }
        if (!child->isLeaf) {
            for (int i = 0; i <= newChild->size; i++) {
                newChild->children[i] = child->children[mid + 1 + i];
            }
        }

        child->size = mid;

        // 부모 노드에 새로운 키와 자식 추가
        for (int i = parent->size; i > index; i--) {
            parent->children[i + 1] = parent->children[i];
            parent->keys[i] = parent->keys[i - 1];
        }

        parent->keys[index] = child->keys[mid];  // 중간값을 부모로 올림
        parent->children[index + 1] = newChild;
        parent->size++;

        // 부모 노드가 꽉 찼을 경우 처리
        if (parent->size >= m) {
            if (parent == root) {
                BTreeNode* newRoot = new BTreeNode(m, false);
                newRoot->children[0] = parent;
                splitNode(newRoot, 0);
                root = newRoot;
            }
        }
    }

    // 트리를 중위 순회하여 출력
    void inorder(BTreeNode* node, ostream& os) const {
        os << "<";

        for (int i = 0; i < node->size; i++) {
            if (!node->isLeaf) {
                inorder(node->children[i], os);
            }
            os << " " << node->keys[i];
            if (i == node->size - 1 || !node->isLeaf)
                os << " ";
        }

        if (!node->isLeaf) {
            inorder(node->children[node->size], os);
        }

        os << ">";
    }

    // 삭제부분
    int getSuccessor(BTreeNode* node) {
        BTreeNode* current = node;
        while (!current->isLeaf) {
            current = current->children[0];
        }
        return current->keys[0];
    }
    void searchPathDel(BTreeNode*& node, int key, int& idx) // 재귀를 통해서 진행하기에 insert랑 조금 다름
    {
        while (idx < node->size && key > node->keys[idx]) {
            idx++;
        }
    }
    void bestSibling(BTreeNode*& node, int key, int idx, bool& retribute, bool& success)
    {
        if (node->children[idx + 1]->size > 1 || !node->children[idx + 1]->isLeaf) {
            int succKey = getSuccessor(node->children[idx + 1]);
            node->keys[idx] = succKey;
            deleteKey(node->children[idx + 1], succKey, retribute, success);
        }
        else if (node->children[idx]->size >= 1 || !node->children[idx]->isLeaf) {
            BTreeNode* pred = node->children[idx];
            while (!pred->isLeaf) {
                pred = pred->children[pred->size];
            }
            int predKey = pred->keys[pred->size - 1];
            node->keys[idx] = predKey;
            deleteKey(node->children[idx], predKey, retribute, success);
        }
        else {
            mergeNode(node, idx);
            deleteKey(node->children[idx], key, retribute, success);
        }
    }
    
    void deleteKey(BTreeNode*& node, int key, bool& retribute, bool& success) {
        int idx = 0;
        searchPathDel(node, key, idx);
        if (idx < node->size && node->keys[idx] == key) {
            if (node->isLeaf) {
                for (int i = idx; i < node->size - 1; i++) {
                    node->keys[i] = node->keys[i + 1];
                }
                node->size--;
            }
            else {
                bestSibling(node, key, idx, retribute, success);
            }
        }
        else {
            if (node->isLeaf) {
                success = false;
                return;
            }

            int mergetflag = false;
            if (node->children[idx]->keys[0] == key && node->children[idx]->size == 1 && node->children[idx]->isLeaf) {
                if (redistributeKeys(node, idx)) mergetflag = true;
            }



            bool flag = (idx == node->size);
            if ((flag && idx > node->size) || mergetflag) {
                deleteKey(node->children[idx - 1], key, retribute, success);
            }
            else {
                deleteKey(node->children[idx], key, retribute, success);
            }
        }

        if (retribute)
        {
            for(int i =0; i< node->size + 1;i++)
            {
                if (node->children[i]->size == 0 && node->children[i]->children[0]->size != 0)
                {
                    redistributeKeys(node,i);
                    retribute = false;
                }
            }
        }
        if (node->size == 0 && !node->isLeaf)
        {
            retribute = true;
        }

    }

    // 병합 처리
    void mergeNode(BTreeNode* parent, int idx) {
        // 병합 대상: parent->children[idx]와 parent->children[idx+1]
        BTreeNode* child = parent->children[idx];
        BTreeNode* sibling = parent->children[idx + 1];

        child->keys[child->size] = parent->keys[idx];

        for (int i = 0; i < sibling->size; i++) {
            child->keys[child->size + 1 + i] = sibling->keys[i];
        }

        // 형제 노드의 자식을 병합 (리프 노드가 아니면)
        if (!child->isLeaf) {
            for (int i = 0; i <= sibling->size; i++) {
                child->children[child->size + 1 + i] = sibling->children[i];
            }
        }

        child->size += sibling->size + 1;

        for (int i = idx; i < parent->size - 1; i++) {
            parent->keys[i] = parent->keys[i + 1];
            parent->children[i + 1] = parent->children[i + 2];
        }

        parent->size--; 
        delete sibling; 
    }

    bool redistributeKeys(BTreeNode* parent, int idx) {
        int leftnum =  (idx > 0) ? parent->children[idx - 1]->size : 0;
        int rightnum = (idx < parent->size) ? parent->children[idx + 1]->size : 0;

        if (idx > 0 && leftnum >= rightnum && parent->children[idx - 1]->size >= (m + 1) / 2)
        {
            borrowFromPrev(parent, idx);
        }
        else if (idx < parent->size &&  parent->children[idx + 1]->size >= (m + 1) / 2)
        {
            borrowFromNext(parent, idx);
        }
        else {
            if (idx != 0) {
                mergeNode(parent, idx - 1);
                return true;
            }
            else {
                mergeNode(parent, idx);
                return false;
            }
        }
        return false;
    }

    void borrowFromPrev(BTreeNode* parent, int idx) {
        BTreeNode* child = parent->children[idx];
        BTreeNode* sibling = parent->children[idx - 1];

        for (int i = child->size - 1; i >= 0; i--) {
            child->keys[i + 1] = child->keys[i];
        }
        if (!child->isLeaf) {
            for (int i = child->size; i >= 0; i--) {
                child->children[i + 1] = child->children[i];
            }
        }

        child->keys[0] = parent->keys[idx - 1];
        if (!child->isLeaf) {
            child->children[0] = sibling->children[sibling->size];
        }

        parent->keys[idx - 1] = sibling->keys[sibling->size - 1];
        child->size++;
        sibling->size--;
    }

    void borrowFromNext(BTreeNode* parent, int idx) {
        BTreeNode* child = parent->children[idx];
        BTreeNode* sibling = parent->children[idx + 1];

        child->keys[child->size] = parent->keys[idx];
        if (!child->isLeaf) {
            child->children[child->size + 1] = sibling->children[0];
        }
        parent->keys[idx] = sibling->keys[0];

        for (int i = 0; i < sibling->size - 1; i++) {
            sibling->keys[i] = sibling->keys[i + 1];
        }
        if (!sibling->isLeaf) {
            for (int i = 0; i < sibling->size; i++) {
                sibling->children[i] = sibling->children[i + 1];
            }
        }
        child->size++;
        sibling->size--;
    }

public:
    BTree(int _m) : m(_m), root(nullptr) {}

    bool insertBT(int key) {
        bool success = true;

        if (!root) {
            root = new BTreeNode(m, true);
            root->keys[0] = key;
            root->size = 1;
        }
        else {
            insertKey(root, key, success);  // 이제 root를 참조로 전달
        }
        return success;
    }

    bool deleteBT(int key) {
        if (!root) {
            return false;
        }
        bool retribute = false;
        bool success = true;
        deleteKey(root, key, retribute, success);

        // 루트가 비어 있으면 삭제
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

    // 트리를 중위 순회하여 출력하는 연산자 오버로딩
    friend ostream& operator<<(ostream& os, const BTree& tree) {
        if (tree.root) {
            tree.inorder(tree.root, os);
        }
        else {
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
    BTree tree(4); // B-Tree 차수 4
    char command;
    int key;

    while (cin >> command >> key) {
        switch (command) {
            case 'i': { // 삽입
                if (tree.insertBT(key)) {
                    cout << tree << endl;
                }
                else {
                    cout << "i " << key << ": The key already exists" << endl;
                }
                break;
            }
            case 'd': { // 삭제
                if (tree.deleteBT(key)) {
                    cout << tree << endl;
                }
                else {
                    cout << "d " << key << ": The key does not exist" << endl;
                }
                break;
            }
            default:
                cout << "Invalid command: " << endl;
        }
    }
    tree.clear();
    return 0;
}
