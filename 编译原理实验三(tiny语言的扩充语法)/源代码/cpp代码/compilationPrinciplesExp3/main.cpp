#include <stdlib.h>
#include <string>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <map>
#include "nlohmann/json.hpp"
#include <cstring>

using namespace std;
using nlohmann::json;
#define MAXRESERVED 16
#define MAXCHILDREN 3


typedef enum {
    ERROR,
    ENDFILE,
    IF,
    THEN,
    ELSE,
    END,
    REPEAT,
    UNTIL,
    READ,
    WRITE,
    ID,
    NUM,
    ASSIGN,
    EQ,
    LT,
    NE,
    GT,
    AND,
    OR,
    NOT,
    LE,
    GE,
    PLUS,
    MINUS,
    TIMES,
    OVER,
    LPAREN,
    RPAREN,
    SEMI,
    FOR,
    DO,
    ENDDO,
    TO,
    DOWNTO,
    MOD,
    POW,
    SELF_PLUS,
    SELF_MINUS
} TokenType;

map<int, string> tokenNameMap = {
        {ERROR,      "ERROR"},
        {ENDFILE,    "ENDFILE"},
        {IF,         "IF"},
        {THEN,       "THEN"},
        {ELSE,       "ELSE"},
        {END,        "END"},
        {REPEAT,     "REPEAT"},
        {UNTIL,      "UNTIL"},
        {READ,       "READ"},
        {WRITE,      "WRITE"},
        {ID,         "ID"},
        {NUM,        "NUM"},
        {ASSIGN,     "ASSIGN"},
        {EQ,         "="},
        {LT,         "<"},
        {NE,         "<>"},
        {GT,         ">="},
        {AND,        "and"},
        {OR,         "or"},
        {NOT,        "not"},
        {LE,         "<="},
        {GE,         ">="},
        {PLUS,       "+"},
        {MINUS,      "-"},
        {TIMES,      "*"},
        {OVER,       "/"},
        {LPAREN,     "LPAREN"},
        {RPAREN,     "RPAREN"},
        {SEMI,       "SEMI"},
        {FOR,        "FOR"},
        {DO,         "DO"},
        {ENDDO,      "ENDDO"},
        {TO,         "TO"},
        {DOWNTO,     "DOWNTO"},
        {MOD,        "%"},
        {POW,        "^"},
        {SELF_PLUS,  "++"},
        {SELF_MINUS, "--"}
};


typedef enum {
    START_STATE, ASSIGN_STATE, COMMENT_STATE, NUM_STATE, ID_STATE, DONE_STATE
} StateType;

/**
 * 语句类型
 */
typedef enum {
    IF_TYPE, REPEAT_TYPE, ASSIGN_TYPE, READ_TYPE, WRITE_TYPE, FOR_TYPE, SELF_PLUS_TYPE,SELF_MINUS_TYPE
} StmtType;

/**
 * 表达式的类型
 */
typedef enum {
    OP_TYPE, CONST_TYPE, ID_TYPE, SINGLE_EXP_TYPE
} ExpType;

/**
 * 节点类型
 */
typedef enum {
    STMT_TYPE, EXP_TYPE
} NodeType;

struct {
    string str;
    TokenType tok;
} reservedWords[MAXRESERVED]
        = {{"if",     IF},
           {"then",   THEN},
           {"else",   ELSE},
           {"end",    END},
           {"repeat", REPEAT},
           {"until",  UNTIL},
           {"read",   READ},
           {"write",  WRITE},
           {"for",    FOR},
           {"to",     TO},
           {"downto", DOWNTO},
           {"and",    AND},
           {"or",     OR},
           {"not",    NOT},
           {"do",     DO},
           {"enddo",  ENDDO}};

/**
 * NodeType分为Statement和Expression
 * Statement分为if,repeat,for,read,write,identifier,assign
 * Expression分为op,const,identifier
 * op分为>,<,>=,<=,<>
 */
