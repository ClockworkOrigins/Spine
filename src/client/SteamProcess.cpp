/*
	This file is part of Spine.

    Spine is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Spine is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Spine.  If not, see <http://www.gnu.org/licenses/>.
 */
// Copyright 2020 Clockwork Origins

#include "SteamProcess.h"

#ifdef Q_OS_WIN

#include <thread>

#include "utils/Conversion.h"
#include "utils/WindowsExtensions.h"

#include <QDesktopServices>
#include <QtConcurrentRun>
#include <QTime>
#include <QUrl>

#include <Windows.h>
#include <Psapi.h>

using namespace spine::client;
using namespace spine::utils;

namespace {

	struct EnumData {
		DWORD procID;
		int * popupCount;
	};

	BOOL CALLBACK enumProcWindow(const HWND hWnd, const LPARAM lParam) {
		// Retrieve storage location for communication data
		EnumData& ed = *reinterpret_cast<EnumData *>(lParam);
		DWORD dwProcessId = 0x0;
		// Query process ID for hWnd
		GetWindowThreadProcessId(hWnd, &dwProcessId);

		if (dwProcessId == ed.procID) {
			TCHAR Buffer[MAX_PATH];
			GetWindowText(hWnd, Buffer, MAX_PATH);
			
			const std::string s = Buffer;
			if (s2q(s).contains("?", Qt::CaseInsensitive)) {
				++*ed.popupCount;

				SetForegroundWindow(hWnd);
				
				std::this_thread::sleep_for(std::chrono::milliseconds(50));
				
				{
					INPUT ip;
					ip.type = INPUT_KEYBOARD;
					ip.ki.wVk = 0;
					ip.ki.wScan = (WORD) MapVirtualKey((WORD) VK_TAB, MAPVK_VK_TO_VSC);
					ip.ki.time = 0;
					ip.ki.dwExtraInfo = 0;

					// Press the "TAB" key
					ip.ki.dwFlags = KEYEVENTF_SCANCODE;
					
					SendInput(1, &ip, sizeof(INPUT));
				}
				{
					INPUT ip;
					ip.type = INPUT_KEYBOARD;
					ip.ki.wVk = 0;
					ip.ki.wScan = (WORD) MapVirtualKey((WORD) VK_TAB, MAPVK_VK_TO_VSC);
					ip.ki.time = 0;
					ip.ki.dwExtraInfo = 0;

					// Press the "TAB" key
					ip.ki.dwFlags = KEYEVENTF_SCANCODE;
					ip.ki.dwFlags |= KEYEVENTF_KEYUP;
					
					SendInput(1, &ip, sizeof(INPUT));
				}
				std::this_thread::sleep_for(std::chrono::milliseconds(50));
				{
					INPUT ip;
					ip.type = INPUT_KEYBOARD;
					ip.ki.wVk = 0;
					ip.ki.wScan = (WORD) MapVirtualKey((WORD) VK_RETURN, MAPVK_VK_TO_VSC);
					ip.ki.time = 0;
					ip.ki.dwExtraInfo = 0;

					// Press the "RETURN" key
					ip.ki.dwFlags = KEYEVENTF_SCANCODE;
					
					SendInput(1, &ip, sizeof(INPUT));
				}
				{
					INPUT ip;
					ip.type = INPUT_KEYBOARD;
					ip.ki.wVk = 0;
					ip.ki.wScan = (WORD) MapVirtualKey((WORD) VK_RETURN, MAPVK_VK_TO_VSC);
					ip.ki.time = 0;
					ip.ki.dwExtraInfo = 0;

					// Press the "RETURN" key
					ip.ki.dwFlags = KEYEVENTF_SCANCODE;
					ip.ki.dwFlags |= KEYEVENTF_KEYUP;
					
					SendInput(1, &ip, sizeof(INPUT));

					//return FALSE;
				}
			}
        }
		
		return TRUE;
	}
	
	void findWindowFromProcessId(int procID, int & popupCount) {
		popupCount = 0;
		EnumData ed = { procID, &popupCount };
		EnumWindows(enumProcWindow, reinterpret_cast<LPARAM>(&ed));
	}
	
}

SteamProcess::SteamProcess(int32_t steamID, QString executable, QStringList arguments) : _steamID(steamID), _executable(executable), _arguments(arguments) {
	qRegisterMetaType<QProcess::ExitStatus>();
}

void SteamProcess::start(int timeoutInSecs) {
	QString url = QString("steam://run/%1").arg(_steamID);

	if (!_arguments.isEmpty()) {
		url += QString("//%2").arg(_arguments.join(' '));
	}
	
	QDesktopServices::openUrl(QUrl(url));

	QtConcurrent::run(this, &SteamProcess::checkIfProcessRunning, timeoutInSecs);
}

void SteamProcess::checkIfProcessRunning(int timeoutInSecs) {
	// check until Steam is running
	const auto startTime = QTime::currentTime();

	int procID;
	
	do {
		procID = GetProcId("Steam.exe");
	} while (procID == 0 && startTime.secsTo(QTime::currentTime()) < timeoutInSecs);

	if (procID == 0) {
		emit finished(0, QProcess::ExitStatus::NormalExit);
		return;
	}

	if (!_arguments.isEmpty()) {
		HANDLE handle;
	
		do {
			handle = GetProcHandle(procID);
		} while (handle == INVALID_HANDLE_VALUE);		

		int popupCount;
		findWindowFromProcessId(procID, popupCount);

		bool popupAppeared = false;
		
		do {
			if (popupCount > 0) {
				popupAppeared = true;
			}
			std::this_thread::sleep_for(std::chrono::seconds(1));
			findWindowFromProcessId(procID, popupCount);
		} while ((!popupAppeared || popupCount > 0) && startTime.secsTo(QTime::currentTime()) < timeoutInSecs);

		CloseHandle(handle);
	}

	const auto exe = q2s(_executable);

	HANDLE handle;
	
	do {
		handle = GetProcHandle(exe.c_str());
	} while (handle == INVALID_HANDLE_VALUE && startTime.secsTo(QTime::currentTime()) < timeoutInSecs);

	if (handle == INVALID_HANDLE_VALUE) {
		emit finished(0, QProcess::ExitStatus::NormalExit);
		return;
	}

	DWORD lpExitCode = 0;
	while (true) {
		const auto b = GetExitCodeProcess(handle, &lpExitCode);
		
		if (!b) break;

		if (lpExitCode != STILL_ACTIVE) break;
		
		std::this_thread::sleep_for(std::chrono::seconds(5));
	}

	CloseHandle(handle);

	QProcess::ExitStatus exitStatus = QProcess::NormalExit;
	if (lpExitCode != 0) {
		exitStatus = QProcess::CrashExit;
	}
	emit finished(0, exitStatus);
}

#endif
