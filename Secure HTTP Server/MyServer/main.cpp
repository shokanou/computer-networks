// Description:
//
//    This sample illustrates how to develop a simple echo server Winsock
//    application using the WSAAsyncSelect() I/O model. This sample is
//    implemented as a console-style application (to reduce the programming
//    complexity of writing a real Windows application) and simply prints
//    messages when connections are established and removed from the server.
//    The application listens for TCP connections on port 5150 and accepts them
//    as they arrive. When this application receives data from a client, it
//    simply echoes the data back in it's original form until the client
//            closes the connection.
//
//    Since the WSAAsyncSelect I/O model requires an application to manage
//    window messages when network event occur, this application creates
//    a window for the I/O model only. The window stays hidden during the
//    entire execution of this application.
//
//    Note: There are no command line options for this sample.
//
// Discard unnecessary/unused headers
#define WIN32_LEAN_AND_MEAN
// Take note that windows.h already contained winsock.h! And by
// putting winsock2.h first, it will block the winsock.h re-inclusion
// Link to ws2_32.lib


#include <stdio.h>
#include <conio.h>
#include <winsock2.h>
#include <windows.h>
#include <ctime>
#include <iostream>
#include <fstream>
#include <regex>
#include <sstream>
#include "Secret.h"

using namespace std;

int PORT = 81;
#define DATA_BUFSIZE 8192

#pragma comment(lib,"ws2_32.lib")

// typedef definition
typedef struct _SOCKET_INFORMATION {
	BOOL RecvPosted;
	CHAR RecBuffer[DATA_BUFSIZE];
	CHAR SendBuffer[DATA_BUFSIZE];
	WSABUF DataBuf;
	SOCKET Socket;
	DWORD BytesSEND;
	DWORD BytesRECV;
	CHAR IPADDRESS[50];
	u_short PORTNUM;
	struct _SOCKET_INFORMATION *Next;
} SOCKET_INFORMATION, *LPSOCKET_INFORMATION;

#define WM_SOCKET (WM_USER + 1)

// Prototypes
void CreateSocketInformation(SOCKET s);
LPSOCKET_INFORMATION GetSocketInformation(SOCKET s);
void FreeSocketInformation(SOCKET s);
HWND MakeWorkerWindow(void);
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
bool startsWith(const char *pre, const char *str);
string regexStr(string &s, string &reg);
string PostRequest(char* ip, int port);
// Global var
LPSOCKET_INFORMATION SocketInfoList;

