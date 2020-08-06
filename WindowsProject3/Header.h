#include "framework.h"

#include"properties.h"

#include <type_traits>

#include <string>
#include <variant>
#include <functional>
#include <unordered_map>
#include <concepts>


inline int get_x(LPARAM l) {
    return  ((int)(short)LOWORD(l));
}


inline int get_y(LPARAM l) {
    return  ((int)(short)HIWORD(l));
}
template<typename... Ts> struct visitor : Ts... { using Ts::operator()...; };
template<typename... Ts> visitor(Ts...)->visitor<Ts...>;


template <typename T>
concept integral_pair = std::is_integral_v<typename T::first_type> and std::is_integral_v<typename T::second_type> and std::is_same_v<std::pair<typename T::first_type, typename T::second_type>, T>;;

template<typename T>
concept integral_range = integral_pair<T> or std::is_integral_v<T>;

template<typename T>
using common_pair_type = std::common_type_t<typename T::first_type, typename T::second_type > ;

 

class dc {
private:
    HDC device_context{};
    HWND window{};
public:
    dc()  {};
    dc(HWND window) : window(window) {
   
        device_context = GetDC(window);
        SelectObject(device_context, GetStockObject(DC_PEN));
        SelectObject(device_context, GetStockObject(DC_BRUSH));
    }
    ~dc() {
        ReleaseDC(window, device_context);
    }

    //верхний левый
    //правый нижний


    template<typename... Types>
    auto bezier(Types... args) {
        return PolyBezier(device_context, { args... }, sizeof...(args));
    }
   BOOL circle(int x, int y, int r) {
        return Ellipse(device_context, x - r, y - r, x + r, y + r);

    }
    bool rectangle(int x1, int y1, int x2, int y2) {
        return Rectangle(device_context, x1, y1, x2, y2);
    }

    bool line(int x1, int y1, int x2, int y2) {
        MoveToEx(device_context, x1, y1, nullptr); //сделать текущими координаты x1, y1
        return LineTo(device_context, x2, y2);
    }
    
    COLORREF set_pixel(int x, int y, COLORREF color = 0ul) {
        return SetPixel(device_context, x, y, color);
    }

    COLORREF set_pen_color(COLORREF color) {
        return SetDCPenColor(device_context, color);
    }


    COLORREF set_brush_color(COLORREF color) {
        SetDCBrushColor(device_context, color);
    }

    

    operator HDC() const noexcept { return device_context; }

};
 



class window {
private:
    HINSTANCE instance;
    std::wstring win_name;
    std::wstring class_name;

    using message_funcs_t = std::variant<std::function<void(WPARAM, LPARAM)>,
        std::function<void(WPARAM)>, std::function<void(LPARAM)>, std::function<void()>>;

    using command_funcs_t = std::variant<std::function<void(LPARAM)>, std::function<void()>>;

    using keydown_funcs_t = command_funcs_t;


    std::unordered_map <long long int, message_funcs_t> message_actions{
            {WM_DESTROY, [] {::PostQuitMessage(0); }}
    };

    std::unordered_map <long long int, command_funcs_t> command_actions;

    std::unordered_map<long long int, keydown_funcs_t> keydown_actions; // < ------- 





    void message_caller(const message_funcs_t& to_call, WPARAM wparam, LPARAM lparam) {
        std::visit(visitor{
           [&](std::function<void(WPARAM, LPARAM)> action) {action(lparam, wparam); },
           [&](std::function<void(WPARAM) > action) {action(wparam); },
           [&](std::function<void(LPARAM) > action) {action(lparam); },
           [&](std::function<void() > action) {action(); }
            }, to_call);

    }

    void commad_caller(const command_funcs_t& to_call, LPARAM lparam) {
        std::visit(visitor{
            [&](std::function<void(LPARAM) > action) {action(lparam); },
            [&](std::function<void() > action) {action(); }
            }, to_call);

    }




public:


    HWND handle{};  //смерть абстракции

    prop<std::wstring> window_name{ win_name,
        [this](std::wstring value) {
            win_name = value;
            ::SetWindowTextW(handle, win_name.c_str());
        }
    };


