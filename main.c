#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <limits.h>
#include <math.h>
#include <stdbool.h>

#define MAX_VERTICES 100
#define MAX_WORD_LEN 20
#define MAX_PATHS 10  // 最多记录的最短路径数量
#define DAMPING_FACTOR 0.85 // PageRank的阻尼系数
#define MAX_ITERATIONS 100 // PageRank最大迭代次数
#define TOLERANCE 1e-6 // PageRank收敛阈值


// 大小写转换函数
char tran(char c) {
    if (c >= 'A' && c <= 'Z') {
        return c + 32;
    }
    return c;
}

// 定义链表节点结构
typedef struct WordNode {
    char word[MAX_WORD_LEN];  // 存储单词
    struct WordNode* next;    // 指向下一个节点
} WordNode;

// 创建新节点
WordNode* create_node(const char* word) {
    WordNode* new_node = (WordNode*)malloc(sizeof(WordNode));
    if (new_node == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }
    
    strncpy(new_node->word, word, MAX_WORD_LEN - 1);
    new_node->word[MAX_WORD_LEN - 1] = '\0';  // 确保字符串终止
    new_node->next = NULL;
    return new_node;
}

// 处理之后的句子转换成链表连接单词
WordNode* sentence_to_list(const char* sentence) {
    if (sentence == NULL || *sentence == '\0') {
        return NULL;
    }

    WordNode* head = NULL;    // 链表头指针
    WordNode* tail = NULL;    // 链表尾指针
    char current_word[MAX_WORD_LEN];
    int word_pos = 0;

    for (int i = 0; sentence[i] != '\0'; i++) {
        if (isspace(sentence[i])) {
            if (word_pos > 0) {  // 遇到空格且当前有单词
                current_word[word_pos] = '\0';
                
                WordNode* new_node = create_node(current_word);
                
                if (head == NULL) {
                    head = tail = new_node;
                } else {
                    tail->next = new_node;
                    tail = new_node;
                }
                
                word_pos = 0;  // 重置单词位置
            }
        } else {
            if (word_pos < MAX_WORD_LEN - 1) {
                current_word[word_pos++] = tran(sentence[i]);  // 统一转小写
            } else {
                // 单词过长，截断处理
                if (word_pos == MAX_WORD_LEN - 1) {
                    current_word[word_pos] = '\0';
                    WordNode* new_node = create_node(current_word);
                    
                    if (head == NULL) {
                        head = tail = new_node;
                    } else {
                        tail->next = new_node;
                        tail = new_node;
                    }
                    
                    word_pos = 0;
                }
            }
        }
    }

    // 处理最后一个单词
    if (word_pos > 0) {
        current_word[word_pos] = '\0';
        WordNode* new_node = create_node(current_word);
        
        if (head == NULL) {
            head = new_node;
        } else {
            tail->next = new_node;
        }
    }

    return head;
}

// 打印链表
void print_list(WordNode* head) {
    WordNode* current = head;
    while (current != NULL) {
        printf("%s", current->word);
        if (current->next != NULL) {
            printf(" -> ");
        }
        current = current->next;
    }
    printf("\n");
}

// 邻接矩阵实现的图数据结构
typedef struct {
    int numVertices;               // 图中顶点的数量
    int adjacencyMatrix[MAX_VERTICES][MAX_VERTICES];  // 邻接矩阵
} Graph;

typedef Graph* pGraph;

// 初始化图（邻接矩阵）
pGraph createGraph(int numVertices) {
    if (numVertices > MAX_VERTICES) {
        fprintf(stderr, "Number of vertices exceeds maximum limit\n");
        exit(EXIT_FAILURE);
    }

    pGraph graph = (pGraph)malloc(sizeof(Graph));
    if (graph == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }
    
    graph->numVertices = numVertices;

    // 初始化邻接矩阵（全 0）
    for (int i = 0; i < numVertices; i++) {
        for (int j = 0; j < numVertices; j++) {
            graph->adjacencyMatrix[i][j] = 0;
        }
    }
    return graph;
}

// 单词表结构，维护顶点编号与单词的映射关系
typedef struct {
    char words[MAX_VERTICES][MAX_WORD_LEN];
    int count;
} WordTable;

// 全局变量单词表
WordTable table = { .count = 0 };

