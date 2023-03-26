#pragma once

#include "pch.h"

bool LoadDatabase();
bool SaveDatabase();

std::wstring InsertDateHyphens(const std::wstring& dateString);
std::wstring RemoveDateHyphens(const std::wstring& dateString);

int ActionToNumber(std::wstring& action);

std::wstring NumberToUnderlying(int number);
std::wstring NumberToAction(int number);



