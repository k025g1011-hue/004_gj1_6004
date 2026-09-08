#include "MapChipField.h"
#include <cmath>
#include <fstream>
#include <map>
#include <sstream>

namespace {
std::map<std::string, MapChipType> mapChipTable = {
    {"0", MapChipType::kBlank },
    {"1", MapChipType::kBlock },
    {"2", MapChipType::kPlayer},
    {"3", MapChipType::kFodder},
    {"4", MapChipType::kDoor  },
    {"5", MapChipType::kStake },
    {"6", MapChipType::kBoss  },
    {"7", MapChipType::kFlyer  },
};
}

void MapChipField::LoadMapChipCsv(const std::string& filePath) {
	data_.clear();
	numHorizontal_ = 0;
	numVertical_ = 0;

	std::ifstream file(filePath);
	if (!file.is_open()) {
		return;
	}

	std::string line;
	while (std::getline(file, line)) {
		if (line.empty()) {
			continue;
		}
		std::vector<MapChipType> row;
		std::istringstream stream(line);
		std::string word;
		while (std::getline(stream, word, ',')) {
			if (!word.empty() && word.back() == '\r') {
				word.pop_back();
			}
			if (mapChipTable.count(word) != 0) {
				row.push_back(mapChipTable[word]);
			} else {
				row.push_back(MapChipType::kBlank);
			}
		}
		if (!row.empty()) {
			data_.push_back(row);
			if (row.size() > numHorizontal_) {
				numHorizontal_ = static_cast<uint32_t>(row.size());
			}
		}
	}
	numVertical_ = static_cast<uint32_t>(data_.size());
	for (auto& row : data_) {
		row.resize(numHorizontal_, MapChipType::kBlank);
	}
}

MapChipType MapChipField::GetMapChipTypeByIndex(int xIndex, int yIndex) const {
	if (xIndex < 0 || yIndex < 0) {
		return MapChipType::kBlock;
	}
	if (static_cast<uint32_t>(xIndex) >= numHorizontal_ || static_cast<uint32_t>(yIndex) >= numVertical_) {
		return MapChipType::kBlock;
	}
	return data_[yIndex][xIndex];
}

KamataEngine::Vector2 MapChipField::GetMapChipPositionByIndex(int xIndex, int yIndex) const { return {kTileWidth * static_cast<float>(xIndex), kTileHeight * static_cast<float>(yIndex)}; }

MapChipField::IndexSet MapChipField::GetMapChipIndexSetByPosition(const KamataEngine::Vector2& position) const {
	IndexSet indexSet;
	indexSet.xIndex = static_cast<int>(std::floor(position.x / kTileWidth));
	indexSet.yIndex = static_cast<int>(std::floor(position.y / kTileHeight));
	return indexSet;
}

MapChipField::Rect MapChipField::GetRectByIndex(int xIndex, int yIndex) const {
	Rect rect;
	rect.left = kTileWidth * static_cast<float>(xIndex);
	rect.right = rect.left + kTileWidth;
	rect.top = kTileHeight * static_cast<float>(yIndex);
	rect.bottom = rect.top + kTileHeight;
	return rect;
}

bool MapChipField::OverlapsBlock(const AABB2& aabb) const {
	IndexSet minI = GetMapChipIndexSetByPosition(aabb.min);
	IndexSet maxI = GetMapChipIndexSetByPosition({aabb.max.x - 0.01f, aabb.max.y - 0.01f});
	for (int y = minI.yIndex; y <= maxI.yIndex; ++y) {
		for (int x = minI.xIndex; x <= maxI.xIndex; ++x) {
			if (GetMapChipTypeByIndex(x, y) == MapChipType::kBlock) {
				return true;
			}
		}
	}
	return false;
}

bool MapChipField::ResolveBlockX(AABB2& aabb, float& outX, float velocityX) const {
	if (!OverlapsBlock(aabb)) {
		return false;
	}
	IndexSet minI = GetMapChipIndexSetByPosition(aabb.min);
	IndexSet maxI = GetMapChipIndexSetByPosition({aabb.max.x - 0.01f, aabb.max.y - 0.01f});
	for (int y = minI.yIndex; y <= maxI.yIndex; ++y) {
		for (int x = minI.xIndex; x <= maxI.xIndex; ++x) {
			if (GetMapChipTypeByIndex(x, y) != MapChipType::kBlock) {
				continue;
			}
			Rect rect = GetRectByIndex(x, y);
			if (velocityX > 0.0f) {
				outX = rect.left - (aabb.max.x - aabb.min.x);
			} else {
				outX = rect.right;
			}
			return true;
		}
	}
	return false;
}

bool MapChipField::ResolveBlockY(AABB2& aabb, float& outY, float velocityY, bool& landed) const {
	landed = false;
	if (!OverlapsBlock(aabb)) {
		return false;
	}
	IndexSet minI = GetMapChipIndexSetByPosition(aabb.min);
	IndexSet maxI = GetMapChipIndexSetByPosition({aabb.max.x - 0.01f, aabb.max.y - 0.01f});
	for (int y = minI.yIndex; y <= maxI.yIndex; ++y) {
		for (int x = minI.xIndex; x <= maxI.xIndex; ++x) {
			if (GetMapChipTypeByIndex(x, y) != MapChipType::kBlock) {
				continue;
			}
			Rect rect = GetRectByIndex(x, y);
			if (velocityY >= 0.0f) {
				outY = rect.top - (aabb.max.y - aabb.min.y);
				landed = true;
			} else {
				outY = rect.bottom;
			}
			return true;
		}
	}
	return false;
}

float MapChipField::SnapFeetToFloor(float x, float height) const {
	IndexSet index = GetMapChipIndexSetByPosition({x + 1.0f, 0.0f});
	for (uint32_t y = 0; y < numVertical_; ++y) {
		if (GetMapChipTypeByIndex(index.xIndex, static_cast<int>(y)) == MapChipType::kBlock) {
			Rect rect = GetRectByIndex(index.xIndex, static_cast<int>(y));
			return rect.top - height;
		}
	}
	return static_cast<float>(numVertical_) * kTileHeight - height;
}