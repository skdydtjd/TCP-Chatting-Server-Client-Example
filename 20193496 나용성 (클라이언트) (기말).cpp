#pragma comment (lib,"ws2_32.lib")
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <iostream>
#include <process.h>

using std::cout;
using std::endl;

void err_display(const char*);
void err_quit(const char*);
void Pton(const char*, PVOID);
unsigned _stdcall DisMes(LPVOID);

#define MAXBUF 80

int main()
{
	WSADATA wsa;

	if (WSAStartup(MAKEWORD(2, 2), &wsa)) // 윈속 초기화
		err_quit("WSAStartup");

	SOCKET s_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (s_sock == INVALID_SOCKET)
		err_quit("socket error");

	SOCKADDR_IN saddr;
	saddr.sin_family = AF_INET;
	saddr.sin_port = htons(8000);
	Pton("127.0.0.1", &saddr.sin_addr);

	if (connect(s_sock, (SOCKADDR*)&saddr, sizeof(saddr)))
		err_quit("connect error");

	_beginthreadex(NULL, 0, &DisMes, (LPVOID)s_sock, 0, NULL);

	char buf[MAXBUF];

	while (1)
	{
		if (!fgets(buf, MAXBUF - 1, stdin))
			break;

		send(s_sock, buf, strlen(buf), 0);
	}
	
	closesocket(s_sock);
	WSACleanup(); //윈속 종료
	cout << "Main: end case" << endl;

	return 0;
}

unsigned  _stdcall DisMes(LPVOID arg)
{
	SOCKET sock = (SOCKET)arg;

	int recvlen;
	char buf[MAXBUF];

	while (1)
	{
		recvlen = recv(sock, buf, MAXBUF - 1, 0);

		if (recvlen == SOCKET_ERROR || recvlen == 0)
			break;

		buf[recvlen] = '\0';
		cout << "From server " << buf << endl;

	}

	return 0;
}

void err_display(const char* mes)
{
	LPVOID buf; // formatmessage가 출력할 에러 문자열을 저장한 메모리의 주소

	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL,
		WSAGetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&buf, 0, NULL);

	cout << "[" << mes << "] " << (LPSTR)buf;

	LocalFree(buf);
}

void err_quit(const char* mes)
{
	LPVOID buf; // formatmessage가 출력할 에러 문자열을 저장한 메모리의 주소

	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL,
		WSAGetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&buf, 0, NULL);

	cout << "[" << mes << "] " << (LPSTR)buf;

	LocalFree(buf);
	exit(1);
}

void Pton(const char* ip, PVOID addr)
{
	if (InetPton(AF_INET, ip, addr) != 1)
		err_quit("InetPton error");
}
