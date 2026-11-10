#pragma once

#include "dx11base.h"
#include <xnamath.h>

class TextureDemo :public Dx11Base
{
public:
	TextureDemo();
	virtual ~TextureDemo();
	bool LoadContent();
	void UnLoadContent();
	void Update(float dt);
	void Render();
private:
	ID3D11VertexShader* solidColorVS_;     //顶点着色器
	ID3D11PixelShader* solidColorPS_;      //像素着色器
	ID3D11InputLayout* inputLayout_;       //输入布局
	ID3D11Buffer* vertexBuffer_;           //顶点缓冲
	ID3D11ShaderResourceView* colorMap_;   //着色器资源视图
	ID3D11SamplerState* colorMapSampler_;  //采样状态
};