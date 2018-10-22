#include <stdio.h>

unsigned int read_word() {
	unsigned int word;
	char *buf = (char *) &word;

	for (int i = 0; i < sizeof word; i++) {
		*buf++ = getchar();
	}

	return word;
}

int main() {
	int w = read_word();

	if (w == 0x00000000) {
		printf("Hello world\n");
	} else if (w == 0x00000001) {
	} else {
		printf("Invalid word 0x%08x\n", w);
		return 1;
	}

	return 0;
}
