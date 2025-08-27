#include "Player.h"
#include "MapChipField.h"
#include "Goal.h"
#include <cmath>

void Player::Update() {
	Move();
	CheckObjectCollisions();
	worldTransform_.MakeAffineMatrix4x4();
	worldTransform_.TransferMatrix();

#ifdef _DEBUG
	// ImGui的调试窗口
	ImGui::Begin("Player Debug");
	ImGui::Text("Player Position: (%.2f, %.2f)", worldTransform_.translation_.x, worldTransform_.translation_.y);
	ImGui::Text("Velocity: (%.2f, %.2f)", velocity.x, velocity.y);
	ImGui::Text("Forward: (%.2f, %.2f)", forward.x, forward.y);
	ImGui::Text("Player Size: (%.2f, %.2f, %.2f)", playerSize_.x, playerSize_.y, playerSize_.z);
	ImGui::Text("On Ground: %s", isOnGround ? "Yes" : "No");
	ImGui::Text("Jump Buffer Timer: %.3f", jumpBufferTimer);
	
	// 蹬墙跳调试信息
	ImGui::Text("On Wall Left: %s", isOnWallLeft ? "Yes" : "No");
	ImGui::Text("On Wall Right: %s", isOnWallRight ? "Yes" : "No");
	ImGui::Text("Wall Jump Buffer Timer: %.3f", wallJumpBufferTimer);
	ImGui::Text("Wall Jump Direction Lock Timer: %.3f", wallJumpDirectionLockTimer);
	
	// 碰撞调试信息
	const char* collisionText[] = {"None", "Left", "Right", "Up", "Down"};
	ImGui::Text("Last Collision: %s", collisionText[static_cast<int>(lastCollisionDirection_)]);
	
	// 允许在运行时调整玩家参数
	if (ImGui::SliderFloat3("Player Size", &playerSize_.x, 0.1f, 2.0f)) {
		// 玩家大小已更改
	}
	if (ImGui::SliderFloat("Speed", &speed, 0.01f, 1.0f)) {
		// 速度已更改
	}
	if (ImGui::SliderFloat("Jump Force", &jumpForce, 0.1f, 2.0f)) {
		// 跳跃力度已更改
	}
	if (ImGui::SliderFloat("Gravity", &gravity, 0.001f, 0.1f)) {
		// 重力已更改
	}
	
	// 蹬墙跳参数调整
	if (ImGui::SliderFloat("Wall Jump Force", &wallJumpForce, 0.1f, 2.0f)) {
		// 蹬墙跳力度已更改
	}
	if (ImGui::SliderFloat("Wall Jump Horizontal Force", &wallJumpHorizontalForce, 0.1f, 1.0f)) {
		// 蹬墙跳水平力度已更改
	}
	if (ImGui::SliderFloat("Wall Slide Speed", &wallSlideSpeed, 0.01f, 0.5f)) {
		// 贴墙滑行速度已更改
	}
	
	ImGui::End();
#endif // DEBUG  
}

void Player::Move() {
	// 处理输入
	HandleInput();
	
	// 检查墙体碰撞
	CheckWallCollision();
	
	// 更新物理
	UpdatePhysics();
	
	// 检查地面碰撞
	CheckGroundCollision();

	// 保存当前位置
	Vector3 currentPosition = worldTransform_.translation_;
	
	// 重置碰撞方向
	lastCollisionDirection_ = CollisionDirection::kNone;
	
	// 分别检查X轴和Y轴的移动
	// X轴移动碰撞检测
	Vector3 newPositionX = currentPosition;
	newPositionX.x += velocity.x;
	
	if (!CheckCollisionAtPosition(newPositionX)) {
		worldTransform_.translation_.x = newPositionX.x;
	} else {
		velocity.x = 0.0f; // 停止X轴移动
		lastCollisionDirection_ = velocity.x > 0 ? CollisionDirection::kRight : CollisionDirection::kLeft;
	}

	// Y轴移动碰撞检测
	Vector3 newPositionY = worldTransform_.translation_;
	newPositionY.y += velocity.y;
	
	if (!CheckCollisionAtPosition(newPositionY)) {
		worldTransform_.translation_.y = newPositionY.y;
	} else {
		// Y轴碰撞处理
		if (velocity.y < 0.0f) {
			// 向下移动碰撞（着地）
			isOnGround = true;
			lastCollisionDirection_ = CollisionDirection::kDown;
		} else {
			// 向上移动碰撞（撞到天花板）
			lastCollisionDirection_ = CollisionDirection::kUp;
		}
		velocity.y = 0.0f;
	}

	// 重置输入状态
	isLeft = false;
	isRight = false;
	isJumpTriggered = false;
}

