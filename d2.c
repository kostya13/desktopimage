/* Desktop Image

��������� ��� ���������� ����������� � ������� JPG/GIF/BMP � ��������� ����
�� ������� �����.

(�) �.���������. ���� 2009�.

*/

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <string.h>
#include <Objbase.h>
#include <ocidl.h>
#include <olectl.h>

#include "myimage.h"
#include "d2res.h"

HINSTANCE hinst;	// ��������� ���������
HWND hwndMain;		// ��������� �� ������� ���� ���������
HKEY hKey;			// ���� ������� ��� �������� ��������
HANDLE hImage;		// ������ ���������� ���������� �� �����

char url[1024];		// ����� ��� �������� �����
DWORD pcbData=1024; // ������� �������� ��������


// �������� ��������� ��������� �������

static BOOL CALLBACK DialogFunc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{

	switch (msg) {
	case WM_INITDIALOG:
		// ���������� ������ �� ������� � ���������� ������ ���� �������
			RegQueryValueEx (hKey, "url", NULL, NULL,  url, &pcbData);
			SetDlgItemText(hwndDlg, ID_URL,  url);
			return 1;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
			case IDOK:
				// ���� ������ ������ "���������", ��������� �������� url � �������...
				if (!GetDlgItemText(hwndDlg, ID_URL, url, 256))    *url=0;
					else
						RegSetValueEx(hKey,"url",0,REG_SZ,(BYTE*) url, strlen(url)+1);
				EndDialog(hwndDlg,1);
				return 1;
			case IDCANCEL:
				// ...����� ������ �� ������, ������ ��������� ������.
				EndDialog(hwndDlg,0);
				return 1;
		}
		break;
	case WM_CLOSE:
		EndDialog(hwndDlg,0);
		return TRUE;

	}
	return FALSE;
}


// ��������� �������� ����������

int OpenImage(HANDLE* hImage, char *url)
{
	pcbData=1024;
	hImage = OpenGraphic(url); //������� ������� ����

	// ��������� ���������
	// ���� ������� ���������, ���� ������, ����� � ����� �������� ������ ���������
	// ���� �� ����� ������� ���������� ��������
	while(!hImage)	{
			if( DialogBox(hinst, MAKEINTRESOURCE(IDD_MAINDIALOG),NULL, (DLGPROC)DialogFunc)==IDOK)
				{
					RegQueryValueEx (hKey, "url", NULL, NULL,  url, &pcbData);
					hImage = OpenGraphic(url);
				}
			else
			    {
				return 0;
				}
		}
	return 1;
}


// ��������� ��������� ������ ����

void MainWndProc_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{


	switch(id) {
		case IDM_CONFIG:
			// ����� ������� ���������
			if(	DialogBox(hinst, MAKEINTRESOURCE(IDD_MAINDIALOG), hwnd, (DLGPROC) DialogFunc)==IDOK)
			 {
				// ���� ������ ������ "���������", ��������� ������ � �������
				pcbData=1024;
				RegQueryValueEx (hKey, "url", NULL, NULL,  url, &pcbData);
				OpenImage(&hImage,url); // ������ ��������� ����������� � ����� �������
				InvalidateRect(hwnd,NULL,TRUE); // ���������� ������
			}
			break;
		case IDM_EXIT:
			PostMessage(hwnd,WM_CLOSE,0,0); // ������ �����
		break;
	}
}


// ������� ��������� ����

LRESULT CALLBACK MainWndProc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
{

	HDC hDC; 				// �������� ���������
	POINT p; 				// ��������� ��� ���������� ��������� ����
	HMENU hmenuTrackPopup; 	// ��������� ������������ ����
	PAINTSTRUCT PaintStruct;// ��������� ��� ���������
	HMENU popmenu;			// ����������� ����
	RECT rc; 				//��������� ��� ������� ��������� ����

	switch (msg)
    {
	case WM_COMMAND:
		// ���� ������� �������, �������� ��������������� ���������
		HANDLE_WM_COMMAND(hwnd,wParam,lParam,MainWndProc_OnCommand);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_NCHITTEST:
		// ������ ���, ����� ����� ���� ������� ���� �� ����� �����
		return HTCAPTION;
	case WM_NCRBUTTONUP:
		// ���� ������ ������ ������ ����, �������� ����
		popmenu=LoadMenu(hinst,MAKEINTRESOURCE(POPUPMENU));
		p.x = LOWORD(lParam); // ������������ ��������� ����
		p.y = HIWORD(lParam);
		hmenuTrackPopup = GetSubMenu(popmenu, 0); // ������� ������ ����
		TrackPopupMenuEx(hmenuTrackPopup,TPM_LEFTALIGN | TPM_RIGHTBUTTON,p.x,p.y,hwnd,NULL); // ������� ��� �� �����
		break;
	case WM_PAINT:
		// ��������� �����������
		hDC= BeginPaint (hwnd, &PaintStruct); // ��������� ���� ��������
		DisplayGraphic(hwnd,hDC); // ��������� ������������ ����������
		EndPaint(hwnd,&PaintStruct); // ��������� ��������
		break;
	case WM_CLOSE:
		// ��� �������� ��������� ���������� ���� � �������
		GetWindowRect(hwndMain,&rc);
		RegSetValueEx(hKey,"x",0,REG_DWORD,(BYTE*) &rc.left,sizeof(rc.left));
		RegSetValueEx(hKey,"y",0,REG_DWORD,(BYTE*) &rc.top,sizeof(rc.left));
	default:
		return DefWindowProc(hwnd,msg,wParam,lParam);
	}
	return 1;
}

