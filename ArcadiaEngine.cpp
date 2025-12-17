// ArcadiaEngine.cpp - STUDENT TEMPLATE
// TODO: Implement all the functions below according to the assignment requirements

#include "ArcadiaEngine.h"
#include <algorithm>
#include <queue>
#include <numeric>
#include <climits>
#include <cmath>
#include <cstdlib>
#include <vector>
#include <string>
#include <iostream>
#include <map>
#include <set>

using namespace std;

// =========================================================
// PART A: DATA STRUCTURES (Concrete Implementations)
// =========================================================

// --- 1. PlayerTable (Double Hashing) ---

class ConcretePlayerTable : public PlayerTable {
private:

    static const int TableSize = 101;
    static constexpr double A = 0.6180339887;
    struct Entry {
        int playerID;
        string name;
        bool occupied;

        Entry() : playerID(-1), name(""), occupied(false) {}
    };
    vector<Entry> table;

    int hash1(int key) {
        return key % TableSize;
    }
    int hash2(int key) {
        return 97 - (abs(key) % 97);
    }

public:
    ConcretePlayerTable() : table(TableSize) {}

    void insert(int playerID, string name) override {
       int h1 = hash1(playerID);
        int h2 = hash2(playerID);

        for (int i = 0; i < TableSize; i++) {
            int idx = (h1 + i * h2) % TableSize;
            if (!table[idx].occupied) {
                table[idx].playerID = playerID;
                table[idx].name = name;
                table[idx].occupied = true;
                return;
            }
        }
        cout << "Table is Full";
    }

    string search(int playerID) override {
        int h1 = hash1(playerID);
        int h2 = hash2(playerID);

        for (int i = 0; i < TableSize; i++) {
            int idx = (h1 + i * h2) % TableSize;

            if (!table[idx].occupied)
                return "";

            if (table[idx].playerID == playerID)
                return table[idx].name;
        }

        return "";
    }
};

// --- 2. Leaderboard (Skip List) ---

class ConcreteLeaderboard : public Leaderboard {
private:
    struct Node {
        int playerID;
        int score;
        vector<Node*> forward;
        Node(int lvl, int id, int s) : playerID(id), score(s), forward(lvl, nullptr) {}
    };
    Node* head;
    int maxLevel;
    int currentLevel;
    const float P = 0.5;

    int randomLevel() {
        int lvl = 1;
        while(((double)rand() / RAND_MAX) < P && lvl < maxLevel)
            lvl++;
        return lvl;
    }

    bool comesBefore(int score1, int id1, int score2, int id2) {
       if(score1 != score2) return score1 > score2;
        return id1 < id2;
    }

public:
    ConcreteLeaderboard() : maxLevel(16), currentLevel(1) {
        head = new Node(maxLevel, -1, INT_MAX);
    }

    void addScore(int playerID, int score) override {
        vector<Node*> update(maxLevel, nullptr);
        Node* current = head;

        for (int i = currentLevel - 1; i >= 0; i--) {
            while (current->forward[i] && comesBefore(current->forward[i]->score,
                               current->forward[i]->playerID, score, playerID)) {
                current = current->forward[i]; }
            update[i] = current;
        }

        int newLevel = randomLevel();
        if(newLevel > currentLevel) {
            for (int i = currentLevel; i < newLevel; i++) update[i] = head;
            currentLevel = newLevel;
        }

        Node* newNode = new Node(newLevel, playerID, score);

        for(int i = 0; i < newLevel; i++) {
            newNode->forward[i] = update[i]->forward[i];
            update[i]->forward[i] = newNode;
        }

    }

    void removePlayer(int playerID) override {
        Node* current = head -> forward[0];
        Node* target = nullptr;
        while(current && current->playerID != playerID) {
            current = current->forward[0];
        }
        if(!current) return;
        target = current;
        vector<Node*> update(maxLevel, nullptr);
        current = head;
        for (int i = currentLevel - 1; i >= 0; i--) {
            while (current->forward[i] && comesBefore(current->forward[i]->score,current->forward[i]->playerID
                , target-> score, target-> playerID)) {
                current = current->forward[i]; }
            update[i] = current;
        }

        int nodeLevel = target -> forward.size();
        for(int i = 0; i < nodeLevel; i++) {
            if(update[i]->forward[i] == target)
               update[i]->forward[i] = target ->forward[i];
        }
        delete target;
        while (currentLevel > 1 && head->forward[currentLevel - 1] == nullptr) {
            currentLevel--;
        }

    }

