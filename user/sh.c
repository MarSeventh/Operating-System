#include <args.h>
#include <lib.h>

#define WHITESPACE " \t\r\n"
#define SYMBOLS "<|>&;()"
#define VARCHECK 0
#define VARSET 1
#define VARDEL 2
#define VARGET 3
#define VARCREAT 4
#define VARCOPY 5
#define VARGETALL 6

#define DEFAULT 0
#define RDONLY 1
#define RDWR 2

int hang = 1;
int newcmd = 1;
int sh_id = 0;
int dupl = 0;
int dupr = 0;
EnviormentValue localVar[100];

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
/* Overview:
 *   Parse the next token from the string at s.
 *
 * Post-Condition:
 *   Set '*p1' to the beginning of the token and '*p2' to just past the token.
 *   Return:
 *     - 0 if the end of string is reached.
 *     - '<' for < (stdin redirection).
 *     - '>' for > (stdout redirection).
 *     - '|' for | (pipe).
 *     - 'w' for a word (command, argument, or file name).
 *
 *   The buffer is modified to turn the spaces after words into zero bytes ('\0'), so that the
 *   returned token is a null-terminated string.
 */
int _gettoken(char *s, char **p1, char **p2) {
	*p1 = 0;
	*p2 = 0;
	if (s == 0) {
		return 0;
	}

	while (strchr(WHITESPACE, *s)) {
		*s++ = 0;
	}
	if (*s == 0) {
		return 0;
	}
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
	if (strchr(SYMBOLS, *s)) {
		int t = *s;
		*p1 = s;
		*s++ = 0;
		*p2 = s;
		return t;
	}

	*p1 = s;
	while (*s && !strchr(WHITESPACE SYMBOLS, *s)) {
		s++;
	}
	*p2 = s;
	return 'w';
}

int gettoken(char *s, char **p1) {
	static int c, nc;
	static char *np1, *np2;

	if (s) {
		nc = _gettoken(s, &np1, &np2);
		return 0;
	}
	c = nc;
	*p1 = np1;
	nc = _gettoken(np2, &np1, &np2);
	return c;
}

#define MAXARGS 128

int parsecmd(char **argv, int *rightpipe) {
	int argc = 0;
	int fid;
	while (1) {
		char *t;
		int fd, r;
		int c = gettoken(0, &t);
		switch (c) {
		case 0:
			return argc;
		case 'w':
			if (argc >= MAXARGS) {
				debugf("too many arguments\n");
				exit();
			}
			argv[argc++] = t;
			break;
		case ';':
		        fid = fork();
			if(fid == 0) {
				return argc;
			} else {
				wait(fid);
				return parsecmd(argv, rightpipe);
			}
			break;
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
		case '<':
			if (gettoken(0, &t) != 'w') {
				debugf("syntax error: < not followed by word\n");
				exit();
			}
			// Open 't' for reading, dup it onto fd 0, and then close the original fd.
			/* Exercise 6.5: Your code here. (1/3) */
                        fd = open(t, O_RDONLY);
			dup(fd, 0);
			close(fd); 
			//user_panic("< redirection not implemented");
                        dupl = 1;
			break;
		case '>':
			if (gettoken(0, &t) != 'w') {
				debugf("syntax error: > not followed by word\n");
				exit();
			}
			// Open 't' for writing, dup it onto fd 1, and then close the original fd.
			/* Exercise 6.5: Your code here. (2/3) */
                        fd = open(t, O_CREAT | O_WRONLY);
			dup(fd, 1);
			close(fd);
			//user_panic("> redirection not implemented");
                        dupr = 1;
			break;
		case '|':;
			/*
			 * First, allocate a pipe.
			 * Then fork, set '*rightpipe' to the returned child envid or zero.
			 * The child runs the right side of the pipe:
			 * - dup the read end of the pipe onto 0
			 * - close the read end of the pipe
			 * - close the write end of the pipe
			 * - and 'return parsecmd(argv, rightpipe)' again, to parse the rest of the
			 *   command line.
			 * The parent runs the left side of the pipe:
			 * - dup the write end of the pipe onto 1
			 * - close the write end of the pipe
			 * - close the read end of the pipe
			 * - and 'return argc', to execute the left of the pipeline.
			 */
			int p[2];
			/* Exercise 6.5: Your code here. (3/3) */
                        pipe(p);
			*rightpipe = fork();
			if(!*rightpipe) {
				dup(p[0], 0);
				close(p[0]);
				close(p[1]);
				return parsecmd(argv, rightpipe);
			} else {
				dup(p[1], 1);
				close(p[1]);
				close(p[0]);
				return argc;
			}
			//user_panic("| not implemented");

			break;
		}
	}

	return argc;
}