// 判断单词是否在单词表中
int is_in_table(const char* word) {
    if (word == NULL) {
        return -1;
    }
    
    for (int i = 0; i < table.count; i++) {
        if (strcmp(word, table.words[i]) == 0) {
            return i;
        }
    }
    return -1;
}

// 处理接收的文本文件，返回处理后的字符串
char* process_text_file(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        perror("Error opening file");
        return NULL;
    }

    // 计算文件大小
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    // 分配足够大的缓冲区来存储处理后的字符串
    char* result = (char*)malloc(file_size * 2 + 1); // 最坏情况下，每个字符后跟一个空格
    if (result == NULL) {
        fclose(file);
        perror("Memory allocation failed");
        return NULL;
    }

    int result_index = 0;
    int in_word = 0; // 标记是否在处理一个单词
    char c;

    while ((c = fgetc(file)) != EOF && result_index < file_size * 2) {
        if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z')) {
            // 如果是字母，添加到结果中
            result[result_index++] = tran(c);
            in_word = 1;
        } else {
            // 处理非字母字符
            if (in_word) {
                // 如果之前在处理单词，现在添加一个空格
                if (result_index < file_size * 2) {
                    result[result_index++] = ' ';
                }
                in_word = 0;
            }
            // 忽略其他字符
        }
    }

    // 确保最后一个单词后有空格（如果最后一个字符不是字母）
    if (in_word && result_index < file_size * 2) {
        result[result_index++] = ' ';
    }

    // 确保字符串以空字符结尾
    result[result_index] = '\0';

    fclose(file);
    return result;
}

// 接收句子链表的头，分析句子加入单词表
void get_table(WordNode* head) {
    if (head == NULL) {
        return;
    }

    WordNode* temp1 = head;
    WordNode* temp2 = head->next;
    
    while (temp1 != NULL && temp2 != NULL && table.count < MAX_VERTICES) {
        int id1 = is_in_table(temp1->word);
        if (id1 == -1) {
            if (table.count >= MAX_VERTICES) {
                fprintf(stderr, "Warning: Word table full, cannot add more words\n");
                break;
            }
            strncpy(table.words[table.count], temp1->word, MAX_WORD_LEN - 1);
            table.words[table.count][MAX_WORD_LEN - 1] = '\0';
            id1 = table.count;
            table.count++;
        }

        int id2 = is_in_table(temp2->word);
        if (id2 == -1) {
            if (table.count >= MAX_VERTICES) {
                fprintf(stderr, "Warning: Word table full, cannot add more words\n");
                break;
            }
            strncpy(table.words[table.count], temp2->word, MAX_WORD_LEN - 1);
            table.words[table.count][MAX_WORD_LEN - 1] = '\0';
            id2 = table.count;
            table.count++;
        }

        temp1 = temp1->next;
        temp2 = temp2 != NULL ? temp2->next : NULL;
    }
}

// 补充图
void get_graph(WordNode* head, pGraph pgraph) {
    if (head == NULL || pgraph == NULL) {
        return;
    }

    WordNode* temp1 = head;
    WordNode* temp2 = head->next;
    
    while (temp1 != NULL && temp2 != NULL) {
        int id1 = is_in_table(temp1->word);
        int id2 = is_in_table(temp2->word);
        
        if (id1 != -1 && id2 != -1 && id1 < pgraph->numVertices && id2 < pgraph->numVertices) {
            pgraph->adjacencyMatrix[id1][id2]++;
        }
        
        temp1 = temp1->next;
        temp2 = temp2 != NULL ? temp2->next : NULL;
    }
}

void exportToDot(pGraph graph, const char* filename) {
    if (graph == NULL || filename == NULL) {
        return;
    }

    FILE* file = fopen(filename, "w");
    if (!file) {
        perror("Failed to open file");
        return;
    }

    // DOT文件头
    fprintf(file, "digraph G {\n");
    fprintf(file, "  rankdir=LR; // 从左到右布局\n");
    fprintf(file, "  node [shape=circle];\n");

    // 遍历所有边
    for (int i = 0; i < graph->numVertices; i++) {
        for (int j = 0; j < graph->numVertices; j++) {
            int weight = graph->adjacencyMatrix[i][j];
            if (weight != 0) { // 存在边
                fprintf(file, "  \"%s\" -> \"%s\" [label=\"%d\"];\n", 
                       table.words[i], table.words[j], weight);
            }
        }
    }

    fprintf(file, "}\n");
    fclose(file);
}

