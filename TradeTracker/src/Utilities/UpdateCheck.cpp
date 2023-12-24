/*

MIT License

Copyright(c) 2023-2024 Paul Squires

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files(the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and /or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions :

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*/

//
// Looks for updated version of the TradeTracker program and displays message.
//

#include "pch.h"

#include "Utilities/AfxWin.h"
#include "Config/Config.h"
#include "CustomLabel/CustomLabel.h"
#include "MainWindow/MainWindow.h"


std::atomic<bool> is_updatecheck_thread_active = false;

std::jthread updatecheck_thread;


#include <wininet.h>
#pragma comment(lib,"Wininet.lib")
#include <shellapi.h>
#pragma comment(lib, "shell32.lib")

void ShowReleasesWebPage() {
	// URL of the webpage you want to open
	const wchar_t* url = L"https://github.com/PaulSquires/TradeTracker/releases";

	// Open the webpage
	HINSTANCE result = ShellExecute(nullptr, L"open", url, nullptr, nullptr, SW_SHOWNORMAL);

	// Check the result
	if ((intptr_t)result <= 32) {
		// An error occurred
		std::cout << "Failed to open releases web page" << std::endl;
	}
}


void UpdateCheckFunction(std::stop_token st) {
	std::cout << "Starting the update check thread" << std::endl;

	is_updatecheck_thread_active = true;

	std::wstring curl_command = L"C:/Windows/System32/curl.exe";
	bool curl_exists = AfxFileExists(curl_command);

	// Do a simple check to see if connected to internet. If fail then simply
	// advise the user via a messgebox.
	std::wstring url = L"https://www.google.com";
	bool is_internet_available = InternetCheckConnection(url.c_str(), FLAG_ICC_FORCE_CONNECTION, 0);

	if (!is_internet_available) {
		std::cout << "Update Check failed: No internet connection." << std::endl;
	}
	else if (!curl_exists) {
		std::cout << "Update Check failed: Curl does not exist." << std::endl;
	}
	else {
		std::wstring local_file = AfxGetExePath() + L"\\_versioncheck.txt";
		std::wstring cmd = L"C:/Windows/System32/curl.exe -o " + 
			local_file + L" \"https://www.planetsquires.com/ibtracker_version.txt\"";
		std::wstring text = AfxExecCmd(cmd);

		std::wstring version_available = L"";

		if (AfxFileExists(local_file)) {
			std::wifstream db;
			db.open(local_file, std::ios::in);
			if (db.is_open()) {

				while (!db.eof()) {
					std::wstring line;
					std::getline(db, line);

					if (line.length() == 0) continue;
					
					// Tokenize the line into a vector based on the equals delimiter
					std::vector<std::wstring> stvec = AfxSplit(line, L'=');

					if (stvec.empty()) break;

					if (stvec.at(0) == L"latest_version") {
						version_available = stvec.at(1);
						break;
					}
				}

			}
			db.close();
		}

		DeleteFile(local_file.c_str());

		if (version_available.length() > 4) {
			if (AfxWStringCompareI(version, version_available) == false) {
				CustomLabel_SetText(GetDlgItem(HWND_MAINWINDOW, IDC_MAINWINDOW_UPDATEAVAILABLE),
					L"** v" + version_available + L" available **");
				ShowWindow(GetDlgItem(HWND_MAINWINDOW, IDC_MAINWINDOW_UPDATEAVAILABLE), SW_SHOWNORMAL);
			}
		}

		std::cout << "Update Check Thread Terminated" << std::endl;
		is_updatecheck_thread_active = false;
	}
}


void DisplayUpdateAvailableMessage() {
	if (!GetAllowUpdateCheck()) {
		std::cout << "Update Check Disabled in Configuration" << std::endl;
		return;
	}

	if (is_updatecheck_thread_active) return;
	updatecheck_thread = std::jthread(UpdateCheckFunction);
}

