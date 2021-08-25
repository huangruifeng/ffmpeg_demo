#pragma once
#include<d3d9.h>
#include<memory>

#pragma comment(lib, "d3d9.lib");

class D3dCapture {
    template<class T>
    struct D3dDeleter {
        void operator()(T* ptr) {
            ptr->Release();
        }
    };
    template<class T>
    using d3d_unique_ptr = std::unique_ptr<T, D3dDeleter<T>>;

public:
    bool Init( size_t width, size_t height)
    {
        m_pDirect3D9.reset(Direct3DCreate9(D3D_SDK_VERSION));
        if (!m_pDirect3D9)
            return false;

        auto d3dpp = GetD3dpresentParameters(GetDesktopWindow(), width, height);

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
            0,
            //D3DCREATE_MULTITHREADED(线程安全) 如果应用程序在一个线程中处理窗口消息，同时在另一个线程中进行 Direct3D API 调用，则该应用程序在创建设备时必须使用此标志
            //D3DCREATE_FPU_PRESERVE 将 Direct3D 浮点计算的精度设置为调用线程使用的精度。如果不指定此标志，Direct3D 默认为单精度舍入到最近模式
            vertexProcessing | D3DCREATE_MULTITHREADED | D3DCREATE_FPU_PRESERVE,
            &d3dpp, &device);
        if (FAILED(ret))
        {
            //todo log 
            m_pDirect3D9 = nullptr;
            return false;
        }
        m_pDirect3DDevice.reset(device);
        return CreateSurface(width,height, D3DFMT_A8R8G8B8);
    }

    bool CaptureScreen(const std::shared_ptr<uint8_t>& data,int size)
    {
        if(m_pDirect3D9 && m_pDirect3DDevice && m_pDirect3DSurface)
        {
            m_pDirect3DDevice->GetFrontBufferData(0,m_pDirect3DSurface.get());

            D3DLOCKED_RECT rect;
            ZeroMemory(&rect, sizeof(rect));
            if(SUCCEEDED(m_pDirect3DSurface->LockRect(&rect,0,0)))
            {
                memcpy(data.get(),rect.pBits,size);
                m_pDirect3DSurface->UnlockRect();
                return true;
            }
        }
        
        return false;
    }

private:
    bool CreateSurface(size_t width, size_t height, D3DFORMAT fmt)
    {
        //todo is support fmt
        IDirect3DSurface9* surface = nullptr;
        auto err = m_pDirect3DDevice->CreateOffscreenPlainSurface(width, height, fmt, D3DPOOL_SCRATCH, &surface, nullptr);
        if (FAILED(err))
        {
            //todo log 
            return false;
        }
        else {
            m_pDirect3DSurface.reset(surface);
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
        锁定后台缓冲区
        */
        d3dpp.Flags  = D3DPRESENTFLAG_LOCKABLE_BACKBUFFER;

        d3dpp.hDeviceWindow = (HWND)(window);

        return d3dpp;
    }
    d3d_unique_ptr<IDirect3D9> m_pDirect3D9;
    d3d_unique_ptr<IDirect3DDevice9> m_pDirect3DDevice;
    d3d_unique_ptr<IDirect3DSurface9>m_pDirect3DSurface;

};