#pragma once

#include <d3d11.h>
#include <d3dx11.h>
#include <DxErr.h>
#include <D3Dcompiler.h>

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

};