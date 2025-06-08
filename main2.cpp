#include "main2.h"

char toLower(char c) {
    return tolower(c);
}

WordNode::WordNode(const std::string& w) : word(w), next(nullptr) {}

Graph::Graph(int vertices) : numVertices(vertices), 
                         adjacencyMatrix(vertices, std::vector<int>(vertices, 0)) {
    if (vertices > MAX_VERTICES) {
        std::cerr << "Number of vertices exceeds maximum limit" << std::endl;
        exit(EXIT_FAILURE);
    }
}

int WordTable::addWord(const std::string& word) {
    auto it = wordToIndex.find(word);
    if (it != wordToIndex.end()) {
        return it->second;
    }
    
    if (words.size() >= MAX_VERTICES) {
        std::cerr << "Warning: Word table full, cannot add more words" << std::endl;
        return -1;
    }
    
    words.push_back(word);
    wordToIndex[word] = words.size() - 1;
    return words.size() - 1;
}

int WordTable::getIndex(const std::string& word) const {
    auto it = wordToIndex.find(word);
    return it != wordToIndex.end() ? it->second : -1;
}

size_t WordTable::size() const { return words.size(); }

std::string processTextFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        perror("Error opening file");
        return "";
    }

    file.seekg(0, std::ios::end);
    size_t file_size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::string result;
    result.reserve(file_size * 2);
    bool in_word = false;

    char c;
    while (file.get(c)) {
        if (isalpha(c)) {
            result += toLower(c);
            in_word = true;
        } else {
            if (in_word) {
                result += ' ';
                in_word = false;
            }
        }
    }

    if (in_word) {
        result += ' ';
    }

    return result;
}

WordNode* sentenceToList(const std::string& sentence) {
    if (sentence.empty()) {
        return nullptr;
    }

    WordNode* head = nullptr;
    WordNode* tail = nullptr;
    std::string current_word;
    current_word.reserve(MAX_WORD_LEN);

    for (char c : sentence) {
        if (isspace(c)) {
            if (!current_word.empty()) {
                WordNode* new_node = new WordNode(current_word);
                
                if (!head) {
                    head = tail = new_node;
                } else {
                    tail->next = new_node;
                    tail = new_node;
                }
                
                current_word.clear();
            }
        } else {
            if (current_word.length() < MAX_WORD_LEN - 1) {
                current_word += toLower(c);
            } else if (current_word.length() == MAX_WORD_LEN - 1) {
                WordNode* new_node = new WordNode(current_word);
                
                if (!head) {
                    head = tail = new_node;
                } else {
                    tail->next = new_node;
                    tail = new_node;
                }
                
                current_word.clear();
                current_word += toLower(c);
            }
        }
    }

    if (!current_word.empty()) {
        WordNode* new_node = new WordNode(current_word);
        
        if (!head) {
            head = new_node;
        } else {
            tail->next = new_node;
        }
    }

    return head;
}

void printList(WordNode* head) {
    WordNode* current = head;
    while (current) {
        std::cout << current->word;
        if (current->next) {
            std::cout << " -> ";
        }
        current = current->next;
    }
    std::cout << std::endl;
}

void populateWordTable(WordNode* head, WordTable& table) {
    if (!head) return;

    WordNode* temp1 = head;
    WordNode* temp2 = head->next;
    
    while (temp1 && temp2) {
        table.addWord(temp1->word);
        table.addWord(temp2->word);
        
        temp1 = temp1->next;
        temp2 = temp2->next;
    }
}

void buildGraph(WordNode* head, Graph& graph, WordTable& table) {
    if (!head) return;

    WordNode* temp1 = head;
    WordNode* temp2 = head->next;
    
    while (temp1 && temp2) {
        int id1 = table.getIndex(temp1->word);
        int id2 = table.getIndex(temp2->word);
        
        if (id1 != -1 && id2 != -1 && id1 < graph.numVertices && id2 < graph.numVertices) {
            graph.adjacencyMatrix[id1][id2]++;
        }
        
        temp1 = temp1->next;
        temp2 = temp2 ? temp2->next : nullptr;
    }
}

