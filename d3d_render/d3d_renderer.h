#pragma once

#include <Windows.h>
#include <d3d9.h>
#include <memory>
#include <mutex>

#pragma comment(lib, "d3d9.lib");

#define D3DFMT_YV12 (D3DFORMAT)MAKEFOURCC('Y', 'V', '1', '2')

// This file defines the arraysize() macro and is derived from Chromium's
// base/macros.h.

// The arraysize(arr) macro returns the # of elements in an array arr.
// The expression is a compile-time constant, and therefore can be
// used in defining new arrays, for example.  If you use arraysize on
// a pointer by mistake, you will get a compile-time error.

// This template function declaration is used in defining arraysize.
// Note that the function doesn't need an implementation, as we only
// use its type.
template <typename T, size_t N> char(&ArraySizeHelper(T(&array)[N]))[N];

#define arraysize(array) (sizeof(ArraySizeHelper(array)))

D3DFORMAT SupportSurfaceFormats[] = { 
    D3DFMT_YV12,
    D3DFMT_X8R8G8B8
};

class D3dRenderer
{
    template<class T>
    struct D3dDeleter {
        void operator()(T* ptr) {
            ptr->Release();
        }
    };
    template<class T>
    using d3d_unique_ptr = std::unique_ptr<T, D3dDeleter<T>>;

public:

    bool Init(void* window, size_t width, size_t height)
    {
        GetClientRect((HWND)window, &m_rtViewport);
        //std::lock_guard<std::mutex> lock(m_mutex);
        m_pDirect3D9.reset(Direct3DCreate9(D3D_SDK_VERSION));
        if (!m_pDirect3D9)
            return false;

        auto d3dpp = GetD3dpresentParameters(window, width, height);

        D3DCAPS9 caps;
        DWORD vertexProcessing;
        m_pDirect3D9->GetDeviceCaps(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, &caps);
        if ((caps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT) == D3DDEVCAPS_HWTRANSFORMANDLIGHT)
        {
            //指定硬件顶点处理
            vertexProcessing = D3DCREATE_HARDWARE_VERTEXPROCESSING;
        }
        else
        {
            //指定混合（软件和硬件）顶点处理。对于 Windows 10 版本 1607 及更高版本，不建议使用此设置
            vertexProcessing = D3DCREATE_SOFTWARE_VERTEXPROCESSING;
        }

        IDirect3DDevice9* device;
        HRESULT ret = m_pDirect3D9->CreateDevice(D3DADAPTER_DEFAULT,
            D3DDEVTYPE_HAL,
            (HWND)(window),
            //D3DCREATE_MULTITHREADED(线程安全) 如果应用程序在一个线程中处理窗口消息，同时在另一个线程中进行 Direct3D API 调用，则该应用程序在创建设备时必须使用此标志
            //D3DCREATE_FPU_PRESERVE 将 Direct3D 浮点计算的精度设置为调用线程使用的精度。如果不指定此标志，Direct3D 默认为单精度舍入到最近模式
            vertexProcessing | D3DCREATE_MULTITHREADED | D3DCREATE_FPU_PRESERVE,
            &d3dpp, &device);
        if (FAILED(ret))
        {
            //todo log 
            m_hwnd = nullptr;
            m_pDirect3D9 = nullptr;
            return false;
        }
        m_pDirect3DDevice.reset(device);

        m_pDirect3DDevice->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0);
        m_pDirect3DDevice->Present(NULL, NULL, NULL, NULL);
        return true;
    }

    bool RenderYV12(int width,int height, uint8_t* y,int strideY, uint8_t* u, int strideU, uint8_t*v, int strideV)
    {
        if (!RenderParameterCheck(width, height, D3DFMT_YV12))
        {
            return false;
        }

        D3DLOCKED_RECT d3dRect;
        auto hr = m_pDirect3DSurfaceRender->LockRect(&d3dRect, NULL, D3DLOCK_DONOTWAIT);
        if (FAILED(hr))
        {
            //todo log 
            return false;
        }
        const int deststride = d3dRect.Pitch;
        const auto destY = static_cast<uint8_t*>(d3dRect.pBits);
        uint8_t* destU = destY + deststride * m_surfaceHeight;
        uint8_t* destV = destU + deststride * m_surfaceHeight / 4;

        if (y != nullptr) {
            for (int i = 0; i < height; i++)
            {
                memcpy(destY + i * deststride, y + i * strideY, strideY);
            }
        }
        int strideUV = deststride / 2;
        if (u != nullptr) {
            for (int i = 0; i < height / 2; i++)
            {
                memcpy(destU + i * strideUV, v + i * strideU, strideU);
            }
        }

        if (v != nullptr) {
            for (int i = 0; i < height / 2; i++)
            {
                memcpy(destV + i * strideUV, u + i * strideU, strideV);
            }
        }

        hr = m_pDirect3DSurfaceRender->UnlockRect();
        if (FAILED(hr)) 
        {
            //todo log 
            return false;
        }

        m_pDirect3DDevice->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0);
        m_pDirect3DDevice->BeginScene();
        IDirect3DSurface9 * pBackBuffer = NULL;
        m_pDirect3DDevice->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &pBackBuffer);
        m_pDirect3DDevice->StretchRect(m_pDirect3DSurfaceRender.get(), NULL, pBackBuffer, NULL, D3DTEXF_LINEAR);
        m_pDirect3DDevice->EndScene();

        m_pDirect3DDevice->Present(NULL, NULL, NULL, NULL);
        pBackBuffer->Release();

        return true;
    }

    bool RenderRGBA(uint8_t* data,int height,int width,int stride)
    {
        if (!RenderParameterCheck(width, height, D3DFMT_X8R8G8B8))
        {
            return false;
        }

        D3DLOCKED_RECT d3dRect;
        auto hr = m_pDirect3DSurfaceRender->LockRect(&d3dRect, NULL, D3DLOCK_DONOTWAIT);
        if (FAILED(hr))
        {
            //todo log 
            return false;
        }
        int deststride = d3dRect.Pitch;
        int channel = 4;
        uint8_t* dest = reinterpret_cast<uint8_t*>(d3dRect.pBits);
        uint8_t* src = data;
        for (int i = 0; i < height; i++)
        {
            memcpy(dest, src, width*channel);
            dest += deststride;
            src += stride* channel;
        }
        hr = m_pDirect3DSurfaceRender->UnlockRect();
        if (FAILED(hr))
        {
            //todo log 
            return false;
        }



        m_pDirect3DDevice->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0);
        m_pDirect3DDevice->BeginScene();
        IDirect3DSurface9 * pBackBuffer = NULL;
        m_pDirect3DDevice->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &pBackBuffer);
        m_pDirect3DDevice->StretchRect(m_pDirect3DSurfaceRender.get(), NULL, pBackBuffer, NULL, D3DTEXF_LINEAR);
        m_pDirect3DDevice->EndScene();

        m_pDirect3DDevice->Present(NULL, NULL, NULL, NULL);
        pBackBuffer->Release();

        return true;
    }
    virtual ~D3dRenderer()
    {
    };

