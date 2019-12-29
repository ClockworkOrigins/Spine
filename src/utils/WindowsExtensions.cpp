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
// Copyright 2018 Clockwork Origins

#include "WindowsExtensions.h"

#ifdef Q_OS_WIN

#include <QApplication>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QProcess>
#include <QSysInfo>

#include <Windows.h>
#include <Psapi.h>
#include <TlHelp32.h>

#pragma comment(lib, "Psapi.lib")

namespace spine {
namespace utils {

	bool IsRunAsAdmin() {
#ifdef Q_OS_WIN
		BOOL fIsRunAsAdmin = FALSE;
		DWORD dwError = ERROR_SUCCESS;
		PSID pAdministratorsGroup = nullptr;

		// Allocate and initialize a SID of the administrators group.
		SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
		if (!AllocateAndInitializeSid(&NtAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &pAdministratorsGroup)) {
			dwError = GetLastError();
			goto Cleanup;
		}

		// Determine whether the SID of administrators group is enabled in 
		// the primary access token of the process.
		if (!CheckTokenMembership(nullptr, pAdministratorsGroup, &fIsRunAsAdmin)) {
			dwError = GetLastError();
			goto Cleanup;
		}

	Cleanup:
		// Centralized cleanup for all allocated resources.
		if (pAdministratorsGroup) {
			FreeSid(pAdministratorsGroup);
			pAdministratorsGroup = nullptr;
		}

		// Throw the error if something failed in the function.
		if (ERROR_SUCCESS != dwError) {
			throw dwError;
		}

		return fIsRunAsAdmin;
#else
		return true;
#endif
	}

	bool makeSymlink(QString sourceFile, QString targetFile) {
		if (!QFile(sourceFile).exists()) {
			return false;
		}
		QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
		env.insert("SPINE_TARGET", targetFile);
		env.insert("SPINE_SOURCE", sourceFile);
		env.insert("SPINE_TYPE", "0");
		QProcess p;
		p.setEnvironment(env.toStringList());
		p.setWorkingDirectory(qApp->applicationDirPath() + "/..");
		p.start("cmd.exe", QStringList() << "/c" << "makeSymlink.bat");
		const bool b = p.waitForFinished();
		return b;
	}

	bool makeSymlinkFolder(QString sourceDir, QString targetDir) {
		if (!QDir(sourceDir).exists()) {
			return false;
		}
		QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
		env.insert("SPINE_TARGET", targetDir);
		env.insert("SPINE_SOURCE", sourceDir);
		env.insert("SPINE_TYPE", "1");
		QProcess p;
		p.setEnvironment(env.toStringList());
		p.setWorkingDirectory(qApp->applicationDirPath() + "/..");
		p.start("cmd.exe", QStringList() << "/c" << "makeSymlink.bat");
		const bool b = p.waitForFinished();
		return b;
	}

	void removeSymlink(QString symLink) {
		QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
		env.insert("SPINE_TARGET", symLink);
		QProcess p;
		p.setEnvironment(env.toStringList());
		p.setWorkingDirectory(qApp->applicationDirPath() + "/..");
		p.start("cmd.exe", QStringList() << "/c" << "removeSymlink.bat");
		p.waitForFinished();
	}

	int GetProcId(const char * ProcName) {
		PROCESSENTRY32   pe32;

		pe32.dwSize = sizeof(PROCESSENTRY32);
		HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

		int procID = 0;

		if (Process32First(hSnapshot, &pe32)) {
			do {
				if (strcmp(pe32.szExeFile, ProcName) == 0) {
					procID = pe32.th32ProcessID;
					break;
				}
			} while (Process32Next(hSnapshot, &pe32));
		}

		if (hSnapshot != INVALID_HANDLE_VALUE)
			CloseHandle(hSnapshot);

		return procID;
	}

	HANDLE GetProcHandle(const char * ProcName) {
		PROCESSENTRY32   pe32;

		pe32.dwSize = sizeof(PROCESSENTRY32);
		const HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

		if (Process32First(hSnapshot, &pe32)) {
			do {
				if (strcmp(pe32.szExeFile, ProcName) == 0) {
					break;
				}
			} while (Process32Next(hSnapshot, &pe32));
		}

		return hSnapshot;
	}

	uint32_t getPRAMValue() { // Note: this value is in KB!
		PROCESS_MEMORY_COUNTERS memCounter;
		GetProcessMemoryInfo(GetCurrentProcess(), &memCounter, sizeof(memCounter));
		const uint32_t result = memCounter.WorkingSetSize / 1024 / 1024;

		return result;
	}

} /* namespace utils */
} /* namespace spine */

#endif
