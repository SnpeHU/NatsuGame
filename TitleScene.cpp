#include "TitleScene.h"  
#include"KamataEngine.h"
#include "SceneManager.h"
using namespace KamataEngine;
void TitleScene::Initialize() 
{ 
    sceneName_ = "Title";
}

void TitleScene::OnEnter() {}  

void TitleScene::Update() {  
    #ifdef _DEBUG  

    ImGui::Begin("Title Scene Debug"); 
    ImGui::Text("Scene Name: %s", sceneName_.c_str());
    ImGui::End(); 
    if (Input::GetInstance()->TriggerKey(DIK_RIGHT)) {
		SceneManager::GetInstance().ChangeScene(SceneManager::SceneType::kGame);
	}
    #endif // _DEBUG  
}  

void TitleScene::Draw() {}  

void TitleScene::OnExit() {}
