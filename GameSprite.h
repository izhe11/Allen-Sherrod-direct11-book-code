#pragma once
#include "dx11base.h"

class GameSprite
{
public:
	GameSprite();
	virtual ~GameSprite();

	XMMATRIX GetWorldMatrix();
	void SetPosition(XMFLOAT2& position);
	void SetRotation(float rotate);
	void SetScale(XMFLOAT2& scale);

private:
	XMFLOAT2 position_;     //坐标位置
	float rotate_;          //旋转
	XMFLOAT2 scale_;        //缩放
};