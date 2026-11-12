#pragma once
#include "dx11base.h"

class Mouse :public Dx11Base
{
public:
	Mouse();
	virtual ~Mouse();
	bool LoadContent();
	void UnLoadContent();
	void Update(float dt);
	void Render();
private:
	ID3D11VertexShader* customColorVS_;     //顶点着色器
	ID3D11PixelShader* customColorPS_;      //像素着色器
	ID3D11InputLayout* inputLayout_;       //输入布局
	ID3D11Buffer* vertexBuffer_;           //顶点缓冲（每帧都要改顶点，所以创建成动态缓冲）
	ID3D11Buffer* colorCB_;                //颜色缓冲
	int selectedColor_;                    //当前颜色索引
	DIMOUSESTATE mouseState_;              //本帧鼠标状态
	DIMOUSESTATE prevMouseState_;          //上一帧鼠标状态，用来判断"按下的那一帧"
	float offsetX_;                        //三角形在x方向的偏移（NDC，-1~1）
	float offsetY_;                        //三角形在y方向的偏移（NDC，-1~1）
	float scale_;                          //三角形缩放
};