    vector<int> getTopN(int n) override {
        vector<int> result;
        Node* current = head->forward[0];
        while(current && n--) {
            result.push_back(current->playerID);
            current = current->forward[0];
        }
        return result;
    }
};

// --- 3. AuctionTree (Red-Black Tree) ---

class ConcreteAuctionTree : public AuctionTree {
private:
    // Hint: Each node needs: id, price, color, left, right, parent pointers
    struct RBTNode {
        int id = 0;
        int price = 0;
        bool is_black = false;
        RBTNode *left = nullptr;
        RBTNode *right = nullptr;
        RBTNode *parent = nullptr;
    };

    RBTNode *root;

    void rotateToLeft(RBTNode *&root) {
        if (root == nullptr)
            cout << "Error in the tree rotation: root is null" << endl;
        else if (root->right == nullptr)
            cout << "Error in the tree: No right subtree to rotate." << endl;

        else {
            RBTNode *rightNode = root->right; //pointer to the root of the right subtree of root
            root->right = rightNode->left; //the left subtree of  rightNode become the right subtree of root
            if (root->right != nullptr)
                root->right->parent = root;

            rightNode->left = root;
            rightNode->parent = root->parent;

            if (rightNode->parent != nullptr) {
                // if the nodes im dealing with are a subtree of a bigger one
                if (rightNode->parent->left == root) // is a left subtree
                    rightNode->parent->left = rightNode;
                else
                    rightNode->parent->right = rightNode;
            }

            root->parent = rightNode;
            root = rightNode; //make rightNode the new root node
        }
    }

    void rotateToRight(RBTNode *&root) {
        if (root == nullptr)
            cout << "Error in the tree rotation: root is null" << endl;
        else if (root->left == nullptr)
            cout << "Error in the tree: No left subtree to rotate." << endl;

        else {
            RBTNode *leftNode = root->left;
            root->left = leftNode->right; //the right subtree of  leftNode becomes the left subtree of root

            if (root->left != nullptr)
                root->left->parent = root;

            leftNode->right = root;
            leftNode->parent = root->parent;

            if (leftNode->parent != nullptr) {
                // if the nodes im dealing with are a subtree of a bigger one
                if (leftNode->parent->right == root) // is a left subtree
                    leftNode->parent->right = leftNode;
                else
                    leftNode->parent->left = leftNode;
            }

            root->parent = leftNode;
            root = leftNode; //make  leftNode the new root node
        }
    }

    RBTNode *insert(int itemID, int itemPrice) {
        RBTNode *current;
        RBTNode *tc = nullptr;
        auto *newNode = new RBTNode();

        //making the new node
        if (newNode != nullptr) {
            newNode->id = itemID;
            newNode->price = itemPrice;
            newNode->left = nullptr;
            newNode->right = nullptr;
            newNode->parent = nullptr;
        } else {
            cout << "Error in the tree" << endl;
        }

        if (root == nullptr) {
            root = newNode;
        } else {
            current = root;
            while (current != nullptr) {
                tc = current;
                if (current->price == itemPrice) {
                    if (current->id < itemID) {
                        current = current->right;
                    } else if (current->id > itemID) {
                        current = current->left;
                    } else {
                        cout << "can't insert an element with the same price and ID " << endl;
                    }
                }
                else if (current->price < itemPrice) {
                    current = current->right;
                } else {
                    current = current->left;
                }
            }
            if (tc->price < itemPrice) {
                tc->right = newNode;
                newNode->parent = tc;
            } else if (tc->price > itemPrice) {
                tc->left = newNode;
                newNode->parent = tc;
            }else {
                if (tc->id < itemID) {
                    tc->right = newNode;
                    newNode->parent = tc;
                }else if (tc->id > itemID) {
                    tc->left = newNode;
                    newNode->parent = tc;
                }else {
                    cout << "cannot insert item with the same price and id" << endl;
                }
            }
        }
        return newNode;
    }

    RBTNode* searchById(RBTNode* root, int id) {
        if (root == nullptr) {
            return nullptr;
        }

        if (root->id == id)
            return root;

        RBTNode* leftResult = searchById(root->left, id);
        if (leftResult != nullptr)
            return leftResult;

        return searchById(root->right, id);
    }

