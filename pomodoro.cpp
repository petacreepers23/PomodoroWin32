#include <windows.h>
#include <string>

// Constants
const char c_windowsClassName[] = "pomodoroClass";
const char c_fontName[] = "Broadway";
const COLORREF c_uninitializedBg = RGB(255,255,255);
const COLORREF c_focusBg = RGB(255, 160, 160);
const COLORREF c_shortBreakBg = RGB(160, 255, 160);
const COLORREF c_longBreakBg = RGB(160, 160, 255);

// Globals
UINT_PTR g_timer_id;
COLORREF g_text_background;
HFONT g_chrono_font;
HFONT g_status_font; // In the future this 2 vars will be calculated from a status one
std::string g_status_text = "Push a status button to start!";
bool g_timer_running = false;
int g_seconds_left = 0;

// Enums
enum botones 
{
    BTN_Null, BTN_Focus, BTN_Short, BTN_Long, BTN_StartStop, BTN_NullEnd
};

// Functions

std::string CalculateNewTimerText() 
{
    int minutes = g_seconds_left / 60;
    int seconds = g_seconds_left % 60;
    return (minutes < 10 ? "0" : "") + std::to_string(minutes) + ":" + (seconds < 10 ? "0" : "") + std::to_string(seconds);
}

void DoCountdownAndUpdateText(HWND hwnd, UINT, UINT_PTR, DWORD)
{
    if (!g_timer_running)
    {
        return;
    }

    InvalidateRect(hwnd, NULL, FALSE);
    
    if (g_seconds_left>0) 
    {
        g_seconds_left--;
    }

    CalculateNewTimerText();

    if (g_seconds_left == 0) 
    {
        g_timer_running = false;
	    Beep(440,2000);
    }
}

void UpdateText(HWND hwnd, const char* texto) 
{
    RECT rc;

    if (!GetUpdateRect(hwnd, &rc, FALSE))
    {
          return;
    }

    PAINTSTRUCT ps;
    BeginPaint(hwnd, &ps);
    
    SelectObject(ps.hdc, g_chrono_font);
    SetBkColor(ps.hdc, g_text_background); 
    DrawText(ps.hdc, texto, -1, &rc, DT_CENTER|DT_VCENTER|DT_SINGLELINE);

    SelectObject(ps.hdc, g_status_font);
    DrawText(ps.hdc, g_status_text.c_str(), -1, &rc, DT_LEFT|DT_BOTTOM|DT_SINGLELINE);

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
    
    CreateWindow(TEXT("button"), TEXT("Stop / Go"),    
		             WS_VISIBLE | WS_CHILD ,
		             250, 250, 100, 50,        
		             hwnd, (HMENU) BTN_StartStop, NULL, NULL);  
}

void HandleStateButtonPress (HWND hwnd,std::string newStatus, COLORREF newBg, int newTime) 
{
    g_timer_running = true;
    g_seconds_left = newTime;
    g_text_background = newBg;

    HBRUSH brush;

    brush = CreateSolidBrush(newBg);
    SetClassLongPtr(hwnd, GCLP_HBRBACKGROUND, (LONG_PTR)brush);
    g_status_text = newStatus;
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
            HandleStateButtonPress(hwnd, "Stay focused!", c_focusBg, 1500);
            break;
        case BTN_Short:
            HandleStateButtonPress(hwnd, "Go away from screen!", c_shortBreakBg, 300);
            break;
        case BTN_Long:
            HandleStateButtonPress(hwnd, "Relax a bit", c_longBreakBg, 900);
            break;
        case BTN_StartStop:
            g_timer_running = !g_timer_running;
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
            KillTimer(hwnd,g_timer_id);
            PostQuitMessage(0);
            break;
        case WM_PAINT:
            UpdateText(hwnd, CalculateNewTimerText().c_str());
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
    wc.lpszClassName = c_windowsClassName;
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
    g_timer_id = SetTimer(hwnd,0,1000,DoCountdownAndUpdateText);

    if(!g_timer_id)
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
        c_windowsClassName,
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
    strcpy(logFont.lfFaceName, c_fontName);
    g_chrono_font = CreateFontIndirect(&logFont);

    logFont.lfHeight = -14;
    logFont.lfWeight = FW_NORMAL;
    g_status_font = CreateFontIndirect(&logFont);

    g_text_background = c_uninitializedBg;
}

// Main

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
