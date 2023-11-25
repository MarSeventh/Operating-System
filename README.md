# 该仓库基本包含lab0到lab6的课下和课上exam、extra以及lab6挑战性任务的所有内容
# Lab0 实验报告

## 思考题

### 1、仔细看看0.10，思考一下箭头中的 add the file 、stage the file 和 commit 分别对应的是 Git 里的哪些命令呢？

答：add the file 对应git add，stage the file 对应git add，commit对应git commit

### 2、

### 		2.1 代码文件 print.c 被错误删除时，应当使用什么命令将其恢复？

​		答：git checkout -- print.c

### 		2.2代码文件 print.c 被错误删除后，执行了 git rm print.c 命令，此时应当 使用什么命令将其恢复？

​		答：git checkout HEAD print.c

### 		2.3无关文件 hello.txt 已经被添加到暂存区时，如何在不删除此文件的前提下 将其移出暂存区？

​		答：git rm --cached hello.txt

### 3.使用你知道的方法（包括重定向）创建下图内容的文件（文件命名为 test）， 将创建该文件的命令序列保存在 command 文件中，并将 test 文件作为批处理文件运行， 将运行结果输出至 result 文件中。给出 command 文件和 result 文件的内容，并对最 后的结果进行解释说明（可以从 test 文件的内容入手）. 具体实现的过程中思考下列问 题: echo echo Shell Start 与 echo `echo Shell Start`效果是否有区别; echo echo $c>file1 与 echo `echo $c>file1`效果是否有区别. 

​		答：command文件内容：

```
  1 echo 'echo Shell Start' > test                                                                     2 echo 'echo set a = 1' >> test
  3 echo 'a=1' >> test
  4 echo 'echo set b = 2' >>test
  5 echo 'b=2' >> test
  6 echo 'set c = a+b' >> test
  7 echo 'c=$[$a+$b]' >> test
  8 echo 'echo c=$c' >> test
  9 echo 'echo save c to ./file1' >> test
 10 echo 'echo $c>file1' >> test
 11 echo 'echo save b to ./file2' >> test
 12 echo 'echo $b>file2' >> test
 13 echo 'echo save a to ./file3' >> test
 14 echo 'echo $a>file3' >>test
 15 echo 'echo save file1 file2 file3 to file4' >> test
 16 echo 'cat file1>file4' >> test
 17 echo 'cat file2>>file4' >> test
 18 echo 'cat file3>>file4' >> test
 19 echo 'echo save file4 to ./result' >> test
 20 echo 'cat file4>>result' >> test
```

result文件内容：

```
  1 3                                                                                                   2 2
  3 1
```

首先command命令生成test文件，而test文件的作用即令a=1,b=2,c=a+b，然后将c、a、b相继输出到file4文件，再将file4输出到result文件中。

echo echo Shell Start 与 echo `echo Shell Start`效果没有区别；

echo echo $c>file1 与 echo `echo $c>file1`效果有区别，前者将echo $c输出到file1，后者则输出echo $c>file1

### 4、思考下列有关 Git 的问题： 

### • 在/home/21xxxxxx/learnGit（已初始化）目录下创建一个名为 README.txt 的文件。执行命令 git status > Untracked.txt。 

### • 在 README.txt 文件中添加任意文件内容，然后使用 add 命令，再执行命令 git status > Stage.txt。

###  • 提交 README.txt，并在提交说明里写入自己的学号。

###  • 执行命令 cat Untracked.txt 和 cat Stage.txt，对比两次运行的结果，体 会 README.txt 两次所处位置的不同。

###  • 修改 README.txt 文件，再执行命令 git status > Modified.txt。

###  • 执行命令 cat Modified.txt，观察其结果和第一次执行 add 命令之前的 status 是否一样，并思考原因。

结果如下：

```
1 Untracked.txt 的内容如下
2
3 # On branch master
4 # Untracked files:
5 # (use "git add <file>..." to include in what will be committed)
6 #
7 # README.txt
8 nothing added to commit but untracked files present (use "git add" to track)
9
10 Stage.txt 的内容如下
11
12 # On branch master
13 # Changes to be committed:
14 # (use "git reset HEAD <file>..." to unstage)
15 #
16 # new file: README.txt
17 #
18
19 Modified.txt 的内容如下
20
21 # On branch master
22 # Changes not staged for commit:
23 # (use "git add <file>..." to update what will be committed)
24 # (use "git checkout -- <file>..." to discard changes in working directory)
25 #
26 # modified: README.txt
27 #
28 no changes added to commit (use "git add" and/or "git commit -a")

```

第一次文件在工作区内处于未跟踪状态,第二次文件处于暂存区中但是未提交，第三次修改后暂存区中文件和工作区中有不同，仍需要使用git add将修改添加到暂存区中，与第一次操作相同。

### 5、思考下列有关 Git 的问题：

###  • 找到在/home/21xxxxxx/learnGit 下刚刚创建的 README.txt 文件，若不存 在则新建该文件。 

### • 在文件里加入 Testing 1，git add，git commit，提交说明记为 1。

###  • 模仿上述做法，把 1 分别改为 2 和 3，再提交两次。

###  • 使用 git log 命令查看提交日志，看是否已经有三次提交，记下提交说明为 3 的哈希值a。 

### • 进行版本回退。执行命令 git reset --hard HEAD^后，再执行 git log，观 察其变化。 

### • 找到提交说明为 1 的哈希值，执行命令 git reset --hard  <hash>后，再执 行 git log，观察其变化。

###  • 现在已经回到了旧版本，为了再次回到新版本，执行 git reset --hard  <hash>，再执行 git log，观察其变化。

