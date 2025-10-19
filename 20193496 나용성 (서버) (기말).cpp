#pragma comment(lib, "ws2_32.lib")

#include <WinSock2.h>
#include <WS2tcpip.h>
#include <iostream>

using std::cout;
using std::endl;

const int MAXBUF = 80;

SOCKET sarr[10];
WSAEVENT earr[10];
int noc; //number of connections

void Close_Session(int);
void Add_Session(int);
void Echo_Serv(int);

int main()
{
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);

    SOCKET lsock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (lsock == INVALID_SOCKET)
        return -1;

    SOCKADDR_IN saddr;
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(8000);
    saddr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(lsock, (SOCKADDR*)&saddr, sizeof(saddr)))
        return -1;

    //lsock, event object
    sarr[noc] = lsock;
    earr[noc] = WSACreateEvent();
    if (WSAEventSelect(sarr[noc], earr[noc], FD_ACCEPT))
    {
        cout << "WSAEventSelect listen socket case" << endl;
        return -1;
    }
    noc++;


    if (listen(lsock, SOMAXCONN))
        return -1;

    DWORD retevent, oevent;
    int idx;
    WSANETWORKEVENTS event;

    while (1)
    {
        retevent = WSAWaitForMultipleEvents(noc, earr, false, WSA_INFINITE, false);

        if (retevent == WSA_WAIT_FAILED)
        {
            cout << "WSAWFME error" << endl;
            continue;
        }

        idx = retevent - WSA_WAIT_EVENT_0;

        for (int i = idx; i < noc; i++)
        {
            oevent = WSAWaitForMultipleEvents(1, &earr[i], true, 0, false);

            if (oevent == WSA_WAIT_TIMEOUT) //non-signaled
                continue;

            //N-E check function
            if (WSAEnumNetworkEvents(sarr[i], earr[i], &event))
            {
                cout << "WSAEnumNetworkEvents error case " << endl;
                continue;
            }

            if (event.lNetworkEvents & FD_ACCEPT)
            {
                //error check
                if (event.iErrorCode[FD_ACCEPT_BIT])
                {
                    cout << "accept error case" << endl;
                    return -1;
                }

                Add_Session(i);
                //accept
            }
            else if (event.lNetworkEvents & FD_READ)
            {
                //error check
                if (event.iErrorCode[FD_READ_BIT])
                {
                    cout << "read error case" << endl;
                    Close_Session(i);
                    continue;
                }

                Echo_Serv(i);
                //recv
            }
            else if (event.lNetworkEvents & FD_CLOSE)
            {
                //error check
                if (event.iErrorCode[FD_CLOSE_BIT])
                {
                    cout << "close error case" << endl;
                    Close_Session(i);
                    continue;
                }

                cout << "Normal close case" << endl;
                Close_Session(i);
                //closesocket
            }
        }//for

    }

    closesocket(lsock);
    WSACleanup();
    return 0;
}

void Add_Session(int idx)
{
    SOCKADDR_IN caddr;
    int namelen = sizeof(caddr);

    SOCKET csock = accept(sarr[idx], (SOCKADDR*)&caddr, &namelen);

    //csock, event object
    sarr[noc] = csock;
    earr[noc] = WSACreateEvent();
    WSAEventSelect(sarr[noc], earr[noc], FD_READ | FD_CLOSE);
    noc++;
}

void Close_Session(int idx)
{
    SOCKET s = sarr[idx];
    WSAEVENT e = earr[idx];

    if (idx != noc - 1)
    {
        sarr[idx] = sarr[noc - 1];
        earr[idx] = earr[noc - 1];
    }

    closesocket(s);
    WSACloseEvent(e);
    noc--;
}

void Echo_Serv(int idx)
{
    char buf[MAXBUF];
    int recvlen = recv(sarr[idx], buf, MAXBUF - 1, 0);
    buf[recvlen] = '\0';
    cout << "From client " << buf;
    send(sarr[idx], buf, recvlen, 0);

    // 추가한 부분
    for (int i = 1; i < noc; i++)
    {
        if (i != idx)
        {
            SOCKET murtisock = sarr[i];
            send(murtisock, buf, recvlen, 0);
        }
    }
}
