#include "Mouse.h"

//顶点结构
struct VertexPos
{
	XMFLOAT3 pos;
};

//三角形在NDC（归一化设备坐标）下的基准顶点。
//鼠标的移动/滚轮只是在此基础上做偏移和缩放，不改变这份原始数据。
static const XMFLOAT3 BASE_VERTICES[3] =
{
	XMFLOAT3(0.5f,  0.5f, 0.5f),
	XMFLOAT3(0.5f, -0.5f, 0.5f),
	XMFLOAT3(-0.5f,-0.5f, 0.5f)
};

Mouse::Mouse() :customColorVS_(0), customColorPS_(0), inputLayout_(0),
vertexBuffer_(0), colorCB_(0), selectedColor_(0),
offsetX_(0.0f), offsetY_(0.0f), scale_(1.0f)
{
	ZeroMemory(&mouseState_, sizeof(mouseState_));
	ZeroMemory(&prevMouseState_, sizeof(prevMouseState_));
}

Mouse::~Mouse()
{

}

bool Mouse::LoadContent()
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

	//顶点数据：先用基准顶点填一次，之后每帧用 Map/Unmap 覆盖
	VertexPos vertices[3];
	for (int i = 0; i < 3; ++i)
	{
		vertices[i].pos = BASE_VERTICES[i];
	}

	//因为顶点每帧都可能变（跟着鼠标动），所以用 D3D11_USAGE_DYNAMIC + CPU 可写
	D3D11_BUFFER_DESC vertexDesc;
	ZeroMemory(&vertexDesc, sizeof(vertexDesc));
	vertexDesc.Usage = D3D11_USAGE_DYNAMIC;
	vertexDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexDesc.ByteWidth = sizeof(VertexPos) * 3;
	vertexDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

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

void Mouse::UnLoadContent()
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

void Mouse::Update(float dt)
{
	//DirectInput 报告的鼠标状态是"相对位移"（lX/lY/lZ）加按键字节，不是绝对坐标。
	//窗口失去焦点时 DirectInput 会自动 Unacquire，此时 GetDeviceState 返回 DIERR_INPUTLOST，
	//必须先 Acquire 回来，否则鼠标会一直"失灵"。
	HRESULT result = mouseDevice_->GetDeviceState(sizeof(DIMOUSESTATE), &mouseState_);
	if (FAILED(result))
	{
		ZeroMemory(&mouseState_, sizeof(mouseState_));
		if (SUCCEEDED(mouseDevice_->Acquire()))
			mouseDevice_->GetDeviceState(sizeof(DIMOUSESTATE), &mouseState_);
	}

	//escape按键按下
	if (GetAsyncKeyState(VK_ESCAPE))
	{
		PostQuitMessage(0);
	}

	//左键按下：切换到下一个颜色（只在按下的那一帧处理一次）
	if (!BUTTONDOWN(prevMouseState_, 0) && BUTTONDOWN(mouseState_, 0))
	{
		selectedColor_++;
	}
	//右键按下：切换到上一个颜色
	if (!BUTTONDOWN(prevMouseState_, 1) && BUTTONDOWN(mouseState_, 1))
	{
		selectedColor_--;
	}
	//中键按下：位置、滚轮、缩放全部复位
	if (!BUTTONDOWN(prevMouseState_, 2) && BUTTONDOWN(mouseState_, 2))
	{
		mousePosX_ = mousePosY_ = mouseWheel_ = 0;
	}

	if (selectedColor_ < 0)
		selectedColor_ = 2;
	if (selectedColor_ > 2)
		selectedColor_ = 0;

	//累积相对位移，模拟出一个"虚拟光标"的位置
	mousePosX_ += mouseState_.lX;
	mousePosY_ += mouseState_.lY;
	mouseWheel_ += mouseState_.lZ;

	RECT rc = { 0, 0, 0, 0 };
	GetClientRect(hwnd_, &rc);
	float width = static_cast<float>(rc.right - rc.left);
	float height = static_cast<float>(rc.bottom - rc.top);
	if (width <= 0.0f || height <= 0.0f)
		return;

	//把虚拟光标限制在窗口范围内（越界后继续划鼠标不会再累积）
	int halfWidth = static_cast<int>(width * 0.5f);
	int halfHeight = static_cast<int>(height * 0.5f);
	if (mousePosX_ < -halfWidth)
		mousePosX_ = -halfWidth;
	if (mousePosX_ > halfWidth)
		mousePosX_ = halfWidth;
	if (mousePosY_ < -halfHeight)
		mousePosY_ = -halfHeight;
	if (mousePosY_ > halfHeight)
		mousePosY_ = halfHeight;

	//像素位移换算成NDC偏移：像素原点在左上角、y轴向下，NDC原点在中心、y轴向上
	offsetX_ = 2.0f * mousePosX_ / width;
	offsetY_ = -2.0f * mousePosY_ / height;

	//滚轮控制缩放（lZ每格通常是120）
	scale_ = 1.0f + mouseWheel_ * 0.001f;
	if (scale_ < 0.2f)
		scale_ = 0.2f;
	if (scale_ > 3.0f)
		scale_ = 3.0f;

	memcpy(&prevMouseState_, &mouseState_, sizeof(mouseState_));
}

void Mouse::Render()
{
	if (d3dContext_ == 0)
		return;

	float clearColor[4] = { 0.0f,0.0f,0.25f,1.0f };
	d3dContext_->ClearRenderTargetView(backBufferTarget_, clearColor);

	//按鼠标输入算出的偏移和缩放生成本帧的三角形顶点
	VertexPos vertices[3];
	for (int i = 0; i < 3; ++i)
	{
		vertices[i].pos.x = BASE_VERTICES[i].x * scale_ + offsetX_;
		vertices[i].pos.y = BASE_VERTICES[i].y * scale_ + offsetY_;
		vertices[i].pos.z = BASE_VERTICES[i].z;
	}

	//动态缓冲不能直接UpdateSubresource，要Map/Unmap；WRITE_DISCARD表示整块丢弃重写
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	ZeroMemory(&mappedResource, sizeof(mappedResource));
	HRESULT result = d3dContext_->Map(vertexBuffer_, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (SUCCEEDED(result))
	{
		memcpy(mappedResource.pData, vertices, sizeof(vertices));
		d3dContext_->Unmap(vertexBuffer_, 0);
	}

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