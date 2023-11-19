#include "types.h"
#include "stat.h"
#include "user.h"

int
main(int argc, char **argv)
{
	int num = getnp();
	int pid ;

	pid = getpid();

	printf(1, "current number of running processes is %d \n", num);
	printf(1, "current pid is %d \n", pid);

	exit();
}
