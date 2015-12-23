#include "stdafx.h"
#include "overlayRecordingIndicator.h"
#include "resource.h"


#include <commctrl.h>

// add your own path for PlayClaw5.lib
#ifdef _DEBUG
#pragma comment(lib, "../playclaw5.lib")
#else
#pragma comment(lib, "../playclaw5.lib")
#endif

HINSTANCE GetDllInstance();

DWORD m_dwPluginID = -1;		// unique (realtime) plugin ID

PluginOverlayGdiPlusHelper	*pRenderHelper = 0;


Gdiplus::SolidBrush*	pNormalBrush = 0;
Gdiplus::SolidBrush*	pPauseBrush = 0;
Gdiplus::SolidBrush*	pRecordBrush = 0;

BOOL					bShowWhenNotRecording = 0;
int						iOpacity = 255;

//
//	Init plugin
//
PLUGIN_EXPORT DWORD PluginInit(DWORD dwPluginID)
{
	m_dwPluginID = dwPluginID;										

	// Initialize GDI+.
	pRenderHelper = new PluginOverlayGdiPlusHelper(dwPluginID);
	return PC_PLUGIN_FLAG_IS_ACTIVE | PC_PLUGIN_FLAG_IS_OVERLAY;	// plugin is active and has overlay features
}

PLUGIN_EXPORT const char* PluginGetTitle()
{
	return "RECORDING_INDICATOR_PLUGIN";
}

//
//	Set default variables
//
PLUGIN_EXPORT void PluginSetDefaultVars()
{
	PC_SetPluginVar(m_dwPluginID, VAR_NORMAL_COLOR, RGB(0, 255, 0));
	PC_SetPluginVar(m_dwPluginID, VAR_PAUSE_COLOR, RGB(255, 255, 0));
	PC_SetPluginVar(m_dwPluginID, VAR_RECORD_COLOR, RGB(255, 0, 0));
	PC_SetPluginVar(m_dwPluginID, VAR_SHOW_WHEN_NOT_RECORDING, 1);
	PC_SetPluginVar(m_dwPluginID, VAR_OPACITY, 255);
}

//
//	Update variables (after profile loading for example)
//
PLUGIN_EXPORT void PluginUpdateVars()
{
	DWORD clr;
	
	bShowWhenNotRecording = PC_GetPluginVarInt(m_dwPluginID, VAR_SHOW_WHEN_NOT_RECORDING);
	iOpacity = PC_GetPluginVarInt(m_dwPluginID, VAR_OPACITY);

	SAFE_DELETE(pNormalBrush);
	clr = PC_GetPluginVarInt(m_dwPluginID, VAR_NORMAL_COLOR);
	pNormalBrush = new Gdiplus::SolidBrush(Gdiplus::Color(iOpacity, GetRValue(clr), GetGValue(clr), GetBValue(clr)));

	SAFE_DELETE(pPauseBrush);
	clr = PC_GetPluginVarInt(m_dwPluginID, VAR_PAUSE_COLOR);
	pPauseBrush = new Gdiplus::SolidBrush(Gdiplus::Color(iOpacity, GetRValue(clr), GetGValue(clr), GetBValue(clr)));

	SAFE_DELETE(pRecordBrush);
	clr = PC_GetPluginVarInt(m_dwPluginID, VAR_RECORD_COLOR);
	pRecordBrush = new Gdiplus::SolidBrush(Gdiplus::Color(iOpacity, GetRValue(clr), GetGValue(clr), GetBValue(clr)));

}

//
//	Shutdown plugin
//
PLUGIN_EXPORT void PluginShutdown()
{
	SAFE_DELETE(pNormalBrush);
	SAFE_DELETE(pRecordBrush);

	SAFE_DELETE(pRenderHelper);
}


//
//	Draw overlay 
//
PLUGIN_EXPORT void PluginUpdateOverlay()
{
	if (!pRenderHelper)
		return;

	// lock overlay image
	auto pLock = pRenderHelper->BeginFrame();
	if (!pLock)
		return;

	int w = pLock->dwWidth;
	int h = pLock->dwHeight;

	Gdiplus::Graphics *pGraphics = pRenderHelper->GetGraphics();

	//-----------------------------------------
	// draw Recording Indicator


	// clear back
	pGraphics->Clear(Gdiplus::Color(0, 0, 0, 0));




	// draw Recording Indicator
	{
		Gdiplus::RectF bound;
		int circleSize = 0;
		if (w < h) {
			circleSize = w;
		}
		else {
			circleSize = h;
		}
		int squareSize = circleSize * 3 / 4;
		int squareStart = circleSize / 8;
		
		if (PC_IsPausedRecording()) {
			//pGraphics->FillEllipse(pPauseBrush, 0, 0, circleSize, circleSize);
			pGraphics->FillRectangle(pPauseBrush, squareStart, squareStart, squareSize/3, squareSize);
			pGraphics->FillRectangle(pPauseBrush, squareStart + squareSize / 3*2, squareStart, squareSize / 3, squareSize);
		} else if (PC_IsRecording()) {
			pGraphics->FillEllipse(pRecordBrush, 0, 0, circleSize, circleSize);
		}
		else if (bShowWhenNotRecording) {
			pGraphics->FillRectangle(pNormalBrush, squareStart, squareStart, squareSize, squareSize);
			//pGraphics->FillEllipse(pNormalBrush, 0, 0, circleSize, circleSize);
		}
	}



	// fill overlay image
	pRenderHelper->EndFrame();
}

//////////////////////////////////////////////////////////////////////////

