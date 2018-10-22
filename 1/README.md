# Step 1

Our first language is a very simple one. We'll be dividing the
implementation into two parts: first, let's write a compiler that
produces an executable file from source code; then we need an
interpreter that is able to execute the instructions in the executable.

For simplicity, we'll implement both the compiler and interpreter as commands
that consume some input and produce some output; this lets us forego file
handling, which is a source of complexity that we don't need right now.

Let's start with the compiler.

# The compiler

Our first programming language has no state whatsoever, not even a concept of a
program counter.  The programmer has at his disposal two commands:

- `hello` prints out the text "Hello world" and stops execution.
- `halt` just stops execution.

We decide to represent these with 32-bit words. The word 0x00000000
means `hello` and the word 0x00000001 means `halt`. All other words are
considered erroneous programs.

The task of the compiler is, then, to read an English word and turn it into a
32-bit machine word.

We decide that a source code representation of a program is the word 'hello' or
'halt', followed by an optional newline.

To output a program, we implement a function that writes a word to the standard
output. We use the function `putchar` to do this byte by byte:

```
#include <stdio.h>

void output(int word) {
	char* buf = (char *) &word;

	for (int i = 0; i < sizeof word; i++) {
		putchar(buf[i]);
	}
}
```

Note that we just introduced a platform-specific factor (the endianness), but
let's just assume that for now, all programs are executed on the same computer
they are written on.

To read the program, we use a buffer of 16 bytes (given that all legal programs
consist of at most 5 characters, it should be enough for everyone):

```
int main() {
	// Initialize a buffer for the program and an index that
	// points to where we currently are:
	char buf[16] = { 0 };
	int i = 0;

	while (1) {
		// part A: read a character
		// part B: decide what to do with it
		// part C: if nothing happened, put it in buf
	}
}
```

Let's implement parts A and B first:

## Part A

We use `getchar` to read the character.

```
		int c = getchar();
```

## Part C

And this is how we write it into the buffer:

```
		if (i + 1 == sizeof buf) {
			fprintf(stderr, "Program too long: %s\n", buf);
			return 1;
		}

		buf[i++] = c;
```

Note that we need to restrict the program length to 15 bytes since there needs
to be one extra zero byte to terminate the string in the error message.

And here comes part B, which is the most complex:

## Part B

```
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
```

If we reached the end of the file (-1) or a newline ('\n'), the program
terminates. We then check for the two possible valid programs and output the
correct words for each, and an error message if the program was not valid. At
all cases the compiler then exits.

Note that we consider white space (except for the newline character) to be part
of the word, which is not quite how it works in English or most programming
languages, but it simplifies our implementation a great deal.

Addition to the code above, the program also needs to include some standard
library headers to work properly. (See [the compiler](./compiler.c) in
entirety.)

To build the program:

```
gcc compiler.c -o compiler
```

Here's how we can see that it works:

```
% echo halt > halt.src
% ./compiler < halt.src > halt.exe
% hexdump halt.exe
0000000 01 00 00 00
0000004

% echo hello > hello.src
% ./compiler < hello.src > hello.exe
% hexdump hello.exe
0000000 00 00 00 00
0000004
```

And that the erroneous cases behave as expected:

```
% echo invalid program | ./compiler
Unknown word: invalid program

% % ./compiler < toolong.src
Program too long: just a bit too
```

We now consider the [compiler](./compiler.c) done.

## The compiler: a post-mortem

The specification is not perfect; for instance, it's a bit weird that only the
first line of the program counts. Also, not taking whitespace (except
for newlines) into account leads into weird situations like:

```
	% echo ' halt' | ./compiler
	Unknown word:  halt
```

However, with the scope of the current language, we consider these
non-factors. Let's get something done instead!

## The interpreter

