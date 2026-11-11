#pragma once
#include "dx11base.h"
#include "GameSprite.h"

class GameSpriteDemo :public Dx11Base
{
public:
	GameSpriteDemo();
	virtual ~GameSpriteDemo();

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
	ID3D11BlendState* alphaBlendState_;     //混合状态
	GameSprite sprites_[2];
	ID3D11Buffer* mvpCB_;
	XMMATRIX vpMatrix_;                     //viewport
};