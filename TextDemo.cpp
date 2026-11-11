#include "TextDemo.h"

struct VertexPos
{
	XMFLOAT3 pos;       //顶点坐标
	XMFLOAT2 tex0;      //贴图坐标
};

TextDemo::TextDemo() :solidColorVS_(0), solidColorPS_(0), inputLayout_(0), vertexBuffer_(0),
                      colorMap_(0), colorMapSampler_(0)
{

}
TextDemo::~TextDemo()
{

}

bool TextDemo::LoadContent()
{
	ID3DBlob* vsBuffer = 0;     //顶点
	//solidGreenColor.fx 是"原样输出位置 + 采样纹理"的 shader，正好用于屏幕坐标下的文字
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

	//顶点结构与 DrawString 中的 VertexPos 一致：位置 + 贴图坐标
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

	//动态顶点缓冲：DrawString 每帧都要用 Map(D3D11_MAP_WRITE_DISCARD) 重写顶点数据，
	//所以这里必须用 D3D11_USAGE_DYNAMIC + D3D11_CPU_ACCESS_WRITE，且容量要按
	//DrawString 里的上限来算：最多 24 个字母，每个字母 6 个顶点（两个三角形）
	D3D11_BUFFER_DESC vertexDesc;
	ZeroMemory(&vertexDesc, sizeof(vertexDesc));
	vertexDesc.Usage = D3D11_USAGE_DYNAMIC;
	vertexDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	vertexDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexDesc.ByteWidth = sizeof(VertexPos) * 6 * 24;

	result = d3dDevice_->CreateBuffer(&vertexDesc, 0, &vertexBuffer_);
	if (FAILED(result))
	{
		DXTRACE_MSG(L"Failed to create vertex buffer!");
		return false;
	}

	//字体位图：DrawString 里用 texWidth = 32.0f / 864.0f 取字模，
	//即整张图是 27 个 32 像素宽的格子（A~Z 共 26 个 + 1 个非字母字符），总宽 27*32 = 864
	result = D3DX11CreateShaderResourceViewFromFile(d3dDevice_, L"images/font.png", 0, 0, &colorMap_, 0);
	if (FAILED(result))
	{
		DXTRACE_MSG(L"Failed to load the font image!");
		return false;
	}

	D3D11_SAMPLER_DESC colorMapDesc;
	ZeroMemory(&colorMapDesc, sizeof(colorMapDesc));
	colorMapDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;    //夹取，避免采到相邻字母
	colorMapDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	colorMapDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	colorMapDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	colorMapDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	colorMapDesc.MaxLOD = D3D11_FLOAT32_MAX;

	result = d3dDevice_->CreateSamplerState(&colorMapDesc, &colorMapSampler_);
	if (FAILED(result))
	{
		DXTRACE_MSG(L"Failed to create color map sampler state!");
		return false;
	}

	//字体图是透明背景，需要 alpha 混合才能只显示字形。
	//Render 里不会改动混合状态，所以在这里设置一次即可
	D3D11_BLEND_DESC blendDesc;
	ZeroMemory(&blendDesc, sizeof(blendDesc));
	blendDesc.RenderTarget[0].BlendEnable = TRUE;
	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = 0x0F;

	ID3D11BlendState* alphaBlendState = 0;
	if (SUCCEEDED(d3dDevice_->CreateBlendState(&blendDesc, &alphaBlendState)))
	{
		float blendFactor[4] = { 0.0f,0.0f,0.0f,0.0f };
		d3dContext_->OMSetBlendState(alphaBlendState, blendFactor, 0xFFFFFFFF);
		//设置到设备上下文后上下文会自己持有引用，这里可以立即释放本地引用
		alphaBlendState->Release();
	}

	return true;
}

void TextDemo::UnLoadContent()
{
	if (colorMapSampler_)
		colorMapSampler_->Release();
	if (colorMap_)
		colorMap_->Release();
	if (vertexBuffer_)
		vertexBuffer_->Release();
	if (inputLayout_)
		inputLayout_->Release();
	if (solidColorPS_)
		solidColorPS_->Release();
	if (solidColorVS_)
		solidColorVS_->Release();

	colorMapSampler_ = 0;
	colorMap_ = 0;
	vertexBuffer_ = 0;
	inputLayout_ = 0;
	solidColorPS_ = 0;
	solidColorVS_ = 0;
}

void TextDemo::Update(float dt)
{

}

void TextDemo::Render()
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
	DrawString("HELLO WORLD", -0.2f, 0.0f);
	swapChain_->Present(1, 0);
}

bool TextDemo::DrawString(const char* message, float startX, float startY)
{
	const int spriteSize = sizeof(VertexPos) * 6;
	const int maxLetters = 24;            //最大渲染字母数量
	int length = strlen(message);
	
	if (length > maxLetters)
		length = maxLetters;

	float charWidth = 32.0f / 800.0f;        //字符在屏幕上显示的宽度
	float charHeight = 32.0f / 640.0f;       //字符在屏幕上显示的高度
	float texWidth = 32.0f / 864.0f;
	const int verticesPerLetter = 6;         //两个三角形，6个点

	D3D11_MAPPED_SUBRESOURCE mapResource;
	HRESULT result = d3dContext_->Map(vertexBuffer_, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapResource);
	if (FAILED(result))
	{
		DXTRACE_MSG(L"Failed to map resource!");
		return false;
	}

	VertexPos* spritePtr = (VertexPos*)mapResource.pData;
	const int indexA = static_cast<char>('A');
	const int indexZ = static_cast<char>('Z');

	for (int i = 0; i < length; ++i)
	{
		float thisStartX = startX + (charWidth * static_cast<float>(i));
		float thisEndX = thisStartX + charWidth;
		float thisEndY = startY + charHeight;

		spritePtr[0].pos = XMFLOAT3(thisEndX, thisEndY, 1.0f);
		spritePtr[1].pos = XMFLOAT3(thisEndX, startY, 1.0f);
		spritePtr[2].pos = XMFLOAT3(thisStartX, startY, 1.0f);
		spritePtr[3].pos = XMFLOAT3(thisStartX, startY, 1.0f);
		spritePtr[4].pos = XMFLOAT3(thisStartX, thisEndY, 1.0f);
		spritePtr[5].pos = XMFLOAT3(thisEndX, thisEndY, 1.0f);

		int texLookup = 0;
		int letter = static_cast<char>(message[i]);
		if (letter<indexA || letter>indexZ)
		{
			texLookup = (indexZ - indexA) + 1;
		}
		else
		{
			//A=0，B=1，Z=25 ...
			texLookup = letter - indexA;
		}

		float tuStart = 0.0f + (texWidth * static_cast<float>(texLookup));
		float tuEnd = tuStart + texWidth;
		spritePtr[0].tex0 = XMFLOAT2(tuEnd, 0.0f);
		spritePtr[1].tex0 = XMFLOAT2(tuEnd, 1.0f);
		spritePtr[2].tex0 = XMFLOAT2(tuStart, 1.0f);
		spritePtr[3].tex0 = XMFLOAT2(tuStart, 1.0f);
		spritePtr[4].tex0 = XMFLOAT2(tuStart, 0.0f);
		spritePtr[5].tex0 = XMFLOAT2(tuEnd, 0.0f);
		spritePtr += 6;
	}

	d3dContext_->Unmap(vertexBuffer_, 0);
	d3dContext_->Draw(6 * length, 0);

	return true;
}