#include "Player.h"
#include "MapChipField.h"
#include "Goal.h"
#include "GameScene.h"
#include <cmath>

void Player::Initialize(Model* model) { 
	Object3d::Initialize(model); 
	playerSize_ = {1.0f, 1.0f, 1.0f}; // 默认玩家大小
	worldTransform_.scale_ = {0.5f, 0.5f, 0.5f};
}

void Player::Update() {
	Move();
	CheckObjectCollisions();
	worldTransform_.MakeAffineMatrix4x4();
	worldTransform_.TransferMatrix();

#ifdef _DEBUG
	ShowDebugWindow();
#endif
}

void Player::Move() {
	// Update frame-independent timers first
	const float deltaTime = 1.0f / 60.0f; // TODO: Get real delta time
	UpdateTimers(deltaTime);
	
	// Handle input and physics
	HandleInput();
	UpdatePhysics();
	
	// Perform movement with collision detection
	ApplyMovementWithCollision();
	
	// Update collision states AFTER movement (critical for wall sliding)
	UpdateCollisionStatesPostMovement();
	
	// Clear frame input flags
	ClearFrameInputs();
}

void Player::UpdateTimers(float deltaTime) {
	// Update all timers in one place
	if (jumpBufferTimer > 0.0f) {
		jumpBufferTimer -= deltaTime;
	}
	
	if (wallJumpBufferTimer > 0.0f) {
		wallJumpBufferTimer -= deltaTime;
	}
	
	if (wallJumpDirectionLockTimer > 0.0f) {
		wallJumpDirectionLockTimer -= deltaTime;
	}
}

void Player::HandleInput() {
	// Process movement input
	isLeft = Input::GetInstance()->PushKey(DIK_A) || Input::GetInstance()->PushKey(DIK_LEFT);
	isRight = Input::GetInstance()->PushKey(DIK_D) || Input::GetInstance()->PushKey(DIK_RIGHT);
	
	// Process jump input
	isJumpTriggered = Input::GetInstance()->TriggerKey(DIK_SPACE) || Input::GetInstance()->TriggerKey(DIK_W);
	
	// Set jump buffers if jump was triggered
	if (isJumpTriggered) {
		jumpBufferTimer = jumpBufferTime;
		wallJumpBufferTimer = wallJumpBufferTime;
	}
}

void Player::UpdatePhysics() {
	// Store previous states
	wasOnGround = isOnGround;
	wasOnWall = isOnWallLeft || isOnWallRight;
	
	// Handle jumping (regular and wall jump)
	HandleJumping();
	
	// Apply horizontal movement (if not locked by wall jump)
	if (wallJumpDirectionLockTimer <= 0.0f) {
		velocity.x = static_cast<float>(isRight - isLeft) * speed;
	}
	
	// Apply gravity and physics
	ApplyGravity();
	
	// Handle wall sliding (before movement application)
	ApplyWallSlide();
}

void Player::ApplyMovementWithCollision() {
	Vector3 currentPos = worldTransform_.translation_;
	Vector3 targetPos = currentPos;
	
	// Reset collision direction
	lastCollisionDirection_ = CollisionDirection::kNone;
	
	// Apply X movement with wall contact optimization
	targetPos.x = currentPos.x + velocity.x;
	
	if (CheckCollisionAtPosition(targetPos)) {
		// X collision detected - find exact contact point instead of reverting
		bool isMovingRight = velocity.x > 0.0f;
		Vector3 adjustedPos = AdjustPositionToWall(currentPos, targetPos, !isMovingRight);
		
		lastCollisionDirection_ = isMovingRight ? CollisionDirection::kRight : CollisionDirection::kLeft;
		velocity.x = 0.0f;
		targetPos.x = adjustedPos.x;
		
#ifdef _DEBUG
		printf("Wall contact: %s - Adjusted from %.3f to %.3f\n", 
			   isMovingRight ? "RIGHT" : "LEFT", 
			   currentPos.x + velocity.x, adjustedPos.x);
#endif
	}
	
	// Apply Y movement
	targetPos.y = currentPos.y + velocity.y;
	
	if (CheckCollisionAtPosition(targetPos)) {
		// Y collision
		if (velocity.y < 0.0f) {
			// Hitting ground
			isOnGround = true;
			lastCollisionDirection_ = CollisionDirection::kDown;
		} else {
			// Hitting ceiling
			lastCollisionDirection_ = CollisionDirection::kUp;
		}
		velocity.y = 0.0f;
		targetPos.y = currentPos.y; // Revert Y position
	}
	
	// Apply final position
	worldTransform_.translation_ = targetPos;
}

