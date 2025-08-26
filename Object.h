#pragma once
#include <KamataEngine.h>
using namespace KamataEngine;
class Object {

public:
	Object() = default;
	virtual ~Object() = default;
	virtual void Initialize() = 0;
	virtual void Update() = 0;
	virtual void Draw() = 0;

protected:
	Object3d* object3d_ = nullptr; // 3Dオブジェクト

};