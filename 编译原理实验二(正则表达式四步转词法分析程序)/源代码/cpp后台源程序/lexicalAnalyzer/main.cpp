#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <set>
#include <algorithm>
#include <queue>
#include <stack>
#include <fstream>
#include "nlohmann/json.hpp"

using namespace std;
using json = nlohmann::json;

class Lex {
public:
    // 预声明（前向声明的类或者结构体只能用来定义指针或者引用）
    class RE2NFA;

    class NFA2DFA;

    class SimplifyDFA;

    class DFA2Code;

    struct Edge {
        int from;
        int to;
        char ch;

        Edge(int from, int to, char ch) : from(from), to(to), ch(ch) {};
    };

    struct NFAToken {
        vector<Edge> nfaEdgeList;
        int endState;
    };


    // 成员变量
    string data; // 输入的RE字符串
    NFAToken nfaToken; // 存储NFA的边信息和终止节点
    map<int, vector<Edge>> nextNfaEdgeMap; // 记录某个节点有哪些后续节点
    vector<set<int>> dfaNodeList; // 记录某个状态集里面有哪些节点
    vector<Edge> dfaEdgeList; // 记录DFA的边信息
    vector<set<int>> simplifiedDfaNodeList; // 记录某个状态集里面有哪些节点
    vector<Edge> simplifiedDfaEdgeList; // 记录简化后DFA的边信息
    int simplifiedDfaStartState; // DFA的初始状态属于哪个状态集，根据算法可知，DFA终结状态的状态集下标一定为1
    string outputCode; // 最终返回的词法分析程序代码


    // 内部类
    RE2NFA *inner1;
    NFA2DFA *inner2;
    SimplifyDFA *inner3;
    DFA2Code *inner4;

    // 内部类定义
    class RE2NFA {
    public:
        Lex *outer;

        RE2NFA(Lex *outer) {
            this->outer = outer;
        }

        /**
         * 获取字符串不在括号内的所有|的下标列表
         * @param token
         * @return
         */
        vector<int> getOuterVerticalLinePosition(string token) {
            vector<int> v;
            int length = token.length();
            for (int i = 0; i < length; ++i) {
                if (token[i] == '|') {
                    string beforeStr = token.substr(0, i);
                    // 前面字符串的(和)数量匹配就说明当前的|在括号外面
                    if (std::count(beforeStr.begin(), beforeStr.end(), '(') ==
                        std::count(beforeStr.begin(), beforeStr.end(), ')')) {
                        v.emplace_back(i);
                    }
                }
            }
            return v;
        }

        /**
         * 分割没有圆括号和|的字符串，将分割后的字符串存到vector里
         * 例如将xxxy*分割为xxx和y*; xxxy*zzz分割为xxx,y*,zzz;
         * @param token
         * @return
         */
        vector<string> splitTokenWithoutRoundBracketAndVerticalLine(string token) {
            vector<string> v;
            int start = 0;
            for (int i = 0; i < token.length(); ++i) {
                if (token[i] == '*') {
                    if (token.length() != 2) {
                        v.emplace_back(token.substr(start, (i - 2) - start + 1));
                    }
                    v.emplace_back(token.substr(i - 1, 2));
                    start = i + 1;
                }
                // 处理*后结尾的部分
                else if (i == token.length() - 1) {
                    v.emplace_back(token.substr(start, i - start + 1));
                    return v;
                }
            }
            return v;
        }

