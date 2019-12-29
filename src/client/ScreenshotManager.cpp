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

#include "ScreenshotManager.h"

#include <functional>
#include <string>
#include <thread>

#include "utils/WindowsExtensions.h"

#include "widgets/LocationSettingsWidget.h"

#include <QDirIterator>
#include <QSysInfo>
#include <QtConcurrentRun>

#ifdef Q_OS_WIN
	#include <Windows.h>
#endif

using namespace spine;
using namespace spine::utils;

namespace {

#ifdef Q_OS_WIN
	HBITMAP ScreenCapture(HWND hWnd) {
		const HDC hScreenDC = GetDC(hWnd);
		// and a device context to put it in
		RECT rc;
		GetClientRect(hWnd, &rc);
		const HDC hMemoryDC = CreateCompatibleDC(hScreenDC);

		const int width = GetDeviceCaps(hScreenDC, HORZRES);
		const int height = GetDeviceCaps(hScreenDC, VERTRES);

		// maybe worth checking these are positive values
		HBITMAP hBitmap = CreateCompatibleBitmap(hScreenDC, rc.right - rc.left, rc.bottom - rc.top);

		// get a new bitmap
		const HBITMAP hOldBitmap = static_cast<HBITMAP>(SelectObject(hMemoryDC, hBitmap));

		BitBlt(hMemoryDC, 0, 0, width, height, hScreenDC, 0, 0, SRCCOPY);
		hBitmap = static_cast<HBITMAP>(SelectObject(hMemoryDC, hOldBitmap));

		// clean up
		DeleteDC(hMemoryDC);
		DeleteDC(hScreenDC);

		return hBitmap;
	}

	PBITMAPINFO CreateBitmapInfoStruct(HBITMAP hBmp) {
		BITMAP bmp;
		PBITMAPINFO pbmi;

		// Retrieve the bitmap color format, width, and height.  
		if (!GetObject(hBmp, sizeof(BITMAP), reinterpret_cast<LPSTR>(&bmp))) {
			return nullptr;
		}

		// Convert the color format to a count of bits.  
		WORD cClrBits = static_cast<WORD>(bmp.bmPlanes * bmp.bmBitsPixel);
		if (cClrBits == 1) {
			cClrBits = 1;
		} else if (cClrBits <= 4) {
			cClrBits = 4;
		} else if (cClrBits <= 8) {
			cClrBits = 8;
		} else if (cClrBits <= 16) {
			cClrBits = 16;
		} else if (cClrBits <= 24) {
			cClrBits = 24;
		} else {
			cClrBits = 32;
		}

		// Allocate memory for the BITMAPINFO structure. (This structure  
		// contains a BITMAPINFOHEADER structure and an array of RGBQUAD  
		// data structures.)  

		if (cClrBits < 24) {
			pbmi = static_cast<PBITMAPINFO>(LocalAlloc(LPTR, sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * (1 << cClrBits)));
		} else {
			// There is no RGBQUAD array for these formats: 24-bit-per-pixel or 32-bit-per-pixel 
			pbmi = static_cast<PBITMAPINFO>(LocalAlloc(LPTR, sizeof(BITMAPINFOHEADER)));
		}

		// Initialize the fields in the BITMAPINFO structure.  

		pbmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		pbmi->bmiHeader.biWidth = bmp.bmWidth;
		pbmi->bmiHeader.biHeight = bmp.bmHeight;
		pbmi->bmiHeader.biPlanes = bmp.bmPlanes;
		pbmi->bmiHeader.biBitCount = bmp.bmBitsPixel;
		if (cClrBits < 24) {
			pbmi->bmiHeader.biClrUsed = (1 << cClrBits);
		}

		// If the bitmap is not compressed, set the BI_RGB flag.  
		pbmi->bmiHeader.biCompression = BI_RGB;

		// Compute the number of bytes in the array of color  
		// indices and store the result in biSizeImage.  
		// The width must be DWORD aligned unless the bitmap is RLE 
		// compressed. 
		pbmi->bmiHeader.biSizeImage = ((pbmi->bmiHeader.biWidth * cClrBits + 31) & ~31) / 8 * pbmi->bmiHeader.biHeight;
		// Set biClrImportant to 0, indicating that all of the  
		// device colors are important.  
		pbmi->bmiHeader.biClrImportant = 0;
		return pbmi;
	}

