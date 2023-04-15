#pragma once


bool SaveConfig();
bool LoadConfig();

std::wstring GetTraderName();
void SetTraderName(std::wstring wszName);

bool GetStartupConnect();
void SetStartupConnect(bool bConnect);
