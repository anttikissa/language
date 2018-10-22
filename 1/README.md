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

We now consider [the compiler](./compiler.c) done.

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

The interpreter is pretty simple. We use the same machine-specific trick
to read a word as we used to write it:

```
#include <stdio.h>

unsigned int read_word() {
	unsigned int word;
	char *buf = (char *) &word;

	for (int i = 0; i < sizeof word; i++) {
		*buf++ = getchar();
	}

	return word;
}
```

We again use `sizeof word` instead of hard-coding the length to 4 bytes,
hiding the bold assumption that an `unsigned int` is always 4 bytes. Of
course, if it suddenly changes, everything continues to work if all
executables are recompiled - a perfectly fine tradeoff given the kature
of the language at this phase.

Then the main program:

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

It's very simple. We can check that it works:

```
% ./interpreter < hello.exe
Hello world
% ./interpreter < halt.exe
%
```

And also that you can't feed it just anything:

% echo rubbish | ./interpreter
Invalid word 0x62627572

One thing to notice, though, is that it can consume valid programs that are
followed by rubbish:

```
% (cat hello.exe; echo rubbish) | ./interpreter
Hello world
```

This is to be expected given the nature of binary code. It's meant for someone
to execute. If the interpreter that executes it notices an error while doing
it, an error should be produced; but if there's something hidden after a
correct program, the interpreter doesn't really need to care. (In this sense,
interpreters and compilers are different; never mind the fact that actually at
this phase our compiler does exactly the same!)

## The interpreter: post-mortem

There's not much to say at this phase. The interpreter seems a little
bit less complex than the compiler, and I expect that they continue to
have this relation as the language grows bigger. Basically, the
interpreter is a simple inner loop that goes through a machine
instruction one at a time (currently, just processing one instruction),
while the compiler might need more complexity eventually.

The whole interpreter [is here.](./interpreter.c)
