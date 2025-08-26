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
		mapChipField_->LoadMapChipCsv("Resources/map/level" + std::to_string(mapID) + ".csv");
	} else {
		mapChipField_->LoadMapChipCsv("Resources/map/select.csv");
	}
    //mapChipField_->LoadMapChipCsv("Resources/map/level.csv");  

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

	ImGui::End();
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

	// 要素数を変更する
	// 列数を設定（縦方向のブロック数）
	worldTransformBlocks_.resize(numBlockVertical);
	for (uint32_t i = 0; i < numBlockVertical; i++) {
		worldTransformBlocks_[i].resize(numBlockHorizontal);
	}
	for (uint32_t i = 0; i < numBlockVertical; i++) {
		for (uint32_t j = 0; j < numBlockHorizontal; j++) {
			if (mapChipField_->GetMapChipTypeByIndex(j, i) == MapChipType::kBlock) {
				WorldTransform* worldTransform = new WorldTransform();
				worldTransformBlocks_[i][j] = worldTransform;
				worldTransformBlocks_[i][j]->Initialize();

				worldTransformBlocks_[i][j]->translation_ = mapChipField_->GetMapChipPositionByIndex(j, i);
			} else if (mapChipField_->GetMapChipTypeByIndex(j, i) == MapChipType::kSpawn) {
				player_ = std::make_unique<Player>();
				player_->Initialize(playerModel_);
				player_->SetCamera(&camera_);
				player_->SetTranslation(mapChipField_->GetMapChipPositionByIndex(j, i));
				// 设置地图碰撞检测引用
				player_->SetMapChipField(mapChipField_);
			} else if (mapChipField_->GetMapChipTypeByIndex(j, i) == MapChipType::kGoal) {
				std::unique_ptr<Goal> goal = std::make_unique<Goal>();
				goal->Initialize(goalModel_);
				goal->SetTranslation(mapChipField_->GetMapChipPositionByIndex(j, i));
				// 如果是选择关卡场景，目标ID为当前选择的关卡ID，否则为0
				if (mapID == 0) {
					goal->SetTargetMapID(j + 1); // 选择关卡场景中，目标ID为横坐标+1
				} else {
					goal->SetTargetMapID(0); // 普通关卡中，目标ID为0
				}

				objects_.push_back(std::move(goal));


			}
			else {
				worldTransformBlocks_[i][j] = nullptr;
			}
		}
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
