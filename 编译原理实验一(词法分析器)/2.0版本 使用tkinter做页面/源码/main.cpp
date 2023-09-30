#include <iostream>
#include <fstream>
#include <string>

using namespace std;

//token 的类型
enum TokenType {
    KEYWORD = 1,
    IDENTIFIER,
    NUM,
    STR,
    OPERATOR,
    DELIMITER
};

//存储 token 信息的结构体
struct Token {
    enum TokenType type;
    string value;
    Token *next = nullptr;

    void setToken(TokenType newType, string newValue) {
        this->type = newType;
        this->value = newValue;
    }
};


void insertIntoList(Token *head, Token *newToken) {
    Token *tmp = head;
    while (tmp->next != nullptr) {
        tmp = tmp->next;
    }
    tmp->next = newToken;
}

string printList(Token *head) {
    string result;
    Token *tmp = head;
    while (tmp->next != nullptr) {
        tmp = tmp->next;
        result += tmp->value + " => ";
        switch (tmp->type) {
            case KEYWORD:
                result += "关键字\n";
                break;
            case IDENTIFIER:
                result += "标识符\n";
                break;
            case NUM:
                result += "数字\n";
                break;
            case STR:
                result += "串\n";
                break;
            case OPERATOR:
                result += "运算符\n";
                break;
            case DELIMITER:
                result += "分隔符\n";
                break;
            default:
                cout << "ERROR: Token类型异常!\n";
                exit(0);
        }
    }
    return result;
}

/**
 * 判断单个字符是否为数字
 * @param ch
 * @return
 */
bool isDigit(char ch) {
    if (ch >= '0' && ch <= '9') {
        return true;
    }
    return false;
}

/**
 * 判断单个字符是否是字母或者下划线
 * @param ch
 * @return
 */
bool isAlpha(char ch) {
    if ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || ch == '_') {
        return true;
    }
    return false;
}

/**
 * 判断单个字符是否为运算符
 * @param ch
 * @return
 */
bool isOperator(char ch) {

    char operators[13] = {
            '+', '-', '*', '/', '%', '<', '>', '=', '&', '|', '!'
    };

    for (char i: operators) {
        if (ch == i) {
            return true;
        }
    }

    return false;
}

/**
 * 判断单个字符是否为分隔符
 * @param ch
 * @return
 */
bool isDelimiter(char ch) {
    char delimiter[] = {
            '(', ')', ',', ';', '{', '}', '#', '.', ':', '<', '>', '[', ']'
    };

    for (char i: delimiter) {
        if (ch == i) {
            return true;
        }
    }

    return false;
}

/**
 * 判断是否为串
 * @param token
 * @return
 */
bool isString(string token) {
    if (token[0] == '\"' && token[token.length() - 1] == '\"') {
        for (int i = 1; i < token.length() - 1; ++i) {
            if (token[i] == '\"' && token[i - 1] != '\\') {
                cout << "ERROR: 字符串内出现非法字符!" << endl;
                exit(0);
            }
        }
        return true;
    }
    if (token[0] == '\'' && token.length() != 3) {
        cout << "ERROR: 单引号内只能包含一个字符!" << endl;
        exit(0);
    }
    if (token[0] == '\'' && token[2] == '\'' && token[1] != '\'') {
        return true;
    }
    return false;
}

/**
 * 判断字符串是否为关键字
 * @param token
 * @return
 */
bool isKeyword(string token) {

    string KeyWord[] = {
            "auto", "bool", "break",
            "case", "catch", "char", "const", "const_cast", "continue",
            "default", "delete", "do", "double", "dynamic_cast",
            "else", "explicit", "export", "enum", "extern",
            "false", "float", "for", "friend", "goto", "if", "inline", "int", "long",
            "mutable", "namespace", "operator", "private", "protected", "public",
            "register", "reinterpret_cast", "return", "short", "signed", "sizeof", "static", "static_cast",
            "struct", "switch", "template", "throw", "true", "try", "typedef", "typeid", "typename",
            "union","unsigned", "using",
            "void", "volatile", "wchar_t", "while"
    };

    for (string a: KeyWord) {
        if (token.compare(a) == 0) {
            return true;
        }
    }

    return false;
}

/**
 * 输入文件，存储到字符串中
 * @param filename
 * @return
 */
