#pragma once
#include <KamataEngine.h>
using namespace KamataEngine;
class MapChipField;
class Player : public Object3d {
public:
	Player() = default;
	~Player() = default;
	//void Initialize(Model* model) override;
	void Update() override;
	//void Draw() override;

	void SetMapChipField(MapChipField* mapChipField) { mapChipField_ = mapChipField; }
	void Move();

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
};