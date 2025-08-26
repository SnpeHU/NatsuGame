#pragma once
#include <KamataEngine.h>
#include "CollisionSystem.h"
#include <memory>

using namespace KamataEngine;

class Enemy : public Object3d {
public:
	Enemy() = default;
	~Enemy() = default;
	
	void Initialize(Model* model) override;
	void Update() override;
	
	// Movement and AI
	void Move();
	void SetPatrolPoints(const Vector3& pointA, const Vector3& pointB);
	
	// Collision handling
	void OnCollisionEnter(Collider* self, Collider* other);
	
	// Getters
	std::shared_ptr<Collider> GetCollider() const { return collider_; }
	bool IsActive() const { return isActive_; }
	
	// Setters
	void SetActive(bool active) { isActive_ = active; }

private:
	// Collision system
	std::shared_ptr<Collider> collider_;
	
	// AI and movement
	Vector3 velocity_ = {0.0f, 0.0f, 0.0f};
	Vector3 patrolPointA_ = {0.0f, 0.0f, 0.0f};
	Vector3 patrolPointB_ = {10.0f, 0.0f, 0.0f};
	bool movingToB_ = true;
	float moveSpeed_ = 0.05f;
	
	// State
	bool isActive_ = true;
	
	// Size constants
	static inline float kWidth_ = 1.5f;
	static inline float kHeight_ = 1.5f;
	static inline float kDepth_ = 1.5f;
};