string inputFile(string filename) {
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
    // cpp读文件会在末尾添加\0
    result = result.substr(0, result.length() - 1);
    return result;
}

void outputFile(string filename, string content) {
    ofstream out(filename, std::ios::out);
    // 检查文件是否成功打开
    if (!out) {
        cout << "无法打开文件！" << endl;
    }
    out << content;
    out.close();
}
/**
 * 过滤注释内容
 * @return
 */
string inputFilter(string input) {
    string result;
    for (int pos = 0; pos < input.length(); ++pos) {
        // 判断某段是否为字符串
        if (input[pos] == '\"' && input[pos - 1] != '\\') {
            result += input[pos++];
            while (input[pos] != '\"' || input[pos - 1] == '\\') {
                result += input[pos++];
            }
            result += input[pos];
            continue;
        }
        // 判断某段是否为字符
        if (input[pos] == '\'' && input[pos - 1] != '\\' && input[pos + 2] == '\'') {
            result += input.substr(pos, 3);
            pos += 2;
            continue;
        }
        // 单行注释过滤
        if (input[pos] == '/' && input[pos + 1] == '/') {
            while (input[pos] != '\n') {
                pos++;
            }
        }
        // 多行注释过滤
        if (input[pos] == '/' && input[pos + 1] == '*') {
            while (!(input[pos] == '*' && input[pos + 1] == '/')) {
                pos++;
            }
            pos += 2;
        }
        // 过滤制表符
        if (input[pos] != '\v' && input[pos] != '\t') {
            result += input[pos];
        }
    }
    return result;
}

/**
 * 通过字符串开头的字符来判断类型
 * 这里要注意顺序：
 * 1. 数字开头    -> 数字
 * 2. "或者'开头  -> 串
 * 3. 特殊符号开头 -> 分隔符
 *              -> 运算符
 * 4. 字母开头    -> 关键字
 *              ->  标识符
 * @param ch
 * @return
 */
int startCharType(char ch) {
    int type = 0;
    if (isDigit(ch)) {
        type = 1;
    } else {
        if (ch == '\"' || ch == '\'') {
            type = 2;
        } else {
            if (isOperator(ch)) {
                type = 3;
            } else {
                if (isDelimiter(ch)) {
                    type = 4;
                } else {
                    if (isAlpha(ch)) {
                        type = 5;
                    } else {
                        if (ch == '\n') {
                            type = 6;
                        } else {
                            type = 7;
                        }
                    }
                }
            }
        }
    }
    return type;
}

/**
 * 判断数字类型所在字符串的位置
 * @param content
 * @param pos
 * @return
 */
int numToken(Token *head, string content, int pos) {
    Token *numToken = new Token;
    string numTokenValue;

    numTokenValue += content[pos++];

    while (isDigit(content[pos]) || content[pos] == '.') {
        numTokenValue += content[pos++];
    }

    numToken->setToken(NUM, numTokenValue);
    insertIntoList(head, numToken);

    // 返回分析进度最新位置(当前tokenValue的末尾index)
    return pos;
}

/**
 * 判断串类型所在字符串的位置
 * @param head
 * @param content
 * @param pos
 * @return
 */
int strToken(Token *head, string content, int pos) {
    Token *strToken = new Token;
    string strTokenValue;

    strTokenValue += content[pos]; // 起始的引号

    if (content[pos] == '\"') {
        pos++;
        while (content[pos] != '\"' || content[pos - 1] == '\\') {
            strTokenValue += content[pos++];
        }
        strTokenValue += content[pos++]; // 加上结尾的引号
    } else if (content[pos] == '\'') {
        pos++;
        while (content[pos] != '\'' || content[pos - 1] == '\\') {
            strTokenValue += content[pos++];
        }
        strTokenValue += content[pos++]; // 加上结尾的引号
    }

    strToken->setToken(STR, strTokenValue);
    insertIntoList(head, strToken);

    // 返回分析进度最新位置
    return pos;
}

/**
 * 判断操作符类型所在字符串的位置
 * @param head
 * @param content
 * @param pos
 * @return
 */
