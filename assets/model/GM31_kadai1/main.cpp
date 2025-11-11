#ifndef UNICODE
#define UNICODE
#endif 

#include <windows.h>
#include <random>
#include "insightcheck.h"
#include "drawgdi.h"

constexpr int CIRCLENUM = 50; // 円の個数

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
    // Register the window class.
    const wchar_t CLASS_NAME[] = L"Sample Window Class";

    WNDCLASS wc = { };

    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;

    RegisterClass(&wc);

    // Create the window.

    HWND hwnd = CreateWindowEx(
        0,                              // Optional window styles.
        CLASS_NAME,                     // Window class
        L"視野範囲内チェック（点）",    // Window text
        WS_OVERLAPPEDWINDOW,            // Window style

        // Size and position
        CW_USEDEFAULT, CW_USEDEFAULT, 800, 800,

        NULL,       // Parent window    
        NULL,       // Menu
        hInstance,  // Instance handle
        NULL        // Additional application data
    );

    if (hwnd == NULL)
    {
        return 0;
    }

    ShowWindow(hwnd, nCmdShow);

    // Run the message loop.

    MSG msg = { };
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    std::mt19937 rnd;
    std::uniform_real_distribution<float> distwidth(10,700);
    std::uniform_real_distribution<float> distheight(10, 700);

    static DirectX::SimpleMath::Vector3 p[CIRCLENUM];
    static bool flag[CIRCLENUM];

    static HPEN     redpen;
    static HPEN     greenpen;
    static HPEN     oldpen;

    DirectX::SimpleMath::Vector3 eye(400, 400, 0);          // initial eye
    DirectX::SimpleMath::Vector3 lookat(600, 600, 0);       // initial lookat

    DirectX::SimpleMath::Vector3 rotlookat;                 // rotation lookat

    static float angle = 0.0f;

    switch (uMsg)
    {
    case WM_TIMER:
        InvalidateRect(hwnd, nullptr, false);
        break;
    case WM_CREATE:

        SetTimer(hwnd, 1, 100, nullptr);

        redpen = CreatePen(PS_SOLID, 2, RGB(255, 0, 0));
        greenpen = CreatePen(PS_SOLID, 2, RGB(0, 255, 0));

        for (int i = 0; i < CIRCLENUM; i++) {
            flag[i] = false;
            p[i].x = distwidth(rnd);
            p[i].y = distheight(rnd);
            p[i].z = 0;
        }

        break;

    case WM_DESTROY:

        KillTimer(hwnd, 1);

        DeleteObject(redpen);

        PostQuitMessage(0);
        return 0;

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);

        FillRect(hdc, &ps.rcPaint, (HBRUSH)(COLOR_WINDOW + 1));

        // 注視点　視点を中心に回転
        DirectX::SimpleMath::Matrix mtx;
        mtx = mtx.CreateRotationZ((DirectX::XM_PI*angle) / 180.0f);//回転スピード
        angle += 1.0f;
        rotlookat = lookat.Transform((lookat-eye), mtx);

        rotlookat += eye;

        // 視野角回転
        mtx = mtx.CreateRotationZ(DirectX::XM_PI / 4.0f);//ここを4.5にすると脱法でいけるで

        DirectX::SimpleMath::Vector3 vecrotz;
        vecrotz = (rotlookat-eye).Transform((rotlookat - eye), mtx);

        DirectX::SimpleMath::Vector3 minusvecrotz;
        mtx = mtx.CreateRotationZ(-DirectX::XM_PI / 4.0f);
        minusvecrotz = (rotlookat - eye).Transform((rotlookat - eye), mtx);

        // 視点と注視点描画
        oldpen = (HPEN)SelectObject(hdc, redpen);
        DrawCircle(10, eye, hdc);
        DrawCircle(10, rotlookat, hdc);
 
        DrawCircle(10.0f, (eye + vecrotz), hdc);
        DrawCircle(10.0f, (eye + minusvecrotz), hdc);

        DrawLine(eye, rotlookat,hdc);
        DrawLine(eye, (eye+vecrotz),hdc);

        DrawLine(eye, (eye + minusvecrotz), hdc);

        for (int i = 0; i < CIRCLENUM; i++) {
            bool sts;

            sts = InsightCheckXZwithCircle(
                eye,
                rotlookat,
                DirectX::XM_PI / 2.0f,                          // FOV
                p[i],                                           // check point
                20.0f,                                          // circle radius
                300.0f);                                        // view length

            if (sts) {
                SelectObject(hdc, greenpen);
                DrawCircle(20.0f, p[i],hdc);
            }
            else {
                SelectObject(hdc, oldpen);
                DrawCircle(20.0f, p[i], hdc);
            }
        }

        EndPaint(hwnd, &ps);
    }
    return 0;

    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}