int main(int argc, char **argv)
{
	MSG msg;
	DWORD Ret;
	SOCKET Listen;
	SOCKADDR_IN InternetAddr;
	HWND Window;
	WSADATA wsaData;

	if (argc == 1) {

	}
	else {
		PORT = atoi(argv[1]);
	}


	//建立窗口
	if ((Window = MakeWorkerWindow()) == NULL)
	{
		printf("窗口建立失败");
		return 1;
	}
	else
		printf("窗口建立成功\n");

	// 准备服务器
	if (WSAStartup((2, 2), &wsaData) != 0)
	{
		printf("Error！： %d\n", WSAGetLastError());
		return 1;
	}
	else
		printf("成功\n");

	//创建socket
	if ((Listen = socket(PF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
	{
		printf("socket创建出错 %d\n", WSAGetLastError());
		return 1;
	}
	else
		printf("socket成功\n");

	//WSAAsyncSelect模型
	if (WSAAsyncSelect(Listen, Window, WM_SOCKET, FD_ACCEPT | FD_CLOSE) == 0)
		printf("WSAAsyncSelect()\n");
	else
		printf("WSAAsyncSelect() Error %d\n", WSAGetLastError());

	InternetAddr.sin_family = AF_INET;
	InternetAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	InternetAddr.sin_port = htons(PORT);

	//绑定
	if (bind(Listen, (PSOCKADDR)&InternetAddr, sizeof(InternetAddr)) == SOCKET_ERROR)
	{
		printf("bind() failed with error %d\n", WSAGetLastError());
		return 1;
	}
	else
		printf("bind succeed！\n");

	//监听
	if (listen(Listen, 5))
	{
		printf("监听失败 %d\n", WSAGetLastError());
		return 1;
	}
	else
		printf("监听成功,监听端口%d...\n", PORT);

	// 编译并且派遣信息OAO
	while (Ret = GetMessage(&msg, NULL, 0, 0))
	{
		if (Ret == -1)
		{
			printf("\n获取信息出错！出错信息： %d\n", GetLastError());
			return 1;
		}
		else
			printf("\n信息获取成功\n");

		printf("编译中\n");
		TranslateMessage(&msg);
		printf("派遣中\n");
		DispatchMessage(&msg);
	}
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	SOCKET Accept;
	LPSOCKET_INFORMATION SocketInfo;
	DWORD RecvBytes;
	DWORD SendBytes;
	DWORD Flags;

	if (uMsg == WM_SOCKET)
	{
		if (WSAGETSELECTERROR(lParam))
		{
			printf("出错！错误信息： %d\n", WSAGETSELECTERROR(lParam));
			FreeSocketInformation(wParam);
		}
		else
		{
			printf("Socket运行中\n");
			switch (WSAGETSELECTEVENT(lParam))
			{
			case FD_ACCEPT:
				if ((Accept = accept(wParam, NULL, NULL)) == INVALID_SOCKET)
				{
					printf("Error %d\n", WSAGetLastError());
					break;
				}
				else
					printf("Accept succeed!");

				// Create a socket information structure to associate with the socket for processing I/O
				CreateSocketInformation(Accept);
				printf("Socket number %d 连接成功\n", Accept);
				WSAAsyncSelect(Accept, hwnd, WM_SOCKET, FD_READ | FD_WRITE | FD_CLOSE);
				break;
			case FD_READ:
				SocketInfo = GetSocketInformation(wParam);
				// Read data only if the receive buffer is empty
				if (SocketInfo->BytesRECV != 0)
				{
					SocketInfo->RecvPosted = TRUE;
					return 0;
				}
				else
				{
					SocketInfo->DataBuf.buf = SocketInfo->RecBuffer;
					SocketInfo->DataBuf.len = DATA_BUFSIZE;

					Flags = 0;
					if (WSARecv(SocketInfo->Socket, &(SocketInfo->DataBuf), 1, &RecvBytes,
						&Flags, NULL, NULL) == SOCKET_ERROR)
					{
						if (WSAGetLastError() != WSAEWOULDBLOCK)
						{
							printf("WSARecv() 错误信息 %d\n", WSAGetLastError());
							FreeSocketInformation(wParam);
							return 0;
						}
					}
					else // No error so update the byte count
					{
						//print request
						printf("WSARecv() \n");
						printf("receive:\n %s \n", SocketInfo->DataBuf.buf);

						//put into file
						ofstream ofile;
						ofile.open("request.txt");
						ofile << SocketInfo->DataBuf.buf << endl;
						ofile.close();

						//if GET request
						if (startsWith(SocketInfo->DataBuf.buf, "GET")) {
							printf("接收到get请求");

							string s(SocketInfo->DataBuf.buf);
							istringstream is(s);

							//find router
							string router;
							is >> router;
							is >> router;

							if (router.compare("/") == 0) {
								ifstream ifile("./test.html");
								string contents((istreambuf_iterator<char>(ifile)), istreambuf_iterator<char>());

								char header[500];
								sprintf(header,
									"HTTP/1.1 200 Ok\r\n"
									"Connection: close\r\n"
									"Content-type: text/html\r\n"
									"Content-Length: %d\r\n"
									"\r\n",
									contents.size()
									);

								sprintf(SocketInfo->SendBuffer, header);
								strcat(SocketInfo->SendBuffer, contents.c_str());

								SocketInfo->BytesSEND = strlen(header) + contents.size();
							}
							else {
								char *header =
									"HTTP/1.1 404 Not Found\r\n"
									"Connection: close\r\n"
									"Content-type: text/html\r\n"
									"\r\n"
									"<h1> Not Found ! </h1>"
									"\r\n";
								sprintf(SocketInfo->SendBuffer,
									header);
								SocketInfo->BytesSEND = strlen(header);
							}

						}

						//if POST request
						if (startsWith(SocketInfo->DataBuf.buf, "POST")) {
							printf("接收到post请求");

							string s(SocketInfo->DataBuf.buf);
							istringstream is(s);

							//find router
							string router;
							is >> router;
							is >> router;

							//router /
							if (router.compare("/") == 0 || router.compare("/test1") == 0) {
								//find boundary
								string boundaryReg = regexStr(s, string("boundary.*"));
								if (!boundaryReg.empty()) {
									string boundary = "--" + regexStr(s, string("boundary.*")).substr(9) + "\r";
									//get form data
									string line;
									string fileName;
									while (getline(is, line)) {
										if (line.compare(boundary) == 0)
										{
											getline(is, line);
											string name = regexStr(line, string("name=[^\r]+")).substr(5);
											if (name.compare("\"filename\"") == 0)
											{
												getline(is, line);
												getline(is, line);
												fileName = line.substr(0, line.size() - 1);
												cout << "request file name: " << fileName << endl;
											}
										}
									}
								}

								//post to another server
								int desPort;
								if (PORT == 80) desPort = 81;
								else desPort = 80;

								string response = PostRequest("127.0.0.1", desPort);


								cout << endl << " ’收到：" << response << endl;

								int idx = response.find("===");

								string h = response.substr(0, idx);

								int idx2 = response.find("===", idx + 3);

								string content = response.substr(idx + 3, idx2);

								string sign = response.substr(idx2 + 3, response.size());


								cout << endl << "签名 " << sign << endl;
								cout << endl << "内容" << content << endl;


								//验证一下签名
								Secret::Decode_RES(sign);

								cout << endl << "解密的签名: " << sign << endl;

								bool ifSign;
								if (PORT == 80)
									ifSign = (sign.compare("server2") == 0);
								else
									ifSign = (sign.compare("server1") == 0);

								char header[500];
								if (ifSign)
								{
									//解密
									char body[8000] = { '\0' };
									memcpy(body, content.c_str(), sizeof(char)*content.size());

									char* sec = "mipami";
									Secret::deal_des((byte*)body, (byte*)sec, -1);

									cout << endl << "解密的内容: " << body << endl;

									sprintf(header, h.c_str());

									sprintf(SocketInfo->SendBuffer, header);
									strcat(SocketInfo->SendBuffer, body);
									SocketInfo->BytesSEND = strlen(header) + strlen(body);
								}
								else{
									sprintf(header,
										"HTTP/1.1 404 Not Found\r\n"
										"Connection: close\r\n"
										"Content-type: text/html\r\n"
										"Content-Length: 22\r\n"
										"\r\n"
										"<h1> Not Found ! </h1>"
										"\r\n");

									sprintf(SocketInfo->SendBuffer, header);
									SocketInfo->BytesSEND = strlen(header) + 1;
								}
							}
							//第二页的路由
							else
							if (router.compare("/page2") == 0){
								int size = 0;

								ifstream ifile("./page2.html");
								string contents((istreambuf_iterator<char>(ifile)), istreambuf_iterator<char>());
								size = contents.size();

								char res[8000] = { '\0' };
								strcpy(res, contents.c_str());

								cout << endl << "Œ¥º”√‹ƒ⁄»›£∫" << res << endl;

                        
								char* sec = "liming";
								Secret::deal_des((byte *)res, (byte*)sec, 1);

								char tmp[1000] = { '\0' };
								string body(tmp);
								body.replace(0, 1024, res, 1024);

								cout << endl << " º”√‹res: " << body << endl;

								
								string sign;
								if (PORT == 80)
									sign = "server1";
								else
									sign = "server2";
								Secret::Encode_RES(sign);

								string h = "HTTP/1.1 200 OK\r\n"
									"Content-length: ";
								h += to_string(size);
								h += "\r\n";
								h += "Content-type: text/html\r\n\r\n===";

								body = h + body;

								body += "===";
								body += sign;

								cout << endl << "加密签名: " << sign << endl;

								cout << endl << "待传输主体: " << body << endl;

								memcpy(SocketInfo->SendBuffer, body.c_str(), body.size()*sizeof(char));
								SocketInfo->BytesSEND = body.size();

								cout << endl << "传输内容：" << body << endl;

								cout << endl << "传输长度：" << SocketInfo->BytesSEND << endl;
							}
							else {
								char header[500];
								sprintf(header,
									"HTTP/1.1 404 Not Found\r\n"
									"\r\n"
									);
								sprintf(SocketInfo->SendBuffer, header);
								SocketInfo->BytesSEND = strlen(header);
							}
						}

					}
				}
				// DO NOT BREAK HERE SINCE WE GOT A SUCCESSFUL RECV. Go ahead
				// and begin writing data to the client
			case FD_WRITE:
				SocketInfo = GetSocketInformation(wParam);
				if (SocketInfo->BytesSEND > 0)
				{
					SocketInfo->DataBuf.buf = SocketInfo->SendBuffer;
					SocketInfo->DataBuf.len = SocketInfo->BytesSEND;

					if (WSASend(SocketInfo->Socket, &(SocketInfo->DataBuf), 1, &SendBytes, 0,
						NULL, NULL) == SOCKET_ERROR)
					{
						if (WSAGetLastError() != WSAEWOULDBLOCK)
						{
							printf("WSASend() failed with error %d\n", WSAGetLastError());
							FreeSocketInformation(wParam);
							return 0;
						}
					}
					else // No error so update the byte count
					{
						printf("WSASend()\n");
						cout << "send :"<< endl << SocketInfo->DataBuf.buf << endl;
						SocketInfo->BytesSEND = 0;
					}
				}

				else
				{
					SocketInfo->BytesSEND = 0;
					SocketInfo->BytesRECV = 0;
					// If a RECV occurred during our SENDs then we need to post an FD_READ notification on the socket
					if (SocketInfo->RecvPosted == TRUE)
					{
						SocketInfo->RecvPosted = FALSE;
						PostMessage(hwnd, WM_SOCKET, wParam, FD_READ);
					}
				}
				break;
			case FD_CLOSE:
				printf("Closing socket %d\n", wParam);
				FreeSocketInformation(wParam);
				break;
			}
		}
		return 0;
	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void CreateSocketInformation(SOCKET s)
{
	LPSOCKET_INFORMATION SI;

	if ((SI = (LPSOCKET_INFORMATION)GlobalAlloc(GPTR, sizeof(SOCKET_INFORMATION))) == NULL)
	{
		printf("GlobalAlloc() failed with error %d\n", GetLastError());
		return;
	}
	else
		printf("GlobalAlloc() for SOCKET_INFORMATION is OK!\n");

	// Prepare SocketInfo structure for use
	SI->Socket = s;
	SI->RecvPosted = FALSE;
	SI->BytesSEND = 0;
	SI->BytesRECV = 0;
	SOCKADDR client_info;
	int size = sizeof(client_info);
	getpeername(s, &client_info, &size);
	sprintf(SI->IPADDRESS, "%s", client_info.sa_data);
	sprintf(SI->IPADDRESS, "%s", inet_ntoa(((SOCKADDR_IN *)&client_info)->sin_addr));
	SI->PORTNUM = ntohs(((SOCKADDR_IN *)&client_info)->sin_port);
	printf("ip stored: %s:%d \n", SI->IPADDRESS, SI->PORTNUM);

	SI->Next = SocketInfoList;
	SocketInfoList = SI;
}

LPSOCKET_INFORMATION GetSocketInformation(SOCKET s)
{
	SOCKET_INFORMATION *SI = SocketInfoList;

	while (SI)
	{
		if (SI->Socket == s)
			return SI;

		SI = SI->Next;
	}

	return NULL;
}

void FreeSocketInformation(SOCKET s)
{
	SOCKET_INFORMATION *SI = SocketInfoList;
	SOCKET_INFORMATION *PrevSI = NULL;

	while (SI)
	{
		if (SI->Socket == s)
		{
			if (PrevSI)
				PrevSI->Next = SI->Next;
			else
				SocketInfoList = SI->Next;

			closesocket(SI->Socket);
			GlobalFree(SI);
			return;
		}

		PrevSI = SI;
		SI = SI->Next;
	}
}

HWND MakeWorkerWindow(void)
{
	WNDCLASS wndclass;
	CHAR *ProviderClass = "AsyncSelect";
	HWND Window;

	wndclass.style = CS_HREDRAW | CS_VREDRAW;
	wndclass.lpfnWndProc = (WNDPROC)WindowProc;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.hInstance = NULL;
	wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndclass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wndclass.lpszMenuName = NULL;
	wndclass.lpszClassName = (LPCWSTR)ProviderClass;

	if (RegisterClass(&wndclass) == 0)
	{
		printf("RegisterClass() failed with error %d\n", GetLastError());
		return NULL;
	}
	else
		printf("RegisterClass()\n");

	// 创建窗口
	if ((Window = CreateWindow(
		(LPCWSTR)ProviderClass,
		L"",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		NULL,
		NULL,
		NULL,
		NULL)) == NULL)
	{
		printf("CreateWindow() failed with error %d\n", GetLastError());
		return NULL;
	}
	else
		printf("CreateWindow() is OK!\n");

	return Window;
}

bool startsWith(const char *pre, const char *str)
{
	size_t lenpre = strlen(pre),
		lenstr = strlen(str);
	bool i = lenstr > lenpre ? false : strncmp(pre, str, lenstr) == 0;
	return i;
}

string regexStr(string &s, string &reg) {
	std::regex word_regex(reg);
	string result;
	auto words_begin =
		std::sregex_iterator(s.begin(), s.end(), word_regex);
	auto words_end = std::sregex_iterator();

	std::cout << "Found "
		<< std::distance(words_begin, words_end)
		<< " words\n";

	for (std::sregex_iterator i = words_begin; i != words_end; ++i) {
		std::smatch match = *i;
		result = match.str();
		std::cout << result << endl;
	}

	return result;
}

string PostRequest(char* ip, int port) {
	SOCKET fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (fd < 0)     throw;
	SOCKADDR_IN service;
	service.sin_family = AF_INET;
	service.sin_port = htons(port);
	LPHOSTENT host = gethostbyname(ip);
	if (!host)          throw;
	service.sin_addr = *((LPIN_ADDR)*host->h_addr_list);
	if (connect(fd, (SOCKADDR *)&service, sizeof(service)) < 0)     throw;
	char header[500];
	sprintf(
		header,
		"POST /page2 HTTP/1.1\r\n"
		"Host: %s\r\n"
		"User-Agent: Mozilla Firefox/4.0\r\n"
		"Content-Length: 0\r\n"
		"Accept-Charset: utf-8\r\n\r\n",
		ip
		);

	send(fd, header, strlen(header), 0);

	char rec[2000] = { '\0' };

	recv(fd, rec, 2000, 0);
	closesocket(fd);

	string result = " ";
	result.replace(0, 1350, rec, 1350);

	return result;
};