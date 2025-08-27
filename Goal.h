#pragma once
#include <KamataEngine.h>
#include <functional>
using namespace KamataEngine;

class Goal : public Object3d {
public:
	Goal() = default;
	~Goal() = default;
	void Update() override;
	void SetActive(bool isActive) { isActive_ = isActive; }
	bool IsActive() const { return isActive_; }

	void SetTargetMapID(int newID) { id = newID; }
	int GetTargetMapID() const { return id; }

	// 碰撞检测相关方法
	bool CheckCollisionWithPlayer(const Vector3& playerPosition, const Vector3& playerSize) const;
	Vector3 GetGoalSize() const { return Vector3(size.x, size.y, 2.0f); }
	void SetGoalSize(const Vector2& newSize) { size = newSize; }

	// 回调函数设置
	void SetOnCollisionCallback(std::function<void(Goal*)> callback) { onCollisionCallback_ = callback; }
	void TriggerCollision() { if (onCollisionCallback_) onCollisionCallback_(this); }

	// 碰撞状态管理
	bool WasCollidingLastFrame() const { return wasCollidingLastFrame_; }
	void SetWasCollidingLastFrame(bool colliding) { wasCollidingLastFrame_ = colliding; }

private:
	bool isActive_ = true;
	int id = 0;
	bool wasCollidingLastFrame_ = false;

	Vector2 size = {1.8f, 1.8f};
	
	// 回调函数：加载对应关卡
	std::function<void(Goal*)> onCollisionCallback_;
};