int operatorToken(Token *head, string content, int pos) {
    Token *operatorToken = new Token;
    string operatorTokenValue;

    if (content[pos] == '+' && content[pos + 1] == '+') {
        operatorTokenValue = "++";
        pos += 2;
    } else if (content[pos] == '+' && content[pos + 1] == '=') {
        operatorTokenValue = "+=";
        pos += 2;
    } else if (content[pos] == '-' && content[pos + 1] == '-') {
        operatorTokenValue = "--";
        pos += 2;
    } else if (content[pos] == '-' && content[pos + 1] == '=') {
        operatorTokenValue = "-=";
        pos += 2;
    } else if (content[pos] == '-' && content[pos + 1] == '>') {
        operatorTokenValue = "->";
        pos += 2;
    } else if (content[pos] == '>' && content[pos + 1] == '>') {
        operatorTokenValue = ">>";
        pos += 2;
    } else if (content[pos] == '>' && content[pos + 1] == '=') {
        operatorTokenValue = ">=";
        pos += 2;
    } else if (content[pos] == '<' && content[pos + 1] == '<') {
        operatorTokenValue = "<<";
        pos += 2;
    } else if (content[pos] == '<' && content[pos + 1] == '=') {
        operatorTokenValue = "<=";
        pos += 2;
    } else if (content[pos] == '!' && content[pos + 1] == '=') {
        operatorTokenValue = "!=";
        pos += 2;
    } else if (content[pos] == '&' && content[pos + 1] == '&') {
        operatorTokenValue = "&&";
        pos += 2;
    } else if (content[pos] == '|' && content[pos + 1] == '|') {
        operatorTokenValue = "||";
        pos += 2;
    } else {
        operatorTokenValue = content[pos++];
    }

    operatorToken->setToken(OPERATOR, operatorTokenValue);
    insertIntoList(head, operatorToken);

    // 返回分析进度最新位置
    return pos;
}

/**
 * 判断分隔符类型所在字符串的位置
 * @param head
 * @param content
 * @param pos
 * @return
 */
int delimiterToken(Token *head, string content, int pos) {
    Token *delimiterToken = new Token;
    string delimiterTokenValue;

    delimiterTokenValue += content[pos++];

    delimiterToken->setToken(DELIMITER, delimiterTokenValue);
    insertIntoList(head, delimiterToken);

    // 返回分析进度最新位置
    return pos;
}

/**
 * 注意判断是关键字还是标识符
 * @param head
 * @param content
 * @param pos
 * @return
 */
int alphaToken(Token *head, string content, int pos) {
    Token *alphaToken = new Token;
    string alphaTokenValue;

    alphaTokenValue += content[pos++];

    //后面字符是字母或者数字
    while (isAlpha(content[pos]) || isDigit(content[pos])) {
        alphaTokenValue += content[pos++];
    }

    // 查表，不是关键字就是标识符
    if (isKeyword(alphaTokenValue)) {
        alphaToken->setToken(KEYWORD, alphaTokenValue);
    } else {
        alphaToken->setToken(IDENTIFIER, alphaTokenValue);
    }

    insertIntoList(head, alphaToken);

    // 返回分析进度最新位置
    return pos;
}


/**
 * 运行c词法分析程序的主程序
 */
string cLexer(string content) {
    Token *head = new Token;
    for (int pos = 0; pos < content.length(); ++pos) {
        while (content[pos] != ' ' && pos < content.length()) {
            switch (startCharType(content[pos])) {
                case 1:
                    pos = numToken(head, content, pos);
                    break;
                case 2:
                    pos = strToken(head, content, pos);
                    break;
                case 3:
                    pos = operatorToken(head, content, pos);
                    break;
                case 4:
                    pos = delimiterToken(head, content, pos);
                    break;
                case 5:
                    pos = alphaToken(head, content, pos);
                    break;
                case 6:
                    pos++;
                    break;
                case 7:
                    cout << "Unknown Character " << content[pos] << endl;
                    pos++;
                    break;
                default:
                    cout << "ERROR!" << endl;
                    break;
            }
        }
    }
    return printList(head);
}

int main(int argc, char* argv[]) {
    if (argc != 2){
        cout << "ERROR INPUT" << endl;
        return 0;
    }
    string sourceFile = argv[1];

    string sourceText = inputFile(sourceFile);
    string sourceCode = inputFilter(sourceText);

    string results = cLexer(sourceCode);
    cout << results;
    return 0;
}
