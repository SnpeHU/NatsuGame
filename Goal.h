#pragma once
#include <KamataEngine.h>
using namespace KamataEngine;
class Goal : public Object3d{
	public:
	Goal() = default;
	~Goal() = default;
	void Update() override;
	void SetActive(bool isActive) { isActive_ = isActive; }
	bool IsActive() const { return isActive_; }

	void SetTargetMapID(int newID) { id = newID; }
	int GetTargetMapID() const { return id; }

	private:
	bool isActive_ = true;
	int id = 0;

};