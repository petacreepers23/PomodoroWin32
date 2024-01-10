#include <windows.h>
#include <string>

const char g_szClassName[] = "pomodoroClass";
std::string g_texto;
UINT_PTR g_timerId;
HFONT g_hfont;

enum botones {BTN_Null, BTN_Focus, BTN_Short, BTN_Long, BTN_StartStop, BTN_NullEnd};

//l_ stands for logic
bool l_timer_running = false;
int l_seconds_left = 0;

void CalculateNewTimerText() {
    int minutes = l_seconds_left / 60;
    int seconds = l_seconds_left % 60;
    g_texto = std::to_string(minutes) + ":" + (seconds < 10 ? "0" : "") + std::to_string(seconds);
}

void DoCountdownAndUpdateText(HWND hwnd, UINT, UINT_PTR, DWORD)
{
    if (!l_timer_running)
    {
        return;
    }

    InvalidateRect(hwnd, NULL, FALSE);
    
    if (l_seconds_left>0) 
    {
        l_seconds_left--;
    }
    else 
    {
        Beep(440,2000);
    }

    CalculateNewTimerText();

    if (l_seconds_left == 0) {
        l_timer_running = false;
    }
}

void UpdateText(HWND hwnd, const char* texto) 
{
    RECT rc;
    if (!GetUpdateRect(hwnd, &rc, FALSE))
          return;
    PAINTSTRUCT ps;
    BeginPaint(hwnd, &ps);

    // set text color and font
    //COLORREF oldTextColor = SetTextColor(_hdc, textColor);
    HFONT oldHFont = (HFONT)SelectObject(ps.hdc, g_hfont);

    //TextOut(_hdc, _Xpos, _Ypos, _szMessage, strlen(_szMessage));
    DrawText(ps.hdc, texto, -1, &rc, DT_CENTER|DT_VCENTER|DT_SINGLELINE);

    // restore text colorand font
    //SetTextColor(_hdc, oldTextColor);
    SelectObject(ps.hdc, oldHFont); 

    EndPaint(hwnd, &ps);
}

void CreateMenuButtons(HWND hwnd) 
{
    CreateWindow(TEXT("button"), TEXT("Focus"),    
		             WS_VISIBLE | WS_CHILD ,
		             50, 50, 100, 50,        
		             hwnd, (HMENU) BTN_Focus, NULL, NULL);  

    CreateWindow(TEXT("button"), TEXT("Short Break"),    
		             WS_VISIBLE | WS_CHILD ,
		             250, 50, 100, 50,        
		             hwnd, (HMENU) BTN_Short, NULL, NULL);  

    CreateWindow(TEXT("button"), TEXT("Long Break"),    
		             WS_VISIBLE | WS_CHILD ,
		             450, 50, 100, 50,        
		             hwnd, (HMENU) BTN_Long, NULL, NULL);  
    
    CreateWindow(TEXT("button"), TEXT("Start/Stop"),    
		             WS_VISIBLE | WS_CHILD ,
		             250, 250, 100, 50,        
		             hwnd, (HMENU) BTN_StartStop, NULL, NULL);  
}

void HandleMenuPress (HWND hwnd, int wParam)
{
    if (wParam > BTN_Null && wParam < BTN_NullEnd)
    {
        InvalidateRect(hwnd, NULL, TRUE);
    }

    switch(wParam)
    {
        case BTN_Focus:
            g_texto = "25:00";
            l_seconds_left = 1500;
            break;
        case BTN_Short:
            g_texto = "5:00";
            l_seconds_left = 300;
            break;
        case BTN_Long:
            g_texto = "15:00";
            l_seconds_left = 900;
            break;
        case BTN_StartStop:
            l_timer_running = !l_timer_running;
            break;
        default:
            break;
    }

}

//Window Procedure
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch(msg)
    {
        case WM_CREATE:
            CreateMenuButtons(hwnd);
            break;
        case WM_COMMAND:
            HandleMenuPress(hwnd, LOWORD(wParam));
            break;
        case WM_CLOSE:
            DestroyWindow(hwnd);
            break;
        case WM_DESTROY:
            KillTimer(hwnd,g_timerId);
            PostQuitMessage(0);
            break;
        case WM_PAINT:
            UpdateText(hwnd, g_texto.c_str());
            break;
        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

void RegisterPomodoroClass (HINSTANCE hInstance) 
{
    WNDCLASSEX wc;

    wc.cbSize        = sizeof(WNDCLASSEX);
    wc.style         = 0;
    wc.lpfnWndProc   = WndProc;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 0;
    wc.hInstance     = hInstance;
    wc.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
    wc.lpszMenuName  = NULL;
    wc.lpszClassName = g_szClassName;
    wc.hIconSm       = LoadIcon(NULL, IDI_APPLICATION);

    if(!RegisterClassEx(&wc))
    {
        MessageBox(NULL, "Window Registration Failed!", "Error!",
            MB_ICONEXCLAMATION | MB_OK);
        exit(-1);
    }

}

void DefinePeriodicTimer(HWND hwnd)
{
    //This is unreliable, not real time, just that you know.
    g_timerId = SetTimer(hwnd,0,1000,DoCountdownAndUpdateText);

    if(!g_timerId)
    {
        MessageBox(NULL, "Timer Creation Failed!", "Error!",
            MB_ICONEXCLAMATION | MB_OK);
        exit(-1);
    }
}

void CreatePomodoroWindow(HINSTANCE hInstance, int nCmdShow)
{
    HWND hwnd;

    hwnd = CreateWindowEx(
        WS_EX_CLIENTEDGE,
        g_szClassName,
        "Pomodoro",
        WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME,
        CW_USEDEFAULT, CW_USEDEFAULT, 625, 400,
        NULL, NULL, hInstance, NULL);

    if(hwnd == NULL)
    {
        MessageBox(NULL, "Window Creation Failed!", "Error!",
            MB_ICONEXCLAMATION | MB_OK);
        exit(-1);
    }

    ShowWindow(hwnd, nCmdShow);

    UpdateWindow(hwnd);

    DefinePeriodicTimer(hwnd);

    RECT rect;

    GetWindowRect(hwnd,&rect);

    SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
}

int DoPomodoroMsgLoopUntilExit()
{
    MSG Msg;

    while(GetMessage(&Msg, NULL, 0, 0) > 0)
    {
        TranslateMessage(&Msg);
        DispatchMessage(&Msg);
    }

    return Msg.wParam;
}

void DefinePomodoroFont()
{
    LOGFONT logFont;
    memset(&logFont, 0, sizeof(logFont));
    logFont.lfHeight = -72; // see PS
    logFont.lfWeight = FW_BOLD;
    strcpy(logFont.lfFaceName, "Broadway");
    g_hfont = CreateFontIndirect(&logFont);
    g_texto = "0:00";
}

// Compile with -lgdi32
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
    LPSTR lpCmdLine, int nCmdShow)
{
    FreeConsole(); //Close unwanted window, i dont want to mess with linker

    RegisterPomodoroClass(hInstance);

    DefinePomodoroFont();

    CreatePomodoroWindow(hInstance,nCmdShow);

    return DoPomodoroMsgLoopUntilExit();
}