void exportToDot(const Graph& graph, const WordTable& table, const std::string& filename) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        perror("Failed to open file");
        return;
    }

    file << "digraph G {\n";
    file << "  rankdir=LR;\n";
    file << "  node [shape=circle];\n";

    for (int i = 0; i < graph.numVertices; i++) {
        for (int j = 0; j < graph.numVertices; j++) {
            int weight = graph.adjacencyMatrix[i][j];
            if (weight != 0) {
                file << "  \"" << table.words[i] << "\" -> \"" << table.words[j] 
                     << "\" [label=\"" << weight << "\"];\n";
            }
        }
    }

    file << "}\n";
}

void printAdjacencyMatrix(const Graph& graph, const WordTable& table) {
    std::cout << "Adjacency Matrix:\n";
    
    std::cout << "    ";
    for (int j = 0; j < graph.numVertices; j++) {
        std::cout << std::left << std::setw(5) << table.words[j] << " ";
    }
    std::cout << std::endl;
    
    for (int i = 0; i < graph.numVertices; i++) {
        std::cout << std::left << std::setw(5) << table.words[i] << " ";
        for (int j = 0; j < graph.numVertices; j++) {
            std::cout << std::left << std::setw(5) << graph.adjacencyMatrix[i][j] << " ";
        }
        std::cout << std::endl;
    }
}

void findBridgeWords(const Graph& graph, const WordTable& table, const std::string& word1, const std::string& word2) {
    int id1 = table.getIndex(word1);
    int id2 = table.getIndex(word2);

    if (id1 == -1 && id2 == -1) {
        std::cout << "No " << word1 << " and " << word2 << " in the graph!" << std::endl;
        return;
    } else if (id1 == -1) {
        std::cout << "No " << word1 << " in the graph!" << std::endl;
        return;
    } else if (id2 == -1) {
        std::cout << "No " << word2 << " in the graph!" << std::endl;
        return;
    }

    std::vector<std::string> bridge_words;

    for (int i = 0; i < graph.numVertices; i++) {
        if (graph.adjacencyMatrix[id1][i] > 0 && graph.adjacencyMatrix[i][id2] > 0) {
            bridge_words.push_back(table.words[i]);
        }
    }

    if (bridge_words.empty()) {
        std::cout << "No bridge words from " << word1 << " to " << word2 << "!" << std::endl;
    } else {
        std::cout << "The bridge words from " << word1 << " to " << word2 << " are: ";
        for (size_t i = 0; i < bridge_words.size(); i++) {
            std::cout << bridge_words[i];
            if (i < bridge_words.size() - 2) {
                std::cout << ", ";
            } else if (i == bridge_words.size() - 2) {
                std::cout << ", and ";
            }
        }
        std::cout << "." << std::endl;
    }
}


std::string selectRandomBridgeWord(const Graph& graph, const WordTable& table, int id1, int id2) {
    std::vector<int> bridge_indices;
    
    for (int i = 0; i < graph.numVertices; i++) {
        if (graph.adjacencyMatrix[id1][i] > 0 && graph.adjacencyMatrix[i][id2] > 0) {
            bridge_indices.push_back(i);
        }
    }
    
    if (bridge_indices.empty()) {
        return "";
    }
    
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, bridge_indices.size() - 1);
    
    return table.words[bridge_indices[dis(gen)]];
}

