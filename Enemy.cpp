#include "Enemy.h"
#include "math/MathUtility.h"

using namespace KamataEngine::MathUtility;

void Enemy::Initialize(Model* model) {
	Object3d::Initialize(model);
	
	// Create collider for the enemy
	collider_ = std::make_shared<Collider>(CollisionTag::Enemy, Vector3{kWidth_, kHeight_, kDepth_});
	collider_->SetPosition(worldTransform_.translation_);
	
	// Register collision callback
	collider_->AddCollisionCallback([this](Collider* self, Collider* other) {
		OnCollisionEnter(self, other);
	});
	
	// Register with collision system
	CollisionSystem::GetInstance()->RegisterCollider(collider_);
}

void Enemy::Update() {
	if (!isActive_) {
		return;
	}
	
	// Move between patrol points
	Move();
	
	// Update world transform
	worldTransform_.MakeAffineMatrix4x4();
	worldTransform_.TransferMatrix();
	
	// Update collider position
	if (collider_) {
		collider_->Update(worldTransform_.translation_);
	}
}

void Enemy::Move() {
	Vector3 currentPos = worldTransform_.translation_;
	Vector3 targetPos = movingToB_ ? patrolPointB_ : patrolPointA_;
	
	// Calculate direction to target
	Vector3 direction = targetPos - currentPos;
	float distanceToTarget = sqrt(direction.x * direction.x + direction.y * direction.y + direction.z * direction.z);
	
	// Check if we've reached the target (with small tolerance)
	if (distanceToTarget < 0.5f) {
		movingToB_ = !movingToB_; // Switch direction
		return;
	}
	
	// Normalize direction and apply speed
	if (distanceToTarget > 0.0f) {
		direction.x /= distanceToTarget;
		direction.y /= distanceToTarget;
		direction.z /= distanceToTarget;
	}
	
	velocity_ = direction * moveSpeed_;
	
	// Update position
	worldTransform_.translation_.x += velocity_.x;
	worldTransform_.translation_.y += velocity_.y;
	worldTransform_.translation_.z += velocity_.z;
}

void Enemy::SetPatrolPoints(const Vector3& pointA, const Vector3& pointB) {
	patrolPointA_ = pointA;
	patrolPointB_ = pointB;
}

void Enemy::OnCollisionEnter(Collider* /*self*/, Collider* other) {
	switch (other->GetTag()) {
		case CollisionTag::Player:
			// Handle player collision (e.g., damage player, enemy behavior change)
			// Example: Push player away, deal damage, etc.
			break;
		case CollisionTag::Block:
			// Handle block collision (e.g., change direction, stop movement)
			// For now, just reverse direction
			movingToB_ = !movingToB_;
			break;
		default:
			break;
	}
}