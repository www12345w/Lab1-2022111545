#include <gtest/gtest.h>
#include "main2.h"
#include <fstream>
#include <sstream>

// 辅助函数：读取文件内容
std::string readFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file) return "";
    std::ostringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

// 辅助函数：执行并打印随机游走
void executeAndPrintRandomWalk(const Graph& graph, const WordTable& table, 
                             const std::string& testCaseName) {
    std::cout << "\n=== Running test: " << testCaseName << " ===\n";
    randomWalk(graph, table);
    std::cout << "=== End of test output ===\n";
}

// 测试用例 1: 空图
TEST(RandomWalkTest, EmptyGraph) {
    Graph graph(0);
    WordTable table;
    
    executeAndPrintRandomWalk(graph, table, "EmptyGraph");
    
    testing::internal::CaptureStdout();
    randomWalk(graph, table);
    std::string output = testing::internal::GetCapturedStdout();
    
    EXPECT_TRUE(output.find("Graph is empty!") != std::string::npos);
}

// 测试用例 2: 节点无出边
TEST(RandomWalkTest, NoOutgoingEdges) {
    Graph graph(2);
    WordTable table;
    table.addWord("A");
    table.addWord("B"); // 邻接矩阵全0，默认无出边
    
    executeAndPrintRandomWalk(graph, table, "NoOutgoingEdges");
    
    testing::internal::CaptureStdout();
    randomWalk(graph, table);
    std::string output = testing::internal::GetCapturedStdout();
    
    EXPECT_TRUE(output.find("no outgoing edges") != std::string::npos);
    EXPECT_TRUE(readFile("random_walk.txt").find("no outgoing edges") != std::string::npos);
}

// 测试用例 3: 重复边终止
TEST(RandomWalkTest, RepeatedEdgeTermination) {
    Graph graph(3);  // 改为3个节点
    WordTable table;
    table.addWord("A");
    table.addWord("B");
    table.addWord("C");
    
    // 创建环状结构：A->B->C->A
    graph.adjacencyMatrix[0][1] = 1; // A -> B
    graph.adjacencyMatrix[1][2] = 1; // B -> C
    graph.adjacencyMatrix[2][0] = 1; // C -> A

    // 模拟用户输入，让程序走A->B->C->A->B (第二次A->B是重复边)
    std::streambuf* orig = std::cin.rdbuf();
    std::istringstream input("\n\n\nq\n"); // 前3步走环，最后输入q停止
    std::cin.rdbuf(input.rdbuf());

    executeAndPrintRandomWalk(graph, table, "RepeatedEdgeTermination");
    
    testing::internal::CaptureStdout();
    randomWalk(graph, table);
    std::string output = testing::internal::GetCapturedStdout();
    std::cin.rdbuf(orig);

    // 检查输出中是否包含重复边提示
    EXPECT_TRUE(output.find("repeated edge") != std::string::npos)
        << "Actual output:\n" << output;
    
    // 检查文件输出
    std::string fileContent = readFile("random_walk.txt");
    EXPECT_TRUE(fileContent.find("repeated edge") != std::string::npos)
        << "File content:\n" << fileContent;
}
// 测试用例 4: 用户主动终止 (需模拟输入 'q')
TEST(RandomWalkTest, UserRequestedStop) {
    Graph graph(3);
    WordTable table;
    table.addWord("A");
    table.addWord("B");
    table.addWord("C");
    graph.adjacencyMatrix[0][1] = 1; // A -> B
    graph.adjacencyMatrix[1][2] = 1; // B -> C
    graph.adjacencyMatrix[2][0] = 1; // C -> A (形成环)

    // 删除 executeAndPrintRandomWalk 调用，避免执行两次
    
    // 模拟用户输入 'q'
    std::streambuf* orig = std::cin.rdbuf();
    std::istringstream input("q\n");
    std::cin.rdbuf(input.rdbuf());

    // 先清空可能存在的旧文件
    std::ofstream clearFile("random_walk.txt");
    clearFile.close();

    testing::internal::CaptureStdout();
    randomWalk(graph, table);
    std::string output = testing::internal::GetCapturedStdout();
    std::cin.rdbuf(orig);

    // 打印实际输出和文件内容，便于调试
    std::cout << "Actual console output:\n" << output << std::endl;
    std::string fileContent = readFile("random_walk.txt");
    std::cout << "Actual file content:\n" << fileContent << std::endl;

    // 检查控制台输出
    EXPECT_TRUE(output.find("User requested stop") != std::string::npos)
        << "Did not find 'User requested stop' in console output";
    
    // 检查文件输出
    EXPECT_TRUE(fileContent.find("User requested stop") != std::string::npos)
        << "Did not find 'User requested stop' in file content";
}

// 测试用例 5: 多边随机游走到重复边
TEST(RandomWalkTest, MultipleEdgesUntilRepeat) {
    Graph graph(3);
    WordTable table;
    table.addWord("A");
    table.addWord("B");
    table.addWord("C");
    graph.adjacencyMatrix[0][1] = 1; // A -> B
    graph.adjacencyMatrix[0][2] = 1; // A -> C
    graph.adjacencyMatrix[1][0] = 1; // B -> A
    graph.adjacencyMatrix[2][1] = 1; // C -> B
    
    executeAndPrintRandomWalk(graph, table, "MultipleEdgesUntilRepeat");
    
    testing::internal::CaptureStdout();
    randomWalk(graph, table);
    std::string output = testing::internal::GetCapturedStdout();
    
    // 至少输出一个 Step 行
    EXPECT_TRUE(output.find("Step 1:") != std::string::npos);
}

// 测试用例 6: 遍历到无出边节点
TEST(RandomWalkTest, WalkToDeadEnd) {
    Graph graph(3);
    WordTable table;
    table.addWord("A");
    table.addWord("B");
    table.addWord("C");
    graph.adjacencyMatrix[0][1] = 1; // A -> B
    graph.adjacencyMatrix[1][2] = 1; // B -> C (C无出边)
    
    executeAndPrintRandomWalk(graph, table, "WalkToDeadEnd");
    
    testing::internal::CaptureStdout();
    randomWalk(graph, table);
    std::string output = testing::internal::GetCapturedStdout();
    
    EXPECT_TRUE(output.find("no outgoing edges") != std::string::npos);
    EXPECT_TRUE(readFile("random_walk.txt").find("no outgoing edges") != std::string::npos);
}

// int main(int argc, char **argv) {
//     testing::InitGoogleTest(&argc, argv);
//     return RUN_ALL_TESTS();
// }