void Player::HandleInput() {
	// 处理左右移动输入
	if (Input::GetInstance()->PushKey(DIK_A) || Input::GetInstance()->PushKey(DIK_LEFT)) {
		isLeft = true;
	}
	if (Input::GetInstance()->PushKey(DIK_D) || Input::GetInstance()->PushKey(DIK_RIGHT)) {
		isRight = true;
	}
	
	// 处理跳跃输入
	//isJumpPressed = Input::GetInstance()->PushKey(DIK_SPACE) || Input::GetInstance()->PushKey(DIK_W);
	isJumpPressed = Input::GetInstance()->TriggerKey(DIK_SPACE) || Input::GetInstance()->PushKey(DIK_W);
	isJumpTriggered = Input::GetInstance()->TriggerKey(DIK_SPACE) || Input::GetInstance()->TriggerKey(DIK_W);
	
	// 跳跃缓冲：如果按下跳跃键，记录缓冲时间
	if (isJumpTriggered) {
		jumpBufferTimer = jumpBufferTime;
		wallJumpBufferTimer = wallJumpBufferTime; // 同时设置蹬墙跳缓冲
	}
	
	// 更新跳跃缓冲计时器
	if (jumpBufferTimer > 0.0f) {
		jumpBufferTimer -= 1.0f / 60.0f; // 假设60FPS
	}
	
	// 更新蹬墙跳缓冲计时器
	if (wallJumpBufferTimer > 0.0f) {
		wallJumpBufferTimer -= 1.0f / 60.0f;
	}
	
	// 更新蹬墙跳方向锁定计时器
	if (wallJumpDirectionLockTimer > 0.0f) {
		wallJumpDirectionLockTimer -= 1.0f / 60.0f;
	}
}

void Player::UpdatePhysics() {
	// 存储上一帧的墙体状态
	wasOnWall = isOnWallLeft || isOnWallRight;
	
	// 处理蹬墙跳
	HandleWallJump();
	
	// 处理普通跳跃
	bool canJump = (isOnGround) && jumpBufferTimer > 0.0f;
	if (canJump) {
		velocity.y = jumpForce;
		isOnGround = false;
		jumpBufferTimer = 0.0f;
	}
	
	// 处理横向移动（考虑蹬墙跳方向锁定）
	if (wallJumpDirectionLockTimer <= 0.0f) {
		forward.x = float(isRight - isLeft);
		velocity.x = forward.x * speed;
	}
	
	// 处理贴墙滑行
	UpdateWallSlide();
	
	// 应用重力
	if (isEnableGravity) {
		velocity.y -= gravity;
		
		// 限制最大下落速度
		if (velocity.y < -maxFallSpeed) {
			velocity.y = -maxFallSpeed;
		}
	}
	
	// 存储上一帧的地面状态
	wasOnGround = isOnGround;
}

void Player::CheckGroundCollision() {
	// 检查是否在地面上
	Vector3 checkPosition = worldTransform_.translation_;
	checkPosition.y -= playerSize_.y / 2.0f + 0.1f; // 稍微向下检查
	
	bool groundDetected = IsOnGround();
	
	isOnGround = groundDetected;
}

void Player::CheckWallCollision() {
	// 检查左墙碰撞
	isOnWallLeft = IsOnWallLeft();
	
	// 检查右墙碰撞
	isOnWallRight = IsOnWallRight();
}