DWORD dwNormalColor, dwPauseColor, dwRecordColor;
BOOL bItalic, bBold;
wstring szFontFamily;
DWORD dwFontSize;

static void InitSettingsDialog(HWND hwnd)
{
//	HWND ctrl;

	PC_LocalizeDialog(m_dwPluginID, hwnd);

	dwNormalColor = PC_GetPluginVarInt(m_dwPluginID, VAR_NORMAL_COLOR);
	dwPauseColor = PC_GetPluginVarInt(m_dwPluginID, VAR_PAUSE_COLOR);
	dwRecordColor = PC_GetPluginVarInt(m_dwPluginID, VAR_RECORD_COLOR);

	Button_SetCheck(GetDlgItem(hwnd, IDC_SHOW_WHEN_NOT_RECORDING), PC_GetPluginVarInt(m_dwPluginID, VAR_SHOW_WHEN_NOT_RECORDING) != 0 ? BST_CHECKED : BST_UNCHECKED);

	HWND slide = GetDlgItem(hwnd, IDC_OPACITY_SLIDER);
	SendMessageW(slide, TBM_SETRANGE, TRUE, MAKELONG(0, 100));
	SendMessageW(slide, TBM_SETPAGESIZE, 0, 10);
	SendMessageW(slide, TBM_SETTICFREQ, 10, 0);

	SendMessageW(slide, TBM_SETPOS, TRUE, iOpacity/255.0*100);
}

static void DrawColorButton(LPDRAWITEMSTRUCT lpDIS, COLORREF clr)
{
	HDC dc = lpDIS->hDC;
	UINT flag = DFCS_BUTTONPUSH;
	if (lpDIS->itemState & ODS_SELECTED)
		flag |= DFCS_PUSHED;
	DrawFrameControl(dc, &lpDIS->rcItem, DFC_BUTTON, flag);

	InflateRect(&lpDIS->rcItem, -3, -3);
	auto br = CreateSolidBrush(clr);
	FillRect(dc, &lpDIS->rcItem, br);
	DeleteObject(br);
}

static INT_PTR CALLBACK DlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	DWORD id;
	HWND slide = GetDlgItem(hwnd, IDC_OPACITY_SLIDER);
	HWND lbl = GetDlgItem(hwnd, IDC_LABEL);
	LRESULT pos;
	int opacity = 0;
	switch (uMsg)
	{
	case WM_COMMAND:
		id = LOWORD(wParam);
		if (id == IDOK || id == IDCANCEL)
		{
			if (id == IDOK)
			{
				PC_SetPluginVar(m_dwPluginID, VAR_NORMAL_COLOR, dwNormalColor);
				PC_SetPluginVar(m_dwPluginID, VAR_PAUSE_COLOR, dwPauseColor);
				PC_SetPluginVar(m_dwPluginID, VAR_RECORD_COLOR, dwRecordColor);

				PC_SetPluginVar(m_dwPluginID, VAR_SHOW_WHEN_NOT_RECORDING, Button_GetCheck(GetDlgItem(hwnd, IDC_SHOW_WHEN_NOT_RECORDING)) == BST_CHECKED);
				int style = (bBold ? 1 : 0) | (bItalic ? 2 : 0);

				pos = SendMessageW(slide, TBM_GETPOS, 0, 0);
				int opacity = (pos / 100.0) * 255;
				PC_SetPluginVar(m_dwPluginID, VAR_OPACITY, opacity);

			}

			EndDialog(hwnd, id);
		}

		if (id == IDC_NORMAL_COLOR_BTN || id == IDC_RECORD_COLOR_BTN || id == IDC_PAUSE_COLOR_BTN)
		{
			DWORD &color_var = (id == IDC_NORMAL_COLOR_BTN) ? dwNormalColor : (id == IDC_PAUSE_COLOR_BTN) ? dwPauseColor : dwRecordColor;
			COLORREF custColors[16];
			CHOOSECOLOR cc = { 0 };
			cc.lStructSize = sizeof(cc);
			cc.hwndOwner = hwnd;
			cc.rgbResult = color_var;
			cc.Flags = CC_FULLOPEN | CC_RGBINIT;
			cc.lpCustColors = custColors;
			if (ChooseColor(&cc))
			{
				color_var = cc.rgbResult;
				InvalidateRect(hwnd, 0, 0);
			}
		}
		break;

	case WM_NOTIFY:
		break;

	case WM_DRAWITEM:
	{
		auto lpDIS = (LPDRAWITEMSTRUCT)lParam;
		if (lpDIS->CtlID == IDC_NORMAL_COLOR_BTN)
			DrawColorButton(lpDIS, dwNormalColor);
		if (lpDIS->CtlID == IDC_PAUSE_COLOR_BTN)
			DrawColorButton(lpDIS, dwPauseColor);
		if (lpDIS->CtlID == IDC_RECORD_COLOR_BTN)
			DrawColorButton(lpDIS, dwRecordColor);
		break;
	}


	case WM_HSCROLL:
		pos = SendMessageW(slide, TBM_GETPOS, 0, 0);

		//int opacity = (int)(pos / 100) * 255;
		wchar_t buf[4];
		opacity = (pos / 100.0) * 255;
		wsprintfW(buf, L"%d", opacity);

		SetWindowTextW(lbl, buf);
		break;


	case WM_INITDIALOG:
		InitSettingsDialog(hwnd);
		return true;
	}

	return false;
}

//
//	Show plugin settings dialog
//
PLUGIN_EXPORT void PluginConfigure(HWND parent)
{
	DialogBox(GetDllInstance(), MAKEINTRESOURCE(IDD_RI_CONFIG_DLG), parent, DlgProc);
}