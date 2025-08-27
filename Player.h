#pragma once
#include <KamataEngine.h>
#include <vector>
#include <memory>
using namespace KamataEngine;
class MapChipField;
class Goal;

enum class CollisionDirection {
    kNone,
    kLeft,
    kRight,
    kUp,
    kDown
};

class Player : public Object3d {
public:
	Player() = default;
	~Player() = default;
	void Update() override;

	void SetMapChipField(MapChipField* mapChipField) { mapChipField_ = mapChipField; }
	void Move();

	// 碰撞检测相关方法
	bool CheckCollisionAtPosition(const Vector3& position);
	CollisionDirection CheckCollisionDirection(const Vector3& currentPos, const Vector3& newPos);
	Vector3 GetPlayerSize() const { return playerSize_; }
	void SetPlayerSize(const Vector3& size) { playerSize_ = size; }

	// 对象碰撞检测相关方法
	void SetObjects(const std::vector<std::unique_ptr<Object3d>>* objects) { objects_ = objects; }
	void CheckObjectCollisions();

private:
	MapChipField* mapChipField_ = nullptr;
	const std::vector<std::unique_ptr<Object3d>>* objects_ = nullptr;
	
	Vector2 forward = {0.0f, 1.0f};
	Vector2 velocity = {0.0f, 0.0f};
	Vector2 acceleration = {0.0f, 0.0f};
	float speed = 0.2f;  // 横向移动速度
	float jumpForce = 0.6f;  // 跳跃力度
	float gravity = 0.025f;   // 重力加速度
	float maxFallSpeed = 0.8f;  // 最大下落速度
	bool isEnableGravity = true;  // 启用重力

	// 输入状态
	bool isLeft = false;
	bool isRight = false;
	bool isJumpPressed = false;
	bool isJumpTriggered = false;

	// 地面检测和跳跃状态
	bool isOnGround = false;
	bool wasOnGround = false;
	float coyoteTime = 0.1f;  // 郊狼时间（离开地面后仍可跳跃的时间）
	float coyoteTimer = 0.0f;
	float jumpBufferTime = 0.1f;  // 跳跃缓冲时间
	float jumpBufferTimer = 0.0f;

	// 玩家碰撞箱大小 (默认稍小于地图瓦片)
	Vector3 playerSize_ = {1.8f, 1.8f, 2.0f};

	// 碰撞调试信息
	CollisionDirection lastCollisionDirection_ = CollisionDirection::kNone;

	// 新增的私有方法
	void HandleInput();
	void UpdatePhysics();
	void CheckGroundCollision();
	bool IsOnGround();
};