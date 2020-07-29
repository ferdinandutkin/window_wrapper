// WindowsProject3.cpp : Определяет точку входа для приложения.
//

#include "framework.h"
#include "Header.h"
#include "WindowsProject3.h"

#define MAX_LOADSTRING 100

// Глобальные переменные:
HINSTANCE hInst;                                // текущий экземпляр
WCHAR szTitle[MAX_LOADSTRING];                  // Текст строки заголовка
WCHAR szWindowClass[MAX_LOADSTRING];            // имя класса главного окна

// Отправить объявления функций, включенных в этот модуль кода:
 
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
  

 
    static window wnd;

   

    wnd.on_regular_message(
        std::make_pair(WM_MOUSEMOVE, WM_SIZE), [] {
            wnd.window_name = L"Moving";
        }
        );



 
   

  

    wnd.create(L"eeee", WS_OVERLAPPEDWINDOW);
   

    return window::pump_messages();
    /*
    // Цикл основного сообщения:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
    */
}

 
