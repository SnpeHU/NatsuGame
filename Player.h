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

	void SetIsDead(bool dead) { isDead = dead; }
	bool GetIsDead() const { return isDead; }

	// 游戏阶段支持方法
	void SetSpawnPosition(const Vector3& spawnPos) { spawnPosition_ = spawnPos; }
	Vector3 GetSpawnPosition() const { return spawnPosition_; }
	float GetDistanceFromSpawn() const;
	bool HasLeftSpawnArea(float threshold = 1.0f) const;

	// 重置玩家状态（用于重新开始关卡）
	void ResetToSpawn();

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
	bool isDead = false;         // 死亡状态

	// 输入状态
	bool isLeft = false;
	bool isRight = false;
	bool isJumpPressed = false;
	bool isJumpTriggered = false;

	// 地面检测和跳跃状态
	bool isOnGround = false;
	bool wasOnGround = false;

	float jumpBufferTime = 0.1f;  // 跳跃缓冲时间
	float jumpBufferTimer = 0.0f;

	// 蹬墙跳相关变量
	bool isOnWallLeft = false;   // 是否贴着左墙
	bool isOnWallRight = false;  // 是否贴着右墙
	bool wasOnWall = false;      // 上一帧是否贴着墙
	float wallJumpForce = 0.45f;  // 蹬墙跳跳跃力度
	float wallJumpHorizontalForce = 0.25f;  // 蹬墙跳水平推力
	float wallSlideSpeed = 0.1f; // 贴墙滑行速度
	float wallJumpBufferTime = 0.15f;  // 蹬墙跳缓冲时间
	float wallJumpBufferTimer = 0.0f;  // 蹬墙跳缓冲计时器
	float wallJumpDirectionLockTime = 0.2f;  // 蹬墙跳后方向锁定时间
	float wallJumpDirectionLockTimer = 0.0f; // 方向锁定计时器
	CollisionDirection wallJumpDirection = CollisionDirection::kNone; // 蹬墙跳方向

	// 玩家碰撞箱大小 (默认稍小于地图瓦片)
	Vector3 playerSize_ = {1.8f, 1.8f, 1.8f};

	// 碰撞调试信息
	CollisionDirection lastCollisionDirection_ = CollisionDirection::kNone;

	// 生成点相关
	Vector3 spawnPosition_ = {0.0f, 0.0f, 0.0f};

	// 私有方法
	void HandleInput();
	void UpdatePhysics();
	void CheckGroundCollision();
	bool IsOnGround();
	
	// 蹬墙跳相关方法
	void CheckWallCollision();
	bool IsOnWallLeft();
	bool IsOnWallRight();
	void HandleWallJump();
	void UpdateWallSlide();
};