#include <unistd.h>

int main() {
    setreuid(0, 0);
    execl("/root/run-moche.sh", "/root/run-moche.sh", NULL);

    return 0;
}
