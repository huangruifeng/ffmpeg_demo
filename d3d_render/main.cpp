/*
 * @Author: huangruifeng
 * @Date: 2021-08-18 12:45:30
 * @LastEditTime: 2021-08-18 12:48:15
 * @LastEditors: huangruifeng
 * @FilePath: \ffmpeg_demo\d3d_render\main.cpp
 * @Description: 
 */


#define  DEBUG
 /**
  * \brief
  * \param x log message
  */
#define LOG(x)\
    std::stringstream ss;\
	ss << x;\
	std::cout<<  ss.str() << std::endl

#define LOG_ERROR(x) LOG(x)


#define LOG_INFO(x) LOG(x)

#ifdef DEBUG
#define LOG_DEBUG(x) LOG(x)
#else
#define LOG_DEBUG(x)
#endif

#include"d3d_renderer.h"
#include"d3d_capture.h"
#include <fstream>
#include<iostream>
#define USEYUV 1
#define CAPTURE 1

const int screen_w = 1920, screen_h = 1080;

D3dRenderer render;
D3dCapture capture;

LRESULT WINAPI MyWndProc(HWND hwnd, UINT msg, WPARAM wparma, LPARAM lparam)
{
    switch (msg) {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hwnd, msg, wparma, lparam);
}

int WINAPI WinMain(__in HINSTANCE hInstance, __in_opt HINSTANCE hPrevInstance, __in LPSTR lpCmdLine, __in int nShowCmd)
{
#if CAPTURE
    int size = GetSystemMetrics(SM_CXSCREEN)*GetSystemMetrics(SM_CYSCREEN) * 4;
    std::shared_ptr<uint8_t> data(new uint8_t[size]);
    #else
    #if USEYUV
    {
        std::shared_ptr<std::ifstream> file(new std::ifstream("test_yuv420p_320x180.yuv",std::ios::binary), [](auto *ptr) {
            ptr->close();
            delete ptr;
        });
        if (!file->is_open())
            return -1;
        int w = 320, h = 180;
        int length = w * h *3 /2;
        std::shared_ptr<uint8_t>data(new uint8_t[length]);
    }
    #endif
#endif
    WNDCLASSEX wc;
    ZeroMemory(&wc, sizeof(wc));

    wc.cbSize = sizeof(wc);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpfnWndProc = (WNDPROC)MyWndProc;
    wc.lpszClassName = L"D3D";
    wc.style = CS_HREDRAW | CS_VREDRAW;

    RegisterClassEx(&wc);

    HWND hwnd = NULL;
    hwnd = CreateWindow(L"D3D", L"Simplest Video Play Direct3D (Surface)", WS_OVERLAPPEDWINDOW, 100, 100, screen_w, screen_h, NULL, NULL, hInstance, NULL);
    if (hwnd == NULL) {
        return -1;
    }

    ShowWindow(hwnd, nShowCmd);
    UpdateWindow(hwnd);

    MSG msg;
    ZeroMemory(&msg, sizeof(msg));

    render.Init(hwnd, screen_w, screen_h);
    capture.Init(GetSystemMetrics(SM_CXSCREEN),GetSystemMetrics(SM_CYSCREEN));

    uint8_t tmp = 255;
    while (msg.message != WM_QUIT) {
        //PeekMessage, not GetMessage
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else {
            //Sleep(40);
#if CAPTURE
            if(capture.CaptureScreen(data,size))
            {
                render.RenderRGBA(data.get(), GetSystemMetrics(SM_CYSCREEN),GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CXSCREEN));
            }
#else
    #if !USEYUV
            int num = screen_h * screen_w * 4;
            std::shared_ptr<uint8_t>data(new uint8_t[num]);
            tmp--;
            for (int j = 0; j < screen_h; j++)
            {
                int num = j * screen_w * 4;
                for (int i = 0; i < screen_w * 4; i += 4)
                {
                    data.get()[num + i] = tmp; //b
                    data.get()[num +i + 1] = 128; //g
                    data.get()[num + i + 2] = tmp; //r
                    data.get()[num + i + 3] =   255; //a

                }
            }
            render.RenderRGBA(data.get(), screen_h, screen_w, screen_w);
    #else 
            file->read(reinterpret_cast<char*>(data.get()), length);
            
            if (file->eof())
            {
                file->clear();
                file->seekg(0);
            }
            auto u = data.get() + w * h;
            auto v = u+ w * h / 4;
            render.RenderYV12(w,h,data.get(),w,u,w/2,v,w/2);

    #endif
#endif
        }

        UnregisterClass(L"D3D", hInstance);

    }
    return 0;
}