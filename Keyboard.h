#pragma once
#include "dx11base.h"

class Keyboard :public Dx11Base
{
public:
	Keyboard();
	virtual ~Keyboard();
	bool LoadContent();
	void UnLoadContent();
	void Update(float dt);
	void Render();
private:
	ID3D11VertexShader* customColorVS_;     //顶点着色器
	ID3D11PixelShader* customColorPS_;      //像素着色器
	ID3D11InputLayout* inputLayout_;       //输入布局
	ID3D11Buffer* vertexBuffer_;           //顶点缓冲
	ID3D11Buffer* colorCB_;                //颜色缓冲
	int selectedColor_;
};