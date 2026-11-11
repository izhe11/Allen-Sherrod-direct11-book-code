#include "TextureDemo.h"

struct VertexPos
{
	XMFLOAT3 pos;       //顶点坐标
	XMFLOAT2 tex0;      //贴图坐标
};

TextureDemo::TextureDemo():solidColorVS_(0), solidColorPS_(0), inputLayout_(0), vertexBuffer_(0)
                         , colorMap_(0), colorMapSampler_(0)
{

}
TextureDemo::~TextureDemo()
{

}

bool TextureDemo::LoadContent()
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

	//贴图纹理
	D3D11_INPUT_ELEMENT_DESC solidColorLayout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,
			0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,
			0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
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
		{ XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT2(1.0f, 1.0f) },
		{ XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT2(1.0f, 1.0f) },
	};
	D3D11_BUFFER_DESC vertexDesc;
	ZeroMemory(&vertexDesc, sizeof(vertexDesc));

	vertexDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexDesc.ByteWidth = sizeof(VertexPos) * 6;

	D3D11_SUBRESOURCE_DATA resourceData;
	ZeroMemory(&resourceData, sizeof(resourceData));
	resourceData.pSysMem = vertices;
	result = d3dDevice_->CreateBuffer(&vertexDesc, &resourceData, &vertexBuffer_);
	if (FAILED(result))
	{
		return false;
	}

	result = D3DX11CreateShaderResourceViewFromFile(d3dDevice_, L"images/R-C.jpg", 0, 0, &colorMap_, 0);
	if (FAILED(result))
	{
		DXTRACE_MSG(L"Failed to load the texture image!");
		return false;
	}

	D3D11_SAMPLER_DESC colorMapDesc;
	ZeroMemory(&colorMapDesc, sizeof(colorMapDesc));
	colorMapDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	colorMapDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	colorMapDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	colorMapDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	colorMapDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	colorMapDesc.MaxLOD = D3D11_FLOAT32_MAX;

	result = d3dDevice_->CreateSamplerState(&colorMapDesc, &colorMapSampler_);
	if (FAILED(result))
	{
		DXTRACE_MSG(L"Failed to create color map sampler state!");
		return false;
	}

	return true;
}

void TextureDemo::UnLoadContent()
{
	if (solidColorVS_)
		solidColorVS_->Release();
	if (solidColorPS_)
		solidColorPS_->Release();
	if (inputLayout_)
		inputLayout_->Release();
	if (vertexBuffer_)
		vertexBuffer_->Release();
	if (colorMap_)
		colorMap_->Release();
	if (colorMapSampler_)
		colorMapSampler_->Release();
	solidColorVS_ = 0;
	solidColorPS_ = 0;
	inputLayout_ = 0;
	vertexBuffer_ = 0;
	colorMapSampler_ = 0;
	colorMap_ = 0;
}

void TextureDemo::Update(float dt)
{

}

void TextureDemo::Render()
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
	d3dContext_->PSSetShaderResources(0, 1, &colorMap_);
	d3dContext_->PSSetSamplers(0, 1, &colorMapSampler_);
	d3dContext_->Draw(6, 0);

	swapChain_->Present(1, 0);
}