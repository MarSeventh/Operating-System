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

