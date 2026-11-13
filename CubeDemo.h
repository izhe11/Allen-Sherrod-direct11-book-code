#pragma once
#include "dx11base.h"
#include <xnamath.h>

class CubeDemo :public Dx11Base
{
public:
	CubeDemo();
	virtual ~CubeDemo();
	bool LoadContent();
	void UnLoadContent();
	void Update(float dt);
	void Render();
private:
	ID3D11VertexShader* solidColorVS_;     //顶点着色器
	ID3D11PixelShader* solidColorPS_;      //像素着色器
	ID3D11InputLayout* inputLayout_;       //输入布局
	ID3D11Buffer* vertexBuffer_;           //顶点缓冲
	ID3D11Buffer* indexBuffer_;            //索引缓冲
	ID3D11ShaderResourceView* colorMap_;   //着色器资源视图
	ID3D11SamplerState* colorMapSampler_;  //采样状态
	ID3D11Buffer* viewCB_;                 //视图buffer
	ID3D11Buffer* projCB_;                 //投影buffer
	ID3D11Buffer* worldCB_;                //世界空间坐标系
	XMMATRIX viewMatrix_;                  //视图矩阵
	XMMATRIX projMatrix_;                  //投影矩阵
};