bool Player::IsOnWallLeft() {
	if (!mapChipField_) {
		return false;
	}
	
	// 检查玩家左侧是否接触墙体
	Vector3 checkPosition = worldTransform_.translation_;
	checkPosition.x -= playerSize_.x / 2.0f + 0.1f; // 向左偏移来检查墙体
	
	Vector3 checkSize = playerSize_;
	checkSize.x = 0.2f; // 只检查很小的宽度
	checkSize.y *= 0.8f; // 减少高度检查范围，避免地面干扰
	
	return mapChipField_->CheckCollisionAtPosition(checkPosition, checkSize);
}

bool Player::IsOnWallRight() {
	if (!mapChipField_) {
		return false;
	}
	
	// 检查玩家右侧是否接触墙体
	Vector3 checkPosition = worldTransform_.translation_;
	checkPosition.x += playerSize_.x / 2.0f + 0.1f; // 向右偏移来检查墙体
	
	Vector3 checkSize = playerSize_;
	checkSize.x = 0.2f; // 只检查很小的宽度
	checkSize.y *= 0.8f; // 减少高度检查范围，避免地面干扰
	
	return mapChipField_->CheckCollisionAtPosition(checkPosition, checkSize);
}

void Player::HandleWallJump() {
	// 检查是否可以进行蹬墙跳
	bool canWallJump = (isOnWallLeft || isOnWallRight || wasOnWall) && !isOnGround && wallJumpBufferTimer > 0.0f;
	
	if (canWallJump) {
		// 确定蹬墙跳方向
		CollisionDirection jumpDirection = CollisionDirection::kNone;
		
		if (isOnWallLeft || (wasOnWall && wallJumpDirection == CollisionDirection::kLeft)) {
			jumpDirection = CollisionDirection::kLeft;
		} else if (isOnWallRight || (wasOnWall && wallJumpDirection == CollisionDirection::kRight)) {
			jumpDirection = CollisionDirection::kRight;
		}
		
		if (jumpDirection != CollisionDirection::kNone) {
			// 执行蹬墙跳
			velocity.y = wallJumpForce;
			
			// 设置水平速度（向远离墙体的方向）
			if (jumpDirection == CollisionDirection::kLeft) {
				velocity.x = wallJumpHorizontalForce; // 向右跳
			} else if (jumpDirection == CollisionDirection::kRight) {
				velocity.x = -wallJumpHorizontalForce; // 向左跳
			}
			
			// 设置方向锁定
			wallJumpDirection = jumpDirection;
			wallJumpDirectionLockTimer = wallJumpDirectionLockTime;
			
			// 重置缓冲计时器
			wallJumpBufferTimer = 0.0f;
			jumpBufferTimer = 0.0f;
			
#ifdef _DEBUG
			printf("Player: Wall jump executed from %s wall\n", 
				   jumpDirection == CollisionDirection::kLeft ? "left" : "right");
#endif
		}
	}
}

void Player::UpdateWallSlide() {
	// 只有在空中且贴着墙时才进行墙体滑行
	if (!isOnGround && (isOnWallLeft || isOnWallRight) && velocity.y < 0.0f) {
		// 减缓下落速度
		if (velocity.y < -wallSlideSpeed) {
			velocity.y = -wallSlideSpeed;
		}
	}
}

bool Player::IsOnGround() {
	if (!mapChipField_) {
		return false;
	}
	
	// 检查玩家底部是否接触地面
	Vector3 checkPosition = worldTransform_.translation_;
	checkPosition.y -= playerSize_.y / 2.0f + 0.1f; // 向下偏移一点点来检查地面
	
	Vector3 checkSize = playerSize_;
	checkSize.y = 0.2f; // 只检查很小的高度
	
	return mapChipField_->CheckCollisionAtPosition(checkPosition, checkSize);
}

bool Player::CheckCollisionAtPosition(const Vector3& position) {
	if (!mapChipField_) {
		return false; // 没有地图数据时不进行碰撞检测
	}
	
	return mapChipField_->CheckCollisionAtPosition(position, playerSize_);
}

