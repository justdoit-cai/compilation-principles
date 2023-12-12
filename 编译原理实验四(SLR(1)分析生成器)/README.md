# 运行方式
下面的第一步和第二步其实已经帮忙做好了。
1. 进入源代码的前端目录中，在其中的 `vue` 项目中运行下面的命令打包前端程序。
```text
npm install
npm run build
```
2. 然后将打包后的 `dist` 目录中的内容放到 `electron` 项目的根目录中。需要注意的是，这里需要修改 `index.html` 中静态文件的路径，将前面的 `/` 去掉，也就是改为相对路径。
```html
# 原始的
<script type="module" crossorigin src="/assets/index-b79980b0.js"></script>
<link rel="stylesheet" href="/assets/index-53c77a11.css">
# 改为下面的形式
<script type="module" crossorigin src="assets/index-b79980b0.js"></script>
<link rel="stylesheet" href="assets/index-53c77a11.css">
```
3. 运行下面的命令打包 `electron` 程序。然后就会得到一个单独的 `exe` 程序。
```text
npm install
electron-builder
```

# 运行界面

## 测试一

```text
E->E+n
E->n
```
![](imgs/1702383790274.png)
![](imgs/img.png)
![](imgs/img_1.png)


## 测试二
```text
S->(S)S
S->@
```
![img_2.png](imgs/img_2.png)
![img_3.png](imgs/img_3.png)
![img_4.png](imgs/img_4.png)


## 测试三
```text
E->aA
E->bB
A->cA
A->d
B->cB
B->d
```
![img_5.png](imgs/img_5.png)
![img_6.png](imgs/img_6.png)
![img_7.png](imgs/img_7.png)

# 不足

如果文法存在 `A->B`, `B->A` 这种循环依赖时，求 `First` 和 `Follow` 集合的时候会导致死循环。这里默认输入的文法不存在这种情况。

