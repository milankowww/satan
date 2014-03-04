#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
	int key, val;
} Value;

typedef struct {
	int size;
	Value *elems;
} Hash;

void Hash_init(Hash *h)
{
	h->elems = malloc(h->size = 0);
}

void Hash_set(Hash *h, int key, int val)
{
	Value v;
	v.key = key;
	v.val = val;

	h->elems = realloc(h->elems, ++h->size * sizeof(Value));
	h->elems[h->size - 1] = v;
}

int Hash_get(Hash *h, int key)
{
	int i;

	for (i = 0; i < h->size; ++i)
		if (h->elems[i].key == key)
			return h->elems[i].val;

	return 0;
}

const char *ops[] = {
	"  ", " \t ", " \t\n", "\n  ", "\n \t", "\n \n", "\n\t ", "\n\t\t", " \n\n",
	" \n ", " \n\t", "\t   ", "\t  \t", "\t  \n", "\t \t ", "\t \t\t", "\t\t ",
	"\t\t\t", "\n\t\n", "\t\n\t ", "\t\n\t\t", "\t\n  ", "\t\n \t", "\n\n\n"
};

char *src;

int arg(void) {
	int val = 0;
	int neg = *src++ == '\t';

	while (*src != '\n')
		val = (val + (*src++ == '\t')) << 1;

	++src;
	return (val >> 1) * (neg ? -1 : 1);
}

void interpret(int *prog, int len)
{
	int i, pc = -1, arg;
	int cs = 0, sp = 0;
	int *stack = malloc(0);
	int *calls = malloc(0);

	Hash heap, labs;
	Hash_init(&heap);
	Hash_init(&labs);

	for (i = 0; i < len; ++i)
		if (prog[i] == 3)
			Hash_set(&labs, prog[i + 1], i + 1);

	while (++pc < len) {
		switch (prog[pc]) {
		case 0: /* push */
			arg = prog[++pc];
			stack = realloc(stack, ++sp * sizeof(int));
			stack[sp - 1] = arg;
			break;
		case 1: /* copy */
			arg = prog[++pc];
			stack = realloc(stack, ++sp * sizeof(int));
			stack[sp - 1] = stack[sp - arg - 2];
			break;
		case 2: /* slide */
			arg = stack[sp - 1];
			stack[(sp -= prog[++pc]) - 1] = arg;
			break;
		case 3: /* label */
			arg = prog[++pc];
			break;
		case 4: /* call */
			arg = prog[++pc];
			calls = realloc(calls, ++cs * sizeof(int));
			calls[cs - 1] = pc;
			pc = Hash_get(&labs, arg);
			break;
		case 5: /* jump */
			arg = prog[++pc];
			pc = Hash_get(&labs, arg);
			break;
		case 6: /* jz */
			arg = prog[++pc];
			if (stack[--sp] == 0)
				pc = Hash_get(&labs, arg);
			break;
		case 7: /* jn */
			arg = prog[++pc];
			if (stack[--sp] < 0)
				pc = Hash_get(&labs, arg);
			break;
		case 8: /* pop */
			--sp;
			break;
		case 9: /* dup */
			stack = realloc(stack, ++sp * sizeof(int));
			stack[sp - 1] = stack[sp - 2];
			break;
		case 10: /* swap */
			arg = stack[sp - 1];
			stack[sp - 1] = stack[sp - 2];
			stack[sp - 2] = arg;
			break;
		case 11: /* add */
			stack[sp - 2] += stack[sp - 1];
			--sp;
			break;
		case 12: /* sub */
			stack[sp - 2] -= stack[sp - 1];
			--sp;
			break;
		case 13: /* mul */
			stack[sp - 2] *= stack[sp - 1];
			--sp;
			break;
		case 14: /* div */
			stack[sp - 2] /= stack[sp - 1];
			--sp;
			break;
		case 15: /* mod */
			stack[sp - 2] %= stack[sp - 1];
			--sp;
			break;
		case 16: /* store */
			Hash_set(&heap, stack[sp - 2], stack[sp - 1]);
			sp -= 2;
			break;
		case 17: /* load */
			arg = stack[sp - 1];
			stack[sp - 1] = Hash_get(&heap, arg);
			break;
		case 18: /* ret TODO */
			pc = calls[--cs];
			break;
		case 19: /* ichr */
			Hash_set(&heap, stack[sp - 1], getchar());
			break;
		case 20: /* inum */
			if (scanf("%d", &arg) == 1)
				Hash_set(&heap, stack[sp - 1], arg);
			break;
		case 21: /* ochr */
			printf("%c", stack[--sp]);
			break;
		case 22: /* onum */
			printf("%d", stack[--sp]);
			break;
		case 23: /* exit */
			free(stack);
			free(heap.elems);
			return;
		}
	}
}

int main(int argc, char **argv)
{
	FILE *in;
	long len;
	int i, *prog, ps = 0;

	if (argc != 2)
		return fprintf(stderr, "Usage: %s FILE\n", *argv);

	if ((in = fopen(argv[1], "r")) == NULL)
		return fprintf(stderr, "No such file: %s\n", argv[1]);

	fseek(in, 0, SEEK_END);
	len = ftell(in);
	rewind(in);
	src = malloc(len + 1);
	if (fread(src, len, 1, in))
		src[len] = 0;

	prog = malloc(0);

	while (*src) {
		for (i = 0; i < 24; ++i) {
			if (strstr(src, ops[i]) == src) {
				prog = realloc(prog, ++ps * sizeof(int));
				prog[ps - 1] = i;
				src += strlen(ops[i]);

				if (i < 8) {
					prog = realloc(prog, ++ps * sizeof(int));
					prog[ps - 1] = arg();
				}
			}
		}
	}

	interpret(prog, ps);

	free(prog);
	free(src - len);
	fclose(in);
	return 0;
}
