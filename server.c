#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <utmp.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <sys/wait.h>
#include <time.h>

#define READ 0
#define WRITE 1
#define MAX_BUFFER 500
#define PARENT 1
#define CHILD 0

int main()
{

	int fromClient, toClient;
	mknod("client2server.txt", S_IFIFO | 0666, 0);
	mknod("server2client.txt", S_IFIFO | 0666, 0);

	fromClient = open("client2server.txt", O_RDONLY);
	if (fromClient == -1)
	{
		perror("Could not write to the MyFifo_Client file \n");
		exit(0);
	}

	toClient = open("server2client.txt", O_WRONLY);
	if (toClient == -1)
	{
		perror("Could not write to the MyFifo_Server file \n");
		exit(0);
	}
	char buffer[5000000];
	int command = 0;
	int loginFlag = 0;
	while (1)
	{

		command = read(fromClient, buffer, MAX_BUFFER);
		buffer[command] = '\0';

		// write(1,buffer,strlen(buffer));
		if (strstr(buffer, "login") != 0)
		{
			// printf("%s\n", buffer);

			// char username[MAX_BUFFER];
			// strcpy(username, buffer + 8);
			//  printf("%s", username);

			pid_t pid;
			pid = fork();

			if (pid == 0) // copil
			{
				char getName[MAX_BUFFER];
				strcpy(getName, buffer + 8);
				// getName[strlen(getName)] = '\0';
				//  write(toClient, getName, strlen(getName));

				int userFD;
				userFD = open("users.txt", O_RDONLY);

				char userName[MAX_BUFFER];
				userName[0] = '\0';

				int userLength;
				userLength = read(userFD, userName, sizeof(userName));
				userName[userLength] = '\0';

				char *name = strtok(userName, ",");
				// write(toClient, name, sizeof(name));
				// write(toClient,userName,strlen(userName));
				while (name)
				{
					// write(toClient, name, sizeof(name));
					if (strncmp(name, getName, 4) == 0)
					{
						// printf("%s\n", name);
						exit(1);
					}
					name = strtok(NULL, ",");
				}
				exit(2);
				close(userFD);
			}
			else // parinte
			{
				int signal;

				waitpid(pid, &signal, 0);

				if (WEXITSTATUS(signal) == 1)
				{
					if (loginFlag == 0)
					{
						loginFlag = 1;
						write(toClient, "You were successfully logged in!", 32);
					}
					else
					{
						write(toClient, "You are already connected", 26);
					}
				}
				else if (WEXITSTATUS(signal) == 2)
				{
					write(toClient, "Wrong username", 15);
				}
				// wait(NULL);
			}
		}
		else if (strncmp(buffer, "logout", 6) == 0)
		{
			pid_t child;
			child = fork();
			// write(toClient, "ceva", 4);

			if (child == 0)
			{
				if (loginFlag == 1)
				{
					exit(1);
				}
				exit(2);
			}
			else
			{
				int signal2;
				waitpid(child, &signal2, 0);

				if (WEXITSTATUS(signal2) == 1)
				{
					loginFlag = 0;
					write(toClient, "You have been disconnected!", 28);
				}
				else if (WEXITSTATUS(signal2) == 2)
				{
					write(toClient, "You are not connected!", 23);
				}
			}
		}
		else if (strncmp(buffer, "quit", 4) == 0)
		{
			// write(toClient, "Please wait", 8);
			write(toClient, "Server has been succesfully shut down", 38);
			break;
		}
		else if (strstr(buffer, "get-logged-users") != 0)
		{
			int socket[2];
			pid_t child2;

			struct utmp *user;

			if (socketpair(AF_UNIX, SOCK_STREAM, 0, socket) < 0)
			{
				perror("socketpair");
				exit(EXIT_FAILURE);
			}
			child2 = fork();

			if (child2 > 0) // parent
			{
				// int signal3;
				// waitpid(child2, &signal3, 0);
				// if (WEXITSTATUS(signal3) == 10)
				//{
				char buffparinte[MAX_BUFFER] = "";
				// printf("%i", loginFlag);
				if (loginFlag == 1)
				{
					close(socket[CHILD]);
					// int signal3;
					// wait(&signal3);
					read(socket[PARENT], buffparinte, sizeof(buffparinte));

					write(toClient, buffparinte, sizeof(buffparinte));

					close(socket[PARENT]);
				}
				else
				{
					write(toClient, "You are not logged in!", 23);
				}
			}
			else if (child2 == 0)
			{
				close(socket[PARENT]);
				char buffLoggedUsers[MAX_BUFFER] = "";
				char info[MAX_BUFFER] = "";
				char clock[MAX_BUFFER];
				time_t tmp;
				setutent();
				user = getutent();
				// int parentLength = read(socket[CHILD], buffLoggedUsers, sizeof(buffLoggedUsers));
				// buffLoggedUsers[parentLength] = '\0';
				while (user)
				{
					if (user->ut_type == USER_PROCESS)
					{
						strncpy(buffLoggedUsers, user->ut_user, 32);
						buffLoggedUsers[32] = '\0';
						// strcat(info,"\0");
						strcat(info, buffLoggedUsers);
						strcat(info, "\n");

						strncpy(buffLoggedUsers, user->ut_host, 32);
						buffLoggedUsers[32] = '\0';
						strcat(info, buffLoggedUsers);
						strcat(info, "\n");
						tmp = user->ut_tv.tv_sec;
						sprintf(clock, "%25s", ctime(&tmp));
						strcat(info, clock);
						//strcat(info, "\n");
					}
					user = getutent();
				}
				write(socket[CHILD], info, sizeof(info));
				close(socket[CHILD]);
				// exit(10);
			}
		}
		else if (strstr(buffer, "get-proc-info: "))
		{
			// write(toClient,"To do",5);
			if (loginFlag == 1)
			{
				int childParent[2];
				pid_t copcil;
				char getId[MAX_BUFFER];
				strcpy(getId, buffer + 15);
				getId[strlen(getId)] = '\0';

				/// proc/<pid>/status
				char sursa[50] = "";
				strcat(sursa, "/proc/");
				strncat(sursa, getId,strlen(getId)-1);
				strcat(sursa, "/status");

				pipe(childParent);
				copcil = fork();
				if (copcil == 0)
				{
					close(childParent[READ]);
					int procFD = open(sursa, O_RDONLY);
					//printf("%s",sursa);

					int count;
					//do
					//{
						char fis[5000];
						count = read(procFD, fis, 5000);
						fis[count] = '\0';

						char *line=strtok(fis,"\n");

						while(line)
						{
							if(strstr(line,"Name")!=0||strstr(line,"State")!=0||strstr(line,"Ppid")!=0||strstr(line,"VmSize")!=0||strstr(line,"Uid")!=0)
							{
								write(childParent[WRITE],line,strlen(line));
								write(childParent[WRITE],"\n",1);

							}
							line=strtok(NULL,"\n");
						}


					//} while (count);
					close(childParent[WRITE]);
				}
				else if(copcil>0)
				{
					close(childParent[WRITE]);
					char buffProc[1000];

					int lenBuff=read(childParent[READ],buffProc,sizeof(buffProc));
					//printf("%i",lenBuff);
					buffProc[lenBuff]='\0';

					close(childParent[READ]);
					write(toClient,buffProc,strlen(buffProc));
				}
			}
			else
			{
				write(toClient, "You are not logged in!", 23);
			}
		}
		else
		{
			write(toClient, "wrong command", 14);
		}
	}
	close(toClient);
	close(fromClient);
}