        /**
         * 将字符串分割为形如a*，(aaa)*，aaa的样子
         * 注意：有外部的|先处理外部的|，即对于(aaa)*bbb|ccc，先要将其划分为(aaa)*bbb和ccc，再分别递归划分
         * @param token
         * @return
         */
        vector<string> splitToken(string token) {
//        cout << "splitToken():  token -> " << token << endl;
            vector<string> v;
            int length = token.length();

            // 去除两边的括号(注意(xxx)(yyy)类似的token就不用去掉)
            if (token[0] == '(' && token[length - 1] == ')') {
                // 判断是不是上面的特例，通过查找字符串开头的(是不是和字符串最后的)匹配，方法是看(的数量和)的数量是否相等
                int tot = 1;
                for (int i = 1; i < token.length(); ++i) {
                    if (token[i] == '(') {
                        tot++;
                    } else if (token[i] == ')' && --tot == 0) {
                        if (i == token.length() - 1) {
                            return splitToken(token.substr(1, length - 2));
                        } else {
                            break;
                        }
                    }
                }
            }

            vector<int> outerVerticalLinePosition = getOuterVerticalLinePosition(token);
            int outerVerticalLinePositionSize = outerVerticalLinePosition.size();
            if (outerVerticalLinePositionSize) { // 有外部的|
                for (int i = 0; i < outerVerticalLinePositionSize; ++i) {
                    if (i == 0) {
                        v.emplace_back(token.substr(0, outerVerticalLinePosition[i]));
                    } else {
                        v.emplace_back(token.substr(outerVerticalLinePosition[i - 1] + 1,
                                                    outerVerticalLinePosition[i] - outerVerticalLinePosition[i - 1] -
                                                    1));
                    }
                    v.emplace_back("|");
                    if (i == outerVerticalLinePositionSize - 1) {
                        v.emplace_back(
                                token.substr(outerVerticalLinePosition[i] + 1, length - outerVerticalLinePosition[i]));
                    }
                }
            } else { // 不存在|,就用()和*分割
                int start = -1, end = -1; // 记录token前后界的位置
                for (int i = 0; i < length; ++i) {
                    if (token[i] == '(') {
                        if (i != end + 1) { // 不是(xxx)(xxx)的形式
                            vector<string> v1 = splitTokenWithoutRoundBracketAndVerticalLine(
                                    token.substr(end + 1, (i - 1) - (end + 1) + 1));
                            v.insert(v.end(), v1.begin(), v1.end());
                        }
                        start = i;
                        int tot = 1;
                        for (int j = i + 1; j < length; ++j) {
                            if (token[j] == '(') {
                                tot++;
                            }
                            if (token[j] == ')' && --tot == 0) {
                                if (j + 1 < length && token[j + 1] == '*') { // 括号后面如果有*也需要带上
                                    end = j + 1;
                                } else {
                                    end = j;
                                }
                                v.emplace_back(token.substr(start, end - start + 1));
                                i = end;
                                break;
                            }
                        }
                    } else if (i == length - 1) {
                        vector<string> v1 = splitTokenWithoutRoundBracketAndVerticalLine(
                                token.substr(end + 1, i - (end + 1) + 1));
                        v.insert(v.end(), v1.begin(), v1.end());
                    }
                }
            }
            return v;
        }

