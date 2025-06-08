#ifndef MAIN2_H
#define MAIN2_H

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cctype>
#include <ctime>
#include <climits>
#include <cmath>
#include <algorithm>
#include <unordered_map>
#include <queue>
#include <random>
#include <iomanip>

const int MAX_VERTICES = 100;
const int MAX_WORD_LEN = 20;
const int MAX_PATHS = 10;
const double DAMPING_FACTOR = 0.85;
const int MAX_ITERATIONS = 100;
const double TOLERANCE = 1e-6;

// Convert character to lowercase
char toLower(char c);

// WordNode class for linked list
class WordNode {
public:
    std::string word;
    WordNode* next;

    WordNode(const std::string& w);
};

// Graph class using adjacency matrix
class Graph {
public:
    int numVertices;
    std::vector<std::vector<int>> adjacencyMatrix;

    Graph(int vertices);
};

// WordTable class to maintain word to index mapping
class WordTable {
public:
    std::vector<std::string> words;
    std::unordered_map<std::string, int> wordToIndex;

    int addWord(const std::string& word);
    int getIndex(const std::string& word) const;
    size_t size() const;
};

// PathList structure for storing multiple paths
struct PathList {
    std::vector<std::vector<int>> paths;
    std::vector<int> path_lengths;
};

// Function declarations
std::string processTextFile(const std::string& filename);
WordNode* sentenceToList(const std::string& sentence);
void printList(WordNode* head);
void populateWordTable(WordNode* head, WordTable& table);
void buildGraph(WordNode* head, Graph& graph, WordTable& table);
void exportToDot(const Graph& graph, const WordTable& table, const std::string& filename);
void printAdjacencyMatrix(const Graph& graph, const WordTable& table);
void findBridgeWords(const Graph& graph, const WordTable& table, const std::string& word1, const std::string& word2);
std::string selectRandomBridgeWord(const Graph& graph, const WordTable& table, int id1, int id2);
void generateNewText(const Graph& graph, const WordTable& table, const std::string& input_text);
void backtrackPaths(const Graph& graph, int u, int v, const std::vector<int>& dist, 
                   std::vector<int>& path, PathList& path_list);
PathList findAllShortestPaths(const Graph& graph, int id1, int id2, const std::vector<int>& dist);
void showShortestPath(const Graph& graph, const WordTable& table, 
                     const std::string& word1, const std::string& word2 = "");
void calculatePageRank(const Graph& graph, const WordTable& table);
void randomWalk(const Graph& graph, const WordTable& table);

#endif // MAIN2_H