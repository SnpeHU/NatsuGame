#pragma once
#include <KamataEngine.h>
#include "CollisionSystem.h"
#include <memory>

using namespace KamataEngine;

class Pickup : public Object3d {
public:
	Pickup() = default;
	~Pickup() = default;
	
	void Initialize(Model* model) override;
	void Update() override;
	
	// Pickup behavior
	void Collect();
	
	// Collision handling
	void OnCollisionEnter(Collider* self, Collider* other);
	
	// Getters
	std::shared_ptr<Collider> GetCollider() const { return collider_; }
	bool IsCollected() const { return isCollected_; }
	
	// Setters
	void SetValue(int value) { value_ = value; }
	int GetValue() const { return value_; }

private:
	// Collision system
	std::shared_ptr<Collider> collider_;
	
	// Pickup properties
	bool isCollected_ = false;
	int value_ = 10; // Points or currency value
	float rotationSpeed_ = 2.0f; // Rotation for visual effect
	
	// Animation
	float bobOffset_ = 0.0f;
	float bobSpeed_ = 3.0f;
	float bobAmount_ = 0.3f;
	Vector3 originalPosition_;
	
	// Size constants
	static inline float kWidth_ = 1.0f;
	static inline float kHeight_ = 1.0f;
	static inline float kDepth_ = 1.0f;
};