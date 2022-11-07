#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>

#define MAX_BUFFER 100

int main()
{
	int toServer, fromServer;
	char command[MAX_BUFFER];
	mknod("client2server.txt", S_IFIFO | 0666, 0);
	mknod("server2client.txt", S_IFIFO | 0666, 0);


	toServer = open("client2server.txt", O_WRONLY);
	if (toServer == -1)
	{
		perror("Could not write to the MyFifo_Client file \n");
		exit(0);
	}

	fromServer = open("server2client.txt", O_RDONLY);

	if (fromServer == -1)
	{
		perror("Could not write to the MyFifo_Server file \n");
		exit(0);
	}

	while (1)
	{
        fgets(command, MAX_BUFFER, stdin);

        write(toServer, command, strlen(command));

        char buffReceive[MAX_BUFFER];
        int msgReceived = 0;

        msgReceived = read(fromServer, buffReceive, MAX_BUFFER);
        buffReceive[msgReceived] = '\0';

        printf("%s\n", buffReceive);

        if (strstr(command, "quit") != 0)
        {
            // sleep(10);
            break;
        }
	}
	close(toServer);
	close(fromServer);
	return 0;
}
