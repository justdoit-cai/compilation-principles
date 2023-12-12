#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <fstream>
#include <queue>
#include "nlohmann/json.hpp"

using namespace std;
using nlohmann::json;
#define EXT_START '~'

struct Production {
    char leftPart;
    string rightPart;
};

struct Project {
    char leftPart;
    string rightPart;

    bool operator==(const Project &project) const { //两个const一个都不能少
        return leftPart == project.leftPart && rightPart == project.rightPart;
    }

    bool operator<(const Project &project) const { //两个const一个都不能少
        if (leftPart != project.leftPart)
            return leftPart < project.leftPart;
        else
            return rightPart < project.rightPart;
    }
};

struct DfaNode {
    set<Project> projectSet; // 相同状态的项目列表
    bool operator==(const DfaNode &dfaNode) const { //两个const一个都不能少
        if (projectSet.size() != dfaNode.projectSet.size())
            return false;
        for (auto i1 = projectSet.begin(), i2 = dfaNode.projectSet.begin(); i1 != projectSet.end(); i1++, i2++) {
            if (i1->leftPart != i2->leftPart || i1->rightPart != i2->rightPart) {
                return false;
            }
        }
        return true;
    }
};

struct DfaEdge {
    int from, to;
    char val;;
};
// 文法产生式的存储
vector<Production> productions;
// 非终结符的存储
set<char> nonTerminatorSet;
// 所有符号的存储，包括@
set<char> charSet = {'@'};
map<char, set<char>> firstSetMap;
map<char, set<char>> followSetMap;
vector<DfaNode> dfaNodeList;
vector<DfaEdge> dfaEdgeList;
bool isSlr1 = true;
string errResult;
// SLR1算法分析表的存储
vector<map<char, string>> slr1Table;

bool isNonTerminator(char ch);

vector<string> split(string str, string spacer);

string readFile(string filename);

void getFirstSet();

void getFollowSet();

string num2str(int num);

string char2str(char ch);

void getLr0Dfa();

void checkSlr1();

void getSlr1Table();

string base64_decode(string encoded_string);

queue<int> workQueue;

int main(int argc, char **argv) {
    if (argc != 2) {
        cout << "format: main.exe <grammars>";
        return 0;
    }
    string str = argv[1];
    str = base64_decode(str);
//    string str = readFile("../test09");
    json ret;
    vector<string> lineList = split(str, "\n");
    for (string line: lineList) {
        if (line.empty()) {
            continue;
        }
        ret["grammars"].push_back(line);
        nonTerminatorSet.insert(line[0]);
        Production p;
        p.leftPart = line[0];
        p.rightPart = line.substr(3);
        productions.push_back(p);
        charSet.insert(p.leftPart);
        for (char ch: p.rightPart) {
            charSet.insert(ch);
        }
    }
    getFirstSet();
    getFollowSet();
    getLr0Dfa();
    checkSlr1();

    if (isSlr1) {
        getSlr1Table();
    }
    for (char ch: nonTerminatorSet) {
        json j;
        j["char"] = char2str(ch);
        for (char ch1: firstSetMap.find(ch)->second) {
            j["firstSet"].push_back(char2str(ch1));
        }
        for (char ch2: followSetMap.find(ch)->second) {
            j["followSet"].push_back(char2str(ch2));
        }
        ret["firstAndFollowSet"].push_back(j);
    }
    for (DfaNode node: dfaNodeList) {
        json j1;
        for (Project p: node.projectSet) {
            json j2;
            j2["left"] = char2str(p.leftPart);
            j2["right"] = p.rightPart;
            j1.push_back(j2);
        }
        ret["lr0"]["node"].push_back(j1);
    }
    for (DfaEdge edge: dfaEdgeList) {
        json j1;
        j1["from"] = edge.from;
        j1["to"] = edge.to;
        j1["label"] = char2str(edge.val);
        ret["lr0"]["edge"].push_back(j1);
    }
    ret["isSlr1"] = isSlr1;
    if (isSlr1) {
        for (map<char, string> m: slr1Table) {
            json j;
            for (auto i: m) {
                j[char2str(i.first)] = i.second;
            }
            ret["slr1"].push_back(j);
        }
    } else {
        ret["errResult"] = errResult;
    }
    cout << ret.dump(2);

    return 0;
}

bool isNonTerminator(char ch) {
    return isupper(ch);
}

