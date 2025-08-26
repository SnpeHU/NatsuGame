#include "Player.h"

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
	worldTransform_.translation_.x += velocity.x;
	worldTransform_.translation_.y += velocity.y;


	isRight = false;
	isLeft = false;
	isUp = false;
	isDown = false;
	//isJumpBottomDown = false;
}