void generateNewText(const Graph& graph, const WordTable& table, const std::string& input_text) {
    if (input_text.empty()) {
        std::cout << "Empty input text." << std::endl;
        return;
    }
    
    std::string processed_input;
    processed_input.reserve(input_text.size());
    
    for (char c : input_text) {
        if (isalpha(c)) {
            processed_input += toLower(c);
        } else if (!processed_input.empty() && processed_input.back() != ' ') {
            processed_input += ' ';
        }
    }
    
    WordNode* input_list = sentenceToList(processed_input);
    if (!input_list) {
        std::cout << "No valid words in input text." << std::endl;
        return;
    }
    
    std::string new_text;
    new_text.reserve(input_text.size() * 2);
    
    WordNode* current = input_list;
    WordNode* next = current->next;
    size_t original_pos = 0;
    
    while (current && next) {
        while (original_pos < input_text.size() && !isalpha(input_text[original_pos])) {
            new_text += input_text[original_pos++];
        }
        
        size_t word_start = original_pos;
        while (original_pos < input_text.size() && isalpha(input_text[original_pos])) {
            new_text += input_text[original_pos++];
        }
        
        std::string lower_current = current->word;
        std::string lower_next = next->word;
        
        int id1 = table.getIndex(lower_current);
        int id2 = table.getIndex(lower_next);
        
        if (id1 != -1 && id2 != -1) {
            std::string bridge_word = selectRandomBridgeWord(graph, table, id1, id2);
            if (!bridge_word.empty()) {
                new_text += ' ';
                
                if (word_start < input_text.size() && isupper(input_text[word_start])) {
                    bridge_word[0] = toupper(bridge_word[0]);
                }
                
                new_text += bridge_word;
            }
        }
        
        current = next;
        next = next->next;
    }
    
    while (original_pos < input_text.size()) {
        new_text += input_text[original_pos++];
    }
    
    std::cout << "Generated new text: " << new_text << std::endl;
    
    while (input_list) {
        WordNode* next_node = input_list->next;
        delete input_list;
        input_list = next_node;
    }
}

void backtrackPaths(const Graph& graph, int u, int v, const std::vector<int>& dist, 
                   std::vector<int>& path, PathList& path_list) {
    path.push_back(u);
    
    if (u == v) {
        if (path_list.paths.size() < MAX_PATHS) {
            path_list.paths.push_back(path);
            path_list.path_lengths.push_back(path.size());
        }
    } else {
        for (int i = 0; i < graph.numVertices; i++) {
            if (graph.adjacencyMatrix[u][i] > 0 && dist[i] == dist[u] + graph.adjacencyMatrix[u][i]) {
                backtrackPaths(graph, i, v, dist, path, path_list);
            }
        }
    }
    
    path.pop_back();
}

PathList findAllShortestPaths(const Graph& graph, int id1, int id2, const std::vector<int>& dist) {
    PathList path_list;
    std::vector<int> path;
    
    backtrackPaths(graph, id1, id2, dist, path, path_list);
    return path_list;
}

