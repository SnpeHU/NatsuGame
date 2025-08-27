#include "Goal.h"
#include <cmath>

void Goal::Update() {
	worldTransform_.MakeAffineMatrix4x4();
	worldTransform_.TransferMatrix();

#ifdef _DEBUG
	// Goal调试信息
	ImGui::Begin("Goal Debug");
	ImGui::Text("Goal Active: %s", isActive_ ? "Yes" : "No");
	ImGui::Text("Target Map ID: %d", id);
	ImGui::Text("Position: (%.2f, %.2f)", worldTransform_.translation_.x, worldTransform_.translation_.y);
	ImGui::Text("Size: (%.2f, %.2f)", size.x, size.y);
	ImGui::Text("Was Colliding: %s", wasCollidingLastFrame_ ? "Yes" : "No");
	
	// 允许运行时调整Goal属性
	if (ImGui::SliderFloat2("Goal Size", &size.x, 0.5f, 3.0f)) {
		// Goal大小已更改
	}
	
	if (ImGui::InputInt("Target Map ID", &id)) {
		// 目标关卡ID已更改
	}
	
	if (ImGui::Checkbox("Active", &isActive_)) {
		// 激活状态已更改
	}
	
	ImGui::End();
#endif // DEBUG
}

bool Goal::CheckCollisionWithPlayer(const Vector3& playerPosition, const Vector3& playerSize) const {
	if (!isActive_) {
		return false;
	}

	// AABB (Axis-Aligned Bounding Box) 碰撞检测
	Vector3 goalPos = worldTransform_.translation_;
	Vector3 goalSize = GetGoalSize();

	// 计算两个矩形的边界
	float goalLeft = goalPos.x - goalSize.x / 2.0f;
	float goalRight = goalPos.x + goalSize.x / 2.0f;
	float goalBottom = goalPos.y - goalSize.y / 2.0f;
	float goalTop = goalPos.y + goalSize.y / 2.0f;

	float playerLeft = playerPosition.x - playerSize.x / 2.0f;
	float playerRight = playerPosition.x + playerSize.x / 2.0f;
	float playerBottom = playerPosition.y - playerSize.y / 2.0f;
	float playerTop = playerPosition.y + playerSize.y / 2.0f;

	// 检查是否重叠
	bool xOverlap = (playerLeft < goalRight) && (playerRight > goalLeft);
	bool yOverlap = (playerBottom < goalTop) && (playerTop > goalBottom);

	bool collision = xOverlap && yOverlap;

#ifdef _DEBUG
	// 碰撞调试信息
	if (collision) {
		ImGui::Begin("Collision Active");
		ImGui::Text("COLLISION DETECTED!");
		ImGui::Text("Goal Position: (%.2f, %.2f)", goalPos.x, goalPos.y);
		ImGui::Text("Player Position: (%.2f, %.2f)", playerPosition.x, playerPosition.y);
		ImGui::Text("X Overlap: %s", xOverlap ? "Yes" : "No");
		ImGui::Text("Y Overlap: %s", yOverlap ? "Yes" : "No");
		ImGui::End();
	}
#endif

	return collision;
}