void Player::UpdateCollisionStatesPostMovement() {
	// Update ground state based on current position
	isOnGround = CheckGroundCollision();
	
	// Check wall contact at current position (after movement)
	Vector3 currentPos = worldTransform_.translation_;
	
	// Use more generous detection for wall sliding
	isOnWallLeft = CheckWallCollisionAtPosition(currentPos, true);
	isOnWallRight = CheckWallCollisionAtPosition(currentPos, false);
	
	// Update wall jump direction based on current contact
	if (isOnWallLeft && !isOnGround) {
		wallJumpDirection = CollisionDirection::kLeft;
	} else if (isOnWallRight && !isOnGround) {
		wallJumpDirection = CollisionDirection::kRight;
	}
	
	// Clear wall states if on ground
	if (isOnGround) {
		if (!wasOnGround) {
			// Just landed - clear wall jump states
			wallJumpBufferTimer = 0.0f;
			wallJumpDirectionLockTimer = 0.0f;
			wallJumpDirection = CollisionDirection::kNone;
		}
	}

#ifdef _DEBUG
	// Debug wall contact
	if ((isOnWallLeft || isOnWallRight) && !isOnGround) {
		printf("Wall contact detected: Left=%s Right=%s Pos=(%.3f,%.3f)\n",
			   isOnWallLeft ? "YES" : "NO",
			   isOnWallRight ? "YES" : "NO",
			   currentPos.x, currentPos.y);
	}
#endif
}

bool Player::CheckWallCollisionAtPosition(const Vector3& position, bool isLeftSide) const {
	if (!mapChipField_) return false;
	
	// Enhanced wall detection with multiple check points
	Vector3 checkPos = position;
	float xOffset = (playerSize_.x / 2.0f + wallDetectionRange_) * (isLeftSide ? -1.0f : 1.0f);
	checkPos.x += xOffset;
	
	// Check multiple points along the player's height for better wall contact detection
	Vector3 checkSize = {0.05f, playerSize_.y * 0.8f, playerSize_.z};
	
	// Primary check at center
	if (CheckCollisionAtPositionWithScale(checkPos, checkSize)) {
		return true;
	}
	
	// Secondary checks at top and bottom thirds (for better wall sliding)
	checkPos.y = position.y + playerSize_.y * 0.25f;
	if (CheckCollisionAtPositionWithScale(checkPos, {0.05f, playerSize_.y * 0.3f, playerSize_.z})) {
		return true;
	}
	
	checkPos.y = position.y - playerSize_.y * 0.25f;
	if (CheckCollisionAtPositionWithScale(checkPos, {0.05f, playerSize_.y * 0.3f, playerSize_.z})) {
		return true;
	}
	
	return false;
}

bool Player::CheckWallCollision(bool isLeftSide) const {
	return CheckWallCollisionAtPosition(worldTransform_.translation_, isLeftSide);
}

Vector3 Player::AdjustPositionToWall(const Vector3& currentPos, const Vector3& targetPos, bool isLeftWall) {
	Vector3 adjustedPos = targetPos;
	
	// Find the exact contact position using binary search-like approach
	float contactX = FindWallContactPosition(currentPos, isLeftWall);
	
	if (contactX != currentPos.x) {
		adjustedPos.x = contactX;
	} else {
		// Fallback: just revert to current position
		adjustedPos.x = currentPos.x;
	}
	
	return adjustedPos;
}

