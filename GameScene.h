#pragma once
#include "KamataEngine.h"
#include "IScene.h"
#include "MapChipField.h"
#include "Player.h"
#include "CameraController.h"
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

	private:

	// block
	std::vector<std::vector<KamataEngine::WorldTransform*>> worldTransformBlocks_;
	KamataEngine::Model* blockModel_ = nullptr;
	MapChipField* mapChipField_ = nullptr;

	// プレイヤー
	std::unique_ptr<Player> player_;
	KamataEngine::Model* playerModel_ = nullptr;


	KamataEngine::WorldTransform worldTransform_;

	KamataEngine::Camera camera_;
	std::unique_ptr<CameraController> cameraController_;
	

	// debugカメラ
	KamataEngine::DebugCamera* debugCamera_ = nullptr;
	bool isDebugCameraActive_ = false;

	
};
