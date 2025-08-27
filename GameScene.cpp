#include "GameScene.h"
#include"KamataEngine.h"
#include "SceneManager.h"
#include "Player.h"
#include "Goal.h"
#include <algorithm>
using namespace KamataEngine;

GameScene::~GameScene() {
	if (debugCamera_) {
		delete debugCamera_;
		debugCamera_ = nullptr;
	}
	if (mapChipField_) {
		delete mapChipField_;
		mapChipField_ = nullptr;
	}
	for (std::vector<WorldTransform*>& worldTransformLine : worldTransformBlocks_) {
		for (WorldTransform* worldTransform : worldTransformLine) {
			delete worldTransform;
		}
		worldTransformLine.clear();
	}
	worldTransformBlocks_.clear();

	if (blockModel_) {
		delete blockModel_;
		blockModel_ = nullptr;
	}

}

void GameScene::Initialize() {
	sceneName_ = "GameScene";
	blockModel_ = Model::CreateFromOBJ("base_block");
	playerModel_ = Model::CreateFromOBJ("player");
	goalModel_ = Model::CreateFromOBJ("goal");
}

void GameScene::OnEnter() {  
    // ワールドトランスフォームの初期化  
    worldTransform_.Initialize();  
    mapChipField_ = new MapChipField();
    

    
	if (mapID != 0) {
		std::string mapPath = "Resources/map/level" + std::to_string(mapID) + ".csv";


		try {
			mapChipField_->LoadMapChipCsv(mapPath);
		} catch (...) {

			mapChipField_->LoadMapChipCsv("Resources/map/test.csv");
		}
	} else {
		std::string mapPath = "Resources/map/select.csv";

		try {
			mapChipField_->LoadMapChipCsv(mapPath);
		} catch (...) {

			mapChipField_->LoadMapChipCsv("Resources/map/test.csv");
		}
	}


    GenerateBlocks();  

    // カメラの初期化  
	camera_.Initialize();

	debugCamera_ = new DebugCamera(WinApp::kWindowWidth, WinApp::kWindowHeight);
	debugCamera_->SetFarZ(2000);  

    cameraController_ = std::make_unique<CameraController>(); 
	cameraController_->Initialize();
    cameraController_->SetTarget(player_.get());

    // Calculate and set map bounds for camera
    SetCameraMapBounds();

    #ifdef _DEBUG  
    // 座標軸  
    AxisIndicator::GetInstance()->SetVisible(true);  
    AxisIndicator::GetInstance()->SetTargetCamera(&debugCamera_->GetCamera());  
    PrimitiveDrawer::GetInstance()->SetCamera(&camera_);  
    #endif // DEBUG  
}

void GameScene::Update() {
	// 处理场景切换延迟
	if (isPendingSceneChange_) {
		sceneChangeTimer_ += 1.0f / 60.0f; // 假设60FPS
		
		if (sceneChangeTimer_ >= sceneChangeDelay_) {
			// 执行场景切换
			SceneManager::GetInstance().SetNextMapID(pendingTargetMapID_);
			SceneManager::GetInstance().ChangeScene(SceneManager::SceneType::kGame);
			return; // 场景即将切换，不需要继续更新
		}
	}

	CameraUpdate();

	player_->Update();
	for (std::unique_ptr<Object3d>& object : objects_) {
		object->Update();
	}
	
	for (std::vector<WorldTransform*>& worldTransformBlockX : worldTransformBlocks_) {
		for (WorldTransform* worldTransformBlock : worldTransformBlockX) {
			if (!worldTransformBlock)
				continue;
			// Affine行列を作成
			worldTransformBlock->MakeAffineMatrix4x4();
			worldTransformBlock->TransferMatrix();
		}
	}
#ifdef _DEBUG

	ImGui::Begin("Game Scene Debug");
	ImGui::Text("Scene Name: %s", sceneName_.c_str());
	ImGui::Text("Map ID: %d", mapID);
	ImGui::Text("Map Size: %dx%d", mapChipField_->GetNumBlockHorizontal(), mapChipField_->GetNumBlockVertical());
	ImGui::Text("Objects Count: %d", static_cast<int>(objects_.size()));
	
	if (isPendingSceneChange_) {
		ImGui::Text("Scene Change Pending...");
		ImGui::Text("Timer: %.2f / %.2f", sceneChangeTimer_, sceneChangeDelay_);
		ImGui::Text("Target Map ID: %d", pendingTargetMapID_);
		ImGui::ProgressBar(sceneChangeTimer_ / sceneChangeDelay_);
	} else {
		ImGui::Text("Scene Change: Ready");
	}

	// Manual scene change testing
	ImGui::Separator();
	ImGui::Text("Manual Scene Change Testing:");
	if (ImGui::Button("Load Map 1")) {
		SceneManager::GetInstance().SetNextMapID(1);
		SceneManager::GetInstance().ChangeScene(SceneManager::SceneType::kGame);
	}
	ImGui::SameLine();
	if (ImGui::Button("Load Map 2")) {
		SceneManager::GetInstance().SetNextMapID(2);
		SceneManager::GetInstance().ChangeScene(SceneManager::SceneType::kGame);
	}
	ImGui::SameLine();
	if (ImGui::Button("Load Map 0 (Select)")) {
		SceneManager::GetInstance().SetNextMapID(0);
		SceneManager::GetInstance().ChangeScene(SceneManager::SceneType::kGame);
	}

	// Show instructions
	ImGui::Separator();
	ImGui::Text("Test Instructions:");
	ImGui::Text("1. Press 1/2/0 keys to switch maps manually");
	ImGui::Text("2. Walk into the goal to trigger automatic switching");
	ImGui::Text("3. Watch console output for debug messages");

	ImGui::End();
	
	// Keyboard shortcuts for testing
	if (Input::GetInstance()->TriggerKey(DIK_1)) {
		SceneManager::GetInstance().SetNextMapID(1);
		SceneManager::GetInstance().ChangeScene(SceneManager::SceneType::kGame);
	}
	if (Input::GetInstance()->TriggerKey(DIK_2)) {
		SceneManager::GetInstance().SetNextMapID(2);
		SceneManager::GetInstance().ChangeScene(SceneManager::SceneType::kGame);
	}
	if (Input::GetInstance()->TriggerKey(DIK_0)) {
		SceneManager::GetInstance().SetNextMapID(0);
		SceneManager::GetInstance().ChangeScene(SceneManager::SceneType::kGame);
	}
	
	if (Input::GetInstance()->TriggerKey(DIK_RIGHT)) {
		SceneManager::GetInstance().ChangeScene(SceneManager::SceneType::kTitle);
	}
#endif // _DEBUG

}

