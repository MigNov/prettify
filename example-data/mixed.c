#include <stdio.h>

/* This function is written using 8 spaces */
int some_function(void)
{
        int i;

        for (i = 0; i < 10; i++) {
		printf("XXX: %d\n", i);
        }
}

/* This function is written using tabs */
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

/* This function is mixed */
int main() {
	int i;

	i = 1;

        printf("i => %d\n", i);
        i = 2;
	printf("i => %d\n", i);

    some_function();
    printf("SOME UGLY HACK\n", i);
	test();

	return 0;
}
