#include "dx11base.h"

//DXTRACE_MSG 在 Debug(_DEBUG/DEBUG) 下会展开为 DXTraceW/DXTraceA，
//这两个函数实现在 DirectX SDK 的 dxerr.lib 中，必须显式链接，否则会报 LNK2019/LNK1120
#pragma comment(lib, "dxerr.lib")
//dxerr.lib 是用旧版 CRT 编译的，会引用新版 UCRT 已移除的 __vsnwprintf，
//需要由 legacy_stdio_definitions.lib 提供，否则链接阶段仍会报 LNK2019/LNK1120
#pragma comment(lib, "legacy_stdio_definitions.lib")
//D3DX11CompileFromFile 的实现在 d3dx11.lib（Debug 版为 d3dx11d.lib）中
#if defined(DEBUG) || defined(_DEBUG)
#pragma comment(lib, "d3dx11d.lib")
#else
#pragma comment(lib, "d3dx11.lib")
#endif

#pragma comment(lib,"dinput8.lib")

Dx11Base::Dx11Base():driverType_(D3D_DRIVER_TYPE_NULL), featureLevel_(D3D_FEATURE_LEVEL_11_0),
					d3dDevice_(0), d3dContext_(0), swapChain_(0), backBufferTarget_(0)
{

}
Dx11Base::~Dx11Base()
{
	Shutdown();
}

