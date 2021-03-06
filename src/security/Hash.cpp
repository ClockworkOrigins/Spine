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

#include "security/Hash.h"

#include <memory>

#include "utils/Conversion.h"

#include <QCryptographicHash>
#include <QDebug>
#include <QDir>
#include <QRegularExpression>
#include <QProcessEnvironment>

#ifdef Q_OS_WIN
	#include <Windows.h>
	#include <iphlpapi.h>
	#include <cstdio>

	#pragma comment(lib, "IPHLPAPI.lib")
#endif

using namespace spine::security;

#ifdef Q_OS_WIN
std::string exec(const char* cmd) {
    QProcess execProcess;
	execProcess.start(s2q(cmd), {}, QIODevice::ReadWrite);
	execProcess.waitForFinished();
	const QByteArray arr = execProcess.readAll();
	std::string result = arr.toStdString();

    return result;
}

QString Hash::calculateSystemHash() {
	const std::string uuid = exec("wmic csproduct get UUID");
	const std::string serialnumber = exec("wmic DISKDRIVE get SerialNumber");
	QString clearString = getProductID() + getHarddriveNumber() /*+ getProcessorID()*/;
	clearString = QString::fromStdString(uuid) + QString::fromStdString(serialnumber);
	
	clearString.remove("UUID", Qt::CaseInsensitive);
	clearString.remove("SerialNumber", Qt::CaseInsensitive);
	clearString = clearString.trimmed();
	clearString.replace(QRegularExpression("[\\s]+"), "-");

	while (clearString.contains("--")) {
		clearString.replace(QRegularExpression("--"), "-");
	}

	QCryptographicHash hash(QCryptographicHash::Sha512);
	hash.addData(clearString.toLatin1());
	QString hashSum = QString::fromLatin1(hash.result().toHex());
	return hashSum;
}

QString Hash::getMAC() {
	IP_ADAPTER_INFO *info = nullptr;
	DWORD size = 0;

	GetAdaptersInfo(info, &size);

	info = static_cast<IP_ADAPTER_INFO *>(malloc(size));

	GetAdaptersInfo(info, &size);

	QString mac;

	for (IP_ADAPTER_INFO * pos = info; pos != nullptr;) {
		mac = QString("%1").arg(static_cast<int>(pos->Address[0]), 2, 16, QChar('0'));
		for (UINT i = 1; i < pos->AddressLength; i++) {
			mac += QString(":%1").arg(static_cast<int>(pos->Address[i]), 2, 16, QChar('0'));
		}
		break;
	}

	free(info);
	return mac;
}

QString Hash::getProductID() {
	TCHAR value[255];
	DWORD bufferSize = 255;
	DWORD dwType = REG_SZ;
	HKEY key = nullptr;
	RegOpenKeyEx(HKEY_LOCAL_MACHINE, R"(SOFTWARE\Microsoft\Windows NT\CurrentVersion)", 0, KEY_QUERY_VALUE | KEY_WOW64_64KEY, &key);
	const int a = RegQueryValueEx(key, "ProductId", nullptr, &dwType, reinterpret_cast<LPBYTE>(&value), &bufferSize);
	QString id = (a == 0) ? QString::fromStdString(value) : QString("XXXXX-XXXXX-XXXXX-XXXXX");
	return id;
}

QString Hash::getHarddriveNumber() {
	TCHAR volumeName[MAX_PATH + 1] = { 0 };
	TCHAR fileSystemName[MAX_PATH + 1] = { 0 };
	DWORD serialNumber = 0;
	DWORD maxComponentLen = 0;
	DWORD fileSystemFlags = 0;
	QString windowsRoot = QProcessEnvironment::systemEnvironment().value("windir");
	windowsRoot = windowsRoot.split(":").front();
	QString serialNumberString;
	if (GetVolumeInformation((windowsRoot + ":\\").toStdString().c_str(), volumeName, ARRAYSIZE(volumeName), &serialNumber, &maxComponentLen, &fileSystemFlags, fileSystemName, ARRAYSIZE(fileSystemName))) {
		printf("Volume Name: %s\n", volumeName);
		printf("Serial Number: %lu\n", serialNumber);
		printf("File System Name: %s\n", fileSystemName);
		printf("Max Component Length: %lu\n", maxComponentLen);
		serialNumberString = QString::number(serialNumber, 16);
	}

	serialNumberString = QString::fromStdString(exec("wmic path win32_physicalmedia get SerialNumber"));

	return serialNumberString;
}

