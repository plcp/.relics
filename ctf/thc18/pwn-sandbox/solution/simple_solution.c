// Compile with gcc simple_solution.c -o simple_solution
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main() { return 0; };

static void preinit(int argc, char **argv, char **envp) {
    char c;
    FILE *file;
    file = fopen("flag", "r");
    while ((c = getc(file)) != EOF) {
	write(1, &c, 1);
    }
    write(1, "\n", 1);
    fclose(file);
}

__attribute__((section(".preinit_array"),
	       used)) static typeof(preinit) *init_p = preinit;
