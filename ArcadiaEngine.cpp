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
	int sanitizeScore(int score) {
        return (score < 0) ? 0 : score;
		cout<< "negative score will be 0";
    }

public:
    ConcreteLeaderboard() : maxLevel(16), currentLevel(1) {
        head = new Node(maxLevel, -1, INT_MAX);
    }

    void addScore(int playerID, int score) override {
		score = sanitizeScore(score);
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
        vector<int> result(n, -1);
        Node* current = head->forward[0];

        int index = 0;
        while (current && index < n) {
            result[index++] = current->playerID;
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
        RBTNode *temp = blackToken;

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
                        blackToken = root;
                    }
                }
            }
            else {
                RBTNode *brother = blackToken->parent->left;

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
                        blackToken = root;
                    }
                }
            }
        }
        if (blackToken) blackToken->is_black = true;
        if (temp != nullptr && temp->parent != nullptr) {
            if (temp->parent->left == temp)
                temp->parent->left = nullptr;
            else if (temp->parent->right == temp)
                temp->parent->right = nullptr;
        }
        if (root) root->is_black = true;
    }
};

// =========================================================
// PART B: INVENTORY SYSTEM (Dynamic Programming)
// =========================================================

static int maximizeCarryValue(int capacity, vector<pair<int, int>> &items) {
	if (capacity <= 0 || items.empty()) { 
		return 0;
	};
	int n = items.size();
	vector<int> packback(capacity + 1, 0);

	for (const auto& item : items) {
		int weight = item.first;
		int value = item.second;

		for (int w = capacity; w >= weight; w--) {
			packback[w] = max(packback[w], packback[w - weight] + value);
		}
	}

	return packback[capacity];
}

int InventorySystem::optimizeLootSplit(int n, vector<int> &coins) {
        int total = 0;
		for (int i = 0; i < n; i++) {
			total += coins[i];
		}
		int half = total / 2;
		vector <bool> possible(half + 1, false);
		possible[0] = true;
		for (int item : coins) {
			for (int j = half; j >= item; j--) {
				possible[j] = possible[j] || possible[j - item];
			}
		}
		for (int j = half; j >= 0; j--) {
			if (possible[j]) {
				return total - (2 * j);
			}
		}
		return total;
}

int InventorySystem::maximizeCarryValue(int capacity, vector<pair<int, int> > &items) {
    	int n = items.size();
		vector<int> packback(capacity + 1, 0);

		for (int i = 0; i < n; i++) {
			int weight = items[i].first;
			int value = items[i].second;

			for (int w = capacity; w >= weight; w--) {
				packback[w] = max(packback[w], packback[w - weight] + value);
			}
		}

		return packback[capacity];
	}


long long InventorySystem::countStringPossibilities(string s) {
        static const int mod = 1e9 + 7;
        int n = s.size();
		vector <long long> str(n + 1, 0);
		str[0] = 1;
		for (int i = 1; i <= n; i++) {
			str[i] = str[i - 1] % mod;
			if (i >= 2 && s.substr(i - 2, 2) == "uu") {
				str[i] = (str[i] + str[i - 2]) % mod;
			}
			if (i >= 2 && s.substr(i - 2, 2) == "nn") {
				str[i] = (str[i] + str[i - 2]) % mod;
			}
		}
		return str[n];
}

// =========================================================
// PART C: WORLD NAVIGATOR (Graphs)
// =========================================================

bool WorldNavigator::pathExists(int n, vector<vector<int>>& edges, int source, int dest) {

    if(source < 0 || source >= n || dest < 0 || dest >= n) return false;
    if (source == dest) return true;

    vector<vector<int>> adjacencyList(n);
    for(int i = 0; i < edges.size(); i++){
        int u = edges[i][0];
        int v = edges[i][1];
        adjacencyList[u].push_back(v);
        adjacencyList[v].push_back(u); // undirected graph
    }

    vector<bool> visited(n, false);
    queue<int> q;

    visited[source] = true;
    q.push(source);
    
    while(!q.empty()){
        int u = q.front();
        q.pop();
        for (int v : adjacencyList[u]){
            if (!visited[v]){
                if (v == dest) return true;
                visited[v] = true;
                q.push(v);
            }
        }
    }
    return false;
}

class DisSubset{
    
private:
    vector<int> parent;
public:
    DisSubset(int n){
        parent.resize(n,-1);
    }

    int find(int u){
        if(parent[u] < 0){
            return u;
        }
        return parent[u] = find(parent[u]);
    }