vector<string> split(string str, string spacer) {
    vector<string> v;
    int pos1, pos2;
    int len = spacer.length();     //记录分隔符的长度
    pos1 = 0;
    pos2 = str.find(spacer);
    while (pos2 != string::npos) {
        v.push_back(str.substr(pos1, pos2 - pos1));
        pos1 = pos2 + len;
        pos2 = str.find(spacer, pos1);    // 从str的pos1位置开始搜寻spacer
    }
    if (pos1 != str.length()) //分割最后一个部分
        v.push_back(str.substr(pos1));
    return v;
}

string readFile(string filename) {
    string result;
    ifstream in;
    in.open(filename);
    if (!in.is_open()) {
        cout << "error opening file!" << endl;
        exit(0);
    }

    while (!in.eof()) {
        result += in.get();
    }
    in.close();

    return result.substr(0, result.length() - 1);
}

void getFirstSet() {
    map<char, set<char>> ret;
    int firstSetSize = nonTerminatorSet.size();
    set<char> solvedSet;

    // 初始化
    for (char it: nonTerminatorSet) {
        ret.insert(make_pair(it, set<char>()));
    }
    // 先无脑放
    for (Production production: productions) {
        // 注意E->E+T的特殊情况
        if (production.leftPart != production.rightPart[0]) {
            ret.find(production.leftPart)->second.insert(production.rightPart[0]);
        }
    }
    // 注意A->BC,B->@的情况，此时等价于A->C,因此还需把C放入A的firstSet
    // 还可能A->BC,B->D,D->@，此时也是等价于A->C
    // A->BC,B->DE,D->@,E->@,也就是等价于A->C
    // 下面的核心逻辑就是拿已经解决了的字符的firstSet去化简别的没有处理好的字符
    // 记录哪些非终结符已经处理好了firstSet
    while (solvedSet.size() < firstSetSize) {
        for (auto it1: ret) {
            bool selfSolved = true;
            char nowCh = it1.first;
            set<char> nowFirstSet = it1.second;
            // 优化：以前判断过就不用判断了
            if (solvedSet.count(nowCh) > 0) {
                selfSolved = true;
            } else {
                for (char ch: nowFirstSet) {
                    // 肯定还没完成
                    if (isNonTerminator(ch) && ch != '@') {
                        selfSolved = false;
                        break;
                    }
                }
            }
            // 如果nowCh已经解决了，就拿这个符号的firstSet去化简其它的符号。
            // 需要注意的是，如果这个符号ch的firstSet中含有空字符@，那么在用这个ch符号的firstSet化简其它符号的时候还需要引入对于那个符号的leftPart中当前ch符号的下一个符号
            if (selfSolved) {
                solvedSet.insert(nowCh);
                for (auto &it2: ret) {
                    if (it2.first == nowCh || !it2.second.count(nowCh)) {
                        continue;
                    }
                    if (nowFirstSet.count('@') > 0) {
                        // 这里移除空字符是不影响原firstSet的
                        nowFirstSet.erase('@');
                        for (Production production: productions) {
                            if (production.leftPart == it2.first) {
                                for (int i = 0; i < production.rightPart.length(); i++) {
                                    char ch = production.rightPart[i];
                                    // 如果当前字符处理好了，并且其中含有空字符，就把下一个放进来
                                    if (solvedSet.count(ch) > 0 && ret.find(ch)->second.count('@') > 0) {
                                        if (i < production.rightPart.length() - 1) {
                                            it2.second.insert(production.rightPart[i + 1]);
                                        } else {
                                            // 如果到了最末尾，就加入空字符进来
                                            it2.second.insert('@');
                                        }
                                    } else {
                                        break;
                                    }
                                }
                            }
                        }
                    }
                    // 删除并替换原元素
                    it2.second.erase(nowCh);
                    it2.second.insert(nowFirstSet.begin(), nowFirstSet.end());
                }
            }
        }
    }
    firstSetMap = ret;
}

