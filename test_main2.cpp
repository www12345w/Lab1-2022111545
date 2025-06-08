#include <gtest/gtest.h>
#include "main2.h"

class BridgeWordsTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 初始化单词表
        table.addWord("the");
        table.addWord("team");
        table.addWord("requested");
        table.addWord("them");
        table.addWord("more");

        // 确保图大小与单词数量匹配
        graph = Graph(table.size());

        // 构建测试图结构
        int the = table.getIndex("the");
        int team = table.getIndex("team");
        int requested = table.getIndex("requested");
        int them = table.getIndex("them");
        int more = table.getIndex("more");

        graph.adjacencyMatrix[the][team] = 1;       // the -> team
        graph.adjacencyMatrix[team][requested] = 1; // team -> requested
        graph.adjacencyMatrix[the][them] = 1;       // the -> them
        graph.adjacencyMatrix[them][requested] = 1; // them -> requested
        graph.adjacencyMatrix[the][more] = 1;       // the -> more
    }

    Graph graph{0}; // 初始化为0，在SetUp中重新初始化
    WordTable table;
};

// 辅助函数：执行并打印结果
void executeAndPrintBridgeWords(const Graph& graph, const WordTable& table, 
                               const std::string& word1, const std::string& word2) {
    std::cout << "Testing bridge words from \"" << word1 << "\" to \"" << word2 << "\": ";
    findBridgeWords(graph, table, word1, word2);
}

// 测试用例1: the -> requested 存在桥接词
TEST_F(BridgeWordsTest, ExistingBridgeWords) {
    executeAndPrintBridgeWords(graph, table, "the", "requested");
    
    testing::internal::CaptureStdout();
    findBridgeWords(graph, table, "the", "requested");
    std::string output = testing::internal::GetCapturedStdout();
    
    // 检查两种可能的输出顺序
    bool hasCorrectOutput = 
        (output.find("The bridge words from the to requested are: team, and them.") != std::string::npos) ||
        (output.find("The bridge words from the to requested are: them, and team.") != std::string::npos);
    
    EXPECT_TRUE(hasCorrectOutput) << "Actual output: " << output;
}

// 测试用例2: the -> them 目标词存在但无桥接词
TEST_F(BridgeWordsTest, TargetWordExistButNoBridge) {
    executeAndPrintBridgeWords(graph, table, "the", "them");
    
    testing::internal::CaptureStdout();
    findBridgeWords(graph, table, "the", "them");
    std::string output = testing::internal::GetCapturedStdout();
    
    EXPECT_TRUE(output.find("No bridge words from the to them!") != std::string::npos)
        << "Actual output: " << output;
}

// 测试用例3: me -> more 源词不存在
TEST_F(BridgeWordsTest, SourceWordNotExist) {
    executeAndPrintBridgeWords(graph, table, "me", "more");
    
    testing::internal::CaptureStdout();
    findBridgeWords(graph, table, "me", "more");
    std::string output = testing::internal::GetCapturedStdout();
    
    EXPECT_TRUE(output.find("No me in the graph!") != std::string::npos);
}

// 测试用例4: the -> more 无桥接词
TEST_F(BridgeWordsTest, NoBridgeWords) {
    executeAndPrintBridgeWords(graph, table, "the", "more");
    
    testing::internal::CaptureStdout();
    findBridgeWords(graph, table, "the", "more");
    std::string output = testing::internal::GetCapturedStdout();
    
    EXPECT_TRUE(output.find("No bridge words from the to more!") != std::string::npos);
}

// 测试用例5: me -> you 两个词都不存在
TEST_F(BridgeWordsTest, BothWordsNotExist) {
    executeAndPrintBridgeWords(graph, table, "me", "you");
    
    testing::internal::CaptureStdout();
    findBridgeWords(graph, table, "me", "you");
    std::string output = testing::internal::GetCapturedStdout();
    
    EXPECT_TRUE(output.find("No me and you in the graph!") != std::string::npos) 
        << "Actual output: " << output;
}

// int main(int argc, char **argv) {
//     testing::InitGoogleTest(&argc, argv);
//     return RUN_ALL_TESTS();
// }