// 打印邻接矩阵
void printAdjacencyMatrix(pGraph graph) {
    if (graph == NULL) {
        return;
    }

    printf("邻接矩阵：\n");
    
    // 打印列标题
    printf("    ");
    for (int j = 0; j < graph->numVertices; j++) {
        printf("%-5s ", table.words[j]);
    }
    printf("\n");
    
    for (int i = 0; i < graph->numVertices; i++) {
        printf("%-5s ", table.words[i]);
        for (int j = 0; j < graph->numVertices; j++) {
            printf("%-5d ", graph->adjacencyMatrix[i][j]);
        }
        printf("\n");
    }
}

// 查找桥接词函数
void find_bridge_words(pGraph graph, const char* word1, const char* word2) {
    // 检查单词是否在图中
    int id1 = is_in_table(word1);
    int id2 = is_in_table(word2);
    
    if (id1 == -1 || id2 == -1) {
        printf("No %s or %s in the graph!\n", word1, word2);
        return;
    }
    
    int bridge_count = 0;
    char bridge_words[MAX_VERTICES][MAX_WORD_LEN];
    
    // 遍历所有可能的桥接词
    for (int i = 0; i < graph->numVertices; i++) {
        // 检查word1->word3和word3->word2的边是否存在
        if (graph->adjacencyMatrix[id1][i] > 0 && graph->adjacencyMatrix[i][id2] > 0) {
            strncpy(bridge_words[bridge_count], table.words[i], MAX_WORD_LEN - 1);
            bridge_words[bridge_count][MAX_WORD_LEN - 1] = '\0';
            bridge_count++;
        }
    }
    
    // 输出结果
    if (bridge_count == 0) {
        printf("No bridge words from %s to %s!\n", word1, word2);
    } else {
        printf("The bridge words from %s to %s are: ", word1, word2);
        for (int i = 0; i < bridge_count; i++) {
            printf("%s", bridge_words[i]);
            if (i < bridge_count - 2) {
                printf(", ");
            } else if (i == bridge_count - 2) {
                printf(", and ");
            }
        }
        printf(".\n");
    }
}

// 随机选择一个桥接词
char* select_random_bridge_word(pGraph graph, int id1, int id2) {
    int bridge_count = 0;
    int bridge_indices[MAX_VERTICES];
    
    // 收集所有桥接词的索引
    for (int i = 0; i < graph->numVertices; i++) {
        if (graph->adjacencyMatrix[id1][i] > 0 && graph->adjacencyMatrix[i][id2] > 0) {
            bridge_indices[bridge_count++] = i;
        }
    }
    
    if (bridge_count == 0) {
        return NULL;
    }
    
    // 随机选择一个桥接词
    int random_index = rand() % bridge_count;
    return table.words[bridge_indices[random_index]];
}