QString Hash::getProcessorID() {
	static const char* szFeatures[] =
	{
		"x87 FPU On Chip",
		"Virtual-8086 Mode Enhancement",
		"Debugging Extensions",
		"Page Size Extensions",
		"Time Stamp Counter",
		"RDMSR and WRMSR Support",
		"Physical Address Extensions",
		"Machine Check Exception",
		"CMPXCHG8B Instruction",
		"APIC On Chip",
		"Unknown1",
		"SYSENTER and SYSEXIT",
		"Memory Type Range Registers",
		"PTE Global Bit",
		"Machine Check Architecture",
		"Conditional Move/Compare Instruction",
		"Page Attribute Table",
		"Page Size Extension",
		"Processor Serial Number",
		"CFLUSH Extension",
		"Unknown2",
		"Debug Store",
		"Thermal Monitor and Clock Ctrl",
		"MMX Technology",
		"FXSAVE/FXRSTOR",
		"SSE Extensions",
		"SSE2 Extensions",
		"Self Snoop",
		"Hyper-threading Technology",
		"Thermal Monitor",
		"Unknown4",
		"Pend. Brk. EN."
	};
	char CPUString[0x20];
	char CPUBrandString[0x40];
	int CPUInfo[4] = { -1 };
	int nSteppingID = 0;
	int nModel = 0;
	int nFamily = 0;
	int nProcessorType = 0;
	int nExtendedmodel = 0;
	int nExtendedfamily = 0;
	int nBrandIndex = 0;
	int nClflusHcachelinesize = 0;
	int nApicPhysicalID = 0;
	int nFeatureInfo = 0;
	int nCacheLineSize = 0;
	int nL2Associativity = 0;
	int nCacheSizeK = 0;
	unsigned i;
	bool    bSSE3NewInstructions = false;
	bool    bMonitorMwait = false;
	bool    bCplQualifiedDebugStore = false;
	bool    bThermalMonitor2 = false;

	// __cpuid with an InfoType argument of 0 returns the number of
	// valid Ids in CPUInfo[0] and the CPU identification string in
	// the other three array elements. The CPU identification string is
	// not in linear order. The code below arranges the information 
	// in a human readable form.
	__cpuid(CPUInfo, 0);
	unsigned nIds = CPUInfo[0];
	memset(CPUString, 0, sizeof(CPUString));
	*reinterpret_cast<int*>(CPUString) = CPUInfo[1];
	*reinterpret_cast<int*>(CPUString + 4) = CPUInfo[3];
	*reinterpret_cast<int*>(CPUString + 8) = CPUInfo[2];

	// Get the information associated with each valid Id
	QString result;
	for (i = 0; i <= nIds; ++i) {
		__cpuid(CPUInfo, i);
		result += QString("\nFor InfoType %1\n").arg(i);
		result += QString("CPUInfo[0] = 0x%1\n").arg(CPUInfo[0]);
		result += QString("CPUInfo[1] = 0x%1\n").arg(CPUInfo[1]);
		result += QString("CPUInfo[2] = 0x%1\n").arg(CPUInfo[2]);
		result += QString("CPUInfo[3] = 0x%1\n").arg(CPUInfo[3]);

		// Interpret CPU feature information.
		if (i == 1) {
			nSteppingID = CPUInfo[0] & 0xf;
			nModel = (CPUInfo[0] >> 4) & 0xf;
			nFamily = (CPUInfo[0] >> 8) & 0xf;
			nProcessorType = (CPUInfo[0] >> 12) & 0x3;
			nExtendedmodel = (CPUInfo[0] >> 16) & 0xf;
			nExtendedfamily = (CPUInfo[0] >> 20) & 0xff;
			nBrandIndex = CPUInfo[1] & 0xff;
			nClflusHcachelinesize = ((CPUInfo[1] >> 8) & 0xff) * 8;
			nApicPhysicalID = (CPUInfo[1] >> 24) & 0xff;
			bSSE3NewInstructions = static_cast<bool>(CPUInfo[2] & 0x1);
			bMonitorMwait = static_cast<bool>(CPUInfo[2] & 0x8);
			bCplQualifiedDebugStore = static_cast<bool>(CPUInfo[2] & 0x10);
			bThermalMonitor2 = static_cast<bool>(CPUInfo[2] & 0x100);
			nFeatureInfo = CPUInfo[3];
		}
	}

	// Calling __cpuid with 0x80000000 as the InfoType argument
	// gets the number of valid extended IDs.
	__cpuid(CPUInfo, 0x80000000);
	const unsigned nExIds = CPUInfo[0];
	memset(CPUBrandString, 0, sizeof(CPUBrandString));

	// Get the information associated with each extended ID.
	for (i = 0x80000000; i <= nExIds; ++i) {
		__cpuid(CPUInfo, i);
		result += QString("\nFor InfoType %1\n").arg(i);
		result += QString("CPUInfo[0] = 0x%1\n").arg(CPUInfo[0]);
		result += QString("CPUInfo[1] = 0x%1\n").arg(CPUInfo[1]);
		result += QString("CPUInfo[2] = 0x%1\n").arg(CPUInfo[2]);
		result += QString("CPUInfo[3] = 0x%1\n").arg(CPUInfo[3]);

		// Interpret CPU brand string and cache information.
		if (i == 0x80000002)
			memcpy(CPUBrandString, CPUInfo, sizeof(CPUInfo));
		else if (i == 0x80000003)
			memcpy(CPUBrandString + 16, CPUInfo, sizeof(CPUInfo));
		else if (i == 0x80000004)
			memcpy(CPUBrandString + 32, CPUInfo, sizeof(CPUInfo));
		else if (i == 0x80000006) {
			nCacheLineSize = CPUInfo[2] & 0xff;
			nL2Associativity = (CPUInfo[2] >> 12) & 0xf;
			nCacheSizeK = (CPUInfo[2] >> 16) & 0xffff;
		}
	}

	// Display all the information in user-friendly format.

	result += QString("\n\nCPU String: %1\n").arg(CPUString);

	if (nIds >= 1) {
		if (nSteppingID)
			result += QString("Stepping ID = %1\n").arg(nSteppingID);
		if (nModel)
			result += QString("Model = %1\n").arg(nModel);
		if (nFamily)
			result += QString("Family = %1\n").arg(nFamily);
		if (nProcessorType)
			result += QString("Processor Type = %1\n").arg(nProcessorType);
		if (nExtendedmodel)
			result += QString("Extended model = %1\n").arg(nExtendedmodel);
		if (nExtendedfamily)
			result += QString("Extended family = %1\n").arg(nExtendedfamily);
		if (nBrandIndex)
			result += QString("Brand Index = %1\n").arg(nBrandIndex);
		if (nClflusHcachelinesize)
			result += QString("CLFLUSH cache line size = %1\n").arg(nClflusHcachelinesize);
		if (nApicPhysicalID)
			result += QString("APIC Physical ID = %1\n").arg(nApicPhysicalID);

		if (nFeatureInfo || bSSE3NewInstructions ||
			bMonitorMwait || bCplQualifiedDebugStore ||
			bThermalMonitor2) {
			result += QString("\nThe following features are supported:\n");

			if (bSSE3NewInstructions)
				result += QString("\tSSE3 New Instructions\n");
			if (bMonitorMwait)
				result += QString("\tMONITOR/MWAIT\n");
			if (bCplQualifiedDebugStore)
				result += QString("\tCPL Qualified Debug Store\n");
			if (bThermalMonitor2)
				result += QString("\tThermal Monitor 2\n");

			i = 0;
			nIds = 1;
			while (i < (sizeof(szFeatures) / sizeof(const char*))) {
				if (nFeatureInfo & nIds) {
					result += QString("\t");
					result += QString(szFeatures[i]);
					result += QString("\n");
				}

				nIds <<= 1;
				++i;
			}
		}
	}

	if (nExIds >= 0x80000004)
		result += QString("\nCPU Brand String: %1\n").arg(CPUBrandString);

	if (nExIds >= 0x80000006) {
		result += QString("Cache Line Size = %1\n").arg(nCacheLineSize);
		result += QString("L2 Associativity = %1\n").arg(nL2Associativity);
		result += QString("Cache Size = %1K\n").arg(nCacheSizeK);
	}
	QCryptographicHash hash(QCryptographicHash::Sha512);
	hash.addData(result.toLatin1());
	QString hashSum = QString::fromLatin1(hash.result().toHex());
	return hashSum;
}
#else
QString Hash::calculateSystemHash() {
	return "";
}

QString Hash::getMAC() {
	return "";
}

QString Hash::getProductID() {
	return "";
}

QString Hash::getHarddriveNumber() {
	return "";
}

QString Hash::getProcessorID() {
	return "";
}
#endif
