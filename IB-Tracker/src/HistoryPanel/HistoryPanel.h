#pragma once

#include "..\Utilities\CWindowBase.h"


class CHistoryPanel : public CWindowBase<CHistoryPanel>
{
public:
    LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);

};


const int IDC_HISTORY_LISTBOX = 110;
const int IDC_HISTORY_SYMBOL = 111;
const int IDC_HISTORY_VSCROLLBAR = 112;

const int HISTORY_LISTBOX_ROWHEIGHT = 24;
const int HISTORYPANEL_WIDTH = 340;