// 生成新文本并插入桥接词
void generate_new_text(pGraph graph, const char* input_text) {
    if (input_text == NULL || *input_text == '\0') {
        printf("Empty input text.\n");
        return;
    }
    
    // 处理输入文本为单词链表
    char* processed_input = (char*)malloc(strlen(input_text) * 2 + 1);
    if (processed_input == NULL) {
        perror("Memory allocation failed");
        return;
    }
    
    // 预处理输入文本（转换为小写并只保留字母）
    int pos = 0;
    for (int i = 0; input_text[i] != '\0'; i++) {
        if (isalpha(input_text[i])) {
            processed_input[pos++] = tran(input_text[i]);
        } else if (pos > 0 && processed_input[pos-1] != ' ') {
            processed_input[pos++] = ' ';
        }
    }
    processed_input[pos] = '\0';
    
    // 转换为单词链表
    WordNode* input_list = sentence_to_list(processed_input);
    free(processed_input);
    
    if (input_list == NULL) {
        printf("No valid words in input text.\n");
        return;
    }
    
    // 准备构建新文本
    char new_text[10000] = ""; // 假设足够大的缓冲区
    int new_text_pos = 0;
    
    WordNode* current = input_list;
    WordNode* next = current->next;
    
    // 处理原始输入文本（保留原始大小写和标点）
    const char* original_ptr = input_text;
    
    while (current != NULL && next != NULL) {
        // 跳过输入文本中的非字母字符
        while (*original_ptr != '\0' && !isalpha(*original_ptr)) {
            if (new_text_pos < sizeof(new_text) - 1) {
                new_text[new_text_pos++] = *original_ptr;
            }
            original_ptr++;
        }
        
        // 复制当前单词（保留原始大小写）
        const char* word_start = original_ptr;
        while (*original_ptr != '\0' && isalpha(*original_ptr)) {
            if (new_text_pos < sizeof(new_text) - 1) {
                new_text[new_text_pos++] = *original_ptr;
            }
            original_ptr++;
        }
        
        // 检查当前单词和下一个单词是否有桥接词
        char lower_current[MAX_WORD_LEN];
        char lower_next[MAX_WORD_LEN];
        
        strncpy(lower_current, current->word, MAX_WORD_LEN - 1);
        lower_current[MAX_WORD_LEN - 1] = '\0';
        
        strncpy(lower_next, next->word, MAX_WORD_LEN - 1);
        lower_next[MAX_WORD_LEN - 1] = '\0';
        
        int id1 = is_in_table(lower_current);
        int id2 = is_in_table(lower_next);
        
        if (id1 != -1 && id2 != -1) {
            char* bridge_word = select_random_bridge_word(graph, id1, id2);
            if (bridge_word != NULL) {
                // 插入桥接词
                if (new_text_pos < sizeof(new_text) - 1) {
                    new_text[new_text_pos++] = ' ';
                }
                
                // 保持桥接词的大小写与当前单词一致
                if (isupper(*word_start)) {
                    bridge_word[0] = toupper(bridge_word[0]);
                }
                
                for (int i = 0; bridge_word[i] != '\0' && new_text_pos < sizeof(new_text) - 1; i++) {
                    new_text[new_text_pos++] = bridge_word[i];
                }
            }
        }
        
        current = next;
        next = next->next;
    }
    
    // 复制剩余的非字母字符
    while (*original_ptr != '\0' && new_text_pos < sizeof(new_text) - 1) {
        new_text[new_text_pos++] = *original_ptr;
        original_ptr++;
    }
    
    new_text[new_text_pos] = '\0';
    
    // 输出结果
    printf("Generated new text: %s\n", new_text);
    
    // 释放链表内存
    WordNode* temp = input_list;
    while (temp != NULL) {
        WordNode* next_node = temp->next;
        free(temp);
        temp = next_node;
    }
}

// 路径结构，用于存储多条最短路径
typedef struct {
    int paths[MAX_PATHS][MAX_VERTICES];  // 存储路径
    int path_lengths[MAX_PATHS];         // 每条路径的长度
    int count;                           // 路径数量
} PathList;

// 递归函数，用于回溯所有最短路径
void backtrack_paths(pGraph graph, int u, int v, int* dist, int* path, int path_len, PathList* path_list) {
    path[path_len++] = u;
    
    if (u == v) {
        // 找到一条路径，保存到path_list中
        if (path_list->count < MAX_PATHS) {
            for (int i = 0; i < path_len; i++) {
                path_list->paths[path_list->count][i] = path[i];
            }
            path_list->path_lengths[path_list->count] = path_len;
            path_list->count++;
        }
    } else {
        // 遍历所有相邻节点
        for (int i = 0; i < graph->numVertices; i++) {
            if (graph->adjacencyMatrix[u][i] > 0 && dist[i] == dist[u] + graph->adjacencyMatrix[u][i]) {
                backtrack_paths(graph, i, v, dist, path, path_len, path_list);
            }
        }
    }
}

// 查找所有最短路径
PathList find_all_shortest_paths(pGraph graph, int id1, int id2, int* dist) {
    PathList path_list = { .count = 0 };
    int path[MAX_VERTICES];
    
    backtrack_paths(graph, id1, id2, dist, path, 0, &path_list);
    return path_list;
}