void GameScene::Draw() { 
	
	//DirectXCommon* dxCommon = DirectXCommon::GetInstance();
	Model::PreDraw();


	if (!mapChipField_) {
		return;
	}
	player_->Draw();
	for (std::unique_ptr<Object3d>& object : objects_) {
		object->Draw();
	}

	for (std::vector<WorldTransform*>& worldTransformLine : worldTransformBlocks_) {
		for (WorldTransform* worldTransform : worldTransformLine) {
			if (worldTransform) {
				blockModel_->Draw(*worldTransform,camera_);
			}
		}
	}

	Model::PostDraw();
#ifdef _DEBUG
	PrimitiveDrawer::GetInstance()->DrawLine3d({100.0f, 0.0f, 0.0f}, {-100.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f});
	PrimitiveDrawer::GetInstance()->DrawLine3d({0.0f, 100.0f, 0.0f}, {0.0f, -100.0f, 0.0f}, {0.0f, 1.0f, 0.0f, 1.0f});
	// PrimitiveDrawer::GetInstance()->DrawLine3d({0.0f, 0.0f, 100.0f}, {0.0f, 0.0f, -100.0f}, {0.0f, 0.0f, 1.0f, 1.0f});
#endif // DEBUG
}

void GameScene::OnExit() {}

void GameScene::GenerateBlocks() {

		// 要素数
	uint32_t numBlockVertical = mapChipField_->GetNumBlockVertical();
	uint32_t numBlockHorizontal = mapChipField_->GetNumBlockHorizontal();

	//关卡个数
	int goalCount = 0;
	int spawnCount = 0;
	int blockCount = 0;

#ifdef _DEBUG
	printf("GameScene: Generating blocks for %dx%d map\n", numBlockHorizontal, numBlockVertical);
#endif

	// 要素数を変更する
	// 列数を設定（縦方向のブロック数）"
	worldTransformBlocks_.resize(numBlockVertical);
	for (uint32_t i = 0; i < numBlockVertical; i++) {
		worldTransformBlocks_[i].resize(numBlockHorizontal);
	}
	for (uint32_t i = 0; i < numBlockVertical; i++) {
		for (uint32_t j = 0; j < numBlockHorizontal; j++) {
			MapChipType chipType = mapChipField_->GetMapChipTypeByIndex(j, i);
			
			if (chipType == MapChipType::kBlock) {
				blockCount++;
				WorldTransform* worldTransform = new WorldTransform();
				worldTransformBlocks_[i][j] = worldTransform;
				worldTransformBlocks_[i][j]->Initialize();

				worldTransformBlocks_[i][j]->translation_ = mapChipField_->GetMapChipPositionByIndex(j, i);
			} else if (chipType == MapChipType::kSpawn) {
				spawnCount++;
#ifdef _DEBUG
				printf("GameScene: Found spawn at (%d, %d)\n", j, i);
#endif
				player_ = std::make_unique<Player>();
				player_->Initialize(playerModel_);
				player_->SetCamera(&camera_);
				player_->SetTranslation(mapChipField_->GetMapChipPositionByIndex(j, i));
				// 设置地图碰撞检测引用
				player_->SetMapChipField(mapChipField_);
			} else if (chipType == MapChipType::kGoal) {
				goalCount++;
#ifdef _DEBUG
				printf("GameScene: Found goal at (%d, %d)\n", j, i);
#endif
				std::unique_ptr<Goal> goal = std::make_unique<Goal>();
				goal->Initialize(goalModel_);
				goal->SetTranslation(mapChipField_->GetMapChipPositionByIndex(j, i));
				
				// 设置目标关卡ID的逻辑
				if (mapID == 0) {
					// 关卡选择场景：目标ID为关卡编号
					goal->SetTargetMapID(goalCount);
				} else {
					// 普通关卡场景的Goal设置
					// 可以根据位置或其他逻辑来设置不同的目标
					// 默认情况：返回关卡选择场景
					goal->SetTargetMapID(0);
					
					// 可选：如果有多个Goal，可以设置不同的目标
					// 例如：最右边的Goal进入下一关，最左边的Goal返回选择场景
					if (j == numBlockHorizontal - 1) {
						// 最右边的Goal：进入下一关
						goal->SetTargetMapID(-1); // -1表示自动下一关
					} else if (j == 0) {
						// 最左边的Goal：返回关卡选择
						goal->SetTargetMapID(0);
					}
				}

				// 设置碰撞回调函数
				goal->SetOnCollisionCallback([this](Goal* goal) {
					this->OnGoalCollision(goal);
				});

				objects_.push_back(std::move(goal));
			}
			else {
				worldTransformBlocks_[i][j] = nullptr;
			}
		}
	}

#ifdef _DEBUG
	printf("GameScene: Generated %d blocks, %d spawns, %d goals\n", blockCount, spawnCount, goalCount);
#endif

	// 如果没有找到玩家生成点，在地图中心创建一个
	if (!player_) {
#ifdef _DEBUG
		printf("GameScene: No spawn point found, creating player at map center\n");
#endif
		player_ = std::make_unique<Player>();
		player_->Initialize(playerModel_);
		player_->SetCamera(&camera_);
		// 设置在地图中心
		Vector3 centerPos = {
			(numBlockHorizontal * MapChipField::kBlockWidth) / 2.0f,
			(numBlockVertical * MapChipField::kBlockHeight) / 2.0f,
			0.0f
		};
		player_->SetTranslation(centerPos);
		player_->SetMapChipField(mapChipField_);
	}

	// 如果没有找到任何Goal，手动创建一个测试用的Goal
	if (goalCount == 0) {
#ifdef _DEBUG
		printf("GameScene: No goals found, creating test goal\n");
#endif
		std::unique_ptr<Goal> goal = std::make_unique<Goal>();
		goal->Initialize(goalModel_);
		// 将Goal放在地图右上角
		Vector3 goalPos = {
			(numBlockHorizontal - 2) * MapChipField::kBlockWidth + MapChipField::kBlockWidth / 2.0f,
			(numBlockVertical - 2) * MapChipField::kBlockHeight + MapChipField::kBlockHeight / 2.0f,
			0.0f
		};
		goal->SetTranslation(goalPos);
		
		// 设置目标关卡ID
		if (mapID == 0) {
			goal->SetTargetMapID(1); // 从选择场景进入关卡1
		} else {
			goal->SetTargetMapID(0); // 从关卡返回选择场景
		}

		// 设置碰撞回调函数
		goal->SetOnCollisionCallback([this](Goal* goal) {
			this->OnGoalCollision(goal);
		});

		objects_.push_back(std::move(goal));
	}

	// 设置玩家的对象引用（用于碰撞检测）
	if (player_) {
		player_->SetObjects(&objects_);
	}
}