void showShortestPath(const Graph& graph, const WordTable& table, 
                     const std::string& word1, const std::string& word2) {
    int id1 = table.getIndex(word1);
    if (id1 == -1) {
        std::cout << "No " << word1 << " in the graph!" << std::endl;
        return;
    }
    
    if (word2.empty()) {
        std::cout << "Calculating shortest paths from '" << word1 << "' to all other words:" << std::endl;
        
        std::vector<int> dist(graph.numVertices, INT_MAX);
        std::vector<int> prev(graph.numVertices, -1);
        std::vector<bool> visited(graph.numVertices, false);
        
        dist[id1] = 0;
        
        for (int count = 0; count < graph.numVertices - 1; count++) {
            int min_dist = INT_MAX;
            int u = -1;
            
            for (int v = 0; v < graph.numVertices; v++) {
                if (!visited[v] && dist[v] < min_dist) {
                    min_dist = dist[v];
                    u = v;
                }
            }
            
            if (u == -1) break;
            visited[u] = true;
            
            for (int v = 0; v < graph.numVertices; v++) {
                if (!visited[v] && graph.adjacencyMatrix[u][v] > 0 && 
                    dist[u] != INT_MAX && dist[u] + graph.adjacencyMatrix[u][v] < dist[v]) {
                    dist[v] = dist[u] + graph.adjacencyMatrix[u][v];
                    prev[v] = u;
                }
            }
        }
        
        for (int i = 0; i < graph.numVertices; i++) {
            if (i == id1) continue;
            
            if (dist[i] == INT_MAX) {
                std::cout << "No path from " << word1 << " to " << table.words[i] << "!" << std::endl;
                continue;
            }
            
            std::vector<int> path;
            for (int at = i; at != -1; at = prev[at]) {
                path.push_back(at);
            }
            std::reverse(path.begin(), path.end());
            
            std::cout << "Shortest path from " << word1 << " to " << table.words[i] 
                 << " (length " << dist[i] << "):" << std::endl;
            for (size_t j = 0; j < path.size(); j++) {
                std::cout << table.words[path[j]];
                if (j < path.size() - 1) {
                    std::cout << " -> ";
                }
            }
            std::cout << std::endl;
        }
        return;
    }
    
    int id2 = table.getIndex(word2);
    if (id2 == -1) {
        std::cout << "No " << word2 << " in the graph!" << std::endl;
        return;
    }
    
    std::vector<int> dist(graph.numVertices, INT_MAX);
    std::vector<int> prev(graph.numVertices, -1);
    std::vector<bool> visited(graph.numVertices, false);
    
    dist[id1] = 0;
    
    for (int count = 0; count < graph.numVertices - 1; count++) {
        int min_dist = INT_MAX;
        int u = -1;
        
        for (int v = 0; v < graph.numVertices; v++) {
            if (!visited[v] && dist[v] < min_dist) {
                min_dist = dist[v];
                u = v;
            }
        }
        
        if (u == -1) break;
        visited[u] = true;
        
        for (int v = 0; v < graph.numVertices; v++) {
            if (!visited[v] && graph.adjacencyMatrix[u][v] > 0 && 
                dist[u] != INT_MAX && dist[u] + graph.adjacencyMatrix[u][v] < dist[v]) {
                dist[v] = dist[u] + graph.adjacencyMatrix[u][v];
                prev[v] = u;
            }
        }
    }
    
    if (dist[id2] == INT_MAX) {
        std::cout << "No path from " << word1 << " to " << word2 << "!" << std::endl;
        return;
    }
    
    PathList path_list = findAllShortestPaths(graph, id1, id2, dist);
    
    if (path_list.paths.empty()) {
        std::cout << "No path found (should not happen)" << std::endl;
        return;
    }
    
    std::cout << "Found " << path_list.paths.size() << " shortest path(s) from " 
         << word1 << " to " << word2 << " (length " << dist[id2] << "):" << std::endl;
    
    for (size_t i = 0; i < path_list.paths.size(); i++) {
        std::cout << "Path " << i+1 << ": ";
        for (size_t j = 0; j < path_list.paths[i].size(); j++) {
            std::cout << table.words[path_list.paths[i][j]];
            if (j < path_list.paths[i].size() - 1) {
                std::cout << " -> ";
            }
        }
        std::cout << std::endl;
    }
    
    std::ofstream file("shortest_path.dot");
    if (!file.is_open()) {
        perror("Failed to open file");
        return;
    }
    
    file << "digraph G {\n";
    file << "  rankdir=LR;\n";
    file << "  node [shape=circle];\n";
    
    const std::vector<std::string> path_colors = {
        "red", "blue", "green", "purple", "orange",
        "brown", "pink", "cyan", "magenta", "gold"
    };
    
    for (int i = 0; i < graph.numVertices; i++) {
        for (int j = 0; j < graph.numVertices; j++) {
            int weight = graph.adjacencyMatrix[i][j];
            if (weight != 0) {
                bool is_path_edge = false;
                size_t path_index = 0;
                
                for (size_t k = 0; k < path_list.paths.size(); k++) {
                    for (size_t l = 0; l < path_list.paths[k].size() - 1; l++) {
                        if (path_list.paths[k][l] == i && path_list.paths[k][l+1] == j) {
                            is_path_edge = true;
                            path_index = k;
                            break;
                        }
                    }
                    if (is_path_edge) break;
                }
                
                if (is_path_edge) {
                    file << "  \"" << table.words[i] << "\" -> \"" << table.words[j]
                         << "\" [label=\"" << weight << "\", color=" << path_colors[path_index % path_colors.size()] 
                         << ", penwidth=2.0, style=bold];\n";
                } else {
                    file << "  \"" << table.words[i] << "\" -> \"" << table.words[j]
                         << "\" [label=\"" << weight << "\"];\n";
                }
            }
        }
    }
    
    file << "  \"" << table.words[id1] << "\" [color=green, penwidth=2.0];\n";
    file << "  \"" << table.words[id2] << "\" [color=blue, penwidth=2.0];\n";
    
    file << "}\n";
    
    std::cout << "Shortest path visualization generated with " << path_list.paths.size() 
         << " paths. Run: dot -Tpng shortest_path.dot -o shortest_path.png" << std::endl;
}