typedef struct treeNode {
    struct treeNode *child[MAXCHILDREN];
    struct treeNode *sibling;
    NodeType nodeType;
    union {
        StmtType stmt;
        ExpType exp;
    } type;  // 更具体的类型
    union {
        TokenType op;
        int val;
        char *name; // 联合体中不能直接用string
    } attr; // 值
    char *op;
} TreeNode;

// 全局变量
int sourceFileCurrentPos = 0;
string sourceFile;
TokenType currentTokenType;
string currentTokenValue;

// 函数预声明
TreeNode *stmt_sequence();

TreeNode *statement();

TreeNode *if_stmt();

TreeNode *for_stmt();

TreeNode *repeat_stmt();

TreeNode *assign_stmt();

TreeNode *read_stmt();

TreeNode *write_stmt();

TreeNode *exp();

TreeNode *simple_exp();

TreeNode *term();

TreeNode *factor();

void match(TokenType expected);

char getNextChar();

void ungetNextChar();

TokenType getNextTokenType();

TreeNode *newStmtNode(StmtType TYPE);

char *copyString(string s);
// 函数定义

char *copyString(string s) {
    int n = s.length() + 1;
    char *t = (char *) malloc(n);
    strcpy(t, s.c_str());
    return t;
}

TokenType reservedLookup(string s) {
    for (int i = 0; i < MAXRESERVED; i++)
        if (s == reservedWords[i].str) {
            return reservedWords[i].tok;
        }
    return ID;
}

/**
 * 新建语法树的节点
 */
TreeNode *newStmtNode(StmtType type) {
    TreeNode *t = (TreeNode *) malloc(sizeof(TreeNode));
    for (int i = 0; i < MAXCHILDREN; i++) {
        t->child[i] = nullptr;
    }
    t->sibling = nullptr;
    t->nodeType = STMT_TYPE;
    t->type.stmt = type;
    return t;
}

TreeNode *newExpNode(ExpType type) {
    TreeNode *t = (TreeNode *) malloc(sizeof(TreeNode));
    for (int i = 0; i < MAXCHILDREN; i++) {
        t->child[i] = nullptr;
    }
    t->sibling = nullptr;
    t->nodeType = EXP_TYPE;
    t->type.exp = type;
    return t;
}

TokenType getNextTokenType() {
    TokenType type;
    StateType state = START_STATE;
    currentTokenValue = "";
    while (state != DONE_STATE) {
        char c = getNextChar();
        currentTokenValue += c;
        switch (state) {
            case START_STATE:
                if (isdigit(c)) {
                    state = NUM_STATE;
                } else if (isalpha(c)) {
                    state = ID_STATE;
                } else if (c == ':') {
                    state = ASSIGN_STATE;
                } else if (c == ' ' || c == '\t' || c == '\n') {
                    currentTokenValue = "";
                } else if (c == '{') {
                    state = COMMENT_STATE;
                } else {
                    state = DONE_STATE;
                    char tmp;
                    switch (c) {
                        case '\0':
                            type = ENDFILE;
                            break;
                        case '=':
                            type = EQ;
                            break;
                        case '<':
                            tmp = getNextChar();
                            currentTokenValue += tmp;
                            if (tmp == '=') {
                                type = LE;
                            } else if (tmp == '>') {
                                type = NE;
                            } else {
                                ungetNextChar();
                                type = LT;
                            }
                            break;
                        case '>':
                            tmp = getNextChar();
                            currentTokenValue += '=';
                            if (tmp == '=') {
                                type = GE;
                            } else {
                                ungetNextChar();
                                type = GT;
                            }
                            break;
                        case '+':
                            tmp = getNextChar();
                            currentTokenValue += '=';
                            if (tmp == '=') {
                                type = SELF_PLUS;
                            } else {
                                ungetNextChar();
                                type = PLUS;
                            }
                            break;
                        case '-':
                            tmp = getNextChar();
                            if (tmp == '=') {
                                currentTokenValue += '=';
                                type = SELF_MINUS;
                            } else {
                                ungetNextChar();
                                type = MINUS;
                            }
                            break;
                        case '*':
                            type = TIMES;
                            break;
                        case '/':
                            type = OVER;
                            break;
                        case '%':
                            type = MOD;
                            break;
                        case '^':
                            type = POW;
                            break;
                        case '(':
                            type = LPAREN;
                            break;
                        case ')':
                            type = RPAREN;
                            break;
                        case ';':
                            type = SEMI;
                            break;
                        default:
                            type = ERROR;
                            break;
                    }
                }
                break;
            case COMMENT_STATE:
                if (c == '}') {
                    state = START_STATE;
                    currentTokenValue = "";
                }
                break;
            case ASSIGN_STATE:
                state = DONE_STATE;
                if (c == '=') {
                    type = ASSIGN;
                } else {
                    cout << "ERROR: ASSIGN_STATE" << endl;
                }
                break;
            case NUM_STATE:
                if (!isdigit(c)) {
                    state = DONE_STATE;
                    ungetNextChar();
                    type = NUM;
                }
                break;
            case ID_STATE:
                if (!isalpha(c)) {
                    ungetNextChar();
                    type = ID;
                    state = DONE_STATE;
                }
                break;
            case DONE_STATE:
            default:
                cout << "ERROR: default" << endl;
                break;
        }
    }
    if (type == ID) {
        type = reservedLookup(currentTokenValue);
    }
    return type;
}