    prop<RECT> client_rect{
        [this] {
            RECT rect;
            GetClientRect(handle, &rect);
            return rect;
        },
        [this](RECT rect) {
            AdjustWindowRectEx(&rect, GetWindowLongW(handle, GWL_STYLE), reinterpret_cast<BOOL>(GetMenu(handle)), GetWindowLongW(handle, GWL_EXSTYLE));
            RECT window_rect;
            GetWindowRect(handle, &window_rect);
            MoveWindow(handle, window_rect.left, window_rect.top, rect.right - rect.left, rect.bottom - rect.top, true);

        }
    };





    


    template <integral_pair Range, typename Func, typename... Rest>
    void on_regular_message(Range&& message, Func&& action, Rest&&... rest) {
        on_regular_message(std::forward<Range>(message), std::forward<Func>(action));
        on_regular_message(std::forward<Rest>(rest)...);
    }

    template <typename Func, typename... Rest>
    void on_regular_message(long long int message, Func&& action, Rest&&... rest) {
        on_regular_message(std::forward<long long int>(message), std::forward<Func>(action));
        on_regular_message(std::forward<Rest>(rest)...);
    }

    template <typename Func>
    void on_regular_message(long long int message, Func&& action) {
        message_actions[message] = std::function(std::forward<Func>(action));
    }

    template <integral_pair Range, typename Func>
    void on_regular_message(Range range, Func&& action) {
        for (long long int message = range.first; message <= range.second; message++) {
            message_actions[message] = std::function(std::forward<Func>(action));
        }

    }

  




    template <typename Func>
    void on_create(Func&& action) {
        message_actions[WM_CREATE] = std::function(std::forward<Func>(action));
    }


    template <typename Func>
    void on_paint(Func&& action) {
        message_actions[WM_PAINT] = std::function(std::forward<Func>(action));
    }


    template <typename Func>
    void on_mouse_move(Func&& action) {
        message_actions[WM_MOUSEMOVE] = std::function(std::forward<Func>(action));

    }

    template <typename CoordsFunc> requires std::is_convertible_v < CoordsFunc, std::function<void(int, int)>>
    void on_mouse_move(CoordsFunc&& action) {
        
        message_actions[WM_MOUSEMOVE] = std::function([&](LPARAM l) {
            action(get_x(l), get_y(l));
            });

    }



    template <typename Func>
    void on_key_down(Func&& action) {
        message_actions[WM_KEYDOWN] = std::function(std::forward<Func>(action));
    }

    template <typename KeyFunc> requires std::is_convertible_v <KeyFunc, std::function<void(char)>>
    void on_key_down(KeyFunc&& action) {
        message_actions[WM_KEYDOWN] = std::function([&](WPARAM w) {
            action(w);
            });
    }















    template <integral_pair Range, typename Func, typename... Rest>  
    void on_command(Range&& command, Func&& action, Rest&&... rest) {
        on_command(std::forward<Range>(command), std::forward<Func>(action));
        on_command(std::forward<Rest>(rest)...);

    }

    template <typename Func, typename... Rest>
    void on_command(long long int command, Func&& action, Rest&&... rest) {
        on_command(std::forward<long long int>(command), std::forward<Func>(action));
        on_command(std::forward<Rest>(rest)...);

    }


    template <typename Func>
    void on_command(long long int command, Func&& action) {
        command_actions[command] = std::function(std::forward<Func>(action));
    }

    template <integral_pair Range, typename Func>
    void on_command(Range&& range, Func&& action) { //[first; last]
        for (long long int command = range.first; command <= range.second; command++) {
            command_actions[command] = std::function(std::forward<Func>(action));
        }
    }



   



    LRESULT procedure(UINT msg, WPARAM wparam, LPARAM lparam) {


        if (msg == WM_COMMAND) {
            if (int command = LOWORD(wparam); command_actions.contains(command)) {

                auto action = command_actions[command];
                commad_caller(action, lparam);
                return 0;
            }
            else return ::DefWindowProcW(handle, msg, wparam, lparam);
        }


        else if (message_actions.contains(msg)) {

            auto action = message_actions[msg];
            message_caller(action, wparam, lparam);
            return 0;

        }
        else return ::DefWindowProcW(handle, msg, wparam, lparam);
    }


