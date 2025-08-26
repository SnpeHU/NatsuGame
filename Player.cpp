#include "Player.h"
#include "MapChipField.h"
#include <cmath>

void Player::Update() {
	Move();
	worldTransform_.MakeAffineMatrix4x4();
	worldTransform_.TransferMatrix();

#ifdef _DEBUG
	// ImGuiのデバッグウィンドウ
	ImGui::Begin("Player Debug");
	ImGui::Text("Player Position: (%.2f, %.2f)", worldTransform_.translation_.x, worldTransform_.translation_.y);
	ImGui::Text("Velocity: (%.2f, %.2f)", velocity.x, velocity.y);
	ImGui::Text("Forward: (%.2f, %.2f)", forward.x, forward.y);
	ImGui::Text("Player Size: (%.2f, %.2f, %.2f)", playerSize_.x, playerSize_.y, playerSize_.z);
	ImGui::Text("On Ground: %s", isOnGround ? "Yes" : "No");
	ImGui::Text("Coyote Timer: %.3f", coyoteTimer);
	ImGui::Text("Jump Buffer Timer: %.3f", jumpBufferTimer);
	
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
	
	ImGui::End();
#endif // DEBUG  
}

void Player::Move() {
	// 处理输入
	HandleInput();
	
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
			coyoteTimer = coyoteTime;
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
	isJumpPressed = Input::GetInstance()->PushKey(DIK_SPACE) || Input::GetInstance()->PushKey(DIK_W);
	isJumpTriggered = Input::GetInstance()->TriggerKey(DIK_SPACE) || Input::GetInstance()->TriggerKey(DIK_W);
	
	// 跳跃缓冲：如果按下跳跃键，记录缓冲时间
	if (isJumpTriggered) {
		jumpBufferTimer = jumpBufferTime;
	}
	
	// 更新跳跃缓冲计时器
	if (jumpBufferTimer > 0.0f) {
		jumpBufferTimer -= 1.0f / 60.0f; // 假设60FPS
	}
}

void Player::UpdatePhysics() {
	// 更新郊狼时间计时器
	if (!isOnGround && coyoteTimer > 0.0f) {
		coyoteTimer -= 1.0f / 60.0f; // 假设60FPS
	}
	
	// 处理跳跃
	bool canJump = (isOnGround || coyoteTimer > 0.0f) && jumpBufferTimer > 0.0f;
	if (canJump) {
		velocity.y = jumpForce;
		isOnGround = false;
		coyoteTimer = 0.0f;
		jumpBufferTimer = 0.0f;
	}
	
	// 处理横向移动
	forward.x = float(isRight - isLeft);
	velocity.x = forward.x * speed;
	
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
	
	// 如果之前在地面上但现在不在，开始郊狼时间
	if (wasOnGround && !groundDetected) {
		coyoteTimer = coyoteTime;
	}
	
	isOnGround = groundDetected;
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