void match(TokenType expected) {
    if (currentTokenType == expected) {
        currentTokenType = getNextTokenType();
    } else {
        cout << "ERROR: in match" << endl;
    }
}

TreeNode *stmt_sequence() {
    TreeNode *t = statement();
    TreeNode *p = t;
    while (currentTokenType != ENDFILE &&
           currentTokenType != ELSE &&
           currentTokenType != UNTIL &&
           currentTokenType != ENDDO) { //标志代码段的结束
        TreeNode *q;
        // 下面是第二次发现的if语句的bug
        if (currentTokenType != ELSE) {
            match(SEMI);
        }
        q = statement();
        if (q != nullptr) {
            if (t == nullptr) {
                t = p = q;
            } else {
                p->sibling = q;
                p = q;
            }
        }
    }
    return t;
}

TreeNode *statement() {
    TreeNode *t = nullptr;
    switch (currentTokenType) {
        case IF :
            t = if_stmt();
            break;
        case REPEAT :
            t = repeat_stmt();
            break;
        case ID :
            t = assign_stmt();
            break;
        case READ :
            t = read_stmt();
            break;
        case WRITE :
            t = write_stmt();
            break;
        case FOR:
            t = for_stmt();
            break;
        case ENDFILE:
            break;
        case ENDDO:
            break;
        case UNTIL:
            break;
        default :
            cout << "statement error" << endl;
            break;
    }
    return t;
}

TreeNode *for_stmt() {
    TreeNode *t = newStmtNode(FOR_TYPE);
    match(FOR);
    t->child[0] = assign_stmt();
    if (currentTokenType == TO) {
        match(TO);
        t->attr.name = copyString("to");
    } else if (currentTokenType == DOWNTO) {
        match(DOWNTO);
        t->attr.name = copyString("downto");
    } else {
        cout << "ERROR in for stmt" << endl;
    }
    t->child[1] = exp();
    match(DO);
    t->child[2] = stmt_sequence();
    match(ENDDO);
    return t;
}

TreeNode *read_stmt() {
    TreeNode *t = newStmtNode(READ_TYPE);
    match(READ);
    if (currentTokenType == ID) {
        t->attr.name = copyString(currentTokenValue);
    }
    match(ID);
    return t;
}


