#include "SceneManager.h"

std::unique_ptr<SceneManager> SceneManager::instance_ = nullptr;

SceneManager& SceneManager::GetInstance() {
	if (!instance_) {
		// 使用 new 创建实例，因为构造函数是私有的
		instance_ = std::unique_ptr<SceneManager>(new SceneManager());
	}
	return *instance_;
}

void SceneManager::Destroy() {
	if (instance_) {
		if (instance_->currentScene_) {
			instance_->currentScene_->OnExit();
		}
		instance_.reset();
	}
}

void SceneManager::Init() { 
	// 初始化时创建标题场景
	//currentScene_ = std::make_unique<TitleScene>();
	//currentScene_->Initialize();
	//currentSceneType_ = SceneType::kTitle;

	currentScene_ = std::make_unique<GameScene>();
	currentScene_->Initialize();
	currentSceneType_ = SceneType::kGame;
	
	if (currentScene_) {
		GameScene* gameScene = dynamic_cast<GameScene*>(currentScene_.get());
		if (gameScene) {
			gameScene->SetMapID(nextMapID_);
		}
		currentScene_->OnEnter();
	}
}

void SceneManager::Update() {
	if (currentScene_) {
		currentScene_->Update();
	}
}

void SceneManager::Draw() {
	if (currentScene_) {
		currentScene_->Draw();
	}
}

void SceneManager::ChangeScene(SceneType newSceneType) { 
#ifdef _DEBUG
	// Debug scene change
	const char* sceneNames[] = {"None", "Title", "Game"};
	printf("SceneManager: Changing scene from %s to %s\n", 
		sceneNames[static_cast<int>(currentSceneType_)], 
		sceneNames[static_cast<int>(newSceneType)]);
	printf("SceneManager: Next Map ID: %d\n", nextMapID_);
#endif

	// 退出并释放当前场景
	if (currentScene_) {
		currentScene_->OnExit();
		currentScene_.reset();  // 立即释放旧场景
	}
	
	// 创建新场景
	switch (newSceneType) {
	case SceneType::kNone:
		// currentScene_ 已经在上面被重置为 nullptr
		break;
	case SceneType::kTitle:
		currentScene_ = std::make_unique<TitleScene>();
		currentScene_->Initialize();
		break;
	case SceneType::kGame:
		currentScene_ = std::make_unique<GameScene>();
		currentScene_->Initialize();
		// Set the map ID for the GameScene
		if (currentScene_) {
			GameScene* gameScene = dynamic_cast<GameScene*>(currentScene_.get());
			if (gameScene) {
				gameScene->SetMapID(nextMapID_);
#ifdef _DEBUG
				printf("SceneManager: Set Map ID %d for GameScene\n", nextMapID_);
#endif
			}
		}
		break;
	default:
		// 保持 currentScene_ 为 nullptr
		newSceneType = SceneType::kNone;
		break;
	}
	
	// 更新当前场景类型
	currentSceneType_ = newSceneType;
	
	// 进入新场景
	if (currentScene_) {
		currentScene_->OnEnter();
	}

#ifdef _DEBUG
	printf("SceneManager: Scene change completed\n");
#endif
}