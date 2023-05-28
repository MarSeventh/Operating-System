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
