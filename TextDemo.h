#pragma once
#include "dx11Base.h"

class TextDemo :public Dx11Base
{
public:
	TextDemo();
	virtual ~TextDemo();
	bool LoadContent();
	void UnLoadContent();
	void Update(float dt);
	void Render();
private:
	bool DrawString(const char* message, float startX, float startY);       //显示指定文本
private:
	ID3D11VertexShader* solidColorVS_;     //顶点着色器
	ID3D11PixelShader* solidColorPS_;      //像素着色器
	ID3D11InputLayout* inputLayout_;       //输入布局
	ID3D11Buffer* vertexBuffer_;           //顶点缓冲
	ID3D11ShaderResourceView* colorMap_;   //着色器资源视图
	ID3D11SamplerState* colorMapSampler_;  //采样状态
};