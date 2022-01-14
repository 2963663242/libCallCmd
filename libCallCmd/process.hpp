#pragma once

#include <iostream>
#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <functional>
using namespace std;
void DisplayError(const char*) {

}
wstring Str2Wstr(string str)
{
	unsigned len = str.size() * 2;// 预留字节数
	setlocale(LC_CTYPE, "");     //必须调用此函数
	wchar_t* p = new wchar_t[len];// 申请一段内存存放转换后的字符串
	mbstowcs(p, str.c_str(), len);// 转换
	std::wstring str1(p);
	delete[] p;// 释放申请的内存
	return str1;
}
int open(wstring pszCmd, std::function<void(const char* bytes, size_t n)> read_stdout)
{
	wchar_t* ExePicPar = (wchar_t *)pszCmd.c_str();//ws_utf8.c_str();
	wchar_t* MyDir = (wchar_t*)L"";
	HANDLE hChildProcess = NULL;
	HANDLE hStdIn = NULL; // Handle to parents std input.
	BOOL bRunThread = TRUE;



	HANDLE hOutputReadTmp, hOutputRead, hOutputWrite;
	HANDLE hInputWriteTmp, hInputRead, hInputWrite;
	HANDLE hErrorWrite;
	SECURITY_ATTRIBUTES sa;


	// Set up the security attributes struct.
	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
	sa.lpSecurityDescriptor = NULL;
	sa.bInheritHandle = TRUE;


	// Create the child output pipe.
	if (!CreatePipe(&hOutputReadTmp, &hOutputWrite, &sa, 0))
		DisplayError("CreatePipe");


	// Create a duplicate of the output write handle for the std error
	// write handle. This is necessary in case the child application
	// closes one of its std output handles.
	if (!DuplicateHandle(GetCurrentProcess(), hOutputWrite,
		GetCurrentProcess(), &hErrorWrite, 0,
		TRUE, DUPLICATE_SAME_ACCESS))
		DisplayError("DuplicateHandle");


	// Create the child input pipe.
	if (!CreatePipe(&hInputRead, &hInputWriteTmp, &sa, 0))
		DisplayError("CreatePipe");


	// Create new output read handle and the input write handles. Set
	// the Properties to FALSE. Otherwise, the child inherits the
	// properties and, as a result, non-closeable handles to the pipes
	// are created.
	if (!DuplicateHandle(GetCurrentProcess(), hOutputReadTmp,
		GetCurrentProcess(),
		&hOutputRead, // Address of new handle.
		0, FALSE, // Make it uninheritable.
		DUPLICATE_SAME_ACCESS))
		DisplayError("DupliateHandle");

	if (!DuplicateHandle(GetCurrentProcess(), hInputWriteTmp,
		GetCurrentProcess(),
		&hInputWrite, // Address of new handle.
		0, FALSE, // Make it uninheritable.
		DUPLICATE_SAME_ACCESS))
		DisplayError("DupliateHandle");


	// Close inheritable copies of the handles you do not want to be
	// inherited.
	if (!CloseHandle(hOutputReadTmp)) DisplayError("CloseHandle");
	if (!CloseHandle(hInputWriteTmp)) DisplayError("CloseHandle");


	//PrepAndLaunchRedirectedChild(hOutputWrite,hInputRead,hErrorWrite,ExePar);


	PROCESS_INFORMATION pi;
	STARTUPINFO si;

	// Set up the start up info struct.
	ZeroMemory(&si, sizeof(STARTUPINFO));
	si.cb = sizeof(STARTUPINFO);
	si.dwFlags = STARTF_USESTDHANDLES;
	si.hStdOutput = hOutputWrite;
	si.hStdInput = hInputRead;
	si.hStdError = hErrorWrite;

	/*wchar_t ExePar[MAX_PATH * 5] = { 0 };
	wchar_t pwd[MAX_PATH * 2] = { 0, };
	_wgetcwd(pwd, MAX_PATH * 2);
	wcscat(ExePar, pwd);
	wcscat(ExePar, MyDir);
	wcscat(ExePar, L" ");
	wcscat(ExePar, ExePicPar);*/


	int pathsize = 0;
	pathsize = wcslen(ExePicPar);
	int totalSize = ((pathsize + 1) * 2) + MAX_PATH * 2;
	wchar_t* ExePar = (wchar_t*)malloc(totalSize);
	memset(ExePar, 0, totalSize);
	wchar_t pwd[MAX_PATH * 2] = { 0, };
	_wgetcwd(pwd, MAX_PATH * 2);
	wcscat(ExePar, pwd);
	wcscat(ExePar, MyDir);
	//wcscat(ExePar, L" ");
	wcscat(ExePar, ExePicPar);

	//if (!CreateProcess(NULL,ExePar,NULL,NULL,TRUE,
	//	 CREATE_SUSPENDED|DEBUG_ONLY_THIS_PROCESS|NORMAL_PRIORITY_CLASS|CREATE_NO_WINDOW,NULL,NULL,&si,&pi))
	//	DisplayError("CreateProcess");
	if (!CreateProcess(NULL, ExePar, NULL, NULL, TRUE,
		CREATE_SUSPENDED | CREATE_NO_WINDOW, NULL, NULL, &si, &pi))
		DisplayError("CreateProcess");
	//调整进程优先级为最低
/*	if (!SetPriorityClass(pi.hProcess, IDLE_PRIORITY_CLASS))
		DisplayError("SetPriorityClass");*/
	free(ExePar);
	//MessageBox(0, ExePar, L"", 0);
	// Set global child process handle to cause threads to exit.
	hChildProcess = pi.hProcess;
	//m_hProcess=(void*)pi.hThread;
	ResumeThread(pi.hThread);
	//提前关闭句柄，有利于管道的关闭，不会卡死，但是进程控制的相关函数就不能使用
	//所以在进程关闭之后，再关闭句柄
	//关闭句柄，读取管道自动关闭，否则卡到读取管道
	// Close any unnecessary handles.
	//if (!CloseHandle(pi.hThread)) DisplayError("CloseHandle");




	// Close pipe handles (do not continue to modify the parent).
	// You need to make sure that no handles to the write end of the
	// output pipe are maintained in this process or else the pipe will
	// not close when the child process exits and the ReadFile will hang.
	if (!CloseHandle(hOutputWrite)) DisplayError("CloseHandle");
	if (!CloseHandle(hInputRead)) DisplayError("CloseHandle");
	if (!CloseHandle(hErrorWrite)) DisplayError("CloseHandle");

	const size_t buffer_size = 131072;
	string strRetTmp;
	char buff[1024] = { 0 };
	DWORD dwRead = 0;
	strRetTmp = buff;
	DWORD n;
	std::unique_ptr<char[]> buffer(new char[buffer_size]);
	char szRecvData[512] = { 0 };
	DWORD dwRecvSize = 0;
	std::string szOutputLine;
	
	while (FALSE != ReadFile(hOutputRead, szRecvData, _countof(szRecvData) - 1, &dwRecvSize, NULL))
	{
		szRecvData[dwRecvSize] = '\0';

		char* szLinesBegin = szRecvData;
		char* szLinesEnd = NULL;

		while (true)
		{
			char* szFound = szLinesBegin + strcspn(szLinesBegin, "\r\n");
			szLinesEnd = *szFound != '\0' ? szFound : NULL;

			if (NULL == szLinesEnd)
			{
				szOutputLine += szLinesBegin;
				break;
			}

			*szLinesEnd = '\0';

			if (false == szOutputLine.empty())
			{
				szOutputLine += szLinesBegin;
				read_stdout((char*)szOutputLine.c_str(), 0);
				//stateInform_s(callBack, (char*)szOutputLine.c_str());
				szOutputLine.clear();
			}
			else
			{
				if (*szLinesBegin != '\0')
				{
					read_stdout(szLinesBegin, 0);
					//stateInform_s(callBack, szLinesBegin);
				}

			}

			szLinesBegin = szLinesEnd + 1;
		}

	}

	

	CloseHandle(hOutputRead);//关闭管道的输出端口；
	return 1;
}