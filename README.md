修改后代码如下：
test_pipe.c
1 #include <stdlib.h>
2 #include <unistd.h>
3
4 int fildes[2];
5 char buf[100];
6 int status;
7
8 int main(){
9
10 status = pipe(fildes);
11
12 if (status == -1 ) {
13 printf("error\n");
14 }
15
16
17 switch (fork()) {
18 case -1:
19 break;
20
30 case 0: /* 子进程 - 作为管道的写者 */
31 close(fildes[0]); /* 关闭不用的读端 */
32 write(fildes[1], "Hello world\n", 12); /* 向管道中写数据 */
33 close(fildes[1]); /* 写入结束，关闭写端 */
34 exit(EXIT_SUCCESS);
        
22 default: /* 父进程 - 作为管道的读者 */
23 close(fildes[1]); /* 关闭不用的写端 */
24 read(fildes[0], buf, 100); /* 从管道中读数据 */
25 printf("child-process read:%s",buf); /* 打印读到的数据 */
26 close(fildes[0]); /* 读取结束，关闭读端 */
27 exit(EXIT_SUCCESS);
28
29
35 }
36 }
