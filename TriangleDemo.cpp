#include "TriangleDemo.h"
#include <xnamath.h>

//顶点结构
struct VertexPos
{
	XMFLOAT3 pos;
};

Triangle::Triangle():solidColorVS_(0), solidColorPS_(0), inputLayout_(0), vertexBuffer_(0)
{

}
Triangle::~Triangle()
{

}

void Triangle::UnLoadContent()
{
	if (solidColorVS_)
		solidColorVS_->Release();
	if (solidColorPS_)
		solidColorPS_->Release();
	if (inputLayout_)
		inputLayout_->Release();
	if (vertexBuffer_)
		vertexBuffer_->Release();
	solidColorVS_ = 0;
	solidColorPS_ = 0;
	inputLayout_ = 0;
	vertexBuffer_ = 0;
}

bool Triangle::LoadContent()
{
	ID3DBlob* vsBuffer = 0;     //顶点
	bool compileResult = CompileD3DShader(L"solidGreenColor.fx", "VS_Main", "vs_5_0", &vsBuffer);
	if (compileResult == false)
	{
		MessageBox(0, L"error loading vertex shader", L"compile error", MB_OK);
		return false;
	}

	HRESULT result;
	result = d3dDevice_->CreateVertexShader(vsBuffer->GetBufferPointer(),
		vsBuffer->GetBufferSize(), 0, &solidColorVS_);
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
	compileResult = CompileD3DShader(L"solidGreenColor.fx", "PS_Main", "ps_5_0", &psBuffer);
	if (compileResult == false)
	{
		MessageBox(0, L"error loading pixel shader", L"compile error", MB_OK);
		return false;
	}
	result = d3dDevice_->CreatePixelShader(psBuffer->GetBufferPointer(), psBuffer->GetBufferSize(),
		0, &solidColorPS_);
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

	return true;
}

void Triangle::Update(float dt)
{

}

void Triangle::Render()
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
	d3dContext_->VSSetShader(solidColorVS_, 0, 0);
	d3dContext_->PSSetShader(solidColorPS_, 0, 0);
	d3dContext_->Draw(3, 0);


	swapChain_->Present(1, 0);
}