#include <stdio.h>
#include <WinSock2.h>
#include <stdlib.h>
#include <direct.h>
#include <string.h>

#pragma warning(disable:4996)
#pragma comment(lib, "ws2_32.lib")

#define MAX_THREADS 2
#define PORT 8000

FILE* fp;

void Error_Handling(char* msg)
{
	printf("%s\n", msg);
	exit(1);
}

int Add(char *filename)
{
	char buf[256];
	fp = fopen(filename, "a+");
	printf("Insert Data : ");
	scanf(" %[^\n]", &buf);
	fprintf(fp, "%s\n", buf);
	fclose(fp);
	return 0;
}

int Delete(char* filename)
{
	char buf[80];
	char temp[80];
	fp = fopen(filename, "rt");
	FILE* Temp = fopen("temp.txt", "wt");
	printf("Insert Data : ");
	scanf(" %[^\n]", &buf);
	while (!feof(fp))
	{
		fscanf(fp, "%s\n", &temp);
		if (strcmp(buf, temp) != 0)
			fprintf(Temp, "%s\n", temp);
	}
	fclose(fp);
	fclose(Temp);
	//DeleteFile(filename);
	//MoveFile("temp.txt", filename);
	return 0;
}

int Show(char *filename)
{
	char buf[80];
	int count = 1;
	fp = fopen(filename, "rt");
	while(!feof(fp))
	{
		fscanf(fp, "%s\n", &buf);
		printf("%d.%s\n", count, buf);
		++count;
	}
	return 0;
}

DWORD WINAPI Modify(LPVOID lpParam)
{
	int sel1, sel2;
	while (1)
	{
		printf("1.Add\n2.Delete\n3.Show\nInput ->");
		scanf("%d", &sel1);
		printf("1.ClassID\n2.ProcessName\n3.Hash\nInput ->");
		scanf("%d", &sel2);
		if (sel1 == 1)
		{
			if (sel2 == 1)
				Add("ClassID.txt");
			else if (sel2 == 2)
				Add("ProcessName.txt");
			else if (sel2 == 3)
				Add("Hash.txt");
		}
		else if (sel1 == 2)
		{
			if (sel2 == 1)
				Delete("ClassID.txt");
			else if (sel2 == 2)
				Delete("ProcessName.txt");
			else if (sel2 == 3)
				Delete("Hash.txt");
		}
		else if (sel1 == 3)
		{
			if (sel2 == 1)
				Show("ClassID.txt");
			else if (sel2 == 2)
				Show("ProcessName.txt");
			else if (sel2 == 3)
				Show("Hash.txt");
		}
	}
}

DWORD WINAPI Update(LPVOID lpParam)
{
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);
	char senddata[80];

	SOCKET Serv_sock, Clnt_sock;
	Serv_sock = socket(AF_INET, SOCK_STREAM, 0);

	if (Serv_sock == INVALID_SOCKET)
		Error_Handling("socket() Error!");

	SOCKADDR_IN Serv_addr, Clnt_addr;
	Serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	Serv_addr.sin_port = htons(PORT);
	Serv_addr.sin_family = AF_INET;

	if (bind(Serv_sock, (SOCKADDR*)&Serv_addr, sizeof(SOCKADDR)) == SOCKET_ERROR)
		Error_Handling("bind() Error!");

	if (listen(Serv_sock, 5) == SOCKET_ERROR)
		Error_Handling("listen() Error!");

	int size = sizeof(SOCKADDR);
	while (1)
	{
		if ((Clnt_sock = accept(Serv_sock, (SOCKADDR*)&Clnt_addr, &size)) == INVALID_SOCKET)
			Error_Handling("accept() Error");

		for (int i = 0; i < 3; i++)
		{
			if (i == 0)
				fp = fopen("ClassID.txt", "rb");
			else if (i == 1)
				fp = fopen("ProcessName.txt", "rb");
			else if (i == 2)
				fp = fopen("Hash.txt", "rb");

			int clntlensize = 0;
			char *clntlen = NULL;
			recv(Clnt_sock, clntlensize, sizeof(clntlensize), 0);
			recv(Clnt_sock, clntlen, clntlensize, 0);

			char* servlen = NULL;
			fseek(fp, 0, SEEK_END);
			itoa(ftell(fp), servlen, 10);

			int clntcntsize = 0;
			char *clntcnt = NULL;
			recv(Clnt_sock, clntcntsize, sizeof(clntcntsize), 0);
			recv(Clnt_sock, clntcnt, clntcntsize, 0);

			int cnt = 0;
			char temp[80];
			char* servcount = NULL;
			fseek(fp, 0, SEEK_SET);
			while (!feof(fp))
			{
				fscanf(fp, "%s\n", &temp);
				++cnt;
			}
			itoa(cnt, servcount, 10);

			if (strcmp(clntlen, servlen) == 0 || strcmp(clntcnt, servcount) == 0)
				send(Clnt_sock, "OK", 3, 0);
			else
			{
				fseek(fp, 0, SEEK_SET);
				while (!feof(fp))
				{
					fscanf(fp, "%s\n", &senddata);
					send(Clnt_sock, senddata, strlen(senddata), 0);
				}
			}
		}
		closesocket(Clnt_sock);
		closesocket(Serv_sock);
	}
}

int main()
{
	DWORD dwThreadIDArray[MAX_THREADS];
	HANDLE hThreadArray[MAX_THREADS];

	hThreadArray[0] = CreateThread(NULL, 0, Modify, NULL, 0, &dwThreadIDArray[0]);
	if (hThreadArray[0] == NULL)
		Error_Handling("CreateThread() Error!");

	hThreadArray[1] = CreateThread(NULL, 0, Update, NULL, 0, &dwThreadIDArray[1]);
	if (hThreadArray[1] == NULL)
		Error_Handling("CreateThread() Error!");

	WaitForMultipleObjects(MAX_THREADS, hThreadArray, TRUE, INFINITE);
	for (int i = 0; i < MAX_THREADS; i++)
		CloseHandle(hThreadArray[i]);
}