#pragma once

#include "..\Themes\Themes.h"
#include "..\Utilities\AfxWin.h"
#include "..\Utilities\UserMessages.h"


#define IDB_LOGO                        105

enum class CustomLabelType
{
	TextOnly,
	ImageOnly,
	ImageAndText,
	LineHorizontal,
	LineVertical
};

enum class CustomLabelAlignment
{
	// Text alignment
	MiddleCenter,
	MiddleLeft,
	MiddleRight,
	TopCenter,
	TopLeft,
	TopRight,
	BottomCenter,
	BottomLeft,
	BottomRight
};

enum class CustomLabelPointer
{
	Arrow,
	Hand
};


class CustomLabel
{
private:
	HDC m_memDC = NULL;           // Double buffering
	HBITMAP m_hbit = NULL;        // Double buffering
	RECT m_rcClient{};
	float m_rx = 0;
	float m_ry = 0;
	bool m_bIsHot = false;

public:
	HWND hWindow = NULL;
	HWND hParent = NULL;
	HINSTANCE hInst = NULL;
	HWND hToolTip = NULL;
	std::wstring wszToolTip;
	int UserData = 0;

	int CtrlId = 0;
	CustomLabelType CtrlType = CustomLabelType::TextOnly;
	bool HotTestEnable = false;
	ThemeElement BackColor{};
	ThemeElement BackColorHot{};
	ThemeElement BackColorButtonDown{};

	// Selection
	bool AllowSelect = false;
	bool IsSelected = false;
	ThemeElement SelectorColor{};
	ThemeElement BackColorSelected{};

	// Lines
	REAL LineWidth = 1;
	ThemeElement LineColor{};
	ThemeElement LineColorHot{};

	// Margins
	int MarginLeft = 0;
	int MarginTop = 0;
	int MarginBottom = 0;
	int MarginRight = 0;

	// Border
	bool BorderVisible = false;
	REAL BorderWidth = 1;
	ThemeElement BorderColor{};
	ThemeElement BorderColorHot{};
	int BorderRoundWidth = 0;
	int BorderRoundHeight = 0;

	// Images
	Bitmap* pImage = nullptr;
	Bitmap* pImageHot = nullptr;
	int ImageWidth = 0;
	int ImageHeight = 0;
	int ImageOffsetLeft = 0;
	int ImageOffsetTop = 0;

	// Text General
	CustomLabelAlignment TextAlignment = CustomLabelAlignment::MiddleCenter;
	int TextOffsetLeft = 0;
	int TextOffsetTop = 0;
	int TextCharacterExtra = 0;

	// Text Normal
	std::wstring wszText;
	std::wstring wszFontName;
	REAL FontSize = 0;
	bool FontBold = false;
	bool FontItalic = false;
	bool FontUnderline = false;
	ThemeElement TextColor{};
	CustomLabelPointer Pointer = CustomLabelPointer::Arrow;

	// Text Hot
	std::wstring wszTextHot;
	std::wstring wszFontNameHot;
	REAL FontSizeHot = 0;
	bool FontBoldHot = false;
	bool FontItalicHot = false;
	bool FontUnderlineHot = false;
	ThemeElement TextColorHot{};
	CustomLabelPointer PointerHot = CustomLabelPointer::Hand;



