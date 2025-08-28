#include "MapChipField.h"
#include <map>
#include <string>
#include <fstream>
#include <sstream>
#include <cassert>
namespace {
    std::map<std::string, MapChipType> mapChipTable = {
        {"-1", MapChipType::kBlank},
        {"0", MapChipType::kBlock},
		{"1", MapChipType::kSpawn},
		{"2",  MapChipType::kGoal },
    };
}
    MapChipField::MapChipField() {}

    MapChipField::~MapChipField() {}

    void MapChipField::ResetMapChipData() {
	    mapChipData_.data_.clear();
	    // Reset to default size
	    numBlockVertical_ = kNumBlockVertical;
	    numBlockHorizontal_ = kNumBlockHorizontal;
	    mapChipData_.data_.resize(numBlockVertical_);
	    for (std::vector<MapChipType>& mapChipDataLine : mapChipData_.data_) {
		    mapChipDataLine.resize(numBlockHorizontal_);
		}
    }

    void MapChipField::LoadMapChipCsv(const std::string& filePath) {  
		std::ifstream file;
	    file.open(filePath);
		assert(file.is_open());

		std::stringstream mapChipCsvStream;
		mapChipCsvStream << file.rdbuf();
		file.close();
		
		// Clear existing data
		mapChipData_.data_.clear();
		
		// Read all lines using while loop
		std::string line;
		uint32_t rowCount = 0;
		uint32_t maxColumnCount = 0;
		
		// First pass: count rows and find maximum columns
		std::stringstream tempStream(mapChipCsvStream.str());
		while (std::getline(tempStream, line)) {
			if (!line.empty()) {
				rowCount++;
				// Count columns in this line
				uint32_t columnCount = 1; // At least one column if line is not empty
				for (char c : line) {
					if (c == ',') {
						columnCount++;
					}
				}
				maxColumnCount = std::max(maxColumnCount, columnCount);
			}
		}
		
		// Update actual dimensions
		numBlockVertical_ = rowCount;
		numBlockHorizontal_ = maxColumnCount;
		
		// Resize the data structure
		mapChipData_.data_.resize(numBlockVertical_);
		for (std::vector<MapChipType>& mapChipDataLine : mapChipData_.data_) {
			mapChipDataLine.resize(numBlockHorizontal_);
		}
		
		// Second pass: read the actual data
		std::stringstream dataStream(mapChipCsvStream.str());
		uint32_t currentRow = 0;
		while (std::getline(dataStream, line) && currentRow < numBlockVertical_) {
			if (!line.empty()) {
				std::istringstream line_stream(line);
				uint32_t currentCol = 0;
				
				std::string word;
				while (std::getline(line_stream, word, ',') && currentCol < numBlockHorizontal_) {
					if (mapChipTable.contains(word)) {
						mapChipData_.data_[currentRow][currentCol] = mapChipTable[word];
					} else {
						mapChipData_.data_[currentRow][currentCol] = MapChipType::kBlank;
					}
					currentCol++;
				}
				
				// Fill remaining columns with blank if line has fewer columns
				while (currentCol < numBlockHorizontal_) {
					mapChipData_.data_[currentRow][currentCol] = MapChipType::kBlank;
					currentCol++;
				}
				
				currentRow++;
			}
		}
    }

    MapChipType MapChipField::GetMapChipTypeByIndex(uint32_t xIndex, uint32_t yIndex) { 
        if (xIndex < 0 || numBlockHorizontal_ - 1 < xIndex) {
		    return MapChipType::kBlank;
	    }
	    if (yIndex < 0 || numBlockVertical_ - 1 < yIndex) {
		    return MapChipType::kBlank;
	    }
        
        return mapChipData_.data_[yIndex][xIndex];
    }

    Vector3 MapChipField::GetMapChipPositionByIndex(uint32_t xIndex, uint32_t yIndex) { 
        
        return Vector3(xIndex * kBlockWidth + kBlockWidth / 2, kBlockHeight * (numBlockVertical_ - 1 - yIndex) + kBlockHeight / 2, 0.0f); 
	}

    IndexSet MapChipField::GetMapChipIndexByPosition(const Vector3& position) { 
		IndexSet indexSet = {};
	    indexSet.xIndex = static_cast<uint32_t>((floor)(position.x / kBlockWidth));
	    indexSet.yIndex = static_cast<uint32_t>(numBlockVertical_ - 1 - (floor)(position.y / kBlockHeight));
		
		return indexSet; 
	}

    MapChipField::Rect MapChipField::GetRectByIndex(uint32_t xIndex, uint32_t yIndex) { 
		Vector3 center = GetMapChipPositionByIndex(xIndex, yIndex);
		
		Rect rect;
	    rect.left = center.x - kBlockWidth / 2.0f;
	    rect.right = center.x + kBlockWidth / 2.0f;
	    rect.top = center.y + kBlockHeight / 2.0f;
	    rect.bottom = center.y - kBlockHeight / 2.0f;
	    return rect;
    }

    MapChipField::Rect MapChipField::GetScaledRectByIndex(uint32_t xIndex, uint32_t yIndex, float scale) { 
		Vector3 center = GetMapChipPositionByIndex(xIndex, yIndex);
		
		float scaledWidth = kBlockWidth * scale;
		float scaledHeight = kBlockHeight * scale;
		
		Rect rect;
	    rect.left = center.x - scaledWidth / 2.0f;
	    rect.right = center.x + scaledWidth / 2.0f;
	    rect.top = center.y + scaledHeight / 2.0f;
	    rect.bottom = center.y - scaledHeight / 2.0f;
	    return rect;
    }

    // 碰撞检测方法实现
    bool MapChipField::CheckCollision(const Rect& playerRect) {
        // 获取玩家矩形覆盖的地图瓦片范围
        int leftIndex = static_cast<int>(playerRect.left / kBlockWidth);
        int rightIndex = static_cast<int>(playerRect.right / kBlockWidth);
        int bottomIndex = static_cast<int>((numBlockVertical_ * kBlockHeight - playerRect.top) / kBlockHeight);
        int topIndex = static_cast<int>((numBlockVertical_ * kBlockHeight - playerRect.bottom) / kBlockHeight);

        // 确保索引在有效范围内
        leftIndex = std::max(0, leftIndex);
        rightIndex = std::min(static_cast<int>(numBlockHorizontal_ - 1), rightIndex);
        bottomIndex = std::max(0, bottomIndex);
        topIndex = std::min(static_cast<int>(numBlockVertical_ - 1), topIndex);

        // 检查范围内的每个瓦片
        for (int y = bottomIndex; y <= topIndex; ++y) {
            for (int x = leftIndex; x <= rightIndex; ++x) {
                if (IsBlockAtIndex(x, y)) {
                    Rect blockRect = GetRectByIndex(x, y);
                    if (RectIntersectsRect(playerRect, blockRect)) {
                        return true;
                    }
                }
            }
        }
        return false;
    }

    bool MapChipField::CheckScaledCollision(const Rect& playerRect, float blockScale) {
        // 获取玩家矩形覆盖的地图瓦片范围
        int leftIndex = static_cast<int>(playerRect.left / kBlockWidth);
        int rightIndex = static_cast<int>(playerRect.right / kBlockWidth);
        int bottomIndex = static_cast<int>((numBlockVertical_ * kBlockHeight - playerRect.top) / kBlockHeight);
        int topIndex = static_cast<int>((numBlockVertical_ * kBlockHeight - playerRect.bottom) / kBlockHeight);

        // 确保索引在有效范围内
        leftIndex = std::max(0, leftIndex);
        rightIndex = std::min(static_cast<int>(numBlockHorizontal_ - 1), rightIndex);
        bottomIndex = std::max(0, bottomIndex);
        topIndex = std::min(static_cast<int>(numBlockVertical_ - 1), topIndex);

        // 检查范围内的每个瓦片
        for (int y = bottomIndex; y <= topIndex; ++y) {
            for (int x = leftIndex; x <= rightIndex; ++x) {
                if (IsBlockAtIndex(x, y)) {
                    Rect blockRect = GetScaledRectByIndex(x, y, blockScale);
                    if (RectIntersectsRect(playerRect, blockRect)) {
                        return true;
                    }
                }
            }
        }
        return false;
    }

    bool MapChipField::CheckCollisionAtPosition(const Vector3& position, const Vector3& size) {
        Rect playerRect = GetPlayerRect(position, size);
        return CheckCollision(playerRect);
    }

    bool MapChipField::CheckScaledCollisionAtPosition(const Vector3& position, const Vector3& size, float blockScale) {
        Rect playerRect = GetPlayerRect(position, size);
        return CheckScaledCollision(playerRect, blockScale);
    }

    bool MapChipField::IsBlockAtIndex(uint32_t xIndex, uint32_t yIndex) {
        MapChipType chipType = GetMapChipTypeByIndex(xIndex, yIndex);
        return chipType == MapChipType::kBlock;
    }

    bool MapChipField::RectIntersectsRect(const Rect& rect1, const Rect& rect2) {
        return !(rect1.right <= rect2.left || 
                 rect1.left >= rect2.right || 
                 rect1.top <= rect2.bottom || 
                 rect1.bottom >= rect2.top);
    }

    MapChipField::Rect MapChipField::GetPlayerRect(const Vector3& position, const Vector3& size) {
        Rect rect;
        rect.left = position.x - size.x / 2.0f;
        rect.right = position.x + size.x / 2.0f;
        rect.bottom = position.y - size.y / 2.0f;
        rect.top = position.y + size.y / 2.0f;
        return rect;
    }

    // 获取与玩家碰撞的所有方块索引
    std::vector<IndexSet> MapChipField::GetCollidingBlocks(const Vector3& position, const Vector3& size) {
        std::vector<IndexSet> collidingBlocks;
        Rect playerRect = GetPlayerRect(position, size);
        
        // 获取玩家矩形覆盖的地图瓦片范围
        int leftIndex = static_cast<int>(playerRect.left / kBlockWidth);
        int rightIndex = static_cast<int>(playerRect.right / kBlockWidth);
        int bottomIndex = static_cast<int>((numBlockVertical_ * kBlockHeight - playerRect.top) / kBlockHeight);
        int topIndex = static_cast<int>((numBlockVertical_ * kBlockHeight - playerRect.bottom) / kBlockHeight);

        // 确保索引在有效范围内
        leftIndex = std::max(0, leftIndex);
        rightIndex = std::min(static_cast<int>(numBlockHorizontal_ - 1), rightIndex);
        bottomIndex = std::max(0, bottomIndex);
        topIndex = std::min(static_cast<int>(numBlockVertical_ - 1), topIndex);

        // 检查范围内的每个瓦片
        for (int y = bottomIndex; y <= topIndex; ++y) {
            for (int x = leftIndex; x <= rightIndex; ++x) {
                if (IsBlockAtIndex(x, y)) {
                    Rect blockRect = GetRectByIndex(x, y);
                    if (RectIntersectsRect(playerRect, blockRect)) {
                        collidingBlocks.push_back({static_cast<uint32_t>(x), static_cast<uint32_t>(y)});
                    }
                }
            }
        }
        return collidingBlocks;
    }

    std::vector<IndexSet> MapChipField::GetScaledCollidingBlocks(const Vector3& position, const Vector3& size, float blockScale) {
        std::vector<IndexSet> collidingBlocks;
        Rect playerRect = GetPlayerRect(position, size);
        
        // 获取玩家矩形覆盖的地图瓦片范围
        int leftIndex = static_cast<int>(playerRect.left / kBlockWidth);
        int rightIndex = static_cast<int>(playerRect.right / kBlockWidth);
        int bottomIndex = static_cast<int>((numBlockVertical_ * kBlockHeight - playerRect.top) / kBlockHeight);
        int topIndex = static_cast<int>((numBlockVertical_ * kBlockHeight - playerRect.bottom) / kBlockHeight);

        // 确保索引在有效范围内
        leftIndex = std::max(0, leftIndex);
        rightIndex = std::min(static_cast<int>(numBlockHorizontal_ - 1), rightIndex);
        bottomIndex = std::max(0, bottomIndex);
        topIndex = std::min(static_cast<int>(numBlockVertical_ - 1), topIndex);

        // 检查范围内的每个瓦片
        for (int y = bottomIndex; y <= topIndex; ++y) {
            for (int x = leftIndex; x <= rightIndex; ++x) {
                if (IsBlockAtIndex(x, y)) {
                    Rect blockRect = GetScaledRectByIndex(x, y, blockScale);
                    if (RectIntersectsRect(playerRect, blockRect)) {
                        collidingBlocks.push_back({static_cast<uint32_t>(x), static_cast<uint32_t>(y)});
                    }
                }
            }
        }
        return collidingBlocks;
    }

    bool MapChipField::IsPositionInMapBounds(const Vector3& position) {
        return position.x >= 0.0f && position.x < (numBlockHorizontal_ * kBlockWidth) &&
               position.y >= 0.0f && position.y < (numBlockVertical_ * kBlockHeight);
    }

    bool MapChipField::IsIndexInMapBounds(uint32_t xIndex, uint32_t yIndex) {
        return xIndex < numBlockHorizontal_ && yIndex < numBlockVertical_;
    }

