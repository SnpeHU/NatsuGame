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
	
	// 碰撞调试信息
	const char* collisionText[] = {"None", "Left", "Right", "Up", "Down"};
	ImGui::Text("Last Collision: %s", collisionText[static_cast<int>(lastCollisionDirection_)]);
	
	// 允许在运行时调整玩家大小
	if (ImGui::SliderFloat3("Player Size", &playerSize_.x, 0.1f, 2.0f)) {
		// 玩家大小已更改
	}
	if (ImGui::SliderFloat("Speed", &speed, 0.01f, 1.0f)) {
		// 速度已更改
	}
	
	ImGui::End();
#endif // DEBUG  
}

void Player::Move() {
	if (Input::GetInstance()->PushKey(DIK_A)) {
		isLeft = true;
	}
	if (Input::GetInstance()->PushKey(DIK_D)) {
		isRight = true;
	}
	if (Input::GetInstance()->PushKey(DIK_W)) {
		isUp = true;
	}
	if (Input::GetInstance()->PushKey(DIK_S)) {
		isDown = true;
	}

	forward.x = float(isRight - isLeft);
	forward.y = float(isUp - isDown);
	velocity.x = forward.x * speed;
	velocity.y = forward.y * speed;
	
	if (isEnableGravity) {
		velocity.y -= gravity;
	}

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
		velocity.y = 0.0f; // 停止Y轴移动
		lastCollisionDirection_ = velocity.y > 0 ? CollisionDirection::kUp : CollisionDirection::kDown;
	}

	isRight = false;
	isLeft = false;
	isUp = false;
	isDown = false;
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