CollisionDirection Player::CheckCollisionDirection(const Vector3& currentPos, const Vector3& newPos) {
	if (!mapChipField_) {
		return CollisionDirection::kNone;
	}
	
	// 检查当前位置是否已经在碰撞中
	if (CheckCollisionAtPosition(currentPos)) {
		return CollisionDirection::kNone; // 已在碰撞中，无法确定方向
	}
	
	// 检查新位置是否会发生碰撞
	if (!CheckCollisionAtPosition(newPos)) {
		return CollisionDirection::kNone; // 没有碰撞
	}
	
	// 计算移动方向
	Vector3 movement = {newPos.x - currentPos.x, newPos.y - currentPos.y, 0.0f};
	
	// 根据移动方向判断碰撞方向
	if (abs(movement.x) > abs(movement.y)) {
		return movement.x > 0 ? CollisionDirection::kRight : CollisionDirection::kLeft;
	} else {
		return movement.y > 0 ? CollisionDirection::kUp : CollisionDirection::kDown;
	}
}

void Player::CheckObjectCollisions() {
	if (!objects_) {
		return;
	}

	int goalCount = 0;
	int activeGoalCount = 0;
	
	for (const auto& object : *objects_) {
		// 尝试转换为Goal对象
		Goal* goal = dynamic_cast<Goal*>(object.get());
		if (goal) {
			goalCount++;
			if (goal->IsActive()) {
				activeGoalCount++;
				bool isColliding = goal->CheckCollisionWithPlayer(worldTransform_.translation_, playerSize_);
				
				// 检查是否为新的碰撞且可以触发
				// 只有当这一帧发生碰撞，但上一帧没有碰撞，且Goal可以触发时，才触发回调
				if (isColliding && !goal->WasCollidingLastFrame() && goal->CanTriggerCollision()) {
#ifdef _DEBUG
					printf("Player: New Goal collision detected, triggering callback\n");
#endif
					goal->TriggerCollision();
				}
				
				// 更新碰撞状态，用于下一帧比较
				goal->SetWasCollidingLastFrame(isColliding);
			}
		}
		
		// 可以在这里添加其他类型的对象碰撞检测
		// 例如：Enemy*, Collectible* 等
	}

#ifdef _DEBUG
	// 在Player Debug窗口中添加对象碰撞信息
	ImGui::Begin("Player Debug");
	// ...existing debug info...
	ImGui::Separator();
	ImGui::Text("Object Collision Info:");
	ImGui::Text("Total Goals: %d", goalCount);
	ImGui::Text("Active Goals: %d", activeGoalCount);
	ImGui::End();
#endif
}

float Player::GetDistanceFromSpawn() const {
	Vector3 currentPos = worldTransform_.translation_;
	return static_cast<float>(sqrt(pow(currentPos.x - spawnPosition_.x, 2) + pow(currentPos.y - spawnPosition_.y, 2)));
}

bool Player::HasLeftSpawnArea(float threshold) const {
	return GetDistanceFromSpawn() > threshold;
}

void Player::ResetToSpawn() {
	// 重置玩家位置到生成点
	worldTransform_.translation_ = spawnPosition_;
	
	// 重置物理状态
	velocity = {0.0f, 0.0f};
	acceleration = {0.0f, 0.0f};
	
	// 重置跳跃状态
	isOnGround = false;
	wasOnGround = false;
	jumpBufferTimer = 0.0f;
	
	// 重置蹬墙跳状态
	isOnWallLeft = false;
	isOnWallRight = false;
	wasOnWall = false;
	wallJumpBufferTimer = 0.0f;
	wallJumpDirectionLockTimer = 0.0f;
	wallJumpDirection = CollisionDirection::kNone;
	
	// 重置死亡状态
	isDead = false;
	
	// 重置碰撞信息
	lastCollisionDirection_ = CollisionDirection::kNone;

#ifdef _DEBUG
	printf("Player: Reset to spawn position (%.2f, %.2f)\n", spawnPosition_.x, spawnPosition_.y);
#endif
}
