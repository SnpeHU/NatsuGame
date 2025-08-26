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