void getFollowSet() {
    map<char, set<char>> ret;
    int followSetSize = nonTerminatorSet.size();
    set<char> solvedSet;

    // 初始化
    for (char it: nonTerminatorSet) {
        ret.insert(make_pair(it, set<char>()));
    }
    // 起始符号的followSet中一定有@
    ret.find(productions[0].leftPart)->second.insert('@');
    // 遍历所有可能的相邻字符
    for (Production production: productions) {
        for (int i = 0; i < production.rightPart.length(); ++i) {
            char ch = production.rightPart[i];
            // 不是非终结符就跳过，因为我们只需要求非终结符的follow集
            if (isNonTerminator(ch)) {
                // 如果当前的终结符不是最后一个元素
                if (i < production.rightPart.length() - 1) {
                    // 获取当前终结符的下一个元素
                    char nextCh = production.rightPart[i + 1];
                    // 如果下一个相邻的字符是终结符就直接加入
                    if (!isNonTerminator(nextCh)) {
                        ret.find(ch)->second.insert(nextCh);
                    }
                        // 如果下一个相邻的字符是非终结符就将First(nextCh)加入，
                        // 并且如果@在First(nextCh)中，则还需要递归的加入并判断下一个nextCh的nextCh的First集，直到到末尾如果当前字符的First集中存在@，就把Follow(leftPart)加入
                    else {
                        // 获取下一个元素的first集
                        set<char> s = firstSetMap.find(nextCh)->second;
                        // 如果可能为空则还需要继续判断后面的字符，因为要注意A->BCD，其中C的first集中含有@，因此B的follow集还需要加上D的first集
                        if (s.count('@') > 0) {
                            // 当前已经是最后一个元素了，如果存在@，就需要加上左边的follow集
                            if (i + 2 == production.rightPart.length()) {
                                ret.find(ch)->second.insert(production.leftPart);
                            }
                            // 如果不是最后一个元素
                            for (int j = i + 2; j < production.rightPart.length(); ++j) {
                                // 一直获取并加入下个字符的first集，还是注意要去掉@，不过要先判断有没有@
                                set<char> tmpChFirstSet = firstSetMap.find(production.rightPart[j])->second;
                                if (tmpChFirstSet.count('@') > 0) {
                                    tmpChFirstSet.erase('@');
                                    // 如果是最后一个元素，则可以直接将左边的符号的follow集加入到当前ret元素的follow集了
                                    if (j == production.rightPart.length() - 1) {
                                        ret.find(ch)->second.insert(production.leftPart);
                                    }
                                }
                                ret.find(ch)->second.insert(tmpChFirstSet.begin(), tmpChFirstSet.end());
                            }
                        }
                        // 加入的时候需要注意去掉@空字符
                        s.erase('@');
                        ret.find(ch)->second.insert(s.begin(), s.end());
                    }
                }
                    // 如果当前的非终结符是最后一个元素就需要直接把生成式左边的终结符的follow集和@加入当前终结符的follow集
                else {
                    ret.find(ch)->second.insert(production.leftPart);
                    ret.find(ch)->second.insert('@');
                }
            }
        }
    }
    // 去除掉自己的followSet包含自己的
    for (auto &it: ret) {
        it.second.erase(it.first);
    }
    while (solvedSet.size() < followSetSize) {
        // 检查哪些已经字符的FollowSet已经处理好了，如果处理好了就用这个字符的FollowSet去化简别人的
        for (auto it1: ret) {
            bool solved = true;
            if (solvedSet.count(it1.first) > 0) {
                solved = true;
            } else {
                for (char ch: it1.second) {
                    if (isNonTerminator(ch)) {
                        solved = false;
                    }
                }
            }
            if (solved) {
                solvedSet.insert(it1.first);
                for (auto &it2: ret) {
                    if (it2.second.count(it1.first) > 0) {
                        it2.second.erase(it1.first);
                        it2.second.insert(it1.second.begin(), it1.second.end());
                    }
                }
            }
        }
    }
    followSetMap = ret;
}

/**
 * 数字转字符串
 * @return
 */
string num2str(int num) {
    string s;
    s += (char) num;
    return s;
}

string char2str(char ch) {
    string s;
    s += ch;
    return s;
}

