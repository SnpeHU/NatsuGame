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
	void Update();  
	void Draw();  

	void ResetMapChipData();  
	void LoadMapChipCsv(const std::string& filePath);  

	MapChipType GetMapChipTypeByIndex(uint32_t xIndex, uint32_t yIndex);  

	// マップチップの位置を取得
	Vector3 GetMapChipPositionByIndex(uint32_t xIndex, uint32_t yIndex);
	IndexSet GetMapChipIndexByPosition(const Vector3& position);

	Rect GetRectByIndex(uint32_t xIndex, uint32_t yIndex);
	Rect GetScaledRectByIndex(uint32_t xIndex, uint32_t yIndex, float scale);

	// 碰撞检测相关方法
	bool CheckCollision(const Rect& playerRect);
	bool CheckCollisionAtPosition(const Vector3& position, const Vector3& size);
	bool IsBlockAtIndex(uint32_t xIndex, uint32_t yIndex);
	bool RectIntersectsRect(const Rect& rect1, const Rect& rect2);
	Rect GetPlayerRect(const Vector3& position, const Vector3& size);
	
	// 新的缩放碰撞检测方法
	bool CheckScaledCollision(const Rect& playerRect, float blockScale);
	bool CheckScaledCollisionAtPosition(const Vector3& position, const Vector3& size, float blockScale);
	std::vector<IndexSet> GetScaledCollidingBlocks(const Vector3& position, const Vector3& size, float blockScale);
	
	// 获取碰撞信息的额外方法
	std::vector<IndexSet> GetCollidingBlocks(const Vector3& position, const Vector3& size);
	bool IsPositionInMapBounds(const Vector3& position);
	bool IsIndexInMapBounds(uint32_t xIndex, uint32_t yIndex);

	uint32_t GetNumBlockHorizontal() const { return numBlockHorizontal_; }
	uint32_t GetNumBlockVertical() const { return numBlockVertical_; }

public:  
	static inline float kBlockWidth = 2.0f;  
	static inline float kBlockHeight = 2.0f;  

	static inline const uint32_t kNumBlockHorizontal = 5;  
	static inline const uint32_t kNumBlockVertical = 5;  

private:  
	MapChipData mapChipData_;
	
	// Actual map dimensions (can be different from constants)
	uint32_t numBlockHorizontal_ = kNumBlockHorizontal;
	uint32_t numBlockVertical_ = kNumBlockVertical;
};