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
		std::vector<CTradeTemplateLeg> legs;
		bool menu = false;
		int ctrlId = 0;
};


extern std::vector<CTradeTemplate> TradeTemplates;

bool LoadTemplates();

