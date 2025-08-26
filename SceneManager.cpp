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
	// 如果要切换到相同的场景类型，直接返回
	if (currentSceneType_ == newSceneType) {
		return;
	}
	
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
}