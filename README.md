# Lab6挑战性任务实验报告

## 必做部分

### 1、实现一行多命令

要实现一行多命令，其实就是将一行的指令用`;`分隔开来，每当遇到`;`就让shellfork一次，然后子shell去执行前面已经解析出来的命令，父shell等待子shell解析完成之后再继续解析后面的指令，直到将一行解析完成。主要在sh中进行以下修改：

```c
case ';':
		    fid = fork();
			if(fid == 0) {
				return argc;
			} else {
				wait(fid);
				return parsecmd(argv, rightpipe);
			}
			break;
```

使用以下命令进行测试：

```bash
touch 1 ; touch 2; touch 3; tree
```

可以看到运行结果如下：

![](https://cdn.jsdelivr.net/gh/MarSeventh/imgbed/posts/image-20230529174552193.png)

该行命令输出后可以看到已经成功创建三个文件并且按照tree命令成功输出。

### 2、实现后台任务

该任务要求在一行中输入两条命令，用&分隔，前面的指令在后台运行，shell只等待后面的指令运行完成就准备接受下一条命令。

要实现该要求，我们首先应该确定shell等待命令的执行是发生在哪里，可以在runcmd里面看到，当spawn指令执行后，如果`child >= 0`，就执行wait指令，也就是在这里shell需要等待spawn出来的进程执行完毕在继续执行。那么为了实现后台任务，我们只需要在这里稍作修改，如果命令是&前面的命令，让shell无需等待其执行即可。具体来说就是添加一个判断是否等待的全局变量，然后在解析到`&`的时候先fork，然后将子shell的该变量设置为false，再让子shell去执行该命令，父shell直接继续执行后续指令即可。

具体实现如下：

```c
//parsecmd
case '&':
			fid = fork();
			if(fid == 0) {
				hang = 0;
				return argc;
			} else {
				hang = 1;
				return parsecmd(argv, rightpipe);
			}
			break;
```

```c
//runcmd
if (child >= 0) {
		if(hang) wait(child);
	} else {
		debugf("spawn %s: %d\n", argv[0], child);
	}
```

为了展示该指令的实现结果，需要前面的指令执行时间较长，可以在后面指令执行完毕后继续输入指令，同时观察到前面指令的执行结果，而tree指令就正好符合该要求。

所以在之前的基础上使用如下指令测试：

```bash
tree & touch 1
```

可以观察到：

![](https://cdn.jsdelivr.net/gh/MarSeventh/imgbed/posts/image-20230529180738572.png)

在开始执行后，tree还没来得及输出，touch指令已经执行完毕，并且shell可以继续输入。所以接下来我们直接输入一条空指令，然后会发现：

![](https://cdn.jsdelivr.net/gh/MarSeventh/imgbed/posts/image-20230529180913856.png)

前面的tree指令继续执行，同时开头直接就destory了一个进程，该进程就是我们之前为了执行tree指令而fork出来的子shell，它没有等待spawn出来的tree进程结束就已经结束了，从而实现了任务的后台执行。

### 3、实现引号支持

该任务要求在解析指令时将`""`内的内容看做是单个字符串，要实现该要求，我们只需要在解析指令的时候对`""`进行特判，如果遇到双引号就直接继续向后解析，直到遇到另一边的双引号，并且将解析到的两个双引号之间的字符串作为参数返回即可。具体的实现如下：

```c
//_gettoken 
if(*s == '"') {
		*s = 0;
		*p1 = ++s;
		while(*s != 0 && *s != '"') {
			s++;
		}
		if(*s == 0) {
			printf("wrong cmd!\n");
			return 0;
		}
		*s = 0;
		*p2 = ++s;
		return 'w';
	}
```

对于该任务的测试，我采用以下命令：

```bash
echo "ls.b | cat.b"
```

输出结果如下：

![](https://cdn.jsdelivr.net/gh/MarSeventh/imgbed/posts/image-20230529183159923.png)

可以看到，成功将双引号内的内容识别成了一个整体的字符串

### 4、实现键入命令时任意位置的修改

要实现该任务，大致思路就是在shell一开始读入字符串的时候对左右方向键进行特判，然后根据输入分别维护命令行的回显以及读入的buf两部分。

方向键在输入时其实是三个字符：

上键：`27 '[' 'A'`	下键：`27 '[' 'B'`	左键：`27 '[' 'D'`	右键：`27 '[' 'C'`

首先得设定一个表示光标位置的变量pos，如果遇到左右方向键就将该输入忽略，同时维护pos变量，让它的值与光标的实际位置保持一致，然后先维护命令行的回显，也就是根据输入的方向键移动命令行的光标。这样就实现了让光标根据我们输入的左右键进行移动，然后我们需要处理添加字符和删除字符的指令。

对于添加字符，当读入字符时，判断pos是否和此时的字符串总长一致，否则说明光标不在字符串末尾，此时我们首先需要删除光标之后的内容，将输入的字符显示出来，然后再将刚才删掉的字符串再次打印，之后维护光标的位置，就实现了添加字符的回显。但是此时我们的buf还没有更新，也就是说刚刚添加的字符依然在buf的末尾，我们需要将其移动到刚才插入的位置，对字符串进行操作即可。最后别忘了维护pos的值。

对于删除字符，当读到删除字符时，依然是先判断pos的位置，如果光标不在字符串末尾，就先删掉光标前一个位置到末尾的所有字符，然后重新打印光标到末尾的所有字符，再维护光标位置，实现shell窗口回显。然后依然是维护buf的内容将刚才删掉的字符对应删除即可。最后维护pos的值。

当然，在实际实现的时候并没有这么简单，还需要对左边界、右边界的各种情况进行特判，要保证光标不能到达他不应该到的地方，pos也应该限制在正常范围内，否则就会出现奇奇怪怪的bug。

具体实现的部分在readline之中：

```c
//维护光标
             else if(ch == 'D'){
				i--;
				if(pos > 0) {
					pos--;
				} else {
					//printf("left!!!");
					fprintf(1, "\x1b[C");
				}
			//	debugf("\npos = %d",pos);
				continue;
			} else if(ch == 'C') {
				i--;
				if(pos <= i) {
					pos++;
				} else {
					fprintf(1, "\x1b[D");
				}
				//pos++;
				continue;
			}
//添加字符
if(pos < i) {
		//	debugf("pos = %d, i = %d",pos,i);
			char tmp[1024];
			int j = pos;
			int k = 0;
			for(;j < i;j++) {
				tmp[k++] = buf[j];
			}
			tmp[k] = 0;
			fprintf(1, "\x1b[K%s\x1b[%dD", tmp, i - pos);
			char cha = buf[i];
			for(j = i;j >= pos + 1;j--) {
				buf[j] = buf[j - 1];
			}
			buf[pos] = cha;
		}
//删除字符
if (buf[i] == '\b' || buf[i] == 0x7f) {
			if (buf[i] != '\b') {
                                printf("\b");
                        }
			if(pos == 0) {
				i--;
				printf("\x1b[C");
				continue;
			}
			if (i > 0) {
				pos--;
				i -= 2;
			} else {
				printf("\x1b[C");
				i = -1;
				continue;
			}
			printf("\x1b[K");
			if(pos <= i) {
			//	printf("\npos = %d, i = %d",pos,i);
				int j = pos + 1;
				for(;j <= i + 1;j++) {
					printf("%c", buf[j]);
				}
				for(j = pos;j <= i;j++) {
					buf[j] = buf[j + 1];
				}
				printf("\x1b[%dD",i + 1 - pos);
			}
			continue;
		}
```

对于该部分的测试，主要就是尽可能地随便移动光标，然后添加删除字符，判断功能实现是否正确，比如我下面的测试（只是瞎输入的一小部分）：

```
tou nnendfo12334enfoeqn ch
tou nne nfqn ch
   1
touch 1
```

最终的执行结果如下：

![](https://cdn.jsdelivr.net/gh/MarSeventh/imgbed/posts/image-20230529185759787.png)

可以看到，尽管我随便输入删除，并且多次触及左右边界，指令依然被正确识别了。

### 5、实现程序名称中 `.b` 的省略

这个任务比较简单，就是给指令改一个名字的问题。因为我们有.b的指令都是通过spawn来执行的，因此只需要在spawn的时候对传入的指令名称进行判断，如果指令名字后面没带.b的话就自己加上一个再打开即可。具体对spawn的修改如下：

```c
if(len < 2 || prog[len - 1] != 'b' || prog[len - 2] != '.') {
		progname[len] = '.';
		progname[len + 1] = 'b';
		progname[len + 2] = '\0';
	}
```

测试命令：

```
cat
1122333344224444
```

结果如下：

![](https://cdn.jsdelivr.net/gh/MarSeventh/imgbed/posts/image-20230529190422078.png)

可以看到，cat没有`.b`依然正确执行了。

### 6、实现tree 、mkdir、touch命令

mkdir指令和touch指令的实现比较类似，本质都是创建一个新文件的过程，只不过文件类型不同。因为目前我们的MOS并没有提供创建文件的用户接口，所以我们先得自己提供一个接口。大致过程就是先在include/fs.h中新建一个Fsreq_create结构体，内容是文件路径以及类型。然后在文件系统端实现serve_create函数，使用已有的file_create来创建一个新的文件。之后在用户态的fsipc.c中增加对应的用户态通讯接口，在file.c中添加创建文件的用户接口。

这样一来我们就有了创建文件的接口，然后我们只需要实现两个用户态指令文件mkdir.c和touch.c即可，这两个文件的实现基本类似，本质就是将参数传递给刚才实现的create接口，让文件系统按照我们的要求创建文件。具体实现如下：

```c
//touch.c
#include <lib.h>

int main(int argc, char **argv) {
	if(argc != 2) {
		fprintf(1, "usage: touch[filename]\n");
		return -1;
	}
	if(open(argv[1], O_RDONLY) >= 0) {
		fprintf(1, "file has existed!\n");
		return -1;
	}
	if(create(argv[1], FTYPE_REG) < 0) {
		fprintf(1, "create the file failed\n");
		return -1;
	}
	return 0;
}
```

mkdir的实现只需要修改对应的文件类型。

接下来是tree指令的实现，其实tree指令的本质就是一个递归问题，只需要递归输出所有的目录和文件名称即可，但是注意要保证一定的格式。具体实现如下：

```c
#include <lib.h>

void print_tab(int num) {
	int i;
	for(i = 0; i < num; i++) {
		fprintf(1, "    ");
	}
}

void walktree(int depth, char *dir) {
	struct Fd *fd;
	struct Filefd *ffd;
	int i;
	int r;

	if((r = open(dir, O_RDONLY)) < 0) {
		return;
	}

	fd = (struct Fd *)num2fd(r);
	ffd = (struct Filefd *)fd;
	int size = ffd->f_file.f_size;
	fprintf(1, "\x1b[35m%s\x1b[0m\n", ffd->f_file.f_name);

	int num = ROUND(size, sizeof(struct File)) / sizeof(struct File);
	struct File *file = (struct File *)fd2data(fd);

	for(i = 0; i < num; i++) {
		struct File now = file[i];
		if(now.f_name[0] == '\0') {
			continue;
		}
		print_tab(depth);
		fprintf(1, "|---");
		if(now.f_type == FTYPE_DIR) {
			char newdir[MAXPATHLEN];
			strcpy(newdir, dir);
			int len = strlen(newdir);
			if(newdir[len - 1] != '/') {
				newdir[len++] = '/';
			}
			strcpy(newdir + len, now.f_name);
			walktree(depth + 1, newdir);
		} else {
			fprintf(1, "%s\n", now.f_name);
		}
	}
}

int main(int argc, char **argv) {
	char *dirname;
	if(argc > 2) {
		fprintf(1, "usage: tree [dirname]\n");
		return -1;
	}
	dirname = (argc == 1)? "/" : argv[1];
	walktree(0, dirname);
	return 0;
}
```

对以上三条命令集中测试：

```
touch a ; touch b; touch c; mkdir newdir;touch newdir/new;tree
```

（部分）结果如下：

![](https://cdn.jsdelivr.net/gh/MarSeventh/imgbed/posts/image-20230529192019468.png)

然后再测试tree打开子目录的功能：

![](https://cdn.jsdelivr.net/gh/MarSeventh/imgbed/posts/image-20230529192106043.png)

当然，该任务到这里还并没有结束，还有一项要求是在管道重定向到一个不存在的文件时，需要自动创建该文件。要完成该项要求，需要实现O_CREAT的文件打开方式，在open中如果遇到该打开方式，首先判断文件是否存在，存在则正常打开，不存在的话就使用file_create创建一个即可，具体实现如下：

```c
if ((r = file_open(rq->req_path, &f)) < 0) {
		if((rq->req_omode &  O_CREAT) == O_CREAT) {
			r = file_create(rq->req_path, &f);
			if(r < 0) {
				ipc_send(envid, r, 0, 0);
				return;
			}
		} else {
		        ipc_send(envid, r, 0, 0);
		        return;
		}
	}
```

当然，还需要在解析到管道指令的时候给文件打开方式加上O_CREAT。

测试如下：

```
echo hello > hello.txt
cat hello.txt
```

执行结果如下：

![](https://cdn.jsdelivr.net/gh/MarSeventh/imgbed/posts/image-20230529193149913.png)

可以看到，在hello.txt不存在的情况下，成功将hello输出到了hello.txt当中。

### 7、实现历史命令功能

该任务的实现主要分为两部分，即`history`指令的实现以及通过上下方向键回溯指令。

要实现这两个功能，基础是得先记录下来所有的输入指令，我们只要在每一次runcmd之前记录一下读取到的指令即可。

具体实现如下：

```c
int save_history(char *cmd) {
        int r = open(".history", O_CREAT | O_WRONLY | O_APPEND);
        if(r < 0) {
               fprintf(1, "can't open history command!");
                return r;
        }
        write(r, cmd, strlen(cmd));
        write(r, "\n", 1);
        close(r);
        return 0;
}
```

history指令只需要读取.history文件的所有内容，将其输出：

```c
#include <lib.h>

int main(int argc, char **argv) {
	if(argc != 1) {
		fprintf(1, "usage: history\n");
		return -1;
	}
	int fd = open(".history", O_RDONLY);
	if(fd < 0) {
		fprintf(1, "can't open history command!\n");
		return fd;
	}
	int count = 1;
	int flag = 1;
	char buf;
	while(read(fd, &buf, 1)) {
		if(flag) {
			fprintf(1, "%8d    ", count);
			flag = 0;
		}
		fprintf(1, "%c", buf);
		if(buf == '\n') {
			count++;
			flag = 1;
		}
	}
	return 0;
}
```

最复杂的部分还是实现通过上下键回溯指令。大致思路是像之前判断左右键一样对上下键进行特判，一旦输入到上下键，首先将已经输入内容清空，然后去.history文件中读取之前的指令并输出到控制台。但是这个过程中要注意许多的细节问题，例如如何记录已经读取到的指令位置，如何对光标的位置进行限制等等。

具体的实现如下：

```c
//readline
if (ch == 'A') {
				if(i) {
				      fprintf(1, "\x1b[1B\x1b[%dD\x1b[K", i);
				} else {
				      fprintf(1, "\x1b[1B");
				}
				i = get_history(1, buf) - 1;
				if(i >= 0) {
			 	      fprintf(1, "%s", buf);
				} else {
					fprintf(1, "%s", tmp);
					strcpy(buf, tmp);
					i = strlen(tmp) - 1;
				}
				pos = i;
			}
			else if(ch == 'B') {
				if(i) {
				      fprintf(1, "\x1b[%dD\x1b[K", i);
				}
				i = get_history(0, buf) - 1;
				if(i >= 0) {
				      fprintf(1, "%s", buf);
				} else {
				      fprintf(1, "%s", tmp);
				      strcpy(buf, tmp);
				      i = strlen(tmp) - 1;
				}
				pos = i;
			}
//get_history
int get_history(int back, char *buf) {
        static struct Fd *fd;
        static int fdnum = 0;
        static char *va;
        static char *begin;
        static char *end;
        if(!fdnum || newcmd) {
              newcmd = 0;
              if((fdnum = open(".history", O_RDONLY)) < 0) {
                   // fprintf(1, "can't find history command!\n");
		    newcmd = 1;
		    return 0;
              }
              fd = (struct Fd *)num2fd(fdnum);
              begin = (char *)fd2data(fd);
              end = begin + ((struct Filefd *)fd)->f_file.f_size;
              va = end - 1;
        }
        if(back) {
              while(*va == 0 || *va == '\n') va--;
              while(*va != '\n' && va > begin) va--;
              char *p;
              int i = 0;
              if(*va == '\n') {
                     p = va + 1;
              }
              for(;*p != '\n' && *p != 0 && p < end; p++){
                     buf[i++] = *p;
	      }
	      buf[i] = 0;
              return i;
        } else {
              while(*va == '\n' && va < end) va++;
              while(*va != '\n' && va < end) va++;
              if(va == end) {
                     *buf = 0;
                     return 0;
              }
              int i = 0;
              char *p;
              if(*va == '\n') {
                      p = va + 1;
              }            
	      while(*p != '\n' && p < end) {
                      buf[i++] = *p++;
              }
              buf[i] = 0;
              return i;
       }                                                                                                                      
}
```

在实现了基本的功能之后，我还增加了保存当前已输入内容的功能，也就是当回溯不到相关指令时，将之前已经输入的字符串再次打印到标准输出，这样一来就与bash更加接近。

测试如下（假装是动态的）：

```
touch 1; touch 2
echo hello
echo bad
history
```

![](https://cdn.jsdelivr.net/gh/MarSeventh/imgbed/posts/image-20230529200706466.png)

上下键回溯发现一切正常，并且因为对光标加以限制，它也不会乱跑到不该去的地方了（

## 选做部分

在选做部分中，我选择的是实现shell的环境变量。

要实现环境变量，首先要知道两种环境变量的类型，也就是环境变量和局部变量。其中，

**环境变量**：对子shell可见，但是子shell修改其内容时对父shell不可见。

**局部变量**：对子shell不可见。

概念中的子shell其实指代的是shell运行过程中通过sh命令spawn出来的shell，而对于fork出来的shell，应该是可以和父shell共享相同的环境变量和局部变量。

对于环境变量和局部变量，我采用了两种存储方式，环境变量存储在内核态中，局部变量则直接作为shell进程的一个全局变量。存储的方式都是使用`EnviormentValue`结构体进行存储，该结构体包含了变量名、变量值、读写方式以及创建id等内容。

对于内核态中存储的环境变量，需要增加对它进行访问和修改的系统调用，具体步骤和之前添加系统调用的老套路一样，先添加中断向量，然后在内核态实现系统调用的具体函数，在用户态添加该调用的接口即可。需要注意，因为子shell对环境变量的修改对父进程不可见，因此需要在创建子shell的时候将父进程环境变量都给子进程复制一份。

对于用户态的局部变量，可以将其当做一个正常的数组进行修改和访问，但是要注意的一点是，为了保证仅仅在fork的时候子进程对局部变量的修改对父进程可见，但是对于spawn出来的shell则不行，需要新增一条地址权限PTE_FORK，使得拥有该权限的地址只和fork出来的子进程进行内存共享，然后给局部变量所在地址映射为该权限。

除此之外，因为对环境变量进行访问时需要判断是否为该进程的环境变量（读取无所谓，主要是修改的时候要用），因此我们需要设置一种标志不同shell的方式，也就是给shell一个id，这个id需要保证fork出来的子shell和父shell相同，但是spawn出来的不同。我的实现思路是在shell中加一个sh_id全局变量，然后在shell刚开始运行时将其赋值为该进程的envid，这样就能保证fork出来的子进程仍然和父进程有同一个sh_id，并且spawn出来的不同，实现了对环境变量的互斥访问。

一些重要的函数实现如下：

```c
//系统调用主函数
int sys_getGlobalVar(char* name, char* val, int mode, int id, int rwMode) {
	//printk("globalVar:%s id:%d sh_id:%d mode:%d\n",globalVar[0].name,globalVar[0].id,id,globalVar[0].mode);
	static int init = 1;
	int i,j;

	if(init) {
		for(i = 0; i < 1000; i++) {
			globalVar[i].mode = 0;
		}
		init = 0;
	}
	if(mode == 0) {//check whether has the envvalue
		for(i = 0;i < 1000;i++) {
			if(globalVar[i].mode != 0 && strcmp(globalVar[i].name, name) == 0 && globalVar[i].id == id) {
				return 0;
			}
		}
		return -1;
	}
	if(mode == 1) {//set value of var[name]
		for(i = 0;i < 1000;i++) {
			if(globalVar[i].mode == 2 && strcmp(globalVar[i].name, name) == 0 && globalVar[i].id == id) {
				if(val) {
				      strcpy(globalVar[i].value, val);
				}
				globalVar[i].mode = rwMode;
				return 0;
			}
		}
		return -1;
	}
	if(mode == 2) {//delete var
		for(i = 0;i < 1000;i++) {
			if(globalVar[i].mode == 2 && strcmp(globalVar[i].name, name) == 0 && globalVar[i].id == id) {
				globalVar[i].mode = 0;
				return 0;
			}
		}
		return -1;
	}
	if(mode == 3) {//get the value
		for(i = 0;i < 1000;i++) {
			if(globalVar[i].mode != 0 && strcmp(globalVar[i].name, name) == 0 && globalVar[i].id == id) {
				strcpy(val, globalVar[i].value);
				return 0;
			}
		}
		return -1;
	}
	if(mode == 4) {//create a new var
		//printk("create a globalVar: %s id:%d! \n", name,id);
		//printk("globalVar:%s id:%d\n",globalVar[0].name,globalVar[0].id);
		for(i = 0;i < 1000;i++) {
			if(globalVar[i].mode == 0) {
				globalVar[i].mode = rwMode;
				strcpy(globalVar[i].name, name);
				strcpy(globalVar[i].value, val);
				globalVar[i].id = id;
				//printk("var shellid:%d\n",globalVar[i].id);
				//printk("globalVar:%s id:%d mode:%d\n",globalVar[0].name,globalVar[0].id,globalVar[0].mode);
				return 0;
			}
		}
		return -1;
	}
	if(mode == 5) {//copy parent to son
		EnviormentValue parent[1000];
		int parsum = 0;
		for(i = 0;i < 1000;i++) {
			if(globalVar[i].mode != 0) {
				int flag = 0;
				for(j = 0;j < parsum;j++) {
					if(strcmp(parent[j].name, globalVar[i].name) == 0) {
						flag = 1;
						break;
					}
				}
				if(flag) continue;
				parent[parsum].mode = globalVar[i].mode;
				strcpy(parent[parsum].name, globalVar[i].name);
				strcpy(parent[parsum].value, globalVar[i].value);
				parsum++;
			}
		}
		j = 0;
		for(i = 0;i < 1000;i++) {
			if(j == parsum) break;
			if(globalVar[i].mode == 0) {
				strcpy(globalVar[i].name, parent[j].name);
				strcpy(globalVar[i].value, parent[j].value);
				globalVar[i].mode = parent[j].mode;
				globalVar[i].id = id;
				j++;
			}
		}
		return 0;
	}
	if(mode == 6) {
		int cur = rwMode;
		//printk("globalVar:%s id:%d sh_id:%d mode:%d\n",globalVar[0].name,globalVar[0].id,id,globalVar[0].mode);
		//printk("get shellid:%d\n",id);
	       //printk("globalVar:%s id:%d cur:%d sh_id:%d\n mode:%d",globalVar[0].name,globalVar[0].id,cur,id,globalVar[0].mode);
		for(i = cur;i < 1000;i++) {
			if(globalVar[i].mode != 0 && globalVar[i].id == id) {
				//printk("find one!\n");
				strcpy(name, globalVar[i].name);
				strcpy(val, globalVar[i].value);
				return i;
			}
		}
		return -1;
	}
	if(mode == 7) {
		for(i = 0;i < 1000;i++) {
			if(globalVar[i].id == id) {
				globalVar[i].mode = 0;
			}
		}
		return 0;
	}
	return 0;
}
```

```c
//环境变量和局部变量操作函数
void getVar(char *name) {
	int i, r;
	char val[16];
	//printf("%s\n", name);
	r = syscall_getGlobalVar(name, val, VARGET, sh_id, 0);
	if(r >= 0) {
		fprintf(1, "globalVar: name: %s value: %s\n", name, val);
	}
	for(i = 0;i < 100;i++) {
		if(localVar[i].mode != 0 && strcmp(localVar[i].name, name) == 0) {
			fprintf(1, "localVar: name: %s value: %s\n",name,localVar[i].value);
			break;
		}
	}
	if(r < 0 && i == 100) {
		fprintf(1, "no such var!!!\n");
	}
}

void showvar(){
	int i,r;
	char name[16];
	char val[16];
	//printf("id: %d\n",sh_id);
	for(i = 0;i < 1000;i++) {
		r = syscall_getGlobalVar(name, val, VARGETALL, sh_id, i);
		if(r >= 0) {
			i = r;
			fprintf(1, "shellId: %d name: %s value: %s\n", sh_id, name, val);
		} else {
			//printf("%d\n",r);
			break;
		}
	}
	for(i = 0;i < 100;i++) {
		if(localVar[i].mode != 0) {
			fprintf(1, "shellId: %d name: %s value: %s\n", sh_id, localVar[i].name, localVar[i].value);
		}
	}
//	fprintf(1,"loaclVar:%s value:%s mode:%d\n",localVar[0].name, localVar[0].value, localVar[0].mode);
//	fprintf(1, "No var found!\n");
}

void declare(int argc, char **argv) {
	int i, r;
	int isglobal = 0;
	int isonlyrd = 0;
	ARGBEGIN
	{
		case 'x':
		         isglobal = 1;
			 break;
	        case 'r':
		         isonlyrd = 1;
			 break;
	        defult:
		         fprintf(1,"usage: declare [name [=value]]\n");
			 break;
	}
	ARGEND
	if(argc == 0) {
		showvar();
		return;
	}
	char name[16] = {0};
	char val[16] = {0};
	int rwMode = (isonlyrd)? 1 : 2;
	i = 0;
	int j = 0;
	while(argv[0][i] != 0 && argv[0][i] != '=') {
		 name[j++] = argv[0][i++];
	}
	j = 0;
	while(argv[0][i] != 0) {
		if(argv[0][i] != '=') {
			val[j++] = argv[0][i];
		}
		i++;
	}
	if(isglobal) {
		if((r = syscall_getGlobalVar(name, val, VARCHECK, sh_id, DEFAULT)) < 0) {
			syscall_getGlobalVar(name, val, VARCREAT, sh_id, rwMode);
		} else {
			if((r = syscall_getGlobalVar(name, val, VARSET, sh_id, rwMode)) < 0) {
				fprintf(1, "you can't set the var!\n");
			}
		}
	} else {
		for(i = 0;i < 100;i++) {
			if(strcmp(localVar[i].name, name) == 0 && localVar[i].mode != 0) {
				if(localVar[i].mode == RDONLY) {
					fprintf(1, "you can't set the var!\n");
				} else {
					localVar[i].mode = rwMode;
					strcpy(localVar[i].value, val);
				}
			}
		}
		if(i == 100) {
			for(i = 0;i < 100;i++) {
				if(localVar[i].mode == 0) {
					//fprintf(1, "set the var %s\n", name);
					localVar[i].mode = rwMode;
					strcpy(localVar[i].name, name);
					strcpy(localVar[i].value, val);
					break;
				}
			}
		}
	}
}

void unset(int argc, char **argv) {
	int i;
	ARGBEGIN
	{
		defult:
		       fprintf(1, "usage: unset[NAME]\n");
		       break;
	}
	ARGEND

	if(argc == 0) {
		fprintf(1, "usage: unset[NAME]\n");
	}
	char *name = argv[0];
	int r;
	if((r = syscall_getGlobalVar(name, 0, VARDEL, sh_id, DEFAULT)) < 0) {
		fprintf(1, "can't delete globalVar:%s\n",name);
	} else {
		fprintf(1, "have deleted globalVar:%s\n",name);
	}

	for(i = 0;i < 100;i++) {
		if(localVar[i].mode == RDWR && strcmp(localVar[i].name, name) == 0) {
			localVar[i].mode = 0;
			fprintf(1,"have deleted localVar:%s\n",name);
			break;
		}
	}
	if(i == 100) {
		fprintf(1, "can't delete localVar:%s\n",name);
	}
}
```

测试：

![](C:\Users\King.xx\AppData\Roaming\Typora\typora-user-images\image-20230529204124878.png)

![](C:\Users\King.xx\AppData\Roaming\Typora\typora-user-images\image-20230529204147576.png)

![image-20230529204224297](C:\Users\King.xx\AppData\Roaming\Typora\typora-user-images\image-20230529204224297.png)

![image-20230529204321322](C:\Users\King.xx\AppData\Roaming\Typora\typora-user-images\image-20230529204321322.png)

可以看到，对于环境变量和局部变量有关的添加、删除、查询操作都正确实现了。

除此之外，还需要支持使用echo指令获取环境变量的值，这个实现只需要我们对echo指令的第一个参数进行特判，如果该参数以`$`开头，就跳转到输出环境变量值的函数执行即可。

测试：

![](C:\Users\King.xx\AppData\Roaming\Typora\typora-user-images\image-20230529205649048.png)

## 总结

总体而言，这一次挑战性任务相对于之前的Lab来说的难度还是相当大的，可以说真的非常具有“挑战性”。因为无论是从里面各种数据结构的实现，还是不同函数之间的关系，亦或是对文件系统、用户进程和内核进程有关功能的添加，都需要我们对整个操作系统的结构非常熟悉，并且还得具有很强的编码和设计能力。这些对我而言都有着不小的难度。

但是将挑战性任务完成之后，我发现我对MOS系统的了解已经相对于之前有了一个质的飞跃。之前尽管完成了Lab6，但是依然对系统中实现的细节感到云里雾里，根本不太清楚各个进程之间交互的具体流程。写完挑战性任务，尤其是选做部分之后，我感觉自己对于系统调用、内核和用户态的交互，还有fork、spawn等部分都已经较为熟悉，对系统的整体架构掌握地更加充分，真正地体验到了作为一个系统设计者的感觉。
