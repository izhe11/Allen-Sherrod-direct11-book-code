#include <d3d11.h>
#include <d3dx11.h>
#include "GameSprite.h"

GameSprite::GameSprite():rotate_(0)
{
	scale_.x = 1.0f;
	scale_.y = 1.0f;
}
GameSprite::~GameSprite()
{

}

XMMATRIX GameSprite::GetWorldMatrix()
{
	XMMATRIX translation = XMMatrixTranslation(position_.x, position_.y, 0.0f);
	XMMATRIX rotationZ = XMMatrixRotationZ(rotate_);
	XMMATRIX sacle = XMMatrixScaling(scale_.x, scale_.y, 1.0f);

	return translation * rotationZ * sacle;
}

void GameSprite::SetPosition(XMFLOAT2& position)
{
	position_ = position;
}

void GameSprite::SetRotation(float rotate)
{
	rotate_ = rotate;
}

void GameSprite::SetScale(XMFLOAT2& scale)
{
	scale_ = scale;
}