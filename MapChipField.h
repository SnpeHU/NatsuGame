#pragma once  
#include <cstdint>  
#include <string>  
#include <vector>
#include <math/Vector3.h>
using namespace KamataEngine;

enum class MapChipType {  
	kBlank, // 空白  
	kBlock, // ブロック
	kSpawn, // スポーン地点
	kGoal,  // ゴール地点
};  

struct MapChipData {  
	std::vector<std::vector<MapChipType>> data_;  
};  

struct IndexSet {
	uint32_t xIndex;
	uint32_t yIndex;
};



class MapChipField {  


public:  
	struct Rect {
		float left;
		float right;
		float top;
		float bottom;
	};
	MapChipField();  
	~MapChipField();  
	//void Initialize(KamataEngine::Model* model, KamataEngine::Camera* camera);  
	void Update();  
	void Draw();  

	void ResetMapChipData();  
	void LoadMapChipCsv(const std::string& filePath);  

	MapChipType GetMapChipTypeByIndex(uint32_t xIndex, uint32_t yIndex);  

	// マップチップの位置を取得
	Vector3 GetMapChipPositionByIndex(uint32_t xIndex, uint32_t yIndex);
	IndexSet GetMapChipIndexByPosition(const Vector3& position);

	Rect GetRectByIndex(uint32_t xIndex, uint32_t yIndex);


	uint32_t GetNumBlockHorizontal() const { return numBlockHorizontal_; }
	uint32_t GetNumBlockVertical() const { return numBlockVertical_; }

public:  
	static inline const float kBlockWidth = 2.0f;  
	static inline const float kBlockHeight = 2.0f;  

	static inline const uint32_t kNumBlockHorizontal = 5;  
	static inline const uint32_t kNumBlockVertical = 5;  

private:  

	MapChipData mapChipData_;
	
	// Actual map dimensions (can be different from constants)
	uint32_t numBlockHorizontal_ = kNumBlockHorizontal;
	uint32_t numBlockVertical_ = kNumBlockVertical;
};