private:
    bool RenderParameterCheck(size_t width, size_t height, D3DFORMAT fmt)
    {
        if (m_backBufferHeight != height || m_backBufferWidth != width)
        {
            auto d3dPar = GetD3dpresentParameters(m_hwnd, width, height);
            HRESULT error = m_pDirect3DDevice->Reset(&d3dPar);
            if (FAILED(error))
            {
                //todo log
                return false;
            }

        }
        return CreateSurface(width, height, fmt);;
    }

    bool CreateSurface(size_t width, size_t height, D3DFORMAT fmt)
    {
        if (width != m_surfaceWidth || height != m_surfaceHeight)
        {
            //todo is support fmt
            IDirect3DSurface9* surface = nullptr;
            auto err = m_pDirect3DDevice->CreateOffscreenPlainSurface(width, height, fmt, D3DPOOL_DEFAULT, &surface, nullptr);
            if (FAILED(err))
            {
                //todo log 
                return false;
            }
            else {
                m_surfaceFormat = fmt;
                m_surfaceWidth = width;
                m_surfaceHeight = height;
                m_pDirect3DSurfaceRender.reset(surface);
            }
        }
        return true;
    }

    D3DPRESENT_PARAMETERS GetD3dpresentParameters(void* window, size_t width, size_t height)
    {
        //https://blog.csdn.net/leixiaohua1020/article/details/40279297
        D3DPRESENT_PARAMETERS d3dpp;
        ZeroMemory(&d3dpp, sizeof(d3dpp));
        d3dpp.Windowed = TRUE; //指定窗口模式。True = 窗口模式；False = 全屏模式
        /*
        指定系统如何将后台缓冲区的内容复制到前台缓冲区，从而在屏幕上显示。它的值有：
        D3DSWAPEFFECT_DISCARD:清除后台缓存的内容
        D3DSWAPEEFECT_FLIP:保留后台缓存的内容。当缓存区>1时。
        D3DSWAPEFFECT_COPY: 保留后台缓存的内容，缓冲区=1时。
        一般情况下使用D3DSWAPEFFECT_DISCARD
        */
        d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;

        /*
        后备缓冲区使用的颜色模式。即颜色深度和格式。
        如果使用全屏模式，可以使用设备支持的任何颜色模式。使用CheckDeviceType方法来检查。
        如果使用窗口模式，则必须使用当前窗口使用的颜色模式。可以使用D3DFMT_UNKOWN，系统会自动获取该值。
        */
        d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;

        d3dpp.hDeviceWindow = (HWND)(window);

        /*
            如果使用全屏模式 使用EnumAdapterModes
            如果使用窗口模式，且width,height为0则根据窗口大小适配。
        */
        d3dpp.BackBufferHeight = height;
        d3dpp.BackBufferHeight = width;
        m_backBufferHeight = height;
        m_backBufferWidth = width;
        return d3dpp;
    }
private:
    HWND m_hwnd;
    int m_surfaceHeight;
    int m_surfaceWidth;
    size_t m_backBufferWidth;
    size_t m_backBufferHeight;
    D3DFORMAT m_surfaceFormat;
    d3d_unique_ptr<IDirect3D9> m_pDirect3D9;
    d3d_unique_ptr<IDirect3DDevice9> m_pDirect3DDevice;
    d3d_unique_ptr<IDirect3DSurface9>m_pDirect3DSurfaceRender;
    std::mutex m_mutex;
    RECT m_rtViewport;
};