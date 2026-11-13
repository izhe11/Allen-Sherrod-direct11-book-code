#pragma once

#include <d3d11.h>
#include <d3dx11.h>
#include <DxErr.h>
#include <D3Dcompiler.h>
#include <xnamath.h>
#include <Dinput.h>

#define KEYDOWN(name,key) (name[key]&0x80)
#define BUTTONDOWN(name,key) (name.rgbButtons[key]&0x80)

class Dx11Base
{
public:
	Dx11Base();
	virtual ~Dx11Base();

	bool Initialize(HINSTANCE hInstance, HWND hwnd);   //初始化
	void Shutdown();         //释放资源

	virtual bool LoadContent();
	virtual void UnLoadContent();
	virtual void Update(float dt) = 0;
	virtual void Render() = 0;

	//编译着色器
	bool CompileD3DShader(LPCWSTR filePath, LPCSTR entry, LPCSTR shaderModel, ID3DBlob** buffer);

protected:
	HINSTANCE hInstance_;						 //实例
	HWND hwnd_;									 //窗口
	D3D_DRIVER_TYPE driverType_;				 //驱动类型
	D3D_FEATURE_LEVEL featureLevel_;			 //特征等级
	ID3D11Device* d3dDevice_;					 //设备
	ID3D11DeviceContext* d3dContext_;			 //环境
	IDXGISwapChain* swapChain_;					 //交换链
	ID3D11RenderTargetView* backBufferTarget_;   //渲染窗口视图
	LPDIRECTINPUT8 directInput_;
	LPDIRECTINPUTDEVICE8 keyboardDevice_;        //键盘输入设备
	LPDIRECTINPUTDEVICE8 mouseDevice_;           //鼠标输入设备
	ID3D11Texture2D* depthTexture_;
	ID3D11DepthStencilView* depthStencilView_;

	char keyboardKeys_[256];                     //按键状态
	char prevKeyboardKeys_[256];                 //上一次按键状态
	int mousePosX_;                             //鼠标x坐标
	int mousePosY_;                             //鼠标y坐标
	int mouseWheel_;                            //鼠标滚轮
};