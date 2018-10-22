#include <stdio.h>
#include <string.h>

void output(int word) {
	char *buf = (char *) &word;

	for (int i = 0; i < sizeof word; i++) {
		putchar(buf[i]);
	}
}

int main() {
	char buf[16] = { 0 };
	int i = 0;

	while (1) {
		int c = getchar();

		if (c == '\n' || c == -1) {
			if (strcmp(buf, "hello") == 0) {
				output(0x00000000);
			} else if (strcmp(buf, "halt") == 0) {
				output(0x00000001);
			} else {
				fprintf(stderr, "Unknown word: %s\n", buf);
				return 1;
			}
			return 0;
		}

		if (i + 1 == sizeof buf) {
			fprintf(stderr, "Program too long: %s\n", buf);
			return 1;
		}

		buf[i++] = c;
	}
}