	void createBmpFile(LPTSTR pszFile, PBITMAPINFO pbi, HBITMAP hBMP, HDC hDC) {
		BITMAPFILEHEADER hdr;       // bitmap file-header  
		DWORD dwTmp;

		const PBITMAPINFOHEADER pbih = reinterpret_cast<PBITMAPINFOHEADER>(pbi);
		const LPBYTE lpBits = static_cast<LPBYTE>(GlobalAlloc(GMEM_FIXED, pbih->biSizeImage));

		if (!lpBits) {
			return;
		}

		// Retrieve the color table (RGBQUAD array) and the bits  
		// (array of palette indices) from the DIB.  
		if (!GetDIBits(hDC, hBMP, 0, static_cast<WORD>(pbih->biHeight), lpBits, pbi, DIB_RGB_COLORS)) {
			return;
		}

		// Create the .BMP file.  
		const HANDLE hf = CreateFile(pszFile, GENERIC_READ | GENERIC_WRITE, static_cast<DWORD>(0), nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, static_cast<HANDLE>(nullptr));
		if (hf == INVALID_HANDLE_VALUE) {
			return;
		}
		hdr.bfType = 0x4d42;        // 0x42 = "B" 0x4d = "M"  
		// Compute the size of the entire file.  
		hdr.bfSize = static_cast<DWORD>(sizeof(BITMAPFILEHEADER) + pbih->biSize + pbih->biClrUsed * sizeof(RGBQUAD) + pbih->biSizeImage);
		hdr.bfReserved1 = 0;
		hdr.bfReserved2 = 0;

		// Compute the offset to the array of color indices.  
		hdr.bfOffBits = static_cast<DWORD>(sizeof(BITMAPFILEHEADER)) + pbih->biSize + pbih->biClrUsed * sizeof(RGBQUAD);

		// Copy the BITMAPFILEHEADER into the .BMP file.  
		if (!WriteFile(hf, static_cast<LPVOID>(&hdr), sizeof(BITMAPFILEHEADER), static_cast<LPDWORD>(&dwTmp), nullptr)) {
			return;
		}

		// Copy the BITMAPINFOHEADER and RGBQUAD array into the file.  
		if (!WriteFile(hf, static_cast<LPVOID>(pbih), sizeof(BITMAPINFOHEADER) + pbih->biClrUsed * sizeof(RGBQUAD), static_cast<LPDWORD>(&dwTmp), (nullptr))) {
			return;
		}

		// Copy the array of color indices into the .BMP file.  
		DWORD cb = pbih->biSizeImage;
		BYTE * hp = lpBits;
		if (!WriteFile(hf, reinterpret_cast<LPSTR>(hp), static_cast<int>(cb), static_cast<LPDWORD>(&dwTmp), nullptr)) {
			return;
		}

		// Close the .BMP file.  
		if (!CloseHandle(hf)) {
			return;
		}

		// Free memory.  
		GlobalFree(static_cast<HGLOBAL>(lpBits));
	}

	struct EnumData {
		DWORD dwProcessId;
		HWND hWnd;
	};

	BOOL CALLBACK enumProc(const HWND hWnd, const LPARAM lParam) {
		// Retrieve storage location for communication data
		EnumData& ed = *reinterpret_cast<EnumData *>(lParam);
		DWORD dwProcessId = 0x0;
		// Query process ID for hWnd
		GetWindowThreadProcessId(hWnd, &dwProcessId);
		// Apply filter - if you want to implement additional restrictions,
		// this is the place to do so.
		if (ed.dwProcessId == dwProcessId) {
			// Found a window matching the process ID
			ed.hWnd = hWnd;
			// Report success
			SetLastError(ERROR_SUCCESS);
			// Stop enumeration
			return FALSE;
		}
		// Continue enumeration
		return TRUE;
	}
	
