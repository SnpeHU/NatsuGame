#pragma once
#include <KamataEngine.h>
#include <vector>
#include <memory>
using namespace KamataEngine;
class MapChipField;
class Goal;
class GameScene;

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
	void Initialize(Model* model) override;

	void Update() override;

	void SetMapChipField(MapChipField* mapChipField) { mapChipField_ = mapChipField; }
	void SetGameScene(GameScene* gameScene) { gameScene_ = gameScene; }
	void Move();

	// 碰撞检测相关方法
	bool CheckCollisionAtPosition(const Vector3& position) const;
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
	GameScene* gameScene_ = nullptr;
	const std::vector<std::unique_ptr<Object3d>>* objects_ = nullptr;
	
	// Physics properties
	Vector2 velocity = {0.0f, 0.0f};
	float speed = 0.25f;
	float jumpForce = 0.6f;
	float gravity = 0.025f;
	float maxFallSpeed = 0.8f;
	bool isEnableGravity = true;

	// State flags
	bool isDead = false;
	bool isOnGround = false;
	bool wasOnGround = false;
	bool isOnWallLeft = false;
	bool isOnWallRight = false;
	bool wasOnWall = false;

	// Input state (frame-based)
	bool isLeft = false;
	bool isRight = false;
	bool isJumpTriggered = false;

	// Jump parameters
	float jumpBufferTime = 0.1f;
	float jumpBufferTimer = 0.0f;

	// Wall jump parameters
	float wallJumpForce = 0.45f;
	float wallJumpHorizontalForce = 0.4f;
	float wallSlideSpeed = 0.1f;
	float wallJumpBufferTime = 0.15f;
	float wallJumpBufferTimer = 0.0f;
	float wallJumpDirectionLockTime = 0.4f;
	float wallJumpDirectionLockTimer = 0.0f;
	CollisionDirection wallJumpDirection = CollisionDirection::kNone;

	// Collision properties
	Vector3 playerSize_ = {0.5f, 0.5f, 0.5f};
	CollisionDirection lastCollisionDirection_ = CollisionDirection::kNone;

	// Collision detection parameters - 优化的碰撞检测参数
	float wallDetectionRange_ = 0.12f;     // 增加墙体检测范围
	float wallContactThreshold_ = 0.08f;   // 墙体接触阈值
	float groundDetectionOffset_ = 0.05f;  // 地面检测偏移

	// Spawn related
	Vector3 spawnPosition_ = {0.0f, 0.0f, 0.0f};

	// Core movement methods
	void UpdateTimers(float deltaTime);
	void HandleInput();
	void UpdatePhysics();
	void ApplyMovementWithCollision();
	void UpdateCollisionStatesPostMovement(); // 移动后更新碰撞状态
	void ClearFrameInputs();

	// Jump handling
	void HandleJumping();
	bool CanWallJump() const;
	bool CanRegularJump() const;
	void PerformWallJump();
	void PerformRegularJump();

	// Physics methods
	void ApplyWallSlide();
	void ApplyGravity();

	// 优化的碰撞检测方法
	bool CheckGroundCollision() const;
	bool CheckWallCollision(bool isLeftSide) const;
	bool CheckWallCollisionAtPosition(const Vector3& position, bool isLeftSide) const;
	bool CheckCollisionAtPositionWithScale(const Vector3& position, const Vector3& size) const;
	
	// 新增：墙体贴合方法
	Vector3 AdjustPositionToWall(const Vector3& currentPos, const Vector3& targetPos, bool isLeftWall);
	float FindWallContactPosition(const Vector3& centerPos, bool isLeftSide) const;

#ifdef _DEBUG
	void ShowDebugWindow();
#endif
};