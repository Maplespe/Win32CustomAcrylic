// Win32CustomAcrylic.cpp : 定义应用程序的入口点。
//

#include "framework.h"
#include "Win32CustomAcrylic.h"
#include "CustomAcrylic.h"
#include <olectl.h>
#include <gdiplus.h>
#pragma comment(lib, "gdiplus.lib")

HINSTANCE hInst;

auto m_className = L"Win32CustomAcrylic";
auto m_wndTitle = m_className;

//GDI资源
HBITMAP m_hBitmap = NULL;
HDC m_hDC = NULL;
void* m_hBmpPtr = nullptr;
Gdiplus::Graphics* m_graphics = nullptr;

CustomAcrylic m_Acrylic;

ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
void                UpdateWindowDC(int width, int height);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    MyRegisterClass(hInstance);

    // 执行应用程序初始化:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return (int) msg.wParam;
}


ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_WIN32CUSTOMACRYLIC));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = NULL;
    wcex.lpszClassName  = m_className;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance;

   //GDI+ 初始化 GDIPlus initialize
   ULONG_PTR gdiplusToken = 0;
   Gdiplus::GdiplusStartupInput gdiplusStartupInput;
   Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
   UpdateWindowDC(800, 600);

   HWND hWnd = CreateWindowExW(WS_EX_LAYERED, m_className, m_wndTitle, WS_POPUPWINDOW,
      CW_USEDEFAULT, 0, 800, 600, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   m_Acrylic.Initialize(hWnd);


   SendMessageW(hWnd, WM_PAINT, 0, 0);

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);

            RECT wRect;
            GetWindowRect(hWnd, &wRect);
            BLENDFUNCTION bf = { AC_SRC_OVER, 0, 255, AC_SRC_ALPHA };
            SIZE wndSize = { wRect.right - wRect.left, wRect.bottom - wRect.top };
            POINT ptPos, ptSrc = { 0, 0 };
            ptPos = { wRect.left, wRect.top };

            static Gdiplus::SolidBrush brush(Gdiplus::Color(120, 255, 255, 255));
            m_graphics->Clear(Gdiplus::Color::Transparent);
            m_graphics->FillRectangle(&brush, 0, 0, INT(wndSize.cx), INT(wndSize.cy));

            UpdateLayeredWindow(hWnd, 0, &ptPos, &wndSize, m_hDC, &ptSrc, 0, &bf, ULW_ALPHA);

            EndPaint(hWnd, &ps);
        }
        break;
    case WM_SIZE:
        RECT wRect;
        GetWindowRect(hWnd, &wRect);
        UpdateWindowDC(wRect.right - wRect.left, wRect.bottom - wRect.top);
        SendMessageW(hWnd, WM_PAINT, 0, 0);
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    case WM_LBUTTONDOWN:
        SendMessageW(hWnd, WM_NCLBUTTONDOWN, HTCAPTION, 0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

void UpdateWindowDC(int width, int height)
{
    static int size[2] = { 0, 0 };
    if (width != size[0] || height != size[1])
    {
        size[0] = width;
        size[1] = height;

        if (m_hDC)
            DeleteDC(m_hDC);
        if (m_hBitmap)
            DeleteObject(m_hBitmap);
        if (m_graphics)
            delete m_graphics;

        m_hDC = CreateCompatibleDC(NULL);

        BITMAPINFO bmi;
        memset(&bmi, 0, sizeof(bmi));
        bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        bmi.bmiHeader.biWidth = width;
        bmi.bmiHeader.biHeight = -height;
        bmi.bmiHeader.biPlanes = 1;
        bmi.bmiHeader.biBitCount = 32;
        bmi.bmiHeader.biCompression = BI_RGB;
        bmi.bmiHeader.biSizeImage = width * height * 32 * 8;

        m_hBitmap = CreateDIBSection(m_hDC, &bmi, DIB_RGB_COLORS, &m_hBmpPtr, 0, 0);

        if (m_hDC)
        {
            memset(m_hBmpPtr, 0, width * height * 4);
            DeleteObject(SelectObject(m_hDC, m_hBitmap));
            m_graphics = new Gdiplus::Graphics(m_hDC);
        }
    }
}