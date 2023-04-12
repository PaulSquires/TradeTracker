#pragma once

#include "..\Utilities\CWindowBase.h"
#include "..\tws-api\Contract.h"


class CReconcile: public CWindowBase<CReconcile>
{
public:
    LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);

};


const int IDC_RECONCILE_TEXTBOX = 100;

void Reconcile_position(const Contract& contract, Decimal position);
void Reconcile_positionEnd();
void Reconcile_Show(std::wstring& wszText);