void getLr0Dfa() {
    DfaNode firstNode;
    firstNode.projectSet.insert(Project{EXT_START, "." + char2str(productions[0].leftPart)});
    // 遍历所有产生式，设置起点符号开头的放入起始状态，这里默认加上第一个产生式的左部为起点符号
    for (Production production: productions) {
        if (production.leftPart == productions[0].leftPart) {
            // 注意如果有S->@，则是转换为S->.，而非S->.@
            if (production.rightPart == "@") {
                firstNode.projectSet.insert(Project{production.leftPart, "."});
            } else {
                firstNode.projectSet.insert(Project{production.leftPart, "." + production.rightPart});
            }
        }
    }
    dfaNodeList.push_back(firstNode);
    // 由起始状态延申至其它状态
    workQueue.push(0);
    while (!workQueue.empty()) {
        int pNodeIdx = workQueue.front();
        workQueue.pop();
        DfaNode pNode = dfaNodeList[pNodeIdx];
        // 记录当前状态集接收到某个字符会到哪个状态，避免当前状态集前后有多个可以接收相同字符的情况
        // 例如：A->.ab, A->.ac
        map<char, int> stateMap;
        // 暂存区
        vector<DfaNode> tmpDfaNodeList;
        // 记录到达tmpDfaNodeList中某个Node经历的是哪个char字符
        map<int, char> edgeMap;
        for (Project project: pNode.projectSet) {
            DfaNode node;
            string rightPart = project.rightPart;
            int dotIdx = rightPart.find('.');
            if (dotIdx == rightPart.length() - 1) {
                continue;
            }
            // 点号后面相邻的字符
            char dotNextChar = rightPart[dotIdx + 1];
            // 点号后移一位
            rightPart.erase(dotIdx, 1);
            rightPart.insert(dotIdx + 1, ".");

            node.projectSet.insert(Project{project.leftPart, rightPart});
            if (dotIdx == rightPart.length() - 2) {
                // 如果以前记录了这个字符的移进
                if (stateMap.count(dotNextChar) > 0) {
                    tmpDfaNodeList[stateMap.find(dotNextChar)->second].projectSet.insert(node.projectSet.begin(),
                                                                                         node.projectSet.end());
                } else {
                    tmpDfaNodeList.push_back(node);
                    stateMap.insert(make_pair(dotNextChar, tmpDfaNodeList.size() - 1));
                    // 记录边
                    edgeMap.insert(make_pair(tmpDfaNodeList.size() - 1, dotNextChar));
                }
                continue;
            }
            char dotNextNextChar = rightPart[dotIdx + 2];
            // 如果dotNextChar后面的字符是非终结符，还需要加上这个非终结符的产生式
            if (isNonTerminator(dotNextNextChar)) {
                for (Production production: productions) {
                    if (production.leftPart == dotNextNextChar) {
                        if (production.rightPart == "@") {
                            node.projectSet.insert(Project{production.leftPart, "."});
                        } else {
                            node.projectSet.insert(Project{production.leftPart, "." + production.rightPart});
                        }
                    }
                }
            }

            // 如果以前记录了这个字符的移进
            if (stateMap.count(dotNextChar) > 0) {
                tmpDfaNodeList[stateMap.find(dotNextChar)->second].projectSet.insert(node.projectSet.begin(),
                                                                                     node.projectSet.end());
            } else {
                tmpDfaNodeList.push_back(node);
                stateMap.insert(make_pair(dotNextChar, tmpDfaNodeList.size() - 1));
                // 记录边
                edgeMap.insert(make_pair(tmpDfaNodeList.size() - 1, dotNextChar));
            }
        }
        // 全部计算完后再将暂存区中的Node放入全局变量dfaNodeList中。
        // 还需要注意这里可能新产生的node会和原来的重复，我们需要判断一下有没有和原来的重复，如过重复我们就不需要插入，而且连边就连原来的就可以了
        // 而且如果重复了，说明原来的node肯定已经遍历过了，这样我们就不需要在把这个node入queue了，不然还是会死循环。
        for (int i = 0; i < tmpDfaNodeList.size(); ++i) {
            DfaNode node1 = tmpDfaNodeList[i];
            bool isNew = true;
            for (int j = 0; j < dfaNodeList.size(); ++j) {
                DfaNode node2 = dfaNodeList[j];
                if (node1 == node2) {
                    isNew = false;
                    // 关于node1的连边就要指向node2
                    dfaEdgeList.push_back(DfaEdge{pNodeIdx, j, edgeMap.find(i)->second});
                    break;
                }
            }
            if (isNew) {
                dfaNodeList.push_back(node1);
                int newIdx = dfaNodeList.size() - 1;
                dfaEdgeList.push_back(DfaEdge{pNodeIdx, newIdx, edgeMap.find(i)->second});
                workQueue.push(newIdx);
            }
        }
    }
}

/**
 * SLR(1)文法的判断方式:
 * 对于任何状态s，满足：
 * 1. 对于在s中的任何项目A->a.Xb,当X是一个终结符,且X在Follow(B)中时，s中没有完整的项目B->y.
 * 对于第一点最好反证：
 * 如果s中存在A->y.，则s中其它形如B->a.Xb，X是一个终结符的项目，Follow(A)中的字符不可能存在X
 * 2. 对于在s中的任何两个完整项目A->a.和B->b.，Follow(A)和Follow(B)的交集为空
 */
