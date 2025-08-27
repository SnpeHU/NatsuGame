#pragma once
#include "GameScene.h"
#include "TitleScene.h"
#include <memory>

class SceneManager {
public:

	enum class SceneType {
		kNone,
		kTitle,
		kGame,

	};
	
	
	SceneManager(const SceneManager&) = delete;
	SceneManager& operator=(const SceneManager&) = delete;
	
	~SceneManager() = default; 

	static SceneManager& GetInstance();
	static void Destroy();

	void Init();
	void Update();
	void Draw();
	void ChangeScene(SceneType newSceneType);

	// Map ID management
	void SetNextMapID(int mapID) { nextMapID_ = mapID; }
	int GetNextMapID() const { return nextMapID_; }

private:
	SceneManager() = default; 
	static std::unique_ptr<SceneManager> instance_;

	std::unique_ptr<IScene> currentScene_ = nullptr; 
	SceneType currentSceneType_ = SceneType::kNone; 

	int nextMapID_ = 0; // Default map ID
};
