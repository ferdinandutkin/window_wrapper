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
  
    window wnd;

    int x{}, y{};

 
    wnd.on_paint([&] {
          dc context{ wnd };
          context.set_pixel(x, y, RGB(255, 0, 0));
        });


    wnd.on_mouse_move([&](int x_cord, int y_cord) {
        x = x_cord;
        y = y_cord;
        wnd.window_name = std::to_wstring(x) + L"    " + std::to_wstring(y);
        });

    wnd.create(L"Test" ,WS_OVERLAPPEDWINDOW);




    return window::pump_messages();
   
}

 