float Player::FindWallContactPosition(const Vector3& centerPos, bool isLeftSide) const {
	if (!mapChipField_) return centerPos.x;
	
	// Binary search for exact wall contact position
	Vector3 testPos = centerPos;
	float currentX = centerPos.x;
	float step = velocity.x;
	float minStep = 0.01f; // Minimum precision
	
	// Start from current position and move towards the wall
	for (int iterations = 0; iterations < 10 && std::abs(step) > minStep; iterations++) {
		testPos.x = currentX + step;
		
		if (CheckCollisionAtPosition(testPos)) {
			// Hit wall, step back
			step *= 0.5f;
		} else {
			// No collision, move forward
			currentX = testPos.x;
			step *= 0.8f; // Reduce step size
		}
	}
	
	// Final adjustment: ensure we're just touching the wall
	testPos.x = currentX;
	float wallContactOffset = wallContactThreshold_ * (isLeftSide ? 1.0f : -1.0f);
	testPos.x += wallContactOffset;
	
	// Verify the contact position doesn't cause collision
	if (!CheckCollisionAtPosition(testPos)) {
		return testPos.x;
	}
	
	return currentX;
}

bool Player::CheckGroundCollision() const {
	if (!mapChipField_) return false;
	
	Vector3 checkPos = worldTransform_.translation_;
	checkPos.y -= playerSize_.y / 2.0f + groundDetectionOffset_;
	
	Vector3 checkSize = {playerSize_.x * 0.9f, 0.1f, playerSize_.z}; // Slightly narrower for more precise detection
	
	return CheckCollisionAtPositionWithScale(checkPos, checkSize);
}

void Player::ApplyWallSlide() {
	// Enhanced wall sliding: check current wall contact state
	bool canSlide = !isOnGround && velocity.y < 0.0f;
	bool hasPreviousWallContact = wasOnWall;
	
	// Allow wall sliding based on current OR previous wall contact (to prevent gaps)
	if (canSlide && (isOnWallLeft || isOnWallRight || hasPreviousWallContact)) {
		if (velocity.y < -wallSlideSpeed) {
			velocity.y = -wallSlideSpeed;
			
#ifdef _DEBUG
			static int slideCounter = 0;
			if (slideCounter++ % 30 == 0) { // Print every half second at 60fps
				printf("Wall sliding: velocity.y = %.3f, walls: L=%s R=%s\n",
					   velocity.y, isOnWallLeft ? "YES" : "NO", isOnWallRight ? "YES" : "NO");
			}
#endif
		}
	}
}

void Player::CheckObjectCollisions() {
	if (!objects_) {
		return;
	}

	for (const auto& object : *objects_) {
		Goal* goal = dynamic_cast<Goal*>(object.get());
		if (goal && goal->IsActive()) {
			bool isColliding = goal->CheckCollisionWithPlayer(worldTransform_.translation_, playerSize_);
			
			if (isColliding && !goal->WasCollidingLastFrame() && goal->CanTriggerCollision()) {
				goal->TriggerCollision();
			}
			
			goal->SetWasCollidingLastFrame(isColliding);
		}
	}
}

float Player::GetDistanceFromSpawn() const {
	Vector3 currentPos = worldTransform_.translation_;
	return static_cast<float>(std::sqrt(
		std::pow(currentPos.x - spawnPosition_.x, 2) + 
		std::pow(currentPos.y - spawnPosition_.y, 2)
	));
}

bool Player::HasLeftSpawnArea(float threshold) const {
	return GetDistanceFromSpawn() > threshold;
}

void Player::ResetToSpawn() {
	// Reset position
	worldTransform_.translation_ = spawnPosition_;
	
	// Reset physics
	velocity = {0.0f, 0.0f};
	
	// Reset states
	isOnGround = false;
	wasOnGround = false;
	isOnWallLeft = false;
	isOnWallRight = false;
	wasOnWall = false;
	isDead = false;
	
	// Reset timers
	jumpBufferTimer = 0.0f;
	wallJumpBufferTimer = 0.0f;
	wallJumpDirectionLockTimer = 0.0f;
	
	// Reset wall jump state
	wallJumpDirection = CollisionDirection::kNone;
	lastCollisionDirection_ = CollisionDirection::kNone;
	
#ifdef _DEBUG
	printf("Player reset to spawn: (%.2f, %.2f)\n", spawnPosition_.x, spawnPosition_.y);
#endif
}

void Player::HandleJumping() {
	// Try wall jump first (higher priority)
	if (CanWallJump()) {
		PerformWallJump();
		return;
	}
	
	// Try regular jump
	if (CanRegularJump()) {
		PerformRegularJump();
	}
}