        /**
		 * RE转NFA（使用Thompson构造法）
		 * https://blog.csdn.net/weixin_44691608/article/details/110195743
         * Thompson构造法的第一步就是先构建所有基本单元
         * 主运行逻辑程序：通过递归处理子Token处理。
         * 递归的终点:
         * 1. 纯连接的Token（也就是不存在|和*），形如aaabbb, (aaa)(bbb)，(aaa)bbb...这些都等价于aaabbb。
         * 2. 形如x*的单个字符+'*'
         * 递归的转换:
         * @param input
         * @param state
         * @return
         */
        NFAToken innerRun(string input, int state = 1) {
            // 纯连接(不存在|和*)的Token可以直接解决
            if (input.find('|') == string::npos && input.find('*') == string::npos) {
                // 去掉所有的(和)
                for (int i = 0; i < input.length(); ++i) {
                    if (input[i] == '(' || input[i] == ')') {
                        input.erase(i, 1);
                    }
                }
                vector<Edge> nfaEdgeList;
                for (int i = 0; i < input.length(); ++i) {
                    nfaEdgeList.emplace_back(state, state + 1, input[i]);
                    if (i != input.length() - 1) {
                        nfaEdgeList.emplace_back(state + 1, state + 2, '#');
                        state = state + 2;
                    } else { // 结尾
                        state++;
                    }
                }

                NFAToken token;
                token.endState = state;
                token.nfaEdgeList = nfaEdgeList;
                return token;
            }

            // x*可以直接解决
            if (input.length() == 2 && input[1] == '*') {
                vector<Edge> nfaEdgeList;
                nfaEdgeList.emplace_back(state, state + 1, '#');
                nfaEdgeList.emplace_back(state + 1, state + 2, input[0]);
                nfaEdgeList.emplace_back(state + 2, state + 1, '#');
                nfaEdgeList.emplace_back(state + 2, state + 3, '#');
                nfaEdgeList.emplace_back(state, state + 3, '#');

                NFAToken token;
                token.endState = state + 3;
                token.nfaEdgeList = nfaEdgeList;
                return token;
            }

            // 无法用最简单基本可处理的状态处理就分割为单元表达式，然后依次递归处理，这里的单元表达式可能存在单个字符"|"
            vector<string> tokenValueList = splitToken(input);

            // 存在|就全部并联
            if (count(tokenValueList.begin(), tokenValueList.end(), "|")) {
                vector<Edge> nfaEdgeList;
                vector<int> tailNfaEdgeList; // 记录当前
                int startState = state;
                state++;
                for (int i = 0; i < tokenValueList.size(); ++i) {
                    if (tokenValueList[i] != "|") {
                        nfaEdgeList.emplace_back(startState, state, '#');
                        NFAToken subToken = innerRun(tokenValueList[i], state); // 递归处理
                        state = subToken.endState + 1; // 处理完的一个token的endState+加1后作为下一个待处理的token的startState
                        tailNfaEdgeList.emplace_back(subToken.endState);

                        nfaEdgeList.insert(nfaEdgeList.end(), subToken.nfaEdgeList.begin(), subToken.nfaEdgeList.end());
                    }
                }
                for (int i = 0; i < tailNfaEdgeList.size(); ++i) {
                    nfaEdgeList.emplace_back(tailNfaEdgeList[i], state, '#');
                }

                NFAToken token;
                token.nfaEdgeList = nfaEdgeList;
                token.endState = state;
                return token;
            }
            // 不存在|就连接，但是要注意(xxx)*
            else {
                vector<Edge> nfaEdgeList;
                int endState; // 记录整个链的endState
                for (int i = 0; i < tokenValueList.size(); ++i) {
                    string item = tokenValueList[i];

                    if (item[item.length() - 1] == '*') { // (xxx)*要递归处理(xxx)
                        nfaEdgeList.emplace_back(state, state + 1, '#');
                        NFAToken token = innerRun(item.substr(0, item.length() - 1), state + 1);
                        nfaEdgeList.emplace_back(token.endState, state + 1, '#');
                        nfaEdgeList.emplace_back(token.endState, token.endState + 1, '#');
                        nfaEdgeList.emplace_back(state, token.endState + 1, '#');

                        if (i != tokenValueList.size() - 1) {
                            nfaEdgeList.emplace_back(token.endState + 1, token.endState + 2, '#');
                            endState = token.endState + 1;
                            state = endState;
                        } else {
                            endState = token.endState + 1;
                            state = endState;
                        }

                        nfaEdgeList.insert(nfaEdgeList.end(), token.nfaEdgeList.begin(), token.nfaEdgeList.end());
                    } else {
                        NFAToken token = innerRun(item, state);
                        nfaEdgeList.insert(nfaEdgeList.end(), token.nfaEdgeList.begin(), token.nfaEdgeList.end());
                        if (i != tokenValueList.size() - 1) {
                            nfaEdgeList.emplace_back(token.endState, token.endState + 1, '#');
                            state = token.endState + 1;
                        } else {
                            endState = token.endState;
                        }
                    }
                }
                NFAToken token;
                token.nfaEdgeList = nfaEdgeList;
                token.endState = endState;
                return token;
            }
        }

        void run() {
            NFAToken token = innerRun(this->outer->data);
            outer->nfaToken = token;
        }

