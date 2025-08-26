#pragma once
#include <KamataEngine.h>
using namespace KamataEngine;
class MapChipField;

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

private:
	MapChipField* mapChipField_ = nullptr;
	Vector2 forward = {0.0f, 1.0f};
	Vector2 velocity = {0.0f, 0.0f};
	Vector2 acceleration = {0.0f, 0.0f};
	float speed = 0.2f;
	bool isEnableGravity = false;
	float gravity = 0.4f;

	bool isLeft = false;
	bool isRight = false;
	bool isUp = false;
	bool isDown = false;

	// 玩家碰撞箱大小 (默认稍小于地图瓦片)
	Vector3 playerSize_ = {1.8f,1.8f, 2.0f};

	// 碰撞调试信息
	CollisionDirection lastCollisionDirection_ = CollisionDirection::kNone;
};