	void SetTextAlignment(StringFormat* stringF)
	{
		switch (TextAlignment)
		{
		case CustomLabelAlignment::BottomCenter:
			stringF->SetAlignment(StringAlignmentCenter);
			stringF->SetLineAlignment(StringAlignmentFar);
			break;

		case CustomLabelAlignment::BottomLeft:
			stringF->SetAlignment(StringAlignmentNear);
			stringF->SetLineAlignment(StringAlignmentFar);
			break;

		case CustomLabelAlignment::BottomRight:
			stringF->SetAlignment(StringAlignmentFar);
			stringF->SetLineAlignment(StringAlignmentFar);
			break;

		case CustomLabelAlignment::MiddleCenter:
			stringF->SetAlignment(StringAlignmentCenter);
			stringF->SetLineAlignment(StringAlignmentCenter);
			break;

		case CustomLabelAlignment::MiddleLeft:
			stringF->SetAlignment(StringAlignmentNear);
			stringF->SetLineAlignment(StringAlignmentCenter);
			break;

		case CustomLabelAlignment::MiddleRight:
			stringF->SetAlignment(StringAlignmentFar);
			stringF->SetLineAlignment(StringAlignmentCenter);
			break;

		case CustomLabelAlignment::TopCenter:
			stringF->SetAlignment(StringAlignmentCenter);
			stringF->SetLineAlignment(StringAlignmentNear);
			break;

		case CustomLabelAlignment::TopLeft:
			stringF->SetAlignment(StringAlignmentNear);
			stringF->SetLineAlignment(StringAlignmentNear);
			break;

		case CustomLabelAlignment::TopRight:
			stringF->SetAlignment(StringAlignmentFar);
			stringF->SetLineAlignment(StringAlignmentNear);
			break;

		default:
			stringF->SetAlignment(StringAlignmentCenter);
			stringF->SetLineAlignment(StringAlignmentCenter);
		}
	}


	void StartDoubleBuffering(HDC hdc)
	{
		SaveDC(hdc);
		GetClientRect(hWindow, &m_rcClient);

		m_rx = AfxScaleRatioX();
		m_ry = AfxScaleRatioY();

		// Determine if we are in a Hot mouseover state
		if (HotTestEnable) m_bIsHot = GetProp(hWindow, L"HOT") ? true : false;

		m_memDC = CreateCompatibleDC(hdc);
		m_hbit = CreateCompatibleBitmap(hdc, m_rcClient.right, m_rcClient.bottom);
		SelectBitmap(m_memDC, m_hbit);

		Graphics graphics(m_memDC);
		graphics.SetTextRenderingHint(TextRenderingHintClearTypeGridFit);

		DWORD nBackColor = (m_bIsHot ? GetThemeColor(BackColorHot) : GetThemeColor(BackColor));
		if (IsSelected && AllowSelect)
			nBackColor = GetThemeColor(BackColorSelected);

		if (GetAsyncKeyState(VK_LBUTTON) & 0x01) { 
			nBackColor = GetThemeColor(BackColorButtonDown);
		}

		// Create the background brush
		SolidBrush backBrush(nBackColor);

		// Paint the background using brush and default pen. 
		int nWidth = (m_rcClient.right - m_rcClient.left);
		int nHeight = (m_rcClient.bottom - m_rcClient.top);
		graphics.FillRectangle(&backBrush, 0, 0, nWidth, nHeight);
	}


	void DrawImageInBuffer()
	{
		switch (CtrlType)
		{
		case CustomLabelType::ImageOnly:
		case CustomLabelType::ImageAndText:
			REAL nLeft = (MarginLeft + ImageOffsetLeft) * m_rx;
			REAL nTop = (MarginTop + ImageOffsetTop) * m_ry;
			REAL nRight = nLeft + (ImageWidth * m_rx);
			REAL nBottom = nTop + (ImageHeight * m_ry);

			RectF rcImage(nLeft, nTop, nRight - nLeft, nBottom - nTop);

			Graphics graphics(m_memDC);
			graphics.SetInterpolationMode(InterpolationModeHighQualityBicubic);
			graphics.DrawImage(m_bIsHot ? pImageHot : pImage, rcImage);
		}
	}


