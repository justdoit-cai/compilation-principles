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

## 测试四
```text
E->E+T
E->E-T
E->T
T->T*F
T->T/F
T->F
F->(E)
F->n
```
![img_16.png](imgs/img_16.png)
![img_17.png](imgs/img_17.png)
![img_8.png](imgs/img_8.png)

## 测试五（测试非SLR1文法的报错）
```text
S->C
S->A
S->B
C->i
A->V=E
B->W=T
V->i
V->a
E->V
E->n
W->i
T->b
```
![img_9.png](imgs/img_9.png)
![img_10.png](imgs/img_10.png)
![img_12.png](imgs/img_12.png)

## 测试六（测试非SLR1文法的报错）
```text
S->I
S->o
I->i()S
I->i()SeS
```
![img_13.png](imgs/img_13.png)
![img_14.png](imgs/img_14.png)
![img_15.png](imgs/img_15.png)
