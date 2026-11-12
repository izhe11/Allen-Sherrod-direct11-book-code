#include "Keyboard.h"

//顶点结构
struct VertexPos
{
	XMFLOAT3 pos;
};

Keyboard::Keyboard() :customColorVS_(0), customColorPS_(0), inputLayout_(0), 
                      vertexBuffer_(0), colorCB_(0)
{

}
Keyboard::~Keyboard()
{

}

bool Keyboard::LoadContent()
{
	ID3DBlob* vsBuffer = 0;     //顶点
	bool compileResult = CompileD3DShader(L"customColor.hlsl", "VS_Main", "vs_5_0", &vsBuffer);
	if (compileResult == false)
	{
		MessageBox(0, L"error loading vertex shader", L"compile error", MB_OK);
		return false;
	}

	HRESULT result;
	result = d3dDevice_->CreateVertexShader(vsBuffer->GetBufferPointer(),
		vsBuffer->GetBufferSize(), 0, &customColorVS_);
	if (FAILED(result))
	{
		if (vsBuffer)
		{
			vsBuffer->Release();
		}
		return false;
	}

	D3D11_INPUT_ELEMENT_DESC solidColorLayout[] =
	{
		{
			"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,
			0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0
		}
	};
	unsigned int totalLayoutElements = ARRAYSIZE(solidColorLayout);

	result = d3dDevice_->CreateInputLayout(solidColorLayout, totalLayoutElements, vsBuffer->GetBufferPointer(),
		vsBuffer->GetBufferSize(), &inputLayout_);
	vsBuffer->Release();
	if (FAILED(result))
	{
		return false;
	}

	ID3DBlob* psBuffer = 0;         //像素
	compileResult = CompileD3DShader(L"customColor.hlsl", "PS_Main", "ps_5_0", &psBuffer);
	if (compileResult == false)
	{
		MessageBox(0, L"error loading pixel shader", L"compile error", MB_OK);
		return false;
	}
	result = d3dDevice_->CreatePixelShader(psBuffer->GetBufferPointer(), psBuffer->GetBufferSize(),
		0, &customColorPS_);
	psBuffer->Release();
	if (FAILED(result))
	{
		return false;
	}

	//顶点数据
	VertexPos vertices[] =
	{
		XMFLOAT3(0.5f,0.5f,0.5f),
		XMFLOAT3(0.5f,-0.5f,0.5f),
		XMFLOAT3(-0.5f,-0.5f,0.5f)
	};
	D3D11_BUFFER_DESC vertexDesc;
	ZeroMemory(&vertexDesc, sizeof(vertexDesc));

	vertexDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexDesc.ByteWidth = sizeof(VertexPos) * 3;

	D3D11_SUBRESOURCE_DATA resourceData;
	ZeroMemory(&resourceData, sizeof(resourceData));
	resourceData.pSysMem = vertices;
	result = d3dDevice_->CreateBuffer(&vertexDesc, &resourceData, &vertexBuffer_);
	if (FAILED(result))
	{
		return false;
	}

	D3D11_BUFFER_DESC constDesc;
	ZeroMemory(&constDesc, sizeof(constDesc));
	constDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	constDesc.ByteWidth = sizeof(XMFLOAT4);
	constDesc.Usage = D3D11_USAGE_DEFAULT;
	result = d3dDevice_->CreateBuffer(&constDesc, 0, &colorCB_);
	if (FAILED(result))
		return false;

	return true;
}

void Keyboard::UnLoadContent()
{
	if (customColorVS_)
		customColorVS_->Release();
	if (customColorPS_)
		customColorPS_->Release();
	if (inputLayout_)
		inputLayout_->Release();
	if (vertexBuffer_)
		vertexBuffer_->Release();
	if (colorCB_)
		colorCB_->Release();
	customColorVS_ = 0;
	customColorPS_ = 0;
	inputLayout_ = 0;
	vertexBuffer_ = 0;
	colorCB_ = 0;
}

void Keyboard::Update(float dt)
{
	keyboardDevice_->GetDeviceState(sizeof(keyboardKeys_), (LPVOID)&keyboardKeys_);

	//escape按键按下
	if (GetAsyncKeyState(VK_ESCAPE))
	{
		PostQuitMessage(0);
	}
	//向上键
	if (KEYDOWN(prevKeyboardKeys_, DIK_DOWN) && !KEYDOWN(keyboardKeys_, DIK_DOWN))
	{
		selectedColor_--;
	}
	//向下键
	if (KEYDOWN(prevKeyboardKeys_, DIK_UP) && !KEYDOWN(keyboardKeys_, DIK_UP))
	{
		selectedColor_++;
	}

	memcpy(prevKeyboardKeys_, keyboardKeys_, sizeof(keyboardKeys_));
	if (selectedColor_ < 0)
		selectedColor_ = 2;
	if (selectedColor_ > 2)
		selectedColor_ = 0;
}

void Keyboard::Render()
{
	if (d3dContext_ == 0)
		return;

	float clearColor[4] = { 0.0f,0.0f,0.25f,1.0f };
	d3dContext_->ClearRenderTargetView(backBufferTarget_, clearColor);

	unsigned int stride = sizeof(VertexPos);
	unsigned int offset = 0;

	d3dContext_->IASetInputLayout(inputLayout_);
	d3dContext_->IASetVertexBuffers(0, 1, &vertexBuffer_, &stride, &offset);
	d3dContext_->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	
	XMFLOAT4 redColor(1.0f, 0.0f, 0.0f, 1.0f);
	XMFLOAT4 greenColor(0.0f, 1.0f, 0.0f, 1.0f);
	XMFLOAT4 blueColor(0.0f, 0.0f, 1.0f, 1.0f);
	if (selectedColor_ == 0)
	{
		d3dContext_->UpdateSubresource(colorCB_, 0, 0, &redColor, 0, 0);
	}
	else if (selectedColor_ == 1)
	{
		d3dContext_->UpdateSubresource(colorCB_, 0, 0, &greenColor, 0, 0);
	}
	else
	{
		d3dContext_->UpdateSubresource(colorCB_, 0, 0, &blueColor, 0, 0);
	}

	d3dContext_->VSSetShader(customColorVS_, 0, 0);
	d3dContext_->PSSetShader(customColorPS_, 0, 0);
	d3dContext_->PSSetConstantBuffers(0, 1, &colorCB_);
	d3dContext_->Draw(3, 0);

	swapChain_->Present(1, 0);
}