	void DrawTextInBuffer()
	{
		switch (CtrlType)
		{
		case CustomLabelType::TextOnly:
		case CustomLabelType::ImageAndText:

			wszFontName = AfxGetDefaultFont();
			if (wszText.substr(0, 2) == L"\\u") {
				wszFontName = L"Segoe UI Symbol";
			}

			wszFontNameHot = wszFontName;
			FontFamily fontFamily(m_bIsHot ? wszFontNameHot.c_str() : wszFontName.c_str());

			REAL fontSize = (m_bIsHot ? FontSizeHot : FontSize);
			int fontStyle = FontStyleRegular;

			if (m_bIsHot) {
				if (FontBoldHot) fontStyle |= FontStyleBold;
				if (FontItalicHot) fontStyle |= FontStyleItalic;
				if (FontUnderlineHot) fontStyle |= FontStyleUnderline;
			}
			else {
				if (FontBold) fontStyle |= FontStyleBold;
				if (FontItalic) fontStyle |= FontStyleItalic;
				if (FontUnderline) fontStyle |= FontStyleUnderline;
			}

			Font         font(&fontFamily, fontSize, fontStyle, Unit::UnitPoint);
			SolidBrush   textBrush(m_bIsHot ? GetThemeColor(TextColorHot) : GetThemeColor(TextColor));

			StringFormat stringF(StringFormatFlagsNoWrap);
			stringF.SetTrimming(StringTrimmingEllipsisWord);
			SetTextAlignment(&stringF);

			if (TextCharacterExtra)
				SetTextCharacterExtra(m_memDC, TextCharacterExtra);

			REAL nLeft = (MarginLeft + ImageWidth + ImageOffsetLeft + TextOffsetLeft) * m_rx;
			REAL nTop = (MarginTop + TextOffsetTop) * m_ry;
			REAL nRight = m_rcClient.right - (MarginRight * m_rx);
			REAL nBottom = m_rcClient.bottom - (MarginBottom * m_ry);

			RectF rcText(nLeft, nTop, nRight - nLeft, nBottom - nTop);
			
			Graphics graphics(m_memDC);
			graphics.SetTextRenderingHint(TextRenderingHintClearTypeGridFit);
			graphics.DrawString(wszText.c_str(), -1, &font, rcText, &stringF, &textBrush);
		}

	}


	void DrawLabelInBuffer()
	{
		switch (CtrlType)
		{
			case CustomLabelType::LineHorizontal:
				REAL nLeft = MarginLeft * m_rx;
				REAL nTop = (MarginTop + TextOffsetTop) * m_ry;
				REAL nRight = m_rcClient.right - (MarginRight * m_rx);
				REAL nBottom = nTop;
				ARGB clrPen = (m_bIsHot ? GetThemeColor(LineColorHot) : GetThemeColor(LineColor));
				Pen pen(clrPen, LineWidth);
				// Draw the horizontal line centered taking margins into account
				Graphics graphics(m_memDC);
				graphics.DrawLine(&pen, nLeft, nTop, nRight, nBottom);

			//case CustomLabelType::LineVertical:
			//	ARGB clrPen = (bIsHot ? pData->LineColorHot : pData->LineColor);
			//	Pen pen(clrPen, pData->LineWidth);
				// Draw the vertical line centered taking margins into account
				//graphics.DrawLine(&pen, rcDraw.GetLeft(), rcDraw.GetTop(), rcDraw.GetRight(), rcDraw.GetBottom());
		}
	}


	void DrawNotchInBuffer()
	{
		// If selection mode is enabled then draw the little right hand side notch
		if (IsSelected) {
			// Create the background brush
			SolidBrush backBrush(GetThemeColor(SelectorColor));
			// Need to center the notch vertically
			REAL nNotchHalfHeight = (16 * m_ry) / 2;
			REAL nTop = (m_rcClient.bottom / 2) - nNotchHalfHeight;
			PointF point1((REAL)m_rcClient.right, nTop);
			PointF point2((REAL)m_rcClient.right - (8 * m_rx), nTop + nNotchHalfHeight);
			PointF point3((REAL)m_rcClient.right, nTop + (nNotchHalfHeight * 2));
			PointF points[3] = { point1, point2, point3 };
			PointF* pPoints = points;
			Graphics graphics(m_memDC);
			graphics.FillPolygon(&backBrush, pPoints, 3);
		}
	}


