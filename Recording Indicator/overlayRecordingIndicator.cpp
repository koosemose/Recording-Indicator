#include "stdafx.h"
#include "overlayRecordingIndicator.h"
#include "resource.h"



// add your own path for PlayClaw5.lib
#ifdef _DEBUG
#pragma comment(lib, "../playclaw5.lib")
#else
#pragma comment(lib, "../playclaw5.lib")
#endif

HINSTANCE GetDllInstance();

DWORD m_dwPluginID = -1;		// unique (realtime) plugin ID

PluginOverlayGdiPlusHelper	*pRenderHelper = 0;


Gdiplus::SolidBrush*	pFPSNormalBrush = 0;
Gdiplus::SolidBrush*	pFPSRecordBrush = 0;
//Gdiplus::SolidBrush*	pFPSBackBrush = 0;

BOOL					bShowWhenNotRecording = 0;
BOOL					bFirstAttach = 0;

//
//	Init plugin
//
PLUGIN_EXPORT DWORD PluginInit(DWORD dwPluginID)
{
	m_dwPluginID = dwPluginID;										

	// Initialize GDI+.
	pRenderHelper = new PluginOverlayGdiPlusHelper(dwPluginID);

	//pBackBrush = new Gdiplus::SolidBrush(Gdiplus::Color(100, 0, 0, 0));
	//pAxisPen = new Gdiplus::Pen(Gdiplus::Color(100, 0, 170, 70));
	//pGraphPen = new Gdiplus::Pen(Gdiplus::Color(255, 0, 200, 70));
	//pTextBrush = new Gdiplus::SolidBrush(Gdiplus::Color(255, 255, 255, 255));
	//pTextFont = new Gdiplus::Font(L"System", 8, Gdiplus::FontStyleBold);

	//pFPSBackBrush = new Gdiplus::SolidBrush(Gdiplus::Color(200, 0, 0, 0));
	bFirstAttach = true;
	return PC_PLUGIN_FLAG_IS_ACTIVE | PC_PLUGIN_FLAG_IS_OVERLAY;	// plugin is active and has overlay features
}

PLUGIN_EXPORT const char* PluginGetTitle()
{
	return "RECORDING_INDICATOR_PLUGIN";
}

//#define VAR_NORMAL_COLOR		"normal_color"
//#define VAR_RECORD_COLOR		"record_color"
//#define VAR_FPS_FONT_FAMILY		"fps_font.family"
//#define VAR_FPS_FONT_SIZE		"fps_font.size"
//#define VAR_FPS_FONT_STYLE		"fps_font.style"

// hack! :)
//#define profile_screenshot_indicator_color	"screenshot.indicator_color"
//#define profile_screenshot_indicator "screenshot.indicator"

//
//	Set default variables
//
PLUGIN_EXPORT void PluginSetDefaultVars()
{
//	PC_SetPluginVar(m_dwPluginID, OVR_VAR_POSITION_X, (int)0);
//	PC_SetPluginVar(m_dwPluginID, OVR_VAR_POSITION_Y, (int)0);
//	PC_SetPluginVar(m_dwPluginID, OVR_VAR_SIZE_X, (int)100);
//	PC_SetPluginVar(m_dwPluginID, OVR_VAR_SIZE_Y, (int)50);

	PC_SetPluginVar(m_dwPluginID, VAR_NORMAL_COLOR, RGB(255, 255, 0));
	PC_SetPluginVar(m_dwPluginID, VAR_RECORD_COLOR, RGB(255, 0, 0));
	PC_SetPluginVar(m_dwPluginID, VAR_SHOW_WHEN_NOT_RECORDING, 1);
}

//
//	Update variables (after profile loading for example)
//
PLUGIN_EXPORT void PluginUpdateVars()
{
	DWORD clr;
	
	SAFE_DELETE(pFPSNormalBrush);
	clr = PC_GetPluginVarInt(m_dwPluginID, VAR_NORMAL_COLOR);
	pFPSNormalBrush = new Gdiplus::SolidBrush(Gdiplus::Color(255, GetRValue(clr), GetGValue(clr), GetBValue(clr)));

	SAFE_DELETE(pFPSRecordBrush);
	clr = PC_GetPluginVarInt(m_dwPluginID, VAR_RECORD_COLOR);
	pFPSRecordBrush = new Gdiplus::SolidBrush(Gdiplus::Color(255, GetRValue(clr), GetGValue(clr), GetBValue(clr)));

	bShowWhenNotRecording = PC_GetPluginVarInt(m_dwPluginID, VAR_SHOW_WHEN_NOT_RECORDING);

}