// 计算最短路径并展示结果
void show_shortest_path(pGraph graph, const char* word1, const char* word2) {
    // 检查单词是否在图中
    int id1 = is_in_table(word1);
    int id2 = -1;
    
    if (id1 == -1) {
        printf("No %s in the graph!\n", word1);
        return;
    }
    
    // 如果只提供了一个单词，计算到所有其他单词的最短路径
    if (word2 == NULL || word2[0] == '\0') {
        printf("Calculating shortest paths from '%s' to all other words:\n", word1);
        
        // 计算到所有顶点的最短距离
        int dist[MAX_VERTICES];
        int prev[MAX_VERTICES];
        int visited[MAX_VERTICES] = {0};
        
        for (int i = 0; i < graph->numVertices; i++) {
            dist[i] = INT_MAX;
            prev[i] = -1;
        }
        dist[id1] = 0;
        
        // Dijkstra算法
        for (int count = 0; count < graph->numVertices - 1; count++) {
            int min_dist = INT_MAX;
            int u = -1;
            for (int v = 0; v < graph->numVertices; v++) {
                if (!visited[v] && dist[v] < min_dist) {
                    min_dist = dist[v];
                    u = v;
                }
            }
            
            if (u == -1) break;
            visited[u] = 1;
            
            for (int v = 0; v < graph->numVertices; v++) {
                if (!visited[v] && graph->adjacencyMatrix[u][v] > 0 && 
                    dist[u] != INT_MAX && dist[u] + graph->adjacencyMatrix[u][v] < dist[v]) {
                    dist[v] = dist[u] + graph->adjacencyMatrix[u][v];
                    prev[v] = u;
                }
            }
        }
        
        // 打印所有最短路径
        for (int i = 0; i < graph->numVertices; i++) {
            if (i == id1) continue;
            
            if (dist[i] == INT_MAX) {
                printf("No path from %s to %s!\n", word1, table.words[i]);
                continue;
            }
            
            // 构建路径
            int path[MAX_VERTICES];
            int path_len = 0;
            for (int at = i; at != -1; at = prev[at]) {
                path[path_len++] = at;
            }
            
            // 反转路径
            for (int j = 0; j < path_len / 2; j++) {
                int temp = path[j];
                path[j] = path[path_len - j - 1];
                path[path_len - j - 1] = temp;
            }
            
            // 打印路径
            printf("Shortest path from %s to %s (length %d):\n", word1, table.words[i], dist[i]);
            for (int j = 0; j < path_len; j++) {
                printf("%s", table.words[path[j]]);
                if (j < path_len - 1) {
                    printf(" -> ");
                }
            }
            printf("\n");
        }
        return;
    }
    
    // 处理两个单词的情况
    id2 = is_in_table(word2);
    if (id2 == -1) {
        printf("No %s in the graph!\n", word2);
        return;
    }
    
    int numVertices = graph->numVertices;
    int dist[MAX_VERTICES];         // 存储从起点到每个顶点的最短距离
    int prev[MAX_VERTICES];         // 存储前驱顶点
    int visited[MAX_VERTICES] = {0}; // 标记顶点是否已访问
    
    // 初始化距离和前驱数组
    for (int i = 0; i < numVertices; i++) {
        dist[i] = INT_MAX;
        prev[i] = -1;
    }
    dist[id1] = 0;
    
    // Dijkstra算法
    for (int count = 0; count < numVertices - 1; count++) {
        // 选择未访问顶点中距离最小的
        int min_dist = INT_MAX;
        int u = -1;
        for (int v = 0; v < numVertices; v++) {
            if (!visited[v] && dist[v] < min_dist) {
                min_dist = dist[v];
                u = v;
            }
        }
        
        if (u == -1) break; // 所有可达顶点都已处理
        
        visited[u] = 1;
        
        // 更新相邻顶点的距离
        for (int v = 0; v < numVertices; v++) {
            if (!visited[v] && graph->adjacencyMatrix[u][v] > 0 && 
                dist[u] != INT_MAX && dist[u] + graph->adjacencyMatrix[u][v] < dist[v]) {
                dist[v] = dist[u] + graph->adjacencyMatrix[u][v];
                prev[v] = u;
            }
        }
    }
    
    // 检查是否找到路径
    if (dist[id2] == INT_MAX) {
        printf("No path from %s to %s!\n", word1, word2);
        return;
    }
    
    // 查找所有最短路径
    PathList path_list = find_all_shortest_paths(graph, id1, id2, dist);
    
    if (path_list.count == 0) {
        printf("No path found (should not happen)\n");
        return;
    }
    
    // 打印所有最短路径
    printf("Found %d shortest path(s) from %s to %s (length %d):\n", 
           path_list.count, word1, word2, dist[id2]);
    
    for (int i = 0; i < path_list.count; i++) {
        printf("Path %d: ", i+1);
        for (int j = 0; j < path_list.path_lengths[i]; j++) {
            printf("%s", table.words[path_list.paths[i][j]]);
            if (j < path_list.path_lengths[i] - 1) {
                printf(" -> ");
            }
        }
        printf("\n");
    }
    
    // 生成带高亮路径的DOT文件
    FILE* file = fopen("shortest_path.dot", "w");
    if (!file) {
        perror("Failed to open file");
        return;
    }
    
    // DOT文件头
    fprintf(file, "digraph G {\n");
    fprintf(file, "  rankdir=LR; // 从左到右布局\n");
    fprintf(file, "  node [shape=circle];\n");
    
    // 定义不同路径的颜色
    const char* path_colors[MAX_PATHS] = {
        "red", "blue", "green", "purple", "orange",
        "brown", "pink", "cyan", "magenta", "gold"
    };
    
    // 先输出所有边，不设置颜色
    for (int i = 0; i < numVertices; i++) {
        for (int j = 0; j < numVertices; j++) {
            int weight = graph->adjacencyMatrix[i][j];
            if (weight != 0) {
                // 检查这条边是否在任意一条最短路径中
                int is_path_edge = 0;
                int path_index = -1;
                
                for (int k = 0; k < path_list.count; k++) {
                    for (int l = 0; l < path_list.path_lengths[k] - 1; l++) {
                        if (path_list.paths[k][l] == i && path_list.paths[k][l+1] == j) {
                            is_path_edge = 1;
                            path_index = k;
                            break;
                        }
                    }
                    if (is_path_edge) break;
                }
                
                if (is_path_edge) {
                    // 如果在最短路径中，使用对应颜色
                    fprintf(file, "  \"%s\" -> \"%s\" [label=\"%d\", color=%s, penwidth=2.0, style=bold];\n", 
                           table.words[i], table.words[j], weight, path_colors[path_index % MAX_PATHS]);
                } else {
                    // 普通边
                    fprintf(file, "  \"%s\" -> \"%s\" [label=\"%d\"];\n", 
                           table.words[i], table.words[j], weight);
                }
            }
        }
    }
    
    // 高亮起点和终点
    fprintf(file, "  \"%s\" [color=green, penwidth=2.0];\n", table.words[id1]);
    fprintf(file, "  \"%s\" [color=blue, penwidth=2.0];\n", table.words[id2]);
    
    fprintf(file, "}\n");
    fclose(file);
    
    printf("Shortest path visualization generated with %d paths. Run: dot -Tpng shortest_path.dot -o shortest_path.png\n", path_list.count);
}


