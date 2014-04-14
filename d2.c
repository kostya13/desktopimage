/* Desktop Image

Программа для размещения изображения в формате JPG/GIF/BMP в отдельном окне
на рабочем столе.

(с) К.Ильяшенко. Июнь 2009г.

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

HINSTANCE hinst;	// экземпляр программы
HWND hwndMain;		// указатель на главное окно программы
HKEY hKey;			// ключ реестра для хранения значений
HANDLE hImage;		// объект изобажения выводимого на экран

char url[1024];		// буфер для хранения строк
DWORD pcbData=1024; // счетчик принятых символов


// Основная процедура обработки диалога

static BOOL CALLBACK DialogFunc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{

	switch (msg) {
	case WM_INITDIALOG:
		// считывание данных из реестра и сохранение строки окне диалога
			RegQueryValueEx (hKey, "url", NULL, NULL,  url, &pcbData);
			SetDlgItemText(hwndDlg, ID_URL,  url);
			return 1;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
			case IDOK:
				// если нажата кнопка "сохранить", сохраняем значение url в реестре...
				if (!GetDlgItemText(hwndDlg, ID_URL, url, 256))    *url=0;
					else
						RegSetValueEx(hKey,"url",0,REG_SZ,(BYTE*) url, strlen(url)+1);
				EndDialog(hwndDlg,1);
				return 1;
			case IDCANCEL:
				// ...иначе ничего не делаем, просто закрываем диалог.
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


// процедура открытия изобажения

int OpenImage(HANDLE* hImage, char *url)
{
	pcbData=1024;
	hImage = OpenGraphic(url); //попытка открыть файл

	// проверяем результат
	// если открыто правильно, идем дальше, иначе в цикле вызываем диалог настройки
	// пока не будет введено корректное значение
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


// процедура обработки команд меню

void MainWndProc_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{


	switch(id) {
		case IDM_CONFIG:
			// вызов диалога настройки
			if(	DialogBox(hinst, MAKEINTRESOURCE(IDD_MAINDIALOG), hwnd, (DLGPROC) DialogFunc)==IDOK)
			 {
				// если нажата кнопка "сохранить", сохраняем данные в реестре
				pcbData=1024;
				RegQueryValueEx (hKey, "url", NULL, NULL,  url, &pcbData);
				OpenImage(&hImage,url); // заново открываем изображения с новым адресом
				InvalidateRect(hwnd,NULL,TRUE); // обновление кэрана
			}
			break;
		case IDM_EXIT:
			PostMessage(hwnd,WM_CLOSE,0,0); // просто выход
		break;
	}
}


// главная процедура окна

LRESULT CALLBACK MainWndProc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
{

	HDC hDC; 				// контекст рисования
	POINT p; 				// стурктура для сохранения коордиант мыши
	HMENU hmenuTrackPopup; 	// экземпляр всплывающего меню
	PAINTSTRUCT PaintStruct;// структура для рисования
	HMENU popmenu;			// всплывающее меню
	RECT rc; 				//структура для хранния координат окна

	switch (msg)
    {
	case WM_COMMAND:
		// если выбрана команда, вызываем соответствующую процедуру
		HANDLE_WM_COMMAND(hwnd,wParam,lParam,MainWndProc_OnCommand);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_NCHITTEST:
		// делаем так, чтобы можно было таскать окно за любое место
		return HTCAPTION;
	case WM_NCRBUTTONUP:
		// если нажата правая кнопка мыши, вызываем меню
		popmenu=LoadMenu(hinst,MAKEINTRESOURCE(POPUPMENU));
		p.x = LOWORD(lParam); // опеределение координат мыши
		p.y = HIWORD(lParam);
		hmenuTrackPopup = GetSubMenu(popmenu, 0); // находим нужное меню
		TrackPopupMenuEx(hmenuTrackPopup,TPM_LEFTALIGN | TPM_RIGHTBUTTON,p.x,p.y,hwnd,NULL); // выводим его на экран
		break;
	case WM_PAINT:
		// отрисовка изобаржения
		hDC= BeginPaint (hwnd, &PaintStruct); // опредляем куда рисовать
		DisplayGraphic(hwnd,hDC); // вызываемп подпрограмму рендеринга
		EndPaint(hwnd,&PaintStruct); // закрываем контекст
		break;
	case WM_CLOSE:
		// при закрытии сохраняем координаты окна в реестре
		GetWindowRect(hwndMain,&rc);
		RegSetValueEx(hKey,"x",0,REG_DWORD,(BYTE*) &rc.left,sizeof(rc.left));
		RegSetValueEx(hKey,"y",0,REG_DWORD,(BYTE*) &rc.top,sizeof(rc.left));
	default:
		return DefWindowProc(hwnd,msg,wParam,lParam);
	}
	return 1;
}

// САМАЯ ГЛАВНАЯ подпрограмма

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hinstPrev, LPSTR lpCmdLine, int nCmdShow)
{
	WNDCLASS wc;			// класс окна
	MSG msg;				// сообщения окна
	LONG lRet;				// результат работы функций реестра
	int sizex, sizey;		// размеры окна
	DWORD x,y;				// координаты окна
	DWORD lpdwDisposition;	// определение типа записи новая или старая
	HBRUSH hbrBackground;	// кисть для фона
	COLORREF bgcolor;		// структура определения цвета


	hinst = hInstance; // создаем глобальную переменную экземпляра программы


	// открываем или создаем новуб веипку в реестре
	lRet = RegCreateKeyEx( HKEY_CURRENT_USER,"Software\\desktopimage\\", 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, &lpdwDisposition );
	if( lRet != ERROR_SUCCESS ){
		MessageBox(NULL,"Ошибка открытия реестра", "Внимание!", MB_OK);
		return FALSE;
	}

	// если ветка новая - заносим значения по умолчанию
	if(lpdwDisposition==REG_CREATED_NEW_KEY)
	{
		x=100;
		y=100;
		strcpy(url,"http://");
		lRet = RegSetValueEx(hKey,"x",0,REG_DWORD,(BYTE*) &x,sizeof(x));
		RegSetValueEx(hKey,"y",0,REG_DWORD,(BYTE*) &y,sizeof(y));
		RegSetValueEx(hKey,"url",0,REG_SZ,(BYTE*) url, strlen(url)+1);
		// выводим сообщения с краткой инструкцией
		MessageBox(NULL,"Первый запуск программы!\n Введите полный путь или URL\n вашего изображения",
	   		 "Сообщение desktop image", MB_OK);
	}

	// если значения уже были сохраняем их в соответствующие переменные
	lRet = RegQueryValueEx (hKey, "url", NULL, NULL,  url, &pcbData);
	lRet = RegQueryValueEx (hKey, "x",NULL, NULL, &x, &pcbData);
	lRet = RegQueryValueEx (hKey, "y",NULL, NULL, &y, &pcbData);


	// открываем изображение
	if (!OpenImage(&hImage, url)) return 0;

	// определяем размеры изображения
	GetSize(&sizex, &sizey);

	// устанавливаем цвет и присваиваем его для кисти
	bgcolor=RGB(5, 75, 35);
	hbrBackground = CreateSolidBrush(bgcolor);

	// выделяем память под класс
	memset(&wc,0,sizeof(wc));

	//инициализируем значения класса
	wc.lpfnWndProc = MainWndProc;
	wc.hInstance = hinst;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = hbrBackground;
	wc.lpszClassName = "dimg";
	RegisterClass(&wc);

	// создаем окно
	hwndMain = CreateWindowEx(WS_EX_LAYERED|WS_EX_TOOLWINDOW,
		"dimg","desktopimage",
		WS_POPUP,
		x,y,sizex,sizey,
		NULL,NULL,hinst,NULL);
	if (hwndMain == (HWND)0)	return 0;

	ShowWindow(hwndMain,SW_SHOW); // показываем окно на экране
	SetLayeredWindowAttributes(hwndMain, bgcolor, 100, LWA_COLORKEY); //устанавливаем прозрачность
	SetWindowPos(hwndMain,HWND_BOTTOM,x,y,sizex,sizey,SWP_NOMOVE|SWP_NOOWNERZORDER|SWP_NOREPOSITION); // и переводим окно на задний план

	// цикл по очереди сообщений
	while (GetMessage(&msg,NULL,0,0)) {
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	// закрываем переменные, выходим из программы
	CloseImage(hImage);
	RegCloseKey( hKey );
	return msg.wParam;

}


