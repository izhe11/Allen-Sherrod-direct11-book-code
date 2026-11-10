#pragma once

#include "dx11base.h"

class BlankDemo : public Dx11Base
{

public:
	BlankDemo();
	virtual ~BlankDemo();
	bool LoadContent();
	void UnLoadContent();
	void Update(float dt);
	void Render();
};