// ����� ������� ������������

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hinstPrev, LPSTR lpCmdLine, int nCmdShow)
{
	WNDCLASS wc;			// ����� ����
	MSG msg;				// ��������� ����
	LONG lRet;				// ��������� ������ ������� �������
	int sizex, sizey;		// ������� ����
	DWORD x,y;				// ���������� ����
	DWORD lpdwDisposition;	// ����������� ���� ������ ����� ��� ������
	HBRUSH hbrBackground;	// ����� ��� ����
	COLORREF bgcolor;		// ��������� ����������� �����


	hinst = hInstance; // ������� ���������� ���������� ���������� ���������


	// ��������� ��� ������� ����� ������ � �������
	lRet = RegCreateKeyEx( HKEY_CURRENT_USER,"Software\\desktopimage\\", 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, &lpdwDisposition );
	if( lRet != ERROR_SUCCESS ){
		MessageBox(NULL,"������ �������� �������", "��������!", MB_OK);
		return FALSE;
	}

	// ���� ����� ����� - ������� �������� �� ���������
	if(lpdwDisposition==REG_CREATED_NEW_KEY)
	{
		x=100;
		y=100;
		strcpy(url,"http://");
		lRet = RegSetValueEx(hKey,"x",0,REG_DWORD,(BYTE*) &x,sizeof(x));
		RegSetValueEx(hKey,"y",0,REG_DWORD,(BYTE*) &y,sizeof(y));
		RegSetValueEx(hKey,"url",0,REG_SZ,(BYTE*) url, strlen(url)+1);
		// ������� ��������� � ������� �����������
		MessageBox(NULL,"������ ������ ���������!\n ������� ������ ���� ��� URL\n ������ �����������",
	   		 "��������� desktop image", MB_OK);
	}

	// ���� �������� ��� ���� ��������� �� � ��������������� ����������
	lRet = RegQueryValueEx (hKey, "url", NULL, NULL,  url, &pcbData);
	lRet = RegQueryValueEx (hKey, "x",NULL, NULL, &x, &pcbData);
	lRet = RegQueryValueEx (hKey, "y",NULL, NULL, &y, &pcbData);


	// ��������� �����������
	if (!OpenImage(&hImage, url)) return 0;

	// ���������� ������� �����������
	GetSize(&sizex, &sizey);

	// ������������� ���� � ����������� ��� ��� �����
	bgcolor=RGB(5, 75, 35);
	hbrBackground = CreateSolidBrush(bgcolor);

	// �������� ������ ��� �����
	memset(&wc,0,sizeof(wc));

	//�������������� �������� ������
	wc.lpfnWndProc = MainWndProc;
	wc.hInstance = hinst;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = hbrBackground;
	wc.lpszClassName = "dimg";
	RegisterClass(&wc);

	// ������� ����
	hwndMain = CreateWindowEx(WS_EX_LAYERED|WS_EX_TOOLWINDOW,
		"dimg","desktopimage",
		WS_POPUP,
		x,y,sizex,sizey,
		NULL,NULL,hinst,NULL);
	if (hwndMain == (HWND)0)	return 0;

	ShowWindow(hwndMain,SW_SHOW); // ���������� ���� �� ������
	SetLayeredWindowAttributes(hwndMain, bgcolor, 100, LWA_COLORKEY); //������������� ������������
	SetWindowPos(hwndMain,HWND_BOTTOM,x,y,sizex,sizey,SWP_NOMOVE|SWP_NOOWNERZORDER|SWP_NOREPOSITION); // � ��������� ���� �� ������ ����

	// ���� �� ������� ���������
	while (GetMessage(&msg,NULL,0,0)) {
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	// ��������� ����������, ������� �� ���������
	CloseImage(hImage);
	RegCloseKey( hKey );
	return msg.wParam;

}