        /**
         * 将输出转为json字符串
         */
        json toJson() {
            json ret;
            ret["start"] = 1;
            ret["end"] = this->outer->nfaToken.endState;
            for (int i = 0; i < this->outer->nfaToken.nfaEdgeList.size(); ++i) {
                Edge edge = this->outer->nfaToken.nfaEdgeList[i];
                string ch = {edge.ch}; // 直接将char放到json会转为数字，因此这里需要转为string
                json tmp;
                tmp["from"] = edge.from;
                tmp["to"] = edge.to;
                tmp["label"] = ch;
                ret["data"].emplace_back(tmp);
            }
            return ret;
        }
    };

    class NFA2DFA {
    public:
        Lex *outer;

        bool visited[1000] = {0}; // 长度应为token.endState + 1
        int dfaNodeListIdx = 0; // 记录当前结尾状态集的下标，等价于dfaNodeList.length()-1，从而不用重复查询dfaNodeList的长度

        NFA2DFA(Lex *outer) {
            this->outer = outer;
        }

        /**
         * @param pNode 当前的节点下标
         * @param currentDfaNodeListIdx 当前节点属于dfaNodeList中的哪个状态集
         * @return 
         */
        void dfs(int pNode, int currentDfaNodeListIdx) {
            if (!outer->nextNfaEdgeMap.count(pNode)) { // 没有后续边
                return;
            }
            vector<Edge> nextEdgeList = outer->nextNfaEdgeMap.find(pNode)->second; // 当前节点有哪些后续边

            for (int i = 0; i < nextEdgeList.size(); ++i) { // 遍历当前节点的后续边
                int to = nextEdgeList[i].to;
                char ch = nextEdgeList[i].ch;
                map<char, int> newStateMap; // 维护不同状态后继节点的集合，避免相同值的后继节点被分到不同状态
                if (ch == '#') { // 同一状态集
                    this->outer->dfaNodeList[currentDfaNodeListIdx].insert(to); // 下一节点加入当前状态集
                    dfs(to, currentDfaNodeListIdx);
                } else { // 不同状态集，需要注意后继节点如果相同，算同一状态集，有多少个不同的后继节点，状态集就加多少，所以还需要知道后继节点的集合
                    if (!visited[pNode]) {
                        visited[pNode] = true; // 避免无限递归，但是只有非空串后续节点需要记录，空串后续节点可以重复遍历
                        if (!newStateMap.count(ch)) { // 没有记录过这个字符的边，状态集就加一
                            dfaNodeListIdx++;
                            outer->dfaEdgeList.emplace_back(Edge(currentDfaNodeListIdx, dfaNodeListIdx, ch));
                            newStateMap.insert(make_pair(ch, dfaNodeListIdx)); // 记录当前节点的哪些后续边的字符之前已经记录过了
                            // 注意新添加dfaNodeList元素的时候要初始化，所以要判断一下插入的那个位置初始化过没
                            if (this->outer->dfaNodeList.size() > dfaNodeListIdx) { // 之前初始化过
                                this->outer->dfaNodeList[dfaNodeListIdx].insert(to);
                            } else { // 没初始化过
                                set<int> s;
                                s.insert(to);
                                this->outer->dfaNodeList.emplace_back(s);
                            }
                            dfs(to, dfaNodeListIdx);
                        } else { // 当前字符之前已经记录过，就直接在map中找下一个节点属于哪个状态集
                            this->outer->dfaNodeList[newStateMap.find(ch)->second].insert(to);
                            dfs(to, newStateMap.find(ch)->second); // ch相同就相同状态集
                        }
                    } else {
                        // 遍历过的非空边集不需要向后遍历，但是需要记录边,这里就要思考怎么知道后继节点是哪个状态的（非空边的后继节点一定只属于一个状态集，空边的后继节点可以属于多个状态集）
                        // 找到非空边的后继节点属于哪个状态集（一定是找的到的，因为是遍历过了的）
                        for (int j = 0; j < this->outer->dfaNodeList.size(); ++j) { // 遍历状态集数组
                            for (auto k: this->outer->dfaNodeList[j]) { // 遍历状态集
                                if (k == to) {
                                    outer->dfaEdgeList.emplace_back(Edge(currentDfaNodeListIdx, j, ch));
                                }
                            }
                        }
                    }
                }
            }
        }
        /**
         * NFA转DFA(使用子集构造法)
         * 在箭头的方向上，一路值为#的都可以看作为同一状态，直到遇到不为#的箭头，才算下一状态。这样状态数就可以减少
         * 1. 消除空串
         * 2. 消除多重转换
         * 要求：
         * 1. DFA初态只有一个
         * 2. DFA中存在的都是非空转换
         * 思路：
         * Q: 如何找到初态?
         * A: 没有箭头指向的节点就是起点（在上面求NFA的算法中可知，编号为1的节点就是起点），然后找到和这个起点同一状态的所有节点集就是初态。
         * Q: 如何找到同一状态的状态集?
         * A: 从某一节点出发，沿着箭头的边向后深度优先遍历，如果遇到非空串就return
         * 遍历某个的节点的时候，如果这个节点的后继节点是空串，就记录这个节点的编号，加入上个节点的状态集，我们需要维护一个变量，存放某个状态集和后续状态集和路径值，
         * 由于一个状态集会有多个后续状态集，因此整个的数据结构类似于树，因此后续状态集需要存放在一个数组里。
         * */
        void innerRun(NFAToken token) {
            vector<Edge> edgeList = token.nfaEdgeList;

            // 初始化outer->nextNfaEdgeMap变量
            for (int i = 0; i < edgeList.size(); ++i) {
                int from = edgeList[i].from;
                if (outer->nextNfaEdgeMap.count(from) == 0) { // 没有记录过
                    vector<Edge> v;
                    v.emplace_back(edgeList[i]);
                    outer->nextNfaEdgeMap.insert(make_pair(from, v));
                } else { // 记录过
                    auto iter = outer->nextNfaEdgeMap.find(from);
                    iter->second.emplace_back(edgeList[i]);
                }
            }

            // 直接将起点放到状态集的开头，根据前面RE2NFA的算法，可以知道下标为1的节点为起点
            set<int> s;
            s.insert(1);
            outer->dfaNodeList.emplace_back(s);

            // 从起点1开始dfs
            dfs(1, 0);
        }

