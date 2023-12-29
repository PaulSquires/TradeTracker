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

#pragma once

#include "Utilities/AfxWin.h"


class CDatabase {
private:
	std::wstring dbFilename_new = L"\\tt-database.db";
	std::wstring dbTradePlan_new = L"\\tt-tradeplan.txt";
	std::wstring dbJournalNotes_new = L"\\tt-journalnotes.txt";

	const std::wstring dbFilename_old = AfxGetExePath() + L"\\IB-Tracker-database.db";
	const std::wstring dbJournalNotes_old = AfxGetExePath() + L"\\IB-Tracker-journalnotes.txt";
	const std::wstring dbTradePlan_old = AfxGetExePath() + L"\\IB-Tracker-tradeplan.txt";

	std::wstring dbFilename;
	
	std::wstring dbJournalNotes;
	std::wstring dbTradePlan;

	std::wstring journal_notes_text = L"";
	std::wstring trade_plan_text = L"";

	bool Version4UpgradeDatabase();
	bool Version4UpgradeJournalNotes();
	bool Version4UpgradeTradePlan();

	int UnderlyingToNumber(const std::wstring& underlying);
	std::wstring NumberToUnderlying(const int number);
	int ActionToNumber(const std::wstring& action);
	std::wstring NumberToAction(const int number);

public:
	std::wstring GetJournalNotesText();
	void SetJournalNotesText(const std::wstring& text);
	std::wstring GetTradePlanText();
	void SetTradePlanText(const std::wstring& text);

	bool LoadDatabase();
	bool SaveDatabase();
};

extern CDatabase db;