	HWND findWindowFromProcessId(const DWORD dwProcessId) {
		EnumData ed = { dwProcessId };
		if (!EnumWindows(enumProc, reinterpret_cast<LPARAM>(&ed)) && GetLastError() == ERROR_SUCCESS) {
			return ed.hWnd;
		}
		return nullptr;
	}
#endif
}

ScreenshotManager::ScreenshotManager(QObject * par) : QObject(par), _running(false), _screenshotDirectory(), _modID() {
	_screenshotDirectory = widgets::LocationSettingsWidget::getInstance()->getScreenshotDirectory();
	connect(widgets::LocationSettingsWidget::getInstance(), SIGNAL(screenshotDirectoryChanged(QString)), this, SLOT(setScreenshotDirectory(QString)));
}

ScreenshotManager::~ScreenshotManager() {
	if (!_workerThread.isFinished()) {
		_workerThread.waitForFinished();
	}
}

void ScreenshotManager::start(int32_t modID) {
	_modID = modID;

	const QString path = _screenshotDirectory + "/" + QString::number(_modID) + "/";
	if (!QDir(path).exists()) {
		bool b = QDir(path).mkpath(path);
		Q_UNUSED(b);
	}

	_running = true;
	_workerThread = QtConcurrent::run(this, &ScreenshotManager::execute);
}

void ScreenshotManager::stop() {
	// convert all screenshots to png
	QDirIterator it(_screenshotDirectory + "/" + QString::number(_modID) + "/", QStringList() << "*.bmp", QDir::Filter::Files);
	while (it.hasNext()) {
		it.next();
		{
			QImage img(it.filePath());
			Q_ASSERT(!img.isNull());
			bool b = img.save(it.fileInfo().absolutePath() + "/" + it.fileInfo().baseName() + ".png");
			Q_UNUSED(b);
		}
		QFile(it.filePath()).remove();
	}
	_running = false;
	_workerThread.waitForFinished();
}

void ScreenshotManager::setScreenshotDirectory(QString screenshotDirectory) {
	_screenshotDirectory = screenshotDirectory;
}

void ScreenshotManager::execute() {
#ifdef Q_OS_WIN
	bool toggled = false;
	while (_running) {
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
		const SHORT tabKeyState = GetAsyncKeyState(VK_F12);

		// Test high bit - if set, key was down when GetAsyncKeyState was called.
		if ((1 << 15) & tabKeyState && !toggled) {
			toggled = true;
			takeScreenshot();
		} else if (!((1 << 15) & tabKeyState) && toggled) {
			toggled = false;
		}
	}
#endif
}

void ScreenshotManager::takeScreenshot() {
#ifdef Q_OS_WIN
	int procID = GetProcId("Gothic.exe");
	if (procID <= 0) {
		procID = GetProcId("GothicMod.exe");
	}
	if (procID <= 0) {
		procID = GetProcId("Gothic2.exe");
	}

	HWND hWnd = findWindowFromProcessId(procID);

	HBITMAP hBitmap = ScreenCapture(hWnd);
	PBITMAPINFO info = CreateBitmapInfoStruct(hBitmap);

	HDC hScreenDC = GetDC(hWnd);
	// and a device context to put it in
	HDC hMemoryDC = CreateCompatibleDC(hScreenDC);

	createBmpFile(LPSTR((_screenshotDirectory.toStdString() + "/" + std::to_string(_modID) + "/screen_" + std::to_string(time(nullptr)) + ".bmp").c_str()), info, hBitmap, hMemoryDC);

	// clean up
	DeleteDC(hMemoryDC);
	DeleteDC(hScreenDC);
#endif
}
