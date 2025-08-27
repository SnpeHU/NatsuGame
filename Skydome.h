#pragma once
#include <KamataEngine.h>
using namespace KamataEngine;
class Skydome : public Object3d{
public:
	Skydome() = default;
	~Skydome() = default;
	void Update() override;

private:
};