        void run() {
            innerRun(this->outer->nfaToken);
        }

        json toJson() {
            json ret;
            ret["start"] = 0;
            vector<int> end;
            for (int i = 0; i < this->outer->dfaNodeList.size(); ++i) { // 包括了NFA终态节点的DFA状态就是DFA的非接受终态
                for (int j: this->outer->dfaNodeList[i]) {
                    if (this->outer->nfaToken.endState == j) {
                        end.emplace_back(i);
                        break;
                    }
                }
            }
            ret["end"] = end;
            for (int i = 0; i < this->outer->dfaEdgeList.size(); ++i) {
                Edge edge = this->outer->dfaEdgeList[i];
                string ch = {edge.ch}; // 直接将char放到json会转为数字
                json tmp;
                tmp["from"] = edge.from;
                tmp["to"] = edge.to;
                tmp["label"] = ch;
                ret["data"]["edgeList"].emplace_back(tmp);
            }
            for (int i = 0; i < this->outer->dfaNodeList.size(); ++i) {
                json tmp;
                for (int j : this->outer->dfaNodeList[i]) {
                    tmp.emplace_back(j);
                }
                ret["data"]["nodeList"].emplace_back(tmp);
            }
            return ret;
        }
    };

    class SimplifyDFA {
    public:
        Lex *outer;

        SimplifyDFA(Lex *outer) {
            this->outer = outer;
        }