第一次git log:

```
commit 1d5ef89017e8bb2f5eb5e9d5ad5c8bd9ca04da82 (HEAD -> master)
Author: 黄一轩 <21373347@buaa.edu.cn>
Date:   Wed Mar 1 14:10:48 2023 +0800

    3

commit 896ab3df8673783bd32e313b27a2433e9a430c02
Author: 黄一轩 <21373347@buaa.edu.cn>
Date:   Wed Mar 1 14:10:24 2023 +0800

    2

commit 7e8e3b7f1f54574cffc618f6052b9c192259793e
Author: 黄一轩 <21373347@buaa.edu.cn>
Date:   Wed Mar 1 14:09:55 2023 +0800

    1
```

第二次 git log:

```
commit 896ab3df8673783bd32e313b27a2433e9a430c02 (HEAD -> master)
Author: 黄一轩 <21373347@buaa.edu.cn>
Date:   Wed Mar 1 14:10:24 2023 +0800

    2

commit 7e8e3b7f1f54574cffc618f6052b9c192259793e
Author: 黄一轩 <21373347@buaa.edu.cn>
Date:   Wed Mar 1 14:09:55 2023 +0800

    1
```

第三次git log:

```
commit 7e8e3b7f1f54574cffc618f6052b9c192259793e (HEAD -> master)
Author: 黄一轩 <21373347@buaa.edu.cn>
Date:   Wed Mar 1 14:09:55 2023 +0800

    1
```

第四次git log:

```
commit 1d5ef89017e8bb2f5eb5e9d5ad5c8bd9ca04da82 (HEAD -> master)
Author: 黄一轩 <21373347@buaa.edu.cn>
Date:   Wed Mar 1 14:10:48 2023 +0800

    3

commit 896ab3df8673783bd32e313b27a2433e9a430c02
Author: 黄一轩 <21373347@buaa.edu.cn>
Date:   Wed Mar 1 14:10:24 2023 +0800

    2

commit 7e8e3b7f1f54574cffc618f6052b9c192259793e
Author: 黄一轩 <21373347@buaa.edu.cn>
Date:   Wed Mar 1 14:09:55 2023 +0800

    1
```

### 6、执行如下命令, 并查看结果 

### • echo first

###  • echo second > output.txt 

### • echo third > output.txt 

### • echo forth >> output.txt

第一次直接将first进行标准输出

第二次将second输出到output.txt，第三次将output.txt改为third，第四次将forth追加到output.txt后面

## 难点分析

本次实验带我们了解了Shell、git、gcc等的基本用法，难度相对较小。其中Shell的使用重点在于能够熟练地掌握各类命令的含义，选项及参数的用法；git的使用主要是要理清git进行版本管理的方式，了解git add、git commit、git push、git pull等的基本用法以及使用git进行版本切换、分支切换等的方式；gcc的使用主要是要理解gcc编译文件的原理，同时要学会make的用法、Makefile的写法。

**1、下面对于Git进行版本管理的方式，即Git的三棵树采用示意图的方式进行总结：**

![Git三棵树](D:\OS\Git三棵树.png)

**2、下面对C语言的编译过程进行总结：**

| 阶段   | 执行操作                                    | 生成文件后缀 |
| ------ | ------------------------------------------- | ------------ |
| 预处理 | 以源文件作为输入，调用cpp生成预处理文件     | .i           |
| 编译   | 以预处理后文件作为输入，调用cc1生成汇编文件 | .s           |
| 汇编   | 以汇编文件作为输入，调用as生成目标文件      | .o           |
| 链接   | 以目标文件作为输入，调用ld生成可执行文件    | .out         |

让gcc只进行预处理操作：

```
$ gcc -E hello.c -o hello.i
```

让gcc进行到编译操作：

```
$ gcc -S hello.i -o hello.s
```

让gcc进行到汇编操作：

```
$ gcc -c hello.s -o hello.o
```



## 实验体会

本次实验的内容细节较多，绝大部分内容需要记忆，对于这部分内容，通过这次实验我的体会便是一定需要动手去实践，看再多遍也不如自己亲手在命令行中打下一条条指令亲手测试的效果好，这一点对于以后的学习也有帮助，在遇到比较繁琐的内容需要学习时，一定不能偷懒，“光说不练假把式”，亲身实践永远是最好的学习方式。

在这一次实验中，令我印象深刻的主要有以下几个点：

### 第一，在进行**Makefile文件**的编写时，一定要提前整理好各个文件之间的依赖关系，这一点是容易出错且容易遗漏的。其次，我们要把Makefile文件当做自己执行手动执行某一过程时所有行为的聚合，这样才能真正理解make和Makefile使用的意义。

注意：Makefile中command行开头要使用Tab进行分隔

如下命令可以改变make的执行目录：

```
($MAKE) -C newdir <tag>
```

### 第二，使用git时，我们需要时刻明白自己仓库目前的状态，可以使用git status来获取当前工作区状态和建议命令，切记不要随意进行提交等操作，否则可能导致仓库十分混乱，影响整个工作的进行。

### 第三，在编写shell脚本的过程中，要注意单引号和双引号的使用问题，单引号不能识别变量，不能进行转义，只会原样输出，而双引号可以识别变量，也可以进行转义。除此之外，也需要注意空格的问题，有些地方需要空格，但有些地方不能空格，例如对变量进行赋值的语句(a=3)中间不能出现空格。

### 第四，在shell中参数的传递是通过$n的形式实现的，其中$*代表全部参数，$#代表输入参数的个数,在使用参数时要注意参数传递的顺序和标号一一对应。