void GameScene::CameraUpdate() {
	if (isDebugCameraActive_) {
		debugCamera_->Update();
		camera_.matView = debugCamera_->GetCamera().matView;
		camera_.matProjection = debugCamera_->GetCamera().matProjection;
		camera_.TransferMatrix();
	} else {
		cameraController_->Update();
		camera_.matView = cameraController_->GetCamera().matView;
		camera_.matProjection = cameraController_->GetCamera().matProjection;
		camera_.TransferMatrix();
	}
#ifdef _DEBUG
	if (Input::GetInstance()->TriggerKey(DIK_T)) {
		isDebugCameraActive_ = !isDebugCameraActive_;
	}
	if (Input::GetInstance()->TriggerKey(DIK_F)) {
		// debugCamera_->GetCamera().
	}
#endif // DEBUG
}

void GameScene::SetCameraMapBounds() {
    if (!mapChipField_) {
        return;
    }

    // Calculate map boundaries based on map dimensions
    uint32_t numHorizontal = mapChipField_->GetNumBlockHorizontal();
    uint32_t numVertical = mapChipField_->GetNumBlockVertical();
    
    // Calculate map bounds
    // Map starts at (0,0) and extends to (numHorizontal * blockWidth, numVertical * blockHeight)
    float mapLeft = 0.0f;
    float mapRight = numHorizontal * MapChipField::kBlockWidth;
    float mapBottom = 0.0f;
    float mapTop = numVertical * MapChipField::kBlockHeight;
    
    // Calculate map dimensions
    float mapWidth = mapRight - mapLeft;
    
    // Calculate map center
    float mapCenterX = (mapLeft + mapRight) * 0.5f;
    float mapCenterY = (mapBottom + mapTop) * 0.5f;
    
    // Set initial camera position to map center
    Vector3 mapCenter = {mapCenterX, mapCenterY, -50.0f}; // Keep the same Z distance
    cameraController_->SetInitialPosition(mapCenter);
    
    // Auto-adjust camera zoom based on map width
    // Reference map width of 20.0f units (10 blocks * 2.0f width each) for default zoom
    float referenceMapWidth = 80.0f; // Adjust this value based on your preferred reference size
    cameraController_->SetAutoZoomByMapWidth(mapWidth, referenceMapWidth);
    
    // Set zoom range to prevent extreme zooming
    cameraController_->SetZoomRange(0.3f, 2.5f); // Min zoom (very zoomed out) to Max zoom (zoomed in)
    
    // Set the movable area for the camera
    Rect mapBounds;
    mapBounds.left = mapLeft;
    mapBounds.right = mapRight;
    mapBounds.bottom = mapBottom;
    mapBounds.top = mapTop;
    
    cameraController_->SetMovableArea(mapBounds);
    
    // Configure camera behavior for optimal map bounds priority
    cameraController_->SetMapBoundsPriority(true);  // Prioritize map bounds
    cameraController_->SetFollowSpeed(0.05f);       // Slightly slower following for smoother movement
    
    // Adjust dead zone based on map size for better proportions
    float deadZoneScale = mapWidth / referenceMapWidth;
    if (deadZoneScale > 2.0f) {
        deadZoneScale = 2.0f; // Cap at 2x
    }
    cameraController_->SetDeadZone(3.0f * deadZoneScale, 3.0f * deadZoneScale);
}

