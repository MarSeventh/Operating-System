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