// 计算PageRank值
void calculate_pagerank(pGraph graph) {
    if (graph == NULL || graph->numVertices == 0) {
        printf("Graph is empty!\n");
        return;
    }

    int numVertices = graph->numVertices;
    double pr[numVertices];    // 当前PR值
    double new_pr[numVertices]; // 新的PR值
    int out_degree[numVertices]; // 每个节点的出度
    int has_out_degree_zero = 0; // 是否有出度为0的节点

    // 初始化PR值和出度
    for (int i = 0; i < numVertices; i++) {
        pr[i] = 1.0 / numVertices; // 初始PR值均匀分布
        out_degree[i] = 0;
        
        // 计算每个节点的出度
        for (int j = 0; j < numVertices; j++) {
            if (graph->adjacencyMatrix[i][j] > 0) {
                out_degree[i] += graph->adjacencyMatrix[i][j];
            }
        }
        
        if (out_degree[i] == 0) {
            has_out_degree_zero = 1;
        }
    }

    // 处理悬挂节点（出度为0的节点）
    if (has_out_degree_zero) {
        printf("Warning: Graph contains dangling nodes (nodes with out-degree=0)\n");
    }

    // PageRank迭代
    int iter;
    double diff;
    for (iter = 0; iter < MAX_ITERATIONS; iter++) {
        diff = 0.0;
        
        // 计算悬挂节点贡献（均匀分配给所有节点）
        double dangling_contribution = 0.0;
        for (int i = 0; i < numVertices; i++) {
            if (out_degree[i] == 0) {
                dangling_contribution += pr[i] / numVertices;
            }
        }

        // 计算新的PR值
        for (int j = 0; j < numVertices; j++) {
            new_pr[j] = (1.0 - DAMPING_FACTOR) / numVertices; // 随机跳转部分
            
            // 加上来自其他节点的贡献
            for (int i = 0; i < numVertices; i++) {
                if (graph->adjacencyMatrix[i][j] > 0 && out_degree[i] > 0) {
                    new_pr[j] += DAMPING_FACTOR * pr[i] * graph->adjacencyMatrix[i][j] / out_degree[i];
                }
            }
            
            // 加上悬挂节点的贡献
            new_pr[j] += DAMPING_FACTOR * dangling_contribution;
            
            // 计算差异
            diff += fabs(new_pr[j] - pr[j]);
        }

        // 检查是否收敛
        if (diff < TOLERANCE) {
            break;
        }

        // 更新PR值
        for (int i = 0; i < numVertices; i++) {
            pr[i] = new_pr[i];
        }
    }

    printf("PageRank converged after %d iterations:\n", iter);
    for (int i = 0; i < numVertices; i++) {
        printf("%s: %.6f\n", table.words[i], pr[i]);
    }
}