bool Player::CanWallJump() const {
	bool isInAir = !isOnGround && (velocity.y != 0.0f || !wasOnGround);
	bool hasWallContact = isOnWallLeft || isOnWallRight || wasOnWall;
	bool hasJumpInput = wallJumpBufferTimer > 0.0f;
	
	return isInAir && hasWallContact && hasJumpInput;
}

bool Player::CanRegularJump() const {
	return isOnGround && jumpBufferTimer > 0.0f;
}

void Player::PerformWallJump() {
	// Determine jump direction
	CollisionDirection jumpDir = CollisionDirection::kNone;
	if (isOnWallLeft) {
		jumpDir = CollisionDirection::kLeft;
	} else if (isOnWallRight) {
		jumpDir = CollisionDirection::kRight;
	} else if (wasOnWall && wallJumpDirection != CollisionDirection::kNone) {
		jumpDir = wallJumpDirection;
	}
	
	if (jumpDir != CollisionDirection::kNone) {
		// Apply jump forces
		velocity.y = wallJumpForce;
		velocity.x = (jumpDir == CollisionDirection::kLeft) ? wallJumpHorizontalForce : -wallJumpHorizontalForce;
		
		// Set direction lock and clear buffers
		wallJumpDirection = jumpDir;
		wallJumpDirectionLockTimer = wallJumpDirectionLockTime;
		wallJumpBufferTimer = 0.0f;
		jumpBufferTimer = 0.0f;
		isOnGround = false;
		
#ifdef _DEBUG
		printf("Wall jump: %s wall -> velocity(%.2f, %.2f)\n", 
			   jumpDir == CollisionDirection::kLeft ? "LEFT" : "RIGHT", velocity.x, velocity.y);
#endif
	}
}

void Player::PerformRegularJump() {
	velocity.y = jumpForce;
	isOnGround = false;
	jumpBufferTimer = 0.0f;
	
#ifdef _DEBUG
	printf("Regular jump: velocity.y = %.2f\n", velocity.y);
#endif
}