        /**
         * https://www.tr0y.wang/2021/04/01/%E7%BC%96%E8%AF%91%E5%8E%9F%E7%90%86%EF%BC%88%E4%B8%80%EF%BC%89%EF%BC%9A%E8%AF%8D%E6%B3%95%E5%88%86%E6%9E%90/#nfa-dfa
         * https://zhuanlan.zhihu.com/p/31166841
         * DFA的初态是包含了NFA初态的那个DFA状态；
         * DFA的终态是所有包含了NFA终态的DFA的状态。
         * DFA的非接受状态等于DFA的终态
         * 状态数最少，即意味者把多余的，等价的状态进行合并,这里关键的就是如何划分等价的状态。
         * Hopcroft 算法步骤：
         * 1. 首先把整个状态划分为接收状态和非接受状态，方法是：包含了NFA终态节点的状态集就一定是DFA的非接收状态
         * 2. 遍历某个状态集中存在的字符，看当前状态集的每个节点接收到这个字符跳转到的下一个状态是否相同，如果相同就继续判断下一个字符，直到所有字符都判断成功就可以成功插入到结果。
         * 如果存在不同，就把当前状态集的节点根据当前字符到的目标状态来划分，划分后的结果重新入栈处理。
         * 要注意等价状态，如果两个状态s，t，对于所有输入符号，状态s和状态t都转换到等价的状态里，那么这两个状态就是等价的
         * 可区分：对于任何两个状态t和s，若从一状态出发接受输入字符串ω，而从另一状态出发不接受ω，或者从t出发和从s出发到达不同的接受状态，则称ω对状态t和s是可区分的。
         * 不可区分：设想任何输入序列ω对s和t均是不可区分的，则说明从s出发和从t出发，分析任何输入序列ω均得到相同结果。因此，s和t可以合并成一个状态
         */
        void simplifyDFA() {
            map<int, int> belongStateMap; // 记录某个节点属于哪个状态集
            set<int> isAcceptedStateSet;  // 接收状态
            set<int> notAcceptedStateSet; // 非接收状态
            int length = 2; // 至少可以划分为接受状态和非接受状态


            // 划分接收状态和非接受状态
            for (int i = 0; i < this->outer->dfaNodeList.size(); ++i) { // 包括了NFA终态节点的DFA状态就是DFA的非接受终态
                bool isAccepted = true;
                for (int j: this->outer->dfaNodeList[i]) {
                    if (this->outer->nfaToken.endState == j) {
                        isAccepted = false;
                        notAcceptedStateSet.insert(i);
                        belongStateMap.insert(make_pair(i, 1));
                        break;
                    }
                }
                if (isAccepted) {
                    isAcceptedStateSet.insert(i);
                    belongStateMap.insert(make_pair(i, 0));
                }
            }

            map<int, vector<Edge>> nextEdgeMap; // key为起点的边
            // 初始化nextEdgeMap
            for (int i = 0; i < this->outer->dfaEdgeList.size(); ++i) {
                int from = this->outer->dfaEdgeList[i].from;
                if (nextEdgeMap.count(from) == 0) {
                    vector<Edge> v;
                    v.emplace_back(this->outer->dfaEdgeList[i]);
                    nextEdgeMap.insert(make_pair(from, v));
                } else {
                    auto iter = nextEdgeMap.find(from);
                    iter->second.emplace_back(this->outer->dfaEdgeList[i]);
                }
            }

            stack<set<int>> workStack;

            workStack.push(isAcceptedStateSet);
            workStack.push(notAcceptedStateSet);

            while (!workStack.empty()) {
                set<int> currentPartition = workStack.top();
                workStack.pop();

                if (currentPartition.empty()) {
                    continue;
                }

                if (currentPartition.size() == 1) { // 节点数为一肯定无法划分
                    this->outer->simplifiedDfaNodeList.emplace_back(currentPartition);
                    continue;
                }
                // 注意：每个点的后续边的字符权值肯定只会在每个边只会出现一次，不然肯定会在NFA转DFA的时候合并到一个状态集，因此我们不用担心一个字符权值出现多次的情况
                map<int, map<char, int>> charStateMapMap; // 记录每个点后续边的ch和toState的关系map
                // 初始化charStateMapMap
                for (int i: currentPartition) {
                    //在map.find的时候一定要先判断map中有没有这个元素，不然可能有时候会报错有时候不会，导致很难debug
                    if (!nextEdgeMap.count(i)) {
                        map<char, int> m;
                        charStateMapMap.insert(make_pair(i, m));
                        continue;
                    }
                    // 没有上面的判断下面会报错，因为这里在nextEdgeMap找不到i，但是编译器却往后走了（看编译器），导致后面的->second为野指针。
                    // terminate called after throwing an instance of 'std::bad_array_new_length'
                    //  what():  std::bad_array_new_length
                    vector<Edge> edgeList = nextEdgeMap.find(i)->second;
                    map<char, int> charStateMap; // map内部会对key进行排序，有利于比较
                    for (Edge edge: edgeList) {
                        charStateMap.insert(make_pair(edge.ch, belongStateMap.find(edge.to)->second));
                    }
                    charStateMapMap.insert(make_pair(i, charStateMap));
                }
                map<char, int> charStateMap = charStateMapMap.find(*currentPartition.begin())->second;
                for (char ch = ' '; ch <= '~'; ++ch) { // 可显示字符：开头为空格，对于的ASCII码为32，结尾为~，对应的ascii码是126
                    map<int, set<int>> partitionMap;

                    if (!charStateMap.count(ch)) { // 不存在某个字符的边就当toState=-1
                        set<int> s;
                        s.insert(*currentPartition.begin());
                        partitionMap.insert(make_pair(-1, s));
                    } else {
                        set<int> s;
                        s.insert(*currentPartition.begin());
                        partitionMap.insert(make_pair(charStateMap.find(ch)->second, s));
                    }

                    for (int i: currentPartition) {
                        if (i == *currentPartition.begin()) {
                            continue;
                        }
                        map<char, int> _charStateMap = charStateMapMap.find(i)->second;
                        int _toState = _charStateMap.find(ch)->second;
                        if (!_charStateMap.count(ch)) { // 不存在ch
                            if (partitionMap.count(-1)) {
                                partitionMap.find(-1)->second.insert(i);
                            } else {
                                set<int> s;
                                s.insert(i);
                                partitionMap.insert(make_pair(-1, s));
                            }
                        } else {
                            if (partitionMap.count(_toState)) {
                                partitionMap.find(_toState)->second.insert(i);
                            } else {
                                set<int> s;
                                s.insert(i);
                                partitionMap.insert(make_pair(_toState, s));
                            }
                        }
                    }

                    // 看看是不是可分割的，如果是就将初步分割后的结果再次入栈
                    if (partitionMap.size() != 1) {
                        for (auto i: partitionMap) {
                            length++;
                            workStack.push(i.second);
                            for (int j: i.second) { // 修改节点所属于的状态集
                                belongStateMap.find(j)->second = length;
                            }
                        }
                        goto last;
                    }
                }

                this->outer->simplifiedDfaNodeList.emplace_back(currentPartition);

                last:;
            }
            // 计算连边，记得去重
            for (int i = 0; i < this->outer->dfaEdgeList.size(); ++i) {
                Edge edge = this->outer->dfaEdgeList[i];
                int from = edge.from;
                int to = edge.to;
                char ch = edge.ch;
                int fromState = belongStateMap.find(from)->second;
                int toState = belongStateMap.find(to)->second;
                bool isExisted = false;
                for (int j = 0; j < this->outer->simplifiedDfaEdgeList.size(); ++j) {
                    if (fromState == this->outer->simplifiedDfaEdgeList[j].from &&
                        toState == this->outer->simplifiedDfaEdgeList[j].to &&
                        ch == this->outer->simplifiedDfaEdgeList[j].ch) {
                        isExisted = true;
                    }
                }
                if (!isExisted) {
                    this->outer->simplifiedDfaEdgeList.emplace_back(Edge(fromState, toState, ch));
                }

                // 找到起点的状态集下标
                this->outer->simplifiedDfaStartState = belongStateMap.find(0)->second;
            }
        }