    RBTNode* bstDelete(RBTNode*& root, int id) {
        RBTNode* node = searchById(root, id);
        if (node == nullptr) {
            cout << "Error in the tree deletion: node to delete is null" << endl;
            return nullptr;
        }

        RBTNode* removedNode = node;
        RBTNode* temp = nullptr;

        // Case 1 & 2: node has at most one child
        if (node->left == nullptr || node->right == nullptr) {
            temp = (node->right == nullptr) ? node->left : node->right;

            if (node->parent == nullptr) {
                root = temp; // node is root
                if (temp != nullptr) temp->parent = nullptr;

            } else if (node->parent->left == node) { // node is the left child
                 temp = node->parent->left;
            } else {
                 temp = node->parent->right;
            }

            if (temp != nullptr)
                temp->parent = node->parent;

            return temp;
        }

        // Case 3: two children use predecessor
        removedNode = node->left;
        while (removedNode->right != nullptr)
            removedNode = removedNode->right;

        // deletedBlack = removedNode->is_black;
        node->id = removedNode->id;
        node->price = removedNode->price;

        return removedNode;

        temp = removedNode->left;

        if (removedNode->parent->left == removedNode) // pred awl node 3la el shemal
            removedNode->parent->left = temp;
        else
            removedNode->parent->right = temp;

        if (temp != nullptr)
            temp->parent = removedNode->parent;

    }
public:
    ConcreteAuctionTree() {
        root = nullptr;
    }

    void insertItem(int itemID, int price) override {
        // Remember to maintain RB-Tree properties with rotations and recoloring
        RBTNode *newNode = insert(itemID, price);
        newNode->is_black = false; // first insert red node
        while (newNode != root && newNode->parent->is_black == false && newNode->is_black == false) { // parent is red so there is violation

            if (newNode->parent == newNode->parent->parent->left) { // node is left of a left parent so uncle is right
                RBTNode *uncle = newNode->parent->parent->right;
                if (uncle != nullptr && uncle->is_black == false) { // case 1
                    newNode->parent->is_black = true;
                    uncle->is_black = true;
                    newNode->parent->parent->is_black = false;
                    newNode = newNode->parent->parent;
                } else {
                    if (newNode == newNode->parent->right) { // case 2 (near child)
                        newNode = newNode->parent;
                        rotateToLeft(newNode);
                        newNode = newNode->left;

                    } else { // case 3 (far child)
                        newNode->parent->is_black = true;
                        newNode->parent->parent->is_black = false;
                        newNode = newNode->parent->parent;
                        rotateToRight(newNode);
                        newNode = newNode->right;
                    }
                }
            } else {
                RBTNode *uncle = newNode->parent->parent->left;
                if (uncle != nullptr && uncle->is_black == false) {
                    newNode->parent->is_black = true;
                    uncle->is_black = true;
                    newNode->parent->parent->is_black = false;
                    newNode = newNode->parent->parent;
                } else {
                    if (newNode == newNode->parent->left) {
                        newNode = newNode->parent;
                        rotateToRight(newNode);
                        newNode = newNode->right;
                    } else {
                        newNode->parent->is_black = true;
                        newNode->parent->parent->is_black = false;
                        newNode = newNode->parent->parent;
                        rotateToLeft(newNode);
                        newNode = newNode->left;
                    }
                }
            }
        }
        if (root == newNode) {
            if (newNode->parent != nullptr) {
                root = newNode->parent;
            }
            root->is_black = true;
            // delete newNode;
        }
    }