bool Dx11Base::Initialize(HINSTANCE hInstance, HWND hwnd)
{
	hInstance_ = hInstance;
	hwnd_ = hwnd;

	//创建窗口
	RECT dimensions;
	GetClientRect(hwnd, &dimensions);
	unsigned int width = dimensions.right - dimensions.left;
	unsigned int height = dimensions.bottom - dimensions.top;

	D3D_DRIVER_TYPE driverTypes[] = {
		D3D_DRIVER_TYPE_HARDWARE,
		D3D_DRIVER_TYPE_WARP,
		D3D_DRIVER_TYPE_REFERENCE,
		D3D_DRIVER_TYPE_SOFTWARE
	};
	const unsigned int totalDriverTypes = static_cast<unsigned int>(ARRAYSIZE(driverTypes));
	D3D_FEATURE_LEVEL featureLevels[] = {
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0
	};
	const unsigned int totalFeatureLevels = static_cast<unsigned int>(ARRAYSIZE(featureLevels));

	//交换链设置
	DXGI_SWAP_CHAIN_DESC swapChain;
	ZeroMemory(&swapChain, sizeof(swapChain));
	swapChain.BufferCount = 1;
	swapChain.BufferDesc.Width = width;
	swapChain.BufferDesc.Height = height;
	swapChain.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChain.BufferDesc.RefreshRate.Numerator = 60;
	swapChain.BufferDesc.RefreshRate.Denominator = 1;
	swapChain.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChain.OutputWindow = hwnd;
	swapChain.Windowed = true;
	swapChain.SampleDesc.Count = 1;
	swapChain.SampleDesc.Quality = 0;

	unsigned int creationFlags = 0;

#ifdef _DEBUG
	creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	HRESULT result = E_FAIL;
	for (unsigned int driver = 0; driver < totalDriverTypes; ++driver)
	{
		result = D3D11CreateDeviceAndSwapChain(0, driverTypes[driver], 0, creationFlags,
			featureLevels, totalFeatureLevels, D3D11_SDK_VERSION, &swapChain, &swapChain_,
			&d3dDevice_, &featureLevel_, &d3dContext_);

		if (SUCCEEDED(result))   //创建成功就停止尝试，否则会重复创建多个设备/交换链并且泄漏
		{
			driverType_ = driverTypes[driver];
			break;
		}

	}
	if (FAILED(result))
	{
		DXTRACE_MSG(L"Failed to create the Direct3D device!");
		return false;
	}

	//渲染
	ID3D11Texture2D* backBufferTexture;
	result = swapChain_->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backBufferTexture);
	if (FAILED(result))
	{
		DXTRACE_MSG(L"Failed to get the swap chain back buffer!");
		return false;
	}
	result = d3dDevice_->CreateRenderTargetView(backBufferTexture, 0, &backBufferTarget_);
	if (backBufferTexture)
		backBufferTexture->Release();
	if (FAILED(result))
	{
		DXTRACE_MSG(L"Failed to create the render target view!");
		return false;
	}
	d3dContext_->OMSetRenderTargets(1, &backBufferTarget_, 0);

	//创建与设置viewport
	D3D11_VIEWPORT viewport;
	viewport.Width = static_cast<float>(width);
	viewport.Height = static_cast<float>(height);
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	viewport.TopLeftX = 0.0f;
	viewport.TopLeftY = 0.0f;

	d3dContext_->RSSetViewports(1, &viewport);

	//键盘设备初始化
	ZeroMemory(keyboardKeys_, sizeof(keyboardKeys_));
	ZeroMemory(prevKeyboardKeys_, sizeof(prevKeyboardKeys_));
	result = DirectInput8Create(hInstance_, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&directInput_, 0);
	if (FAILED(result))
	{
		return false;
	}

	result = directInput_->CreateDevice(GUID_SysKeyboard, &keyboardDevice_, 0);
	if (FAILED(result))
	{
		return false;
	}
	result = keyboardDevice_->SetDataFormat(&c_dfDIKeyboard);
	if (FAILED(result))
	{
		return false;
	}
	result = keyboardDevice_->SetCooperativeLevel(hwnd_, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
	if (FAILED(result))
	{
		return false;
	}
	result = keyboardDevice_->Acquire();
	if (FAILED(result))
	{
		return false;
	}

	//初始化鼠标设备
	mousePosX_ = mousePosY_ = mouseWheel_ = 0;
	result = directInput_->CreateDevice(GUID_SysMouse, &mouseDevice_, 0);
	if (FAILED(result))
	{
		return false;
	}
	result = mouseDevice_->SetDataFormat(&c_dfDIMouse);
	if (FAILED(result))
	{
		return false;
	}
	result = mouseDevice_->SetCooperativeLevel(hwnd_, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
	if (FAILED(result))
	{
		return false;
	}
	result = mouseDevice_->Acquire();
	if (FAILED(result))
	{
		return false;
	}

	return LoadContent();
}

bool Dx11Base::LoadContent()
{
	return true;
}

void Dx11Base::UnLoadContent()
{

}

void Dx11Base::Shutdown()
{
	UnLoadContent();

	if (backBufferTarget_)
		backBufferTarget_->Release();
	if (d3dDevice_)
		d3dDevice_->Release();
	if (d3dContext_)
		d3dContext_->Release();
	if (swapChain_)
		swapChain_->Release();
	if (keyboardDevice_)
	{
		keyboardDevice_->Unacquire();
		keyboardDevice_->Release();
	}
	if (mouseDevice_)
	{
		mouseDevice_->Unacquire();
		mouseDevice_->Release();
	}
	if (directInput_)
		directInput_->Release();
	backBufferTarget_ = 0;
	d3dDevice_ = 0;
	d3dContext_ = 0;
	swapChain_ = 0;
	keyboardDevice_ = 0;
	mouseDevice_ = 0;
	directInput_ = 0;
}

bool Dx11Base::CompileD3DShader(LPCWSTR filePath, LPCSTR entry, LPCSTR shaderModel, ID3DBlob** buffer)
{
	DWORD shaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;

#if defined (DEBUG)|| defined(_DEBUG)
	shaderFlags |= D3DCOMPILE_DEBUG;
#endif

	ID3DBlob* errorBuffer = 0;
	HRESULT result;
	result = D3DX11CompileFromFile(filePath, 0, 0, entry, shaderModel, shaderFlags, 0, 0, buffer, &errorBuffer, 0);

	if (FAILED(result))
	{
		if (errorBuffer != 0)
		{
			OutputDebugStringA((char*)errorBuffer->GetBufferPointer());
			errorBuffer->Release();
		}
		return false;
	}

	if (errorBuffer != 0)
		errorBuffer->Release();

	return true;
}