void checkSlr1() {
    for (int i = 0; i < dfaNodeList.size(); ++i) {
        DfaNode node = dfaNodeList[i];
        for (Project project1: node.projectSet) {
            char leftPart1 = project1.leftPart;
            if (leftPart1 == EXT_START) {
                continue;
            }
            set<char> followSet1 = followSetMap.find(leftPart1)->second;
            string rightPart1 = project1.rightPart;
            int dotIdx1 = rightPart1.find('.');
            // A->a.的形式
            if (dotIdx1 == rightPart1.length() - 1) {
                for (Project project2: node.projectSet) {
                    if (project1 == project2) {
                        continue;
                    }
                    char leftPart2 = project2.leftPart;
                    if (leftPart2 == EXT_START) {
                        continue;
                    }
                    set<char> followSet2 = followSetMap.find(leftPart2)->second;
                    string rightPart2 = project2.rightPart;
                    int dotIdx2 = rightPart2.find('.');
                    // 判断第二点
                    if (dotIdx2 == rightPart2.length() - 1) {
                        set<char> intersection;
                        set_intersection(followSet1.begin(), followSet1.end(),
                                         followSet2.begin(), followSet2.end(),
                                         inserter(intersection, intersection.begin()));
                        if (intersection.size() != 0) {
                            isSlr1 = false;
                            errResult = "状态" + to_string(i) + "中的项目" + project1.leftPart + "->" + project1.rightPart + "和项目"
                                    + project2.leftPart + "->" + project2.rightPart + "发生了冲突! Follow(" + project1.leftPart + ")和Follow(" + project2.rightPart + ")的交集不为空!";
                        }
                    }
                        // 判断第一点
                    else {
                        char dotNextChar2 = rightPart2[dotIdx2 + 1];
                        if (!isNonTerminator(dotNextChar2)) {
                            if (followSet1.count(dotNextChar2) > 0) {
                                isSlr1 = false;
                                errResult = "状态" + to_string(i) + "中的项目" + project1.leftPart + "->" + project1.rightPart + "和项目"
                                            + project2.leftPart + "->" + project2.rightPart + "发生了冲突! " + dotNextChar2 + "是一个终结符，并且其在Follow(" + project1.rightPart + ")中!";
                            }
                        }
                    }
                }
            }
        }
    }
}

void getSlr1Table() {
    vector<map<char, string>> ret;
    for (int i = 0; i < dfaNodeList.size(); ++i) {
        DfaNode node = dfaNodeList[i];
        map<char, string> row;
        // 通过边寻找移进
        for (DfaEdge edge: dfaEdgeList) {
            if (edge.from == i) {
                if (isNonTerminator(edge.val)) {
                    row.insert(make_pair(edge.val, to_string(edge.to)));
                } else {
                    row.insert(make_pair(edge.val, "s" + to_string(edge.to)));
                }
            }
        }
        // 通过状态里的项目寻找归约
        for (Project p: node.projectSet) {
            if (p.rightPart.find('.') == p.rightPart.length() - 1) {
                if (p.leftPart == EXT_START) {
                    row.insert(make_pair('@', "acc"));
                    continue;
                }
                set<char> followSet = followSetMap.find(p.leftPart)->second;
                for (char ch: followSet) {
                    // 去掉末尾的.，但是如果只有一个点就转为@
                    if (p.rightPart == ".") {
                        row.insert(make_pair(ch, "r(" + char2str(p.leftPart) + "->@)"));
                    } else {
                        row.insert(make_pair(ch, "r(" + char2str(p.leftPart) + "->" +
                                                 p.rightPart.substr(0, p.rightPart.length() - 1) + ")"));
                    }
                }
            }
        }
        for (char ch: charSet) {
            // 没有记录过，也就是无法移进的，也无法归约的，填入error标志
            if (row.count(ch) == 0) {
                row.insert(make_pair(ch, ""));
            }
        }
        ret.push_back(row);
    }
    slr1Table = ret;
}

string base64_decode(string encoded_string) {
    const string base64_chars =
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "abcdefghijklmnopqrstuvwxyz"
            "0123456789+/";

    string decoded_string;
    int in_len = encoded_string.size();
    int i = 0, j = 0, in_ = 0;
    unsigned char char_array_4[4], char_array_3[3];

    while (in_len-- && (encoded_string[in_] != '=') &&
           (isalnum(encoded_string[in_]) || (encoded_string[in_] == '+') || (encoded_string[in_] == '/'))) {
        char_array_4[i++] = encoded_string[in_];
        in_++;
        if (i == 4) {
            for (i = 0; i < 4; i++) {
                char_array_4[i] = base64_chars.find(char_array_4[i]);
            }

            char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
            char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
            char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

            for (i = 0; i < 3; i++) {
                decoded_string += char_array_3[i];
            }
            i = 0;
        }
    }

    if (i) {
        for (j = i; j < 4; j++) {
            char_array_4[j] = 0;
        }

        for (j = 0; j < 4; j++) {
            char_array_4[j] = base64_chars.find(char_array_4[j]);
        }

        char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
        char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
        char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

        for (j = 0; j < i - 1; j++) {
            decoded_string += char_array_3[j];
        }
    }

    return decoded_string;
}