    window() : class_name(L"Sample class name"), instance(::GetModuleHandleW(nullptr)) {

        WNDCLASSEX wcex{ sizeof(WNDCLASSEX) };

        if (bool not_registered = !::GetClassInfoExW(instance, class_name.c_str(), &wcex)) {

            wcex.style = CS_HREDRAW | CS_VREDRAW;

            wcex.lpfnWndProc = [](HWND handle, UINT msg, WPARAM wparam, LPARAM lparam) {
                if (msg == WM_NCCREATE) [[unlikely]] {
                    auto cs = reinterpret_cast<CREATESTRUCTW*>(lparam);

                    auto owner = static_cast<window*>(cs->lpCreateParams);
                    owner->handle = handle;

                    ::SetWindowLongW(handle, 0, reinterpret_cast<LONG_PTR>(owner));
                }
                else if (auto owner = reinterpret_cast<window*>(::GetWindowLongW(handle, 0))) [[likely]] {
                        return owner->procedure(msg, wparam, lparam);
                }
                return ::DefWindowProcW(handle, msg, wparam, lparam);
            };

            wcex.cbWndExtra = sizeof(this);

            wcex.hInstance = instance;

            wcex.hIcon = LoadIcon((HINSTANCE)NULL,
                IDI_APPLICATION);

            wcex.hCursor = LoadCursor((HINSTANCE)NULL,
                IDC_ARROW);

            wcex.hbrBackground = (HBRUSH)::GetStockObject(WHITE_BRUSH);

            wcex.lpszClassName = class_name.c_str();

            ::RegisterClassExW(&wcex);

        }
    }

    window(WNDCLASSEXW wcex) : class_name(wcex.lpszClassName), instance(wcex.hInstance) {

        wcex.lpfnWndProc = [](HWND handle, UINT msg, WPARAM wparam, LPARAM lparam) {
            if (msg == WM_NCCREATE) [[unlikely]] {
                auto cs = reinterpret_cast<CREATESTRUCTW*>(lparam);

                auto owner = static_cast<window*>(cs->lpCreateParams);
                owner->handle = handle;

                ::SetWindowLongW(handle, 0, reinterpret_cast<LONG_PTR>(owner));
            }
            else if (auto owner = reinterpret_cast<window*>(::GetWindowLongW(handle, 0))) [[likely]] {
                    return owner->procedure(msg, wparam, lparam);
            }
            return ::DefWindowProcW(handle, msg, wparam, lparam);
        };

        wcex.cbWndExtra = sizeof(this);

        ::RegisterClassExW(&wcex);
    }

    void redraw() {}

    void show() const {
        ::ShowWindow(handle, SW_SHOW);
        ::UpdateWindow(handle);

    }
    void create(const wchar_t* window_name,
        DWORD style, int width = CW_USEDEFAULT, int height = CW_USEDEFAULT,
        int x = CW_USEDEFAULT, int y = CW_USEDEFAULT,
        DWORD ex_style = 0ul, HWND parent = nullptr,
        HMENU menu = nullptr) {

        this->window_name = window_name;

        handle = ::CreateWindowExW(ex_style, class_name.c_str(), window_name, style, x, y,
            width, height, parent, menu, instance, this);

        show();
    }

    operator HWND() const { return handle; };

    void resize(int height, int widht) {
        ::SetWindowPos(handle, nullptr, 0, 0, widht, height, SWP_NOMOVE);
    }

    static int pump_messages() {
        MSG msg{};
        while (int result = ::GetMessageW(&msg, nullptr, 0, 0)) {
            if (result == -1) {
                throw std::runtime_error("Critical error");
            }
            ::TranslateMessage(&msg);
            ::DispatchMessageW(&msg);
        }
        return static_cast<int>(msg.wParam);
    }






};