        void run() {
            simplifyDFA();
        }

        json toJson() {
            json ret;
            ret["start"] = this->outer->simplifiedDfaStartState;
            ret["end"] = 1;
            for (int i = 0; i < this->outer->simplifiedDfaEdgeList.size(); ++i) {
                Edge edge = this->outer->simplifiedDfaEdgeList[i];
                string ch = {edge.ch}; // 直接将char放到json会转为数字
                json tmp;
                tmp["from"] = edge.from;
                tmp["to"] = edge.to;
                tmp["label"] = ch;
                ret["data"].emplace_back(tmp);
            }
            return ret;
        }
    };

    class DFA2Code {
    public:
        Lex *outer;

        DFA2Code(Lex *outer) {
            this->outer = outer;
        }

        string innerRun(int startState, int endState) {
            map<int, vector<Edge>> nextEdgeMap; // 某个节点有哪些边
            for (auto i: this->outer->simplifiedDfaEdgeList) {
                int from = i.from;
                int to = i.to;
                char ch = i.ch;
                if (nextEdgeMap.count(from)) { // 存在
                    nextEdgeMap.find(from)->second.emplace_back(i);
                } else {
                    vector<Edge> v;
                    v.emplace_back(i);
                    nextEdgeMap.insert(make_pair(from, v));
                }
            }

            string ret;
            ret += "#include <iostream>\n"
                   "#include <string>\n"
                   "using namespace std;\n"
                   "\n"
                   "bool isAccepted(string str) {\n";

            ret += "    int currentState = " + to_string(startState) + ";\n";
            ret += "    int endState = " + to_string(endState) + ";\n";
            ret += "    for (int i = 0; i < str.size(); ++i) {\n"
                   "        switch (currentState) {\n";

            for (auto i: nextEdgeMap) {
                for (int j = 0; j < i.second.size(); ++j) {
                    int from = i.second[j].from;
                    int to = i.second[j].to;
                    string ch = {i.second[j].ch};

                    if (j == 0) {
                        ret += "            case " + to_string(from) + ":\n";
                        ret += "                if (str[i] == '" + ch + "') {\n";
                        ret += "                    currentState = " + to_string(to) + ";\n";
                        ret += "                }\n";
                    } else {
                        ret += "                else if (str[i] == '" + ch + "') {\n";
                        ret += "                    currentState = " + to_string(to) + ";\n";
                        ret += "                }\n";
                    }
                }
                ret += "                else {\n"
                       "                    return false;\n"
                       "                }\n"
                       "                break;\n";
            }
            ret += "            default:\n"
                   "                return false;\n"
                   "        }\n"
                   "    }\n"
                   "    if (currentState == endState) {\n"
                   "        return true;\n"
                   "    }\n"
                   "    return false;\n"
                   "}\n"
                   "\n"
                   "int main() {\n"
                   "    string str;\n"
                   "    cin >> str;\n"
                   "    if (isAccepted(str)) {\n"
                   "        cout << \"accepted\" << endl;\n"
                   "    }\n"
                   "    else {\n"
                   "        cout << \"can not accepted\" << endl;\n"
                   "    }\n"
                   "    return 0;\n"
                   "}";

            return ret;
        }

        void run() {
            this->outer->outputCode = innerRun(this->outer->simplifiedDfaStartState, 1);
        }
    };

    void run() {
        inner1->run();
        inner2->run();
        inner3->run();
        inner4->run();
    }

    json toJson() {
        json ret;
        ret["inner1"] = inner1->toJson();
        ret["inner2"] = inner2->toJson();
        ret["inner3"] = inner3->toJson();
        ret["outputCode"] = outputCode;

        return ret;
    }

    Lex() {};

    Lex(string data) {
        this->data = data;
        // 让内部类可以访问外部类
        this->inner1 = new RE2NFA(this);
        this->inner2 = new NFA2DFA(this);
        this->inner3 = new SimplifyDFA(this);
        this->inner4 = new DFA2Code(this);
    }

};


int main(int argc, char **argv) {
    // ./xx.exe <re>
    if (argc != 2) {
        cout << "[ERROR]: incorrect parameter number!" << endl;
        return 0;
    }
    Lex lex = Lex(argv[1]);
    lex.run();
    cout << lex.toJson().dump(4);
    return 0;
}
