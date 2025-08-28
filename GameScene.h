#pragma once
#include "KamataEngine.h"
#include "IScene.h"
#include "MapChipField.h"
#include "Player.h"
#include "CameraController.h"
#include "Goal.h"
#include "Skydome.h"
#include "Fade.h"

// 游戏阶段枚举
enum class GameStage {
	kPreparation,   // 准备阶段：加载关卡后的默认阶段
	kGameplay,      // 游戏阶段：当玩家离开Spawn格子时进入游戏阶段
	kEnding         // 结束阶段：玩家到达Goal格子或死亡时进入结束阶段
};

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

	// 游戏阶段管理
	void SetGameStage(GameStage stage);
	GameStage GetGameStage() const { return currentStage_; }
	void UpdateGameStage();
	void HandlePreparationStage();
	void HandleGameplayStage();
	void HandleEndingStage();

	// 玩家死亡处理
	void OnPlayerDeath();

	// 新的方法：更新地图方块缩放
	void UpdateBlockScaling();
	float GetCurrentBlockScale() const;

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

	// Skydome
	std::unique_ptr<Skydome> skydome_;
	KamataEngine::Model* skydomeModel_ = nullptr;

	//fade
	std::unique_ptr<Fade> fade_;

	//titie
	KamataEngine::Sprite* titleSprite_ = nullptr;


	// 存储所有除玩家和地图外的物体
	std::vector<std::unique_ptr<Object3d>> objects_;


	KamataEngine::WorldTransform worldTransform_;

	KamataEngine::Camera camera_;
	std::unique_ptr<CameraController> cameraController_;
	

	// debugカメラ
	KamataEngine::DebugCamera* debugCamera_ = nullptr;
	bool isDebugCameraActive_ = false;

	int mapID = 0;

	// 场景切换相关
	bool isPendingSceneChange_ = false;
	float sceneChangeTimer_ = 0.0f;
	float sceneChangeDelay_ = 1.0f; // 1秒延迟
	int pendingTargetMapID_ = 0;

	// 游戏阶段相关
	GameStage currentStage_ = GameStage::kPreparation;
	GameStage previousStage_ = GameStage::kPreparation;
	Vector3 spawnPosition_ = {0.0f, 0.0f, 0.0f};  // 玩家初始生成位置
	bool hasLeftSpawn_ = false;  // 玩家是否已经离开生成点
	float stageTransitionTimer_ = 0.0f;  // 阶段转换计时器
	float endingStageDelay_ = 1.0f;  // 结束阶段延迟时间

	// 游戏倒计时相关
	float gameLifeTime_ = 25.0f;  // 游戏生命时间（秒）
	float maxGameLifeTime_ = 25.0f;  // 最大游戏生命时间
	bool isLifeTimerActive_ = false;  // 生命计时器是否激活

	// 地图方块缩放相关
	float currentBlockScale_ = 1.0f;  // 当前方块缩放比例
	float minBlockScale_ = 0.01f;      // 最小方块缩放比例
	bool isBlockScalingEnabled_ = true;  // 是否启用方块缩放
};
