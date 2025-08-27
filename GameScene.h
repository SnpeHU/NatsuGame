#pragma once
#include "KamataEngine.h"
#include "IScene.h"
#include "MapChipField.h"
#include "Player.h"
#include "CameraController.h"
#include "Goal.h"

class GameScene : public IScene{
	public:
	GameScene() = default;
	~GameScene();

	void Initialize() override;
	void OnEnter() override;

	void Update() override;

	void Draw() override;

	void OnExit() override;

	void GenerateBlocks();
	void CameraUpdate();
	void SetCameraMapBounds();

	void SetMapID(int newMapID) { mapID = newMapID; }
	int GetMapID() const { return mapID; }

	// 碰撞回调函数
	void OnGoalCollision(Goal* goal);

	private:

	// block
	std::vector<std::vector<KamataEngine::WorldTransform*>> worldTransformBlocks_;
	KamataEngine::Model* blockModel_ = nullptr;
	MapChipField* mapChipField_ = nullptr;

	// プレイヤー
	std::unique_ptr<Player> player_;
	KamataEngine::Model* playerModel_ = nullptr;

	//Goal
	KamataEngine::Model* goalModel_ = nullptr;

	// 存储所有除玩家和地图外的物体
	std::vector<std::unique_ptr<Object3d>> objects_;


	KamataEngine::WorldTransform worldTransform_;

	KamataEngine::Camera camera_;
	std::unique_ptr<CameraController> cameraController_;
	

	// debugカメラ
	KamataEngine::DebugCamera* debugCamera_ = nullptr;
	bool isDebugCameraActive_ = false;

	int mapID = 1;

	// 场景切换相关
	bool isPendingSceneChange_ = false;
	float sceneChangeTimer_ = 0.0f;
	float sceneChangeDelay_ = 1.0f; // 1秒延迟
	int pendingTargetMapID_ = 0;
	
};