TreeNode *write_stmt() {
    TreeNode *t = newStmtNode(WRITE_TYPE);
    match(WRITE);
    t->child[0] = exp();
    return t;
}

/**
 * 规定if语句的代码段里面只能有一个语句（也就是statement而非statement_sequence）
 * @return
 */
TreeNode *if_stmt() {
    TreeNode *t = newStmtNode(IF_TYPE);
    match(IF);
    match(LPAREN);
    t->child[0] = exp();
    match(RPAREN);
    t->child[1] = statement();
    match(SEMI);
    if (currentTokenType == ELSE) {
        match(ELSE);
        t->child[2] = statement();
    }
    return t;
}

TreeNode *exp() {
    TreeNode *t = simple_exp();
    if ((currentTokenType == LT) || (currentTokenType == EQ) || (currentTokenType == GT) ||
        (currentTokenType == LE) || (currentTokenType == GE) || (currentTokenType == NE)) {
        TreeNode *p = newExpNode(OP_TYPE);
        p->child[0] = t;
        p->attr.op = currentTokenType;
        t = p;
        match(currentTokenType);
        t->child[1] = simple_exp();
    }
    return t;
}

/**
 * 暂时把and,or和+,-划分在一起
 * @return
 */
TreeNode *simple_exp() {
    TreeNode *t = term();
    while ((currentTokenType == PLUS) || (currentTokenType == MINUS) ||
           (currentTokenType == AND) || (currentTokenType == OR)) {
        TreeNode *p = newExpNode(OP_TYPE);
        p->child[0] = t;
        p->attr.op = currentTokenType;
        t = p;
        match(currentTokenType);
        t->child[1] = term();
    }
    return t;
}

/**
 * 暂时将%，^和乘除法划分在一起
 * @return
 */
TreeNode *term() {
    TreeNode *t = factor();
    while ((currentTokenType == TIMES) || (currentTokenType == OVER) ||
            (currentTokenType == MOD) || currentTokenType == POW) {
        TreeNode *p = newExpNode(OP_TYPE);
        p->child[0] = t;
        p->attr.op = currentTokenType;
        t = p;
        match(currentTokenType);
        p->child[1] = factor();
    }
    return t;
}

TreeNode *factor() {
    TreeNode *t = nullptr;
    switch (currentTokenType) {
        case NUM :
            t = newExpNode(CONST_TYPE);
            if (currentTokenType == NUM)
                t->attr.val = atoi(currentTokenValue.c_str());
            match(NUM);
            break;
        case ID :
            t = newExpNode(ID_TYPE);
            if (currentTokenType == ID)
                t->attr.name = copyString(currentTokenValue);
            match(ID);
            break;
        case LPAREN :
            match(LPAREN);
            t = exp();
            match(RPAREN);
            break;
        case NOT:
            t = newExpNode(SINGLE_EXP_TYPE);
            if (currentTokenType == NOT)
                t->attr.name = copyString(currentTokenValue);
            match(NOT);
            t->child[0] = factor();
            break;
        default:
            cout << "ERROR: factor" << endl;
            break;
    }
    return t;
}

char getNextChar() {
    return sourceFile[sourceFileCurrentPos++];
}

void ungetNextChar() {
    sourceFileCurrentPos--;
    currentTokenValue = currentTokenValue.substr(0, currentTokenValue.length() - 1);
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

    return result.substr(0, result.length() - 1) + '\0';
}

TreeNode *assign_stmt() {
    TreeNode *t = newStmtNode(ASSIGN_TYPE);
    if (currentTokenType == ID) {
        t->attr.name = copyString(currentTokenValue);
    }
    match(ID);
    // match(ASSIGN);
    if (currentTokenType == ASSIGN) {
        t->op = ":=";
    } else if (currentTokenType == SELF_PLUS) {
        t->op = "+=";
    } else if (currentTokenType == SELF_MINUS) {
        t->op = "-=";
    } else {
        cout << "ERROR" << endl;
    }
    currentTokenType = getNextTokenType();
    t->child[0] = exp();
    return t;
}

