#pragma once
#include "AABB2.h"
#include "KamataEngine.h"
#include <cstdint>
#include <string>
#include <vector>

enum class MapChipType {
	kBlank = 0,
	kBlock = 1,
	kPlayer = 2,
	kFodder = 3,
	kDoor = 4,
	kStake = 5,
	kBoss = 6,
	kFlyer = 7,
};

class MapChipField {
public:
	struct IndexSet {
		int xIndex = 0;
		int yIndex = 0;
	};

	struct Rect {
		float left = 0.0f;
		float right = 0.0f;
		float top = 0.0f;
		float bottom = 0.0f;
	};

	void LoadMapChipCsv(const std::string& filePath);

	MapChipType GetMapChipTypeByIndex(int xIndex, int yIndex) const;
	KamataEngine::Vector2 GetMapChipPositionByIndex(int xIndex, int yIndex) const;
	IndexSet GetMapChipIndexSetByPosition(const KamataEngine::Vector2& position) const;
	Rect GetRectByIndex(int xIndex, int yIndex) const;
	bool OverlapsBlock(const AABB2& aabb) const;
	bool ResolveBlockX(AABB2& aabb, float& outX, float velocityX) const;
	bool ResolveBlockY(AABB2& aabb, float& outY, float velocityY, bool& landed) const;
	float SnapFeetToFloor(float x, float height) const;

	uint32_t GetNumBlockHorizontal() const { return numHorizontal_; }
	uint32_t GetNumBlockVertical() const { return numVertical_; }
	float GetMapRight() const { return static_cast<float>(numHorizontal_) * kTileWidth; }
	float GetMapBottom() const { return static_cast<float>(numVertical_) * kTileHeight; }

	static inline const float kTileWidth = 64.0f;
	static inline const float kTileHeight = 64.0f;
	static inline const int kTilesPerRoom = 20;

private:
	std::vector<std::vector<MapChipType>> data_;
	uint32_t numHorizontal_ = 0;
	uint32_t numVertical_ = 0;
};

inline int GetSectionByTileX(int xIndex) { return xIndex / MapChipField::kTilesPerRoom + 1; }