    bool Union(int x, int y){
        int xRoot = find(x);
        int yRoot = find(y);
        if(xRoot == yRoot){
            return false;
        }
        if(parent[xRoot] < parent[yRoot]){
            parent[xRoot] += parent[yRoot];
            parent[yRoot] = xRoot;
        }else{
            parent[yRoot] += parent[xRoot];
            parent[xRoot] = yRoot;
        }
        return true;
    }

};

long long WorldNavigator::minBribeCost(int n, int m, long long goldRate, long long silverRate,vector<vector<int>>& roadData) {

    struct Edge {
        int u, v;
        long long totalCost;
        bool operator<(const Edge& other) const {
            return totalCost < other.totalCost;
        }
    };

    vector<Edge> edges;

    for(int i = 0; i < roadData.size(); i++){
        int u = roadData[i][0];
        int v = roadData[i][1];
        long long goldCost = roadData[i][2];
        long long silverCost = roadData[i][3];
        long long cost = goldCost * goldRate + silverCost * silverRate;
        edges.push_back({u, v, cost});
    }

    sort(edges.begin(), edges.end());

    DisSubset ds(n);
    long long totalCost = 0;
    int numEdges = 0;

    for(int i = 0; i < edges.size(); i++){
        int u = edges[i].u;
        int v = edges[i].v;
        long long edgeCost = edges[i].totalCost;

        if(ds.find(u) != ds.find(v)){
            ds.Union(u, v);
            totalCost += edgeCost;
            numEdges++;
            
            if(numEdges == n - 1){
                return totalCost;
            }
        }
    }

    return -1;
}

string WorldNavigator::sumMinDistancesBinary(int n, vector<vector<int>>& roads) {

    const long long inf = 1e18;

    vector<vector<long long>> dist(n, vector<long long>(n, inf));
    for(int i = 0; i < n; i++){
        dist[i][i] = 0;
    }

    for( int i = 0; i < roads.size(); i++){
        int u = roads[i][0];
        int v = roads[i][1];
        long long w = roads[i][2];
        dist[u][v] = w; 
    }
    for(int k = 0; k < n; k++){
        for(int i = 0; i < n; i++){
            if(dist[i][k] == inf) continue;
            for(int j = 0; j < n; j++){
                if(dist[k][j] == inf) continue;
                dist[i][j] = min(dist[i][j], dist[i][k] + dist[k][j]);
            }
        }
    }
    long long sum = 0;
    for(int i = 0; i < n; i++){
        for(int j = i + 1; j < n; j++){
            if(dist[i][j] == inf) continue;
            sum += dist[i][j];
        }
    }

    string binary = "";
    if(sum == 0) return "0";

    while(sum > 0){
        if(sum % 2 == 1){
            binary = "1" + binary;
        }else{
            binary = "0" + binary;
        }
        sum /= 2;
    }

    return binary;
}

// =========================================================
// PART D: SERVER KERNEL (Greedy)
// =========================================================

int ServerKernel::minIntervals(vector<char> &tasks, int n) {
     if (tasks.empty() || tasks.size() > 10000) {
        return 0;  
    }

    if (n < 0) n = 0;
    if (n > 100) n = 100;

      for (char &t : tasks) {
    if (t >= 'a' && t <= 'z') {
        t = t - 'a' + 'A';   
    }
    else if (t < 'A' || t > 'Z') {
        return 0;           
    }
    map<char, int> freq;
    for (auto task : tasks) {
        freq[task]++;
    }

    vector<pair<char, int>> freq_list(freq.begin(), freq.end());

    //greedy approach to place the most frequent tasks first
    sort(freq_list.begin(), freq_list.end(),
         [](auto &a, auto &b) {
             return a.second > b.second;
         });

    int max_freq = freq_list[0].second; // Highest frequency
    // decode the empty slots to be filled between the most frequent tasks

    int interval_needed = (max_freq - 1) * (n + 1) + 1;
    int empty_slots = (max_freq - 1) * n;

    for (int i = 1; i < freq_list.size(); i++) { //start from the second index because the first is already considered

        int current_freq = freq_list[i].second; // Current task frequency

        if (empty_slots == 0) {
            interval_needed += current_freq;
            continue;
        }

        if (current_freq == max_freq) { // If current task has the same max frequency
            interval_needed += 1;
            empty_slots -= (max_freq - 1);
        }
        else {
            if (current_freq > empty_slots) {
                interval_needed += current_freq - empty_slots;
                empty_slots = 0;
            } else {
                empty_slots -= current_freq;
            }
        }
    }

    return interval_needed;
}
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