void runcmd(char *s) {
	save_history(s);
	gettoken(s, 0);
	char *argv[MAXARGS];
	int rightpipe = 0;
	int argc = parsecmd(argv, &rightpipe);
	if (argc == 0) {
		return;
	}
	argv[argc] = 0;
	if(strcmp(argv[0], "echo") == 0) {
		if(argv[1][0] == '$') {
			char name[50];
			int i = 1;
			while(argv[1][i] != 0) {
				name[i-1] = argv[1][i];
				i++;
			}
			name[i-1] = 0;
			getVar(name);
			close_all();
			goto end;
		}
	}
        if(strcmp(argv[0], "declare") == 0) {
		declare(argc, argv);
		close_all();
		goto end;
	}
	if(strcmp(argv[0], "unset") == 0) {
		unset(argc, argv);
		close_all();
		goto end;
	}
	int child = spawn(argv[0], argv);
	close_all();
	if (child >= 0) {
		if(hang) wait(child);
	} else {
		debugf("spawn %s: %d\n", argv[0], child);
	}

end:	if (rightpipe) {
		wait(rightpipe);
	}
	exit();
}

void readline(char *buf, u_int n) {
	int r;
	int pos = 0;
	int newinput = 1;
	for (int i = 0; i < n; i++) {
		if ((r = read(0, buf + i, 1)) != 1) {
			if (r < 0) {
				debugf("read error: %d\n", r);
			}
			exit();
		}
		if(buf[i] != '\x1b') {
                        newinput = 1;
                }
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
		if (buf[i] == '\r' || buf[i] == '\n') {
			buf[i] = 0;
			return;
		}
		if (buf[i] == '\x1b') {
			char tmp[1024];
			if(newcmd || newinput) {
			     newinput = 0;
			     int j = 0;
			     for(j = 0; j < i;j++){
				     tmp[j] = buf[j];
			     }
			     tmp[i] = 0;
			}
			char ch;
			read(0, &ch, 1);
			if(ch != '[') {
				user_panic("illegal cmd!\n");
			}
			read(0, &ch, 1);
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
			} else if(ch == 'D'){
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
		}
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
		pos++;
	}
	debugf("line too long\n");
	while ((r = read(0, buf, 1)) == 1 && buf[0] != '\r' && buf[0] != '\n') {
		;
	}
	buf[0] = 0;
}

char buf[1024];

void usage(void) {
	debugf("usage: sh [-dix] [command-file]\n");
	exit();
}


int main(int argc, char **argv) {
	int r;
	int interactive = iscons(0);
	int echocmds = 0;
	sh_id = syscall_getenvid();//update sh_id
	printf("sh_id: %d\n", sh_id);
	syscall_getGlobalVar(0, 0, 5, sh_id, 0);
	syscall_mem_map(0, localVar, 0, localVar, PTE_V | PTE_D | PTE_FORK);
	debugf("\n:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::\n");
	debugf("::                                                         ::\n");
	debugf("::                   MOS Shell Pro Max                     ::\n");
	debugf("::                                                         ::\n");
	debugf(":::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::\n");
	ARGBEGIN {
	case 'i':
		interactive = 1;
		break;
	case 'x':
		echocmds = 1;
		break;
	default:
		usage();
	}
	ARGEND

	if (argc > 1) {
		usage();
	}
	if (argc == 1) {
		close(0);
		if ((r = open(argv[1], O_RDONLY)) < 0) {
			user_panic("open %s: %d", argv[1], r);
		}
		user_assert(r == 0);
	}
	for (;;) {
		if (interactive) {
			printf("\n$ ");
		}
		readline(buf, sizeof buf);

		if (buf[0] == '#') {
			continue;
		}
		if (buf[0] == '!') {
			break;
		}
		if (echocmds) {
			printf("# %s\n", buf);
		}
		if ((r = fork()) < 0) {
			user_panic("fork: %d", r);
		}
		if (r == 0) {
		//	fprintf(1, "id: %d\n", sh_id);
			runcmd(buf);
			exit();
		} else {
			newcmd = 1;
			wait(r);
		}
	}
	syscall_getGlobalVar(0, 0, 7, sh_id, 0);
	return 0;
}