void GameScene::OnGoalCollision(Goal* goal) {
	if (!goal || !goal->IsActive() || isPendingSceneChange_) {
		return; // 如果已经在等待场景切换，则忽略新的碰撞
	}

	int targetMapID = goal->GetTargetMapID();

#ifdef _DEBUG
	printf("GameScene: Goal collision detected! Current Map: %d, Target Map: %d\n", mapID, targetMapID);
#endif

#ifdef _DEBUG
	static bool showCollisionDebug = false;
	if (showCollisionDebug) {
		ImGui::Begin("Collision Debug");
		ImGui::Text("Goal collision detected!");
		ImGui::Text("Current Map ID: %d", mapID);
		ImGui::Text("Target Map ID: %d", targetMapID);
		ImGui::Text("Goal Position: (%.2f, %.2f)", goal->GetTranslation().x, goal->GetTranslation().y);
		ImGui::Text("Player Position: (%.2f, %.2f)", player_->GetTranslation().x, player_->GetTranslation().y);
		if (ImGui::Button("Close Debug")) {
			showCollisionDebug = false;
		}
		ImGui::End();
	}
#endif

	// 确定最终的目标关卡ID
	int finalTargetMapID = targetMapID;
	
	// 根据目标关卡ID执行不同的操作
	if (mapID == 0) {
		// 在关卡选择场景中，切换到对应关卡
		if (targetMapID > 0) {
			finalTargetMapID = targetMapID;
		}
	} else {
		// 在普通关卡中的处理
		if (targetMapID == 0) {
			// 返回关卡选择场景
			finalTargetMapID = 0;
		} else if (targetMapID > 0) {
			// 切换到指定关卡（下一关或特定关卡）
			finalTargetMapID = targetMapID;
		} else if (targetMapID == -1) {
			// 自动切换到下一关
			finalTargetMapID = mapID + 1;
		}
	}

#ifdef _DEBUG
	printf("GameScene: Final target map ID: %d\n", finalTargetMapID);
#endif

	// 启动场景切换延迟
	isPendingSceneChange_ = true;
	sceneChangeTimer_ = 0.0f;
	pendingTargetMapID_ = finalTargetMapID;

	// 禁用已触发的Goal以防重复触发
	goal->SetActive(false);

#ifdef _DEBUG
	showCollisionDebug = true;
	printf("GameScene: Scene change pending, timer started\n");
#endif
}