//
//	Shutdown plugin
//
PLUGIN_EXPORT void PluginShutdown()
{
	SAFE_DELETE(pFPSNormalBrush);
	SAFE_DELETE(pFPSRecordBrush);

	SAFE_DELETE(pRenderHelper);
}


//
//	Draw overlay 
//
PLUGIN_EXPORT void PluginUpdateOverlay()
{
	if (!pRenderHelper)
		return;

	//SAFE_DELETE(pFPSNormalBrush);
	//pFPSNormalBrush = new Gdiplus::SolidBrush(Gdiplus::Color(255, 255, 255, 255));
	if (bFirstAttach) {
		bFirstAttach = false;
		PC_StartRecording();
		PC_DebugPrint(L"Triggered");
	}
	else {
		PC_DebugPrint(L"Not Triggered");
	}
	// lock overlay image
	auto pLock = pRenderHelper->BeginFrame();
	if (!pLock)
		return;

	int w = pLock->dwWidth;
	int h = pLock->dwHeight;

	Gdiplus::Graphics *pGraphics = pRenderHelper->GetGraphics();

	//-----------------------------------------
	// draw FPS counter

	// set options
	//pGraphics->SetTextRenderingHint(Gdiplus::TextRenderingHintAntiAlias);

	// clear back
	pGraphics->Clear(Gdiplus::Color(0, 0, 0, 0));




	// draw fps
	{
		Gdiplus::RectF bound;
		//bound.
		int circleSize = 0;
		if (w < h) {
			circleSize = w;
		}
		else {
			circleSize = h;
		}
		if (PC_IsRecording()) {
			pGraphics->FillEllipse(pFPSRecordBrush, 0, 0, circleSize, circleSize);
		}
		else if (bShowWhenNotRecording) {
			pGraphics->FillEllipse(pFPSNormalBrush, 0, 0, circleSize, circleSize);
		}
	}


	//	graphics.Flush(FlushIntentionSync);

	// fill overlay image
	pRenderHelper->EndFrame();
}

//////////////////////////////////////////////////////////////////////////

DWORD dwNormalColor, dwRecordColor;
BOOL bItalic, bBold;
wstring szFontFamily;
DWORD dwFontSize;

static void InitSettingsDialog(HWND hwnd)
{
//	HWND ctrl;

	PC_LocalizeDialog(m_dwPluginID, hwnd);

	dwNormalColor = PC_GetPluginVarInt(m_dwPluginID, VAR_NORMAL_COLOR);
	dwRecordColor = PC_GetPluginVarInt(m_dwPluginID, VAR_RECORD_COLOR);

	Button_SetCheck(GetDlgItem(hwnd, IDC_SHOW_WHEN_NOT_RECORDING), PC_GetPluginVarInt(m_dwPluginID, VAR_SHOW_WHEN_NOT_RECORDING) != 0 ? BST_CHECKED : BST_UNCHECKED);
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
//	NMHDR *p;


	switch (uMsg)
	{
	case WM_COMMAND:
		id = LOWORD(wParam);
		if (id == IDOK || id == IDCANCEL)
		{
			if (id == IDOK)
			{
				PC_SetPluginVar(m_dwPluginID, VAR_NORMAL_COLOR, dwNormalColor);
				PC_SetPluginVar(m_dwPluginID, VAR_RECORD_COLOR, dwRecordColor);

				PC_SetPluginVar(m_dwPluginID, VAR_SHOW_WHEN_NOT_RECORDING, Button_GetCheck(GetDlgItem(hwnd, IDC_SHOW_WHEN_NOT_RECORDING)) == BST_CHECKED);
				int style = (bBold ? 1 : 0) | (bItalic ? 2 : 0);
			}

			EndDialog(hwnd, id);
		}

		if (id == IDC_NORMAL_COLOR_BTN || id == IDC_RECORD_COLOR_BTN)
		{
			DWORD &color_var = (id == IDC_NORMAL_COLOR_BTN) ? dwNormalColor : dwRecordColor;
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
		if (lpDIS->CtlID == IDC_RECORD_COLOR_BTN)
			DrawColorButton(lpDIS, dwRecordColor);
		break;
	}


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
	DialogBox(GetDllInstance(), MAKEINTRESOURCE(IDD_FPS_CONFIG_DLG), parent, DlgProc);
}