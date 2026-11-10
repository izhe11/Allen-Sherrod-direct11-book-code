#pragma once

#include "dx11base.h"

//绘制三角形类
class Triangle :public Dx11Base
{
public:
	Triangle();
	virtual ~Triangle();

	bool LoadContent();
	void UnLoadContent();
	void Update(float dt);
	void Render();

private:
	ID3D11VertexShader* solidColorVS_;     //顶点着色器
	ID3D11PixelShader* solidColorPS_;      //像素着色器
	ID3D11InputLayout* inputLayout_;       //输入布局
	ID3D11Buffer* vertexBuffer_;           //顶点缓冲
};