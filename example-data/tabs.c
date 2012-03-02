/* This file is written using tabs-only */
#include <stdio.h>

int some_function(void)
{
	int i;

	for (i = 0; i < 10; i++) {
		printf("XXX: %d\n", i);
	}
}

int test(int is_space)
{
	int i;

	for (i = 0; i < 10; i++) {
		if (is_space)
			printf("SPACE %d\n", i);
		else
			printf("NON-SPACE %d\n", i);
	}
}

int main() {
	int i;

	/* Init value of i */
	i = 1;
	printf("i => %d\n", i);

	/* Override value of i */
	i = 2;
	printf("i => %d\n", i);

	some_function();
	printf("SOME UGLY HACK\n", i);
	test();

	return 0;
}