// 随机遍历函数
void random_walk(pGraph graph) {
    if (graph == NULL || graph->numVertices == 0) {
        printf("Graph is empty!\n");
        return;
    }

    // 创建文件保存遍历结果
    FILE *walk_file = fopen("random_walk.txt", "w");
    if (walk_file == NULL) {
        perror("Failed to create walk file");
        return;
    }

    printf("Starting random walk (press 'q' and Enter to stop)...\n");
    fprintf(walk_file, "Random walk traversal:\n");

    // 随机选择起点
    int current = rand() % graph->numVertices;
    printf("Start at: %s\n", table.words[current]);
    fprintf(walk_file, "Start at: %s\n", table.words[current]);

    // 记录访问过的边 (from, to)
    bool visited_edges[MAX_VERTICES][MAX_VERTICES] = {false};
    int steps = 0;
    bool stop = false;
    bool has_out_edge = false;

    while (!stop) {
        // 检查用户输入
        // if (kbhit()) {
        //     char ch = getchar();
        //     if (ch == 'q' || ch == 'Q') {
        //         printf("\nUser requested stop.\n");
        //         fprintf(walk_file, "\nUser requested stop after %d steps.\n", steps);
        //         stop = true;
        //         break;
        //     }
        // }

        // 检查当前节点是否有出边
        has_out_edge = false;
        for (int i = 0; i < graph->numVertices; i++) {
            if (graph->adjacencyMatrix[current][i] > 0) {
                has_out_edge = true;
                break;
            }
        }

        if (!has_out_edge) {
            printf("\nStopped at node %s (no outgoing edges).\n", table.words[current]);
            fprintf(walk_file, "\nStopped at node %s (no outgoing edges).\n", table.words[current]);
            break;
        }

        // 收集所有可能的出边
        int out_edges[MAX_VERTICES];
        int edge_count = 0;
        for (int i = 0; i < graph->numVertices; i++) {
            if (graph->adjacencyMatrix[current][i] > 0) {
                out_edges[edge_count++] = i;
            }
        }

        // 随机选择一条出边
        int next = out_edges[rand() % edge_count];
        steps++;

        // 检查是否已经访问过这条边
        if (visited_edges[current][next]) {
            printf("\nStopped at edge %s -> %s (repeated edge).\n", 
                  table.words[current], table.words[next]);
            fprintf(walk_file, "\nStopped at edge %s -> %s (repeated edge).\n", 
                   table.words[current], table.words[next]);
            break;
        }

        // 记录访问的边
        visited_edges[current][next] = true;
        printf("Step %d: %s -> %s\n", steps, table.words[current], table.words[next]);
        fprintf(walk_file, "Step %d: %s -> %s\n", steps, table.words[current], table.words[next]);

        // 移动到下一个节点
        current = next;
    }

    printf("Random walk completed. Total steps: %d\n", steps);
    fprintf(walk_file, "Random walk completed. Total steps: %d\n", steps);
    fclose(walk_file);
    printf("Walk results saved to random_walk.txt\n");
}