    void deleteItem(int itemID) override {
        // This is complex - handle all cases carefully
        RBTNode *blackToken = bstDelete(root, itemID);

        while (blackToken != root && (blackToken == nullptr || blackToken->is_black == true)) {

            if(blackToken == blackToken->parent->left) { // left child
                RBTNode *brother = blackToken->parent->right;

                if (brother->is_black == false) { // case 1
                    blackToken->parent->is_black = false;
                    brother->is_black = true;
                    rotateToLeft(blackToken->parent);
                    brother = blackToken->parent->right;
                }

                else if ((brother->left == nullptr || brother->left->is_black)
                    && (brother->right == nullptr || brother->right->is_black)) { // case 2
                    brother->is_black = false;
                    blackToken = blackToken->parent;
                }

                else {
                    if (brother->right->is_black || brother->right == nullptr) { // case 3
                        if (brother->left != nullptr) brother->left->is_black = true;
                        brother->is_black = false;
                        rotateToRight(brother);
                        brother = blackToken->parent->right;
                    } else {// case 4
                        if (brother->right != nullptr) brother->right->is_black = true;
                        brother->is_black = blackToken->parent->is_black;
                        blackToken->parent->is_black = true;
                        rotateToLeft(blackToken->parent);
                        blackToken = root; //+++++++++++++++++++++++++++++
                    }
                }
            }
            else {
                RBTNode *brother = blackToken->parent->left;

                // if (!brother) break;

                if (brother->is_black == false) { // case 1
                    blackToken->parent->is_black = false;
                    brother->is_black = true;
                    rotateToRight(blackToken->parent);
                    brother = blackToken->parent->left;
                }

                if ((brother->right->is_black || brother->right == nullptr) &&
                    (brother->left->is_black || brother->left == nullptr)) { // case 2
                    brother->is_black = false;
                    blackToken = blackToken->parent;
                }

                else {
                    if (brother->left->is_black || brother->left == nullptr) { // case 3
                        if (brother->right != nullptr) brother->right->is_black = true;
                        brother->is_black = false;
                        rotateToLeft(brother);
                        brother = blackToken->parent->left;
                    } else {// case 4
                        if (brother->left != nullptr) brother->left->is_black = true;
                        brother->is_black = blackToken->parent->is_black;
                        blackToken->parent->is_black = true;
                        rotateToRight(blackToken->parent);
                        blackToken = root; //+++++++++++++++++++++++++++++
                    }
                }
            }
        }
        if (blackToken) blackToken->is_black = true;
        if (root) root->is_black = true;
    }
};

// =========================================================
// PART B: INVENTORY SYSTEM (Dynamic Programming)
// =========================================================

int InventorySystem::optimizeLootSplit(int n, vector<int> &coins) {
    // TODO: Implement partition problem using DP
    // Goal: Minimize |sum(subset1) - sum(subset2)|
    // Hint: Use subset sum DP to find closest sum to total/2
    return 0;
}

int InventorySystem::maximizeCarryValue(int capacity, vector<pair<int, int> > &items) {
    // TODO: Implement 0/1 Knapsack using DP
    // items = {weight, value} pairs
    // Return maximum value achievable within capacity
    return 0;
}

long long InventorySystem::countStringPossibilities(string s) {
    // TODO: Implement string decoding DP
    // Rules: "uu" can be decoded as "w" or "uu"
    //        "nn" can be decoded as "m" or "nn"
    // Count total possible decodings
    return 0;
}

// =========================================================
// PART C: WORLD NAVIGATOR (Graphs)
// =========================================================

bool WorldNavigator::pathExists(int n, vector<vector<int> > &edges, int source, int dest) {
    // TODO: Implement path existence check using BFS or DFS
    // edges are bidirectional
    return false;
}

long long WorldNavigator::minBribeCost(int n, int m, long long goldRate, long long silverRate,
                                       vector<vector<int> > &roadData) {
    // TODO: Implement Minimum Spanning Tree (Kruskal's or Prim's)
    // roadData[i] = {u, v, goldCost, silverCost}
    // Total cost = goldCost * goldRate + silverCost * silverRate
    // Return -1 if graph cannot be fully connected
    return -1;
}

string WorldNavigator::sumMinDistancesBinary(int n, vector<vector<int> > &roads) {
    // TODO: Implement All-Pairs Shortest Path (Floyd-Warshall)
    // Sum all shortest distances between unique pairs (i < j)
    // Return the sum as a binary string
    // Hint: Handle large numbers carefully
    return "0";
}

// =========================================================
// PART D: SERVER KERNEL (Greedy)
// =========================================================

int ServerKernel::minIntervals(vector<char> &tasks, int n) {
    // TODO: Implement task scheduler with cooling time
    // Same task must wait 'n' intervals before running again
    // Return minimum total intervals needed (including idle time)
    // Hint: Use greedy approach with frequency counting
    return 0;
}

// =========================================================
// FACTORY FUNCTIONS (Required for Testing)
// =========================================================

extern "C" {
PlayerTable *createPlayerTable() {
    return new ConcretePlayerTable();
}

Leaderboard *createLeaderboard() {
    return new ConcreteLeaderboard();
}

AuctionTree *createAuctionTree() {
    return new ConcreteAuctionTree();
}
}