	void DrawBordersInBuffer()
	{
		// Finally, draw any applicable border around the control after everything
		// else has been painted.
		switch (CtrlType)
		{
		case CustomLabelType::ImageOnly:
		case CustomLabelType::ImageAndText:
		case CustomLabelType::TextOnly:
		{
			ARGB clrPen = (m_bIsHot ? GetThemeColor(BorderColorHot) : GetThemeColor(BorderColor));
			if (!BorderVisible) clrPen = GetThemeColor(BackColor);
			Pen pen(clrPen, BorderWidth);

			RectF rectF(0, 0, (REAL)m_rcClient.right - BorderWidth, (REAL)m_rcClient.bottom - BorderWidth);
			Graphics graphics(m_memDC);
			graphics.DrawRectangle(&pen, rectF);
		}
		}
	}


	void EndDoubleBuffering(HDC hdc)
	{
		// Copy the entire memory bitmap to the main display
		BitBlt(hdc, 0, 0, m_rcClient.right, m_rcClient.bottom, m_memDC, 0, 0, SRCCOPY);

		// Restore the original state of the DC
		RestoreDC(hdc, -1);

		// Cleanup
		DeleteObject(m_hbit);
		DeleteDC(m_memDC);
	}

};


CustomLabel* CustomLabel_GetOptions(HWND hCtrl);
int CustomLabel_SetOptions(HWND hCtrl, CustomLabel* pData);
void CustomLabel_SetText(HWND hCtrl, std::wstring wszText);
std::wstring CustomLabel_GetText(HWND hCtrl);
void CustomLabel_SetTextColor(HWND hCtrl, ThemeElement TextColor);
void CustomLabel_SetBackColor(HWND hCtrl, ThemeElement BackColor);
ThemeElement CustomLabel_GetBackColor(HWND hCtrl);
void CustomLabel_Select(HWND hCtrl, bool IsSelected);
void CustomLabel_SetFont(HWND hCtrl, std::wstring wszFontName, int FontSize);
void CustomLabel_SetUserData(HWND hCtrl, int UserData);
int CustomLabel_GetUserData(HWND hCtrl);
void CustomLabel_SetBorder(HWND hCtrl, REAL BorderWidth, ThemeElement BorderColor, ThemeElement BorderColorHot);
void CustomLabel_SetTextOffset(HWND hCtrl, int OffsetLeft, int OffsetTop);

HWND CustomLabel_SimpleLabel(HWND hParent, int CtrlId, std::wstring wszText,
	ThemeElement TextColor, ThemeElement BackColor, CustomLabelAlignment alignment = CustomLabelAlignment::MiddleLeft,
	int nLeft = 0, int nTop = 0, int nWidth = 0, int nHeight = 0);

HWND CustomLabel_ButtonLabel(HWND hParent, int CtrlId, std::wstring wszText,
	ThemeElement TextColor, ThemeElement BackColor, ThemeElement BackColorHot, ThemeElement BackColorButtonDown,
	CustomLabelAlignment alignment = CustomLabelAlignment::MiddleLeft,
	int nLeft = 0, int nTop = 0, int nWidth = 0, int nHeight = 0);

	
HWND CustomLabel_SimpleImageLabel(HWND hParent, int CtrlId,
	std::wstring wszImage, std::wstring wszImageHot,
	int ImageWidth, int ImageHeight,
	int nLeft, int nTop, int nWidth, int nHeight);

HWND CreateCustomLabel(HWND hWndParent, LONG_PTR CtrlId, CustomLabelType nCtrlType,
	int nLeft, int nTop, int nWidth, int nHeight);

Gdiplus::Bitmap* LoadImageFromResource(HMODULE hMod, const wchar_t* resid, const wchar_t* restype);
