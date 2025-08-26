#include "Pickup.h"
#include <cmath>

void Pickup::Initialize(Model* model) {
	Object3d::Initialize(model);
	
	// Store original position for bobbing animation
	originalPosition_ = worldTransform_.translation_;
	
	// Create collider for the pickup (as trigger)
	collider_ = std::make_shared<Collider>(CollisionTag::Pickup, Vector3{kWidth_, kHeight_, kDepth_});
	collider_->SetPosition(worldTransform_.translation_);
	collider_->SetTrigger(true); // Set as trigger so it doesn't block movement
	
	// Register collision callback
	collider_->AddCollisionCallback([this](Collider* self, Collider* other) {
		OnCollisionEnter(self, other);
	});
	
	// Register with collision system
	CollisionSystem::GetInstance()->RegisterCollider(collider_);
}

void Pickup::Update() {
	if (isCollected_) {
		// Could add collection animation here before destroying
		return;
	}
	
	// Bobbing animation
	bobOffset_ += bobSpeed_ * (1.0f / 60.0f); // Assuming 60 FPS
	worldTransform_.translation_.y = originalPosition_.y + sin(bobOffset_) * bobAmount_;
	
	// Rotation for visual appeal
	worldTransform_.rotation_.y += rotationSpeed_ * (1.0f / 60.0f);
	
	// Update world transform
	worldTransform_.MakeAffineMatrix4x4();
	worldTransform_.TransferMatrix();
	
	// Update collider position
	if (collider_) {
		collider_->Update(worldTransform_.translation_);
	}
}

void Pickup::Collect() {
	if (!isCollected_) {
		isCollected_ = true;
		
		// Disable collider
		if (collider_) {
			collider_->SetActive(false);
		}
		
		// Could trigger sound effect, particle effect, etc. here
		// For now, just mark as collected
	}
}

void Pickup::OnCollisionEnter(Collider* /*self*/, Collider* other) {
	if (isCollected_) {
		return;
	}
	
	switch (other->GetTag()) {
		case CollisionTag::Player:
			// Player touched the pickup - collect it
			Collect();
			break;
		default:
			break;
	}
}