#pragma once

#include "..\Utilities\CWindowBase.h"


class CHistoryPanel : public CWindowBase<CHistoryPanel>
{
public:
    LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
};


const int HISTORYPANEL_WIDTH = 300;

const int NUM_COLUMNS = 8;


