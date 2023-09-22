# 原理
## 整体框架

通过 `electron` 桌面程序框架，将前端页面和本地文件系统联系起来。将前端的操作转换为执行本地的 `cpp` 程序。而本地的 `cpp` 写成命令行程序，通过 `argv` 接收命令行参数（ `main.exe <输入文件路径> <输出文件路径>` ），来实现词法分析器的功能。

## 前端设计思路

当用户点击选择文件按钮时，通过 `electron` 渲染进程和主进程双向通信的方式（参考`https://www.electronjs.org/zh/docs/latest/tutorial/ipc` ），
来将用户选择的输入文件路径保存到一个变量中，然后在用户点击开始转换按钮的时候，通过判断那个变量是否为空来判断用户是否选择了输入文件路径，然后接着在主进程中调用 `Nodejs` 的接口，执行系统命令。

## cpp词法分析器设计思路

将每一个词当作一个 `Token`，在顺序遍历源文件的时候依次通过分析当前开头的字符可能会匹配哪种类型的 `Token` ，如果可能匹配多种类型的 `Token` ，就再顺序遍历到下一个字符，再次判断此时可能匹配哪些 `Token` （状态转移的思路）

```cpp
// 存储token的类型
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
```

`startCharType` 函数最能说明这个设计思路的原理。看开头的字符符合哪些 `Token` 的可能性。

```cpp
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
```

# 参考文章
```
# electron打包exe的方式
https://juejin.cn/post/7133163924576272397
https://www.electron.build/configuration/contents.html#extrafiles
https://oldmoon.top/post/112
https://blog.csdn.net/weixin_36185028/article/details/84674042
# electron前后端进程间双向通信的方式
https://www.electronjs.org/zh/docs/latest/tutorial/ipc#%E6%A8%A1%E5%BC%8F-2%E6%B8%B2%E6%9F%93%E5%99%A8%E8%BF%9B%E7%A8%8B%E5%88%B0%E4%B8%BB%E8%BF%9B%E7%A8%8B%E5%8F%8C%E5%90%91
```
