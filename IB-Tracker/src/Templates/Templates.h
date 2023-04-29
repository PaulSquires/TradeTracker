#pragma once


struct CTradeTemplateLeg {
	std::wstring quantity;
	std::wstring PutCall;
	std::wstring action;
};


class CTradeTemplate
{
	public:
		std::wstring name;
		std::wstring description;
		std::wstring ticker;
		std::wstring company;
		std::wstring multiplier = L"100";
		std::vector<CTradeTemplateLeg> legs;
		bool menu = false;
};


extern std::vector<CTradeTemplate> TradeTemplates;

bool LoadTemplates();

