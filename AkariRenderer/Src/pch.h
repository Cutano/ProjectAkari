# pragma once

#include <winsdkver.h>
#define _WIN32_WINNT 0x0A00
#include <sdkddkver.h>

// Use the C++ standard templated min/max
#define NOMINMAX

// DirectX apps don't need GDI
#define NODRAWTEXT
#define NOGDI
#define NOBITMAP

// Include <mcx.h> if you need this
#define NOMCX

// Include <winsvc.h> if you need this
#define NOSERVICE

// WinHelp is deprecated
#define NOHELP

#define WIN32_LEAN_AND_MEAN

#include <Windows.h>

#include <wrl/client.h>
#include <wrl/event.h>

#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include <DirectXColors.h>
#include <DirectXCollision.h>

#include <d3d12.h>
#include <dxgi1_6.h>
#include "d3dx12.h"
#ifdef _DEBUG
    #include <dxgidebug.h>
#endif

#define D3D12_GPU_VIRTUAL_ADDRESS_NULL      ((D3D12_GPU_VIRTUAL_ADDRESS)0)
#define D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN   ((D3D12_GPU_VIRTUAL_ADDRESS)-1)
#define MY_IID_PPV_ARGS                     IID_PPV_ARGS


#include <cstdint>
#include <cstdio>
#include <cstdarg>
#include <cwctype>
#include <exception>
#include <memory>
#include <vector>
#include <string>
#include <array>
#include <unordered_map>
#include <algorithm>
#include <random>
#include <fstream>
#include <functional>
#include <ppltasks.h>

#include <spdlog/spdlog.h>

extern "C" {
inline __declspec(dllexport) extern constexpr UINT D3D12SDKVersion = 606;
inline __declspec(dllexport) extern const char8_t* D3D12SDKPath = u8".\\D3D12\\";
}
