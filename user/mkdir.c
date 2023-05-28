#include <lib.h>

int main(int argc, char **argv) {
	if(argc != 2) {
		fprintf(1, "usage: mkdir[dirname]\n");
		return -1;
	}
	if(open(argv[1], O_RDONLY) >= 0) {
		fprintf(1, "directory has existed!\n");
		return -1;
	}
	if(create(argv[1], FTYPE_DIR) < 0) {
		fprintf(1, "create directory failed!\n");
		return -1;
	}
	return 0;
}