TreeNode *repeat_stmt() {
    TreeNode *t = newStmtNode(REPEAT_TYPE);
    match(REPEAT);
    t->child[0] = stmt_sequence();
    match(UNTIL);
    t->child[1] = exp();
    return t;
}

TreeNode *parse() {
    TreeNode *t;
    currentTokenType = getNextTokenType();
    t = stmt_sequence();
    return t;
}

json toJson(TreeNode *t) {
    vector<json> ret;
    while (t != nullptr) {
        json tmp;
        switch (t->nodeType) {
            case STMT_TYPE:
                switch (t->type.stmt) {
                    case IF_TYPE:
                        tmp["IfStatement"]["TestExpression"] = toJson(t->child[0]);
                        tmp["IfStatement"]["MainBody"] = toJson(t->child[1]);
                        if (t->child[2] != nullptr) {
                            tmp["IfStatement"]["ElseBody"] = toJson(t->child[2]);
                        }
                        ret.push_back(tmp);
                        break;
                    case REPEAT_TYPE:
                        tmp["RepeatStatement"]["body"] = toJson(t->child[0]);
                        tmp["RepeatStatement"]["end"] = toJson(t->child[1]);
                        ret.push_back(tmp);
                        break;
                    case ASSIGN_TYPE:
                        tmp["assign"]["identifier"] = t->attr.name;
                        tmp["assign"]["assignExpression"] = toJson(t->child[0]);
                        tmp["assign"]["type"] = t->op;
                        ret.push_back(tmp);
                        break;
                    case READ_TYPE:
                        tmp["read"] = t->attr.name;
                        ret.push_back(tmp);
                        break;
                    case WRITE_TYPE:
                        tmp["write"] = toJson(t->child[0]);
                        ret.push_back(tmp);
                        break;
                    case FOR_TYPE:
                        tmp["for"]["initExpression"] = toJson(t->child[0]);
                        tmp["for"]["endExpression"] = toJson(t->child[1]);
                        tmp["for"]["body"] = toJson(t->child[2]);
                        tmp["for"]["order"] = t->attr.name;
                        ret.push_back(tmp);
                        break;
                    default:
                        cout << "error node" << endl;
                        break;
                }
                break;
            case EXP_TYPE:
                switch (t->type.exp) {
                    case OP_TYPE:
                        tmp["op"]["value"] = tokenNameMap.find(t->attr.op)->second;
                        tmp["op"]["leftExpression"] = toJson(t->child[0]);
                        tmp["op"]["rightExpression"] = toJson(t->child[1]);
                        ret.push_back(tmp);
                        break;
                    case CONST_TYPE:
                        tmp["const"] = t->attr.val;
                        ret.push_back(tmp);
                        break;
                    case ID_TYPE:
                        tmp["identifier"] = t->attr.name;
                        ret.push_back(tmp);
                        break;
                    case SINGLE_EXP_TYPE:
                        tmp["single-expression"]["op"] = t->attr.name;
                        tmp["single-expression"]["expression"] = toJson(t->child[0]);
                        ret.push_back(tmp);
                        break;
                    default:
                        cout << "error node" << endl;
                        break;
                }
                break;
            default:
                cout << "error node" << endl;
                break;
        }
        t = t->sibling;
    }
    return ret;
}

int main(int argc, char** argv) {
    if (argc != 2) {
        cout << "[ERROR]: argument format: xxx.exe <filepath>";
        return 0;
    }
    sourceFile = readFile(argv[1]);
//    sourceFile = readFile(R"(D:\Project\cproject\compilationPrinciplesExp3\sample1.tny)");
//    sourceFile = readFile(R"(D:\1.txt)");
    TreeNode *t = parse();
    json program;
    program["Program"] = toJson(t);
    cout << program.dump(4);
    return 0;
}