void calculatePageRank(const Graph& graph, const WordTable& table) {
    if (graph.numVertices == 0) {
        std::cout << "Graph is empty!" << std::endl;
        return;
    }

    int numVertices = graph.numVertices;
    std::vector<double> pr(numVertices, 1.0 / numVertices);
    std::vector<double> new_pr(numVertices);
    std::vector<int> out_degree(numVertices, 0);
    bool has_out_degree_zero = false;

    for (int i = 0; i < numVertices; i++) {
        for (int j = 0; j < numVertices; j++) {
            if (graph.adjacencyMatrix[i][j] > 0) {
                out_degree[i] += graph.adjacencyMatrix[i][j];
            }
        }
        
        if (out_degree[i] == 0) {
            has_out_degree_zero = true;
        }
    }

    if (has_out_degree_zero) {
        std::cout << "Warning: Graph contains dangling nodes (nodes with out-degree=0)" << std::endl;
    }

    int iter;
    double diff;
    for (iter = 0; iter < MAX_ITERATIONS; iter++) {
        diff = 0.0;
        
        double dangling_contribution = 0.0;
        for (int i = 0; i < numVertices; i++) {
            if (out_degree[i] == 0) {
                dangling_contribution += pr[i] / numVertices;
            }
        }

        for (int j = 0; j < numVertices; j++) {
            new_pr[j] = (1.0 - DAMPING_FACTOR) / numVertices;
            
            for (int i = 0; i < numVertices; i++) {
                if (graph.adjacencyMatrix[i][j] > 0 && out_degree[i] > 0) {
                    new_pr[j] += DAMPING_FACTOR * pr[i] * graph.adjacencyMatrix[i][j] / out_degree[i];
                }
            }
            
            new_pr[j] += DAMPING_FACTOR * dangling_contribution;
            
            diff += abs(new_pr[j] - pr[j]);
        }

        if (diff < TOLERANCE) {
            break;
        }

        pr = new_pr;
    }

    std::cout << "PageRank converged after " << iter << " iterations:" << std::endl;
    for (int i = 0; i < numVertices; i++) {
        std::cout << table.words[i] << ": " << std::fixed << std::setprecision(6) << pr[i] << std::endl;
    }
}

void randomWalk(const Graph& graph, const WordTable& table) {
    if (graph.numVertices == 0) {
        std::cout << "Graph is empty!" << std::endl;
        return;
    }
    std::ofstream walk_file("random_walk.txt");
    if (!walk_file.is_open()) {
        perror("Failed to create walk file");
        return;
    }
    std::cout << "Starting random walk (press 'q' and Enter to stop)..." << std::endl;
    walk_file << "Random walk traversal:\n";
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, graph.numVertices - 1);
    int current = dis(gen);
    std::cout << "Start at: " << table.words[current] << std::endl;
    walk_file << "Start at: " << table.words[current] << std::endl;
    std::vector<std::vector<bool>> visited_edges(graph.numVertices, std::vector<bool>(graph.numVertices, false));
    int steps = 0;
    bool stop = false;
    while (!stop) {
        if (std::cin.peek() == 'q' || std::cin.peek() == 'Q') {
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "\nUser requested stop." << std::endl;
            walk_file << "\nUser requested stop after " << steps << " steps." << std::endl;
            stop = true;
            break;
        }
        bool has_out_edge = false;
        for (int i = 0; i < graph.numVertices; i++) {
            if (graph.adjacencyMatrix[current][i] > 0) {
                has_out_edge = true;
                break;
            }
        }
        if (!has_out_edge) {
            std::cout << "\nStopped at node " << table.words[current] 
                 << " (no outgoing edges)." << std::endl;
            walk_file << "\nStopped at node " << table.words[current] 
                      << " (no outgoing edges)." << std::endl;
            break;
        }
        std::vector<int> out_edges;
        for (int i = 0; i < graph.numVertices; i++) {
            if (graph.adjacencyMatrix[current][i] > 0) {
                out_edges.push_back(i);
            }
        }
        std::uniform_int_distribution<> edge_dis(0, out_edges.size() - 1);
        int next = out_edges[edge_dis(gen)];
        steps++;
        if (visited_edges[current][next]) {
            std::cout << "\nStopped at edge " << table.words[current] << " -> " 
                 << table.words[next] << " (repeated edge)." << std::endl;
            walk_file << "\nStopped at edge " << table.words[current] << " -> " 
                      << table.words[next] << " (repeated edge)." << std::endl;
            break;
        }
        visited_edges[current][next] = true;
        std::cout << "Step " << steps << ": " << table.words[current] << " -> " 
             << table.words[next] << std::endl;
        walk_file << "Step " << steps << ": " << table.words[current] << " -> " 
                  << table.words[next] << std::endl;
        current = next;
    }
    std::cout << "Random walk completed. Total steps: " << steps << std::endl;
    walk_file << "Random walk completed. Total steps: " << steps << std::endl;
    walk_file.close();
    std::cout << "Walk results saved to random_walk.txt" << std::endl;
}

