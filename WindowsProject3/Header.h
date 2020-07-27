#include "framework.h"
#include"properties.h"



#include <string>
#include <variant>
#include <functional>
#include <unordered_map>

template<class... Ts> struct visiter : Ts... { using Ts::operator()...; };
template<class... Ts> visiter(Ts...)->visiter<Ts...>;



class window {
private:
    HINSTANCE instance;
    std::wstring win_name;
    std::wstring class_name;

    using message_funcs_t = std::variant<std::function<void(WPARAM, LPARAM)>,
        std::function<void(WPARAM)>, std::function<void(LPARAM)>, std::function<void()>>;

    using command_funcs_t = std::variant<std::function<void(LPARAM)>, std::function<void()>>;

    using keydown_funcs_t = command_funcs_t;


    std::unordered_map < int, message_funcs_t> message_actions{
            {WM_DESTROY, [] {::PostQuitMessage(0); }}
    };

    std::unordered_map < int, command_funcs_t> command_actions;

    std::unordered_map<int, keydown_funcs_t> keydown_actions; // < ------- 





    void message_caller(message_funcs_t to_call, WPARAM wparam, LPARAM lparam) {
        std::visit(visiter{
           [&](std::function<void(WPARAM, LPARAM)> action) {action(lparam, wparam); },
           [&](std::function<void(WPARAM) > action) {action(wparam); },
           [&](std::function<void(LPARAM) > action) {action(lparam); },
           [&](std::function<void() > action) {action(); }
            }, to_call);

    }

    void commad_caller(command_funcs_t to_call, LPARAM lparam) {
        std::visit(visiter{
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

    template <typename Func>
    void on_regular_message(int message, Func action) {
        message_actions[message] = std::function(action);
    }


    template <typename Func, typename... Rest>
    void on_regular_message(int message, Func action, Rest... rest) {
        on_regular_message(message, action);
        on_regular_message(rest...);
    }



    template <typename Func>
    void on_create(Func action) {
        message_actions[WM_CREATE] = std::function(action);
    }
    

    template <typename Func, typename... Rest>
    void on_command(int command, Func action, Rest... rest) {
        on_command(command, action);
        on_command(rest...);

    }

    template <typename Func>
    void on_command(int command, Func action) {
        command_actions[command] = std::function(action);
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