#ifdef _DEBUG
void Player::ShowDebugWindow() {
	ImGui::Begin("Player Debug");
	
	// Basic info
	ImGui::Text("Position: (%.3f, %.3f)", worldTransform_.translation_.x, worldTransform_.translation_.y);
	ImGui::Text("Velocity: (%.3f, %.3f)", velocity.x, velocity.y);
	ImGui::Text("Player Size: (%.2f, %.2f, %.2f)", playerSize_.x, playerSize_.y, playerSize_.z);
	
	// Ground state
	ImGui::Separator();
	ImGui::Text("=== GROUND STATE ===");
	ImGui::Text("On Ground: %s", isOnGround ? "YES" : "NO");
	ImGui::Text("Was On Ground: %s", wasOnGround ? "YES" : "NO");
	ImGui::Text("Jump Buffer: %.3f", jumpBufferTimer);
	
	// Wall jump state with enhanced info
	ImGui::Separator();
	ImGui::Text("=== WALL CONTACT STATE ===");
	ImGui::Text("Wall Left: %s", isOnWallLeft ? "YES" : "NO");
	ImGui::Text("Wall Right: %s", isOnWallRight ? "YES" : "NO");
	ImGui::Text("Was On Wall: %s", wasOnWall ? "YES" : "NO");
	ImGui::Text("Wall Buffer: %.3f", wallJumpBufferTimer);
	ImGui::Text("Direction Lock: %.3f", wallJumpDirectionLockTimer);
	
	// Wall sliding analysis
	bool canSlide = !isOnGround && velocity.y < 0.0f && (isOnWallLeft || isOnWallRight);
	ImGui::Text("Can Wall Slide: %s", canSlide ? "YES" : "NO");
	if (canSlide) {
		ImGui::Text("Slide Speed: %.3f / %.3f", velocity.y, -wallSlideSpeed);
	}
	
	// Conditions
	ImGui::Separator();
	ImGui::Text("=== CONDITIONS ===");
	ImGui::Text("Can Regular Jump: %s", CanRegularJump() ? "YES" : "NO");
	ImGui::Text("Can Wall Jump: %s", CanWallJump() ? "YES" : "NO");
	
	// Enhanced collision detection parameters
	ImGui::Separator();
	ImGui::Text("=== COLLISION DETECTION ===");
	ImGui::SliderFloat("Wall Detection Range", &wallDetectionRange_, 0.01f, 0.3f, "%.3f");
	ImGui::SliderFloat("Wall Contact Threshold", &wallContactThreshold_, 0.01f, 0.2f, "%.3f");
	ImGui::SliderFloat("Ground Detection Offset", &groundDetectionOffset_, 0.01f, 0.15f, "%.3f");
	
	// Test collision at current position
	Vector3 currentPos = worldTransform_.translation_;
	bool leftWallTest = CheckWallCollisionAtPosition(currentPos, true);
	bool rightWallTest = CheckWallCollisionAtPosition(currentPos, false);
	bool groundTest = CheckGroundCollision();
	
	ImGui::Text("Collision Tests:");
	ImGui::Text("  Left Wall: %s", leftWallTest ? "COLLISION" : "clear");
	ImGui::Text("  Right Wall: %s", rightWallTest ? "COLLISION" : "clear");
	ImGui::Text("  Ground: %s", groundTest ? "COLLISION" : "clear");
	
	// Movement parameters
	ImGui::Separator();
	ImGui::Text("=== MOVEMENT PARAMETERS ===");
	ImGui::SliderFloat("Speed", &speed, 0.01f, 1.0f);
	ImGui::SliderFloat("Jump Force", &jumpForce, 0.1f, 2.0f);
	ImGui::SliderFloat("Wall Jump Force", &wallJumpForce, 0.1f, 2.0f);
	ImGui::SliderFloat("Wall Jump Horizontal", &wallJumpHorizontalForce, 0.1f, 1.0f);
	ImGui::SliderFloat("Wall Slide Speed", &wallSlideSpeed, 0.01f, 0.5f);
	
	// Wall contact visualization
	ImGui::Separator();
	ImGui::Text("=== WALL CONTACT VISUALIZATION ===");
	
	// Show exact wall detection positions
	Vector3 leftCheckPos = currentPos;
	leftCheckPos.x -= (playerSize_.x / 2.0f + wallDetectionRange_);
	Vector3 rightCheckPos = currentPos;
	rightCheckPos.x += (playerSize_.x / 2.0f + wallDetectionRange_);
	
	ImGui::Text("Left Check Pos: (%.3f, %.3f)", leftCheckPos.x, leftCheckPos.y);
	ImGui::Text("Right Check Pos: (%.3f, %.3f)", rightCheckPos.x, rightCheckPos.y);
	
	// Distance to nearest walls
	float leftDistance = 999.0f;
	float rightDistance = 999.0f;
	
	// Simple distance calculation (could be enhanced)
	for (float testX = currentPos.x - 2.0f; testX <= currentPos.x + 2.0f; testX += 0.1f) {
		Vector3 testPos = {testX, currentPos.y, currentPos.z};
		if (CheckCollisionAtPosition(testPos)) {
			if (testX < currentPos.x) {
				leftDistance = std::min(leftDistance, currentPos.x - testX);
			} else {
				rightDistance = std::min(rightDistance, testX - currentPos.x);
			}
		}
	}
	
	ImGui::Text("Distance to Left Wall: %.3f", leftDistance);
	ImGui::Text("Distance to Right Wall: %.3f", rightDistance);
	
	ImGui::End();
}
#endif

bool Player::CheckCollisionAtPositionWithScale(const Vector3& position, const Vector3& size) const {
	if (gameScene_) {
		float currentBlockScale = gameScene_->GetCurrentBlockScale();
		return mapChipField_->CheckScaledCollisionAtPosition(position, size, currentBlockScale);
	} else {
		return mapChipField_->CheckCollisionAtPosition(position, size);
	}
}

bool Player::CheckCollisionAtPosition(const Vector3& position) const {
	return CheckCollisionAtPositionWithScale(position, playerSize_);
}

void Player::ApplyGravity() {
	if (isEnableGravity) {
		velocity.y -= gravity;
		
		if (velocity.y < -maxFallSpeed) {
			velocity.y = -maxFallSpeed;
		}
	}
}

void Player::ClearFrameInputs() {
	isLeft = false;
	isRight = false;
	isJumpTriggered = false;
}
