#include "framework.h"
#include "Header.h"
#include "WindowsProject3.h"

 

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
   
    
    window wnd;

    int x{0}, y{10};

    COLORREF color{};

    wnd.on_paint([&] {
          dc context{ wnd };
          x++;
          y = (sin(x) * 100) + 10;
          
          
          context.set_pixel(x, y);
         

        });


    /*wnd.on_mouse_move([&](int x_cord, int y_cord) {
        x = x_cord;
        y = y_cord;
        wnd.window_name = std::to_wstring(x) + L"    " + std::to_wstring(y);
        });
        */
    wnd.on_key_down([&](char key) {
        switch (key) {
        case '1': color = RGB(255, 0, 0); break;
        case '2': color = RGB(0, 255, 0); break;
        case '3':  color = RGB(0, 0, 255); break;
        }

        });



    wnd.create(L"Test" ,WS_OVERLAPPEDWINDOW);




    return window::pump_messages();
   
}

 
