#pragma once

#include <iostream>
#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <functional>
#include <direct.h>
#include <thread>
#include <mutex>
#include <string>
using namespace std;

typedef std::function<void(const char* bytes)>  Callback;
class Process {
public:
	int open(string  command, Callback callback) {
		do {
			this->m_finished = 0;
			HANDLE hread, hwrite;
			SECURITY_ATTRIBUTES sa = { sizeof(SECURITY_ATTRIBUTES), NULL, TRUE };
			if (!CreatePipe(&hread, &hwrite, &sa, 0))
				DisplayError("CreatePipe");
			PROCESS_INFORMATION pi;
			STARTUPINFOA si;
			ZeroMemory(&si, sizeof(STARTUPINFO));
			si.cb = sizeof(STARTUPINFO);
			si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
			si.hStdOutput = hwrite;
			si.wShowWindow = SW_HIDE; //隐藏窗口；
			//m_lock.lock();
			this->m_lock_stop.lock();
			if (this->enablekill == true) {
				this->m_lock_stop.unlock();
				break;
			}
				
			if (!CreateProcessA(NULL, (char*)command.c_str(), NULL, NULL, TRUE,
				CREATE_NO_WINDOW, NULL, NULL, &si, &pi))
				DisplayError("CreateProcess");
			this->m_lock_stop.unlock();
			//m_lock.unlock();
			if (!CloseHandle(hwrite)) DisplayError("CloseHandle(hwrite)");
			this->m_pid = pi.dwProcessId;
			this->hProcess = pi.hProcess;
			this->hThread = pi.hThread;

			/*	std::thread threadObj([=] {
					ansyread(hread, callback);
					});*/
					/*threadObj.detach();*/
			char szRecvData[512] = { 0 };
			DWORD dwRecvSize = 0;
			std::string szOutputLine;
			while (FALSE != ReadFile(hread, szRecvData, _countof(szRecvData) - 1, &dwRecvSize, NULL))
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
						callback(szOutputLine.c_str());
						//stateInform_s(callBack, (char*)szOutputLine.c_str());
						szOutputLine.clear();
					}
					else
					{
						if (*szLinesBegin != '\0')
						{
							callback(szLinesBegin);
							//stateInform_s(callBack, szLinesBegin);
						}

					}

					szLinesBegin = szLinesEnd + 1;
				}

			}
			if (!CloseHandle(hread)) DisplayError("CloseHandle(hread)");
			m_lock.lock();
			if (!CloseHandle(this->hThread)) DisplayError("CloseHandle(this->hThread)");
			this->hThread = 0;
			if (!CloseHandle(this->hProcess)) DisplayError("CloseHandle(this->hProcess)");
			this->hProcess = 0;
			m_lock.unlock();
		} while (false);
		
		DisplayError("open over");
		this->m_finished = 1;
		this->enablekill = 0;
		return  1;
	}
	void kill()
	{
		
		this->m_lock_stop.lock();
		this->enablekill = 1;
		std::string szBuf = std::string("taskkill /PID ") + std::to_string((unsigned)this->m_pid) + (" /T /F");
		WinExec(szBuf.c_str(), SW_HIDE);
		this->m_lock_stop.unlock();
		m_lock.lock();
		if (!CloseHandle(this->hThread)) DisplayError("CloseHandle(this->hThread)");
		this->hThread = 0;
		if (!CloseHandle(this->hProcess)) DisplayError("CloseHandle(this->hProcess)");
		this->hProcess = 0;
		m_lock.unlock();
	}
	int get_exit_status() {
		if (this->m_pid == 0)
			return -1;
		DWORD exit_status;
		WaitForSingleObject(this->hProcess, INFINITE);
		if (!GetExitCodeProcess(this->hProcess, &exit_status))
			exit_status = -1;
		while (true) {

			if (this->m_finished == 1)
				break;
		}
		return static_cast<int>(exit_status);
	}
	int aysnOpen(string  command, Callback callback) {
		HANDLE eve = CreateEvent(NULL, FALSE, FALSE, NULL);
		this->enablekill = 0;
		thread thread1([=]() {
			SetEvent(eve);
			this->open(command, callback);
			});
		thread1.detach();
		WaitForSingleObject(eve, INFINITE);
		CloseHandle(eve);
		return 1;
	}
private:
	void DisplayError(const char* promt) {
		cout << promt << " error" << endl;
	}
private:
	HANDLE hProcess=0, hThread=0;
	std::mutex m_lock;
	std::mutex m_lock_stop;
	DWORD m_pid = 0;
	BOOL m_finished = 1;
	bool enablekill = 0;
};