// int main() {
//     std::srand(std::time(nullptr));
    
//     std::string processed_text = processTextFile("Easy Test.txt");
//     if (!processed_text.empty()) {
//         std::cout << "Processed text: " << processed_text << std::endl;
        
//         WordNode* word_list = sentenceToList(processed_text);
//         WordTable table;
        
//         populateWordTable(word_list, table);
        
//         if (table.size() > 0) {
//             Graph graph(table.size());
//             buildGraph(word_list, graph, table);
            
//             exportToDot(graph, table, "graph.dot");
//             std::cout << "DOT file generated. Run: dot -Tpng graph.dot -o graph.png" << std::endl;
            
//             int choice;
//             std::string word1, word2;
//             std::string input_text;
            
//             while (true) {
//                 std::cout << "\nChoose an option:\n";
//                 std::cout << "1. Find bridge words between two words\n";
//                 std::cout << "2. Generate new text with bridge words\n";
//                 std::cout << "3. Calculate shortest path between two words\n";
//                 std::cout << "4. Calculate shortest paths from one word to all others\n";
//                 std::cout << "5. Calculate PageRank\n";
//                 std::cout << "6. Perform random walk\n";
//                 std::cout << "0. Exit\n";
//                 std::cout << "Enter your choice (0-6): ";
                
//                 if (!(std::cin >> choice)) {
//                     std::cin.clear();
//                     std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
//                     std::cout << "Invalid input. Please enter a number." << std::endl;
//                     continue;
//                 }
                
//                 switch (choice) {
//                     case 0:
//                         std::cout << "Exiting program." << std::endl;
//                         while (word_list) {
//                             WordNode* next_node = word_list->next;
//                             delete word_list;
//                             word_list = next_node;
//                         }
//                         return 0;
                        
//                     case 1:
//                         std::cout << "Enter two words (separated by space): ";
//                         std::cin >> word1 >> word2;
//                         std::transform(word1.begin(), word1.end(), word1.begin(), ::tolower);
//                         std::transform(word2.begin(), word2.end(), word2.begin(), ::tolower);
//                         findBridgeWords(graph, table, word1, word2);
//                         break;
                        
//                     case 2:
//                         std::cout << "Enter a line of text: ";
//                         std::cin.ignore();
//                         std::getline(std::cin, input_text);
//                         generateNewText(graph, table, input_text);
//                         break;
                        
//                     case 3:
//                         std::cout << "Enter two words (separated by space): ";
//                         std::cin >> word1 >> word2;
//                         std::transform(word1.begin(), word1.end(), word1.begin(), ::tolower);
//                         std::transform(word2.begin(), word2.end(), word2.begin(), ::tolower);
//                         showShortestPath(graph, table, word1, word2);
//                         break;
                        
//                     case 4:
//                         std::cout << "Enter a word: ";
//                         std::cin >> word1;
//                         std::transform(word1.begin(), word1.end(), word1.begin(), ::tolower);
//                         showShortestPath(graph, table, word1);
//                         break;
                        
//                     case 5:
//                         calculatePageRank(graph, table);
//                         break;
                        
//                     case 6:
//                         randomWalk(graph, table);
//                         break;
                        
//                     default:
//                         std::cout << "Invalid choice. Please enter a number between 0 and 6." << std::endl;
//                 }
                
//                 std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
//             }
//         }
//     }
    
//     return 0;
// }