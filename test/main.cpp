#include <iostream>
#include <stack>
#include <algorithm> // std::max

#include "set.hpp"

// 20203070 박우혁 B-트리 구현

class Node {
public:
    int n; // Number of keys
    int* K; // Keys
    Node** P; // Child pointers

    Node(int m) : n(0) {
        K = new int[m - 1];
        P = new Node*[m];
        for(int i = 0; i < m; ++i) {
            P[i] = nullptr;
        }
        for(int i = 0; i < m - 1; ++i) {
            K[i] = 0;
        }
    }

    ~Node() {
        delete[] K;
        delete[] P;
    }
};

// 공통적으로 사용되는 함수들입니다.
void inorderBT(const Node* T);
void clearBT(Node*& T);

// fucntion prototypes
Node* getBTNode(int m);
bool searchPath(Node* T, int key, std::stack<Node*> &stack_path);
void insertKey(Node* x, Node* y, int newKey);
std::pair<int, Node*> splitNode(int m, Node* x, Node* y, int newKey);
void deleteKey(Node* x, int oldKey);
int bestSibling(Node* x, Node* y);
void redistributeKeys(Node* x, Node* y, int bestSib);
void mergeNode(Node* x, Node* y, int bestSib);
void insertBT(Node*& T, int m, int newKey);
bool deleteBT(Node*& T, int m, int oldKey);


Node* getBTNode(int m) {
    return new Node(m);
}

// T에서 key를 발견하면 ture, 아니면 false 리턴
bool searchPath(Node* T, int key, std::stack<Node*> &stack_path) { 
    if (T == nullptr) {
        return false;
    }
    Node* x = T; // x는 현재 노드를 가리키
    int i = 0;

    do {
        i = 0;
        while (i < x->n && key > x->K[i]) {
            i++;
        }

        // 삽입할 키를 발견하지 못함, 즉 K[i-1] < key < K[i]
        stack_path.push(x);

        // 삽입할 키가 이미 존재함. 삽입 불가
        if (i < x->n && key == x->K[i]) {
            return true;
        }
    } while((x = x->P[i]) != nullptr);

    return false;
}

void insertKey(Node* x, Node* y, int newKey) { // x : 삽입할 노드, y : nullptr??
    int i = x->n - 1;
    while (i >= 0 && newKey < x->K[i]) { // newKey 보다 큰 키들을, 오른쪽으로 한 칸씩 이동
        x->K[i + 1] = x->K[i];
        x->P[i + 2] = x->P[i + 1];
        i--;
    }

    // newKey를 삽입
    x->K[i + 1] = newKey;
    x->P[i + 2] = y;
    x->n++;
}

std::pair<int, Node*> splitNode(int m, Node* x, Node* y, int newKey) { // x: 오버플로우가 발생한 노드, y: 새로 분할한 노드
    Node *tempNode = getBTNode(m+1); // 오버플로우를 위한 임시 노드 (x와 newKey를 저장)
    
    // copy x to tempNode
    for(int i = 0; i < m - 1; ++i) {
        tempNode->K[i] = x->K[i];
        tempNode->P[i] = x->P[i];
    }
    tempNode->P[m - 1] = x->P[m - 1];
    tempNode->n = m - 1;

    // std::cout << "x: ";
    // inorderBT(x);
    // std::cout << std::endl;

    // std::cout << "tempNode: ";
    // inorderBT(tempNode);
    // std::cout << std::endl;

    insertKey(tempNode, y, newKey); // ?? nullptr is y??

    int centerKey = tempNode->K[m / 2]; // 분할 기준인 키의 중앙값

    // std::cout << "insert after tempNode: ";
    // inorderBT(tempNode);
    // std::cout << std::endl;

    x->n = 0; // centerKey 이전 값을 노드 x로 복사
    int i = 0;
    while (tempNode->K[i] < centerKey){
        x->K[i] = tempNode->K[i];
        x->P[i] = tempNode->P[i];
        i++;
        x->n++;
    }
    x->P[i] = tempNode->P[i];

    Node * newNode = getBTNode(m); // centerKey 이후 값을 노드 newNode로 복사
    i++;
    while(i < tempNode->n){
        newNode->K[newNode->n] = tempNode->K[i];
        newNode->P[newNode->n] = tempNode->P[i];
        i++;
        newNode->n++;
    }
    newNode->P[newNode->n] = tempNode->P[i];
    
    delete tempNode;

    // std::cout << "centerKey: " << centerKey << std::endl;

    return std::make_pair(centerKey, newNode);
}

void deleteKey(Node* x, int oldKey) {
    int i = 0;
    while (i < x->n && oldKey > x->K[i]) {
        i++;
    }

    while (i < x->n - 1) {
        x->K[i] = x->K[i + 1];
        x->P[i + 1] = x->P[i + 2];
        i++;
    }
    x->n--;
}

int bestSibling(Node* x, Node* y) {
    int i = 0;
    while (y->P[i] != x) {
        i++;
    }

    if (i == 0){
        return i + 1;
    } else if (i == y->n){
        return i - 1;
    } else if (y->P[i - 1]->n >= y->P[i + 1]->n){
        return i - 1;
    } 
    return i+1;
}

void redistributeKeys(Node* x, Node* y, int bestSib) {
    int i=0;
    while(y->P[i] != x){
        i++;
    }

    Node* bestNode = y->P[bestSib];
    if(bestSib < i){
        int lastKey = bestNode->K[bestNode->n - 1];
        insertKey(x, nullptr, y->K[i - 1]);
        x->P[1] = x->P[0];
        x->P[0] = bestNode->P[bestNode->n];
        bestNode->P[bestNode->n] = nullptr;
        deleteKey(bestNode, lastKey);
        y->K[i - 1] = lastKey;
    } else {
        int firstKey = bestNode->K[0];
        insertKey(x, nullptr, y->K[i]);
        x->P[x->n] = bestNode->P[0];
        bestNode->P[0] = bestNode->P[1];
        deleteKey(bestNode, firstKey);
        y->K[i] = firstKey;
    }
}

void mergeNode(Node* x, Node* y, int bestSib) {
    int i = 0;
    while (y->P[i] != x) {
        i++;
    }

    Node* bestNode = y->P[bestSib];

    if (bestSib > i) {
        int tmp = i; i = bestSib; bestSib = tmp;
        Node* tmpNode = x; x = bestNode; bestNode = tmpNode;
    }
    bestNode->K[bestNode->n] = y->K[i - 1];
    bestNode->n++;

    int j = 0;
    while (j < x->n) {
        bestNode->K[bestNode->n] = x->K[j];
        bestNode->P[bestNode->n] = x->P[j];
        bestNode->n++;
        j++;
    }
    bestNode->P[bestNode->n] = x->P[x->n];
    deleteKey(y, y->K[i - 1]);
    delete x;
}

void insertBT(Node*& T, int m, int newKey) {
    if (T == nullptr) {
        T = getBTNode(m);
        T->K[0] = newKey;
        T->n = 1;

        return;
    }

    std::stack<Node*> stack_path;
    bool found = searchPath(T, newKey, stack_path);
    if (found) {
        return;
    }

    bool finished = false;

    Node* x = stack_path.top(); stack_path.pop();
    Node* y = nullptr;

    do {
        if (x->n < m - 1){
            insertKey(x, y, newKey); // ? 무조건 y = nullptr 인가?
            finished = true;
        } else {
            std::__1::pair<int, Node *> pairs = splitNode(m, x, y, newKey);
            newKey = pairs.first;
            Node* newNode = pairs.second;
            y = newNode;
            if (!stack_path.empty()){
                x = stack_path.top();
                stack_path.pop();
            } else {
                T = getBTNode(m);
                T->K[0] = newKey;
                T->P[0] = x;
                T->P[1] = y;
                T->n = 1;
                finished = true;
            }
        }
    } while (!finished);


}

bool deleteBT(Node*& T, int m, int oldKey) {
    std::stack<Node*> stack_path;
    bool found = searchPath(T, oldKey, stack_path);
    if (found == false) {
        return false;
    }

    Node* x = stack_path.top(); stack_path.pop();
    Node* y = nullptr;

    // if (oldKey is not in terminal node)
    bool isNotTerminal = false;
    for (int i = 0; i <= x->n; i++) { // Check if oldKey is not in terminal node
        if (x->P[i] != nullptr) {
            isNotTerminal = true;
            break;
        }
    }
    if (isNotTerminal) {
        Node* internalNode = x;
        int i = 0;
        // Find index of oldKey // TODO 앞에서 찾도록 할 수도 있음
        while (i < internalNode->n && internalNode->K[i] < oldKey) {
            i++;
        } // index of oldKey in internalNode;

        stack_path.push(x);
        searchPath(x->P[i + 1], x->K[i], stack_path);


        x = stack_path.top();
        stack_path.pop();

        int temp = internalNode->K[i];
        internalNode->K[i] = x->K[0];
        x->K[0] = temp;
    }

    bool finished = false;
    deleteKey(x, oldKey);

    if (!stack_path.empty()){
        y = stack_path.top();
        stack_path.pop();
    }

    do {
        if (T == x || x->n >= (m - 1) / 2) {
            finished = true;
        } else {
            int bestSib = bestSibling(x, y);

            if (y->P[bestSib]->n > (m - 1) / 2) {
                redistributeKeys(x, y, bestSib);
                finished = true;
            } else {
                mergeNode(x, y, bestSib);
                x = y;
                if (!stack_path.empty()){
                    y = stack_path.top();
                    stack_path.pop();
                } else {
                    finished = true;
                }
            }
        }
    } while (!finished);

    // 만약 루트가 자식을 하나만 가지고 있으면 그 자식을 새로운 루트로 만든다.
    if (T->n == 0) {
        Node* temp = T;
        T = T->P[0];
        delete temp;
    }
    
    return true;
}

void inorderBT(const Node* T) {
    if (T == nullptr) return;
    std::cout << "<";
    for(int i = 0; i < T->n; ++i) {
        inorderBT(T->P[i]);
        std::cout << " ";
        std::cout << T->K[i];
        if (T->P[i+1] != nullptr) std::cout << " ";
    }
    if (T->P[T->n] == nullptr) std::cout << " ";
    inorderBT(T->P[T->n]);
    std::cout << ">";
}

void clearBT(Node*& T) {
    if (T == nullptr) return;
    for(int i = 0; i <= T->n; ++i) {
        clearBT(T->P[i]);
    }
    delete T;
    T = nullptr;
}

int main(){
    int M = 4;
    char command;
    int key;
    Node *root = nullptr;
    while(std::cin >> command >> key){
        switch(command){
            case 'i':
                // Check if key already exists
                {
                    std::stack<Node*> stack_check;
                    bool exists = searchPath(root, key, stack_check);
                    if(exists){
                        std::cerr << "i " << key << ": The key already exists\n";
                    }
                    else{
                        insertBT(root, M, key);
                        inorderBT(root);
                        std::cout << "\n";
                    }
                }
                break;
            case 'd':
                if(deleteBT(root, M, key)){
                    inorderBT(root);
                    std::cout << "\n";
                }
                else{
                    std::cerr << "d " << key << ": The key does not exist\n";
                }
                break;
            default:
                std::cerr << "Invalid command: " << command << "\n";
                clearBT(root);
                return 1;
        }
    }
    clearBT(root);
    return 0;
}