// 检查键盘输入的辅助函数 (跨平台)
#ifdef _WIN32
#include <conio.h>
int kbhit() {
    return _kbhit();
}
#else
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
int kbhit() {
    struct termios oldt, newt;
    int ch;
    int oldf;

    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

    ch = getchar();

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    fcntl(STDIN_FILENO, F_SETFL, oldf);

    if (ch != EOF) {
        ungetc(ch, stdin);
        return 1;
    }

    return 0;
}
#endif


int main() {
    srand(time(NULL)); // 初始化随机数种子
    
    //char* processed_text = process_text_file("test.txt");
    char* processed_text = process_text_file("Easy Test.txt");
    //char* processed_text = process_text_file("Cursed Be The Treasure.txt");
    if (processed_text != NULL) {
        printf("Processed text: %s\n", processed_text);  //打印处理之后的文本
        
        WordNode* word_list = sentence_to_list(processed_text);
        //printf("Word list: ");
        //print_list(word_list);

        get_table(word_list);
        //printf("Word table contains %d words\n", table.count);
        
        if (table.count > 0) {
            pGraph pgraph = createGraph(table.count);
            get_graph(word_list, pgraph);
            
            //printAdjacencyMatrix(pgraph);
            
            exportToDot(pgraph, "graph.dot");
            printf("DOT file generated. Run: dot -Tpng graph.dot -o graph.png\n");
            
            int choice;
            char word1[MAX_WORD_LEN], word2[MAX_WORD_LEN];
            char input_text[1000];
            
            while (1) {
                printf("\nChoose an option:\n");
                printf("1. Find bridge words between two words\n");
                printf("2. Generate new text with bridge words\n");
                printf("3. Calculate shortest path between two words\n");
                printf("4. Calculate shortest paths from one word to all others\n");
                printf("5. Calculate PageRank\n");
                printf("6. Perform random walk\n");
                printf("0. Exit\n");
                printf("Enter your choice (0-6): ");
                
                if (scanf("%d", &choice) != 1) {
                    printf("Invalid input. Please enter a number.\n");
                    while (getchar() != '\n'); // 清除输入缓冲区
                    continue;
                }
                // WordNode* current = word_list;
                switch (choice) {
                    case 0:
                        printf("Exiting program.\n");
                        free(pgraph);
                        WordNode* current = word_list;
                        while (current != NULL) {
                            WordNode* next = current->next;
                            free(current);
                            current = next;
                        }
                        free(processed_text);
                        return 0;
                        
                    case 1:
                        printf("Enter two words (separated by space): ");
                        scanf("%19s %19s", word1, word2);
                        for (int i = 0; word1[i]; i++) word1[i] = tran(word1[i]);
                        for (int i = 0; word2[i]; i++) word2[i] = tran(word2[i]);
                        find_bridge_words(pgraph, word1, word2);
                        break;
                        
                    case 2:
                        printf("Enter a line of text: ");
                        getchar(); // 消耗换行符
                        fgets(input_text, sizeof(input_text), stdin);
                        input_text[strcspn(input_text, "\n")] = '\0';
                        generate_new_text(pgraph, input_text);
                        break;
                        
                    case 3:
                        printf("Enter two words (separated by space): ");
                        scanf("%19s %19s", word1, word2);
                        for (int i = 0; word1[i]; i++) word1[i] = tran(word1[i]);
                        for (int i = 0; word2[i]; i++) word2[i] = tran(word2[i]);
                        show_shortest_path(pgraph, word1, word2);
                        break;
                        
                    case 4:
                        printf("Enter a word: ");
                        scanf("%19s", word1);
                        for (int i = 0; word1[i]; i++) word1[i] = tran(word1[i]);
                        show_shortest_path(pgraph, word1, NULL);
                        break;
                        
                    case 5:
                        calculate_pagerank(pgraph);
                        break;
                        
                    case 6:
                        random_walk(pgraph);
                        break;
                        
                    default:
                        printf("Invalid choice. Please enter a number between 0 and 6.\n");
                }
            }
        }
    }
    return 0;
}