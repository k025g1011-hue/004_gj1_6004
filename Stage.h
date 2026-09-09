#pragma once
#include "KamataEngine.h"
#include <vector>

/// <summary>
/// エリア識別。1〜3が雑魚部屋、4がボス部屋。
/// </summary>
enum class StageId {
	kStage1,
	kStage2,
	kStage3,
	kBoss,
};

/// <summary>
/// 雑魚1体の初期配置。
/// section はどの部屋の敵かを表す（死亡時はその部屋だけリセットする）。
/// </summary>
struct EnemySpawn {
	KamataEngine::Vector2 position{};
	float minX = 0.0f;
	float maxX = 0.0f;
	int section = 1;
};

/// <summary>
/// 部屋の右端にある扉。
/// openAfterSection の部屋をクリアすると開き、触ると leadsToSection へ移動する。
/// </summary>
struct DoorDesc {
	float x = 0.0f;
	float y = 0.0f;
	float width = 64.0f;
	float height = 64.0f;
	int openAfterSection = 1;
	int leadsToSection = 2;
};

/// <summary>
/// 1本につながったワールド全体の定数。
/// 各部屋の幅は 1280（画面1枚分）。後で map.csv に差し替える前提。
/// </summary>
struct WorldDesc {
	float groundY = 640.0f;
	float mapTop = 0.0f;
	float mapBottom = 720.0f;
	//float roomW = 1280.0f;
	float roomW = 2560.0f;

	KamataEngine::Vector2 spawn1{180.0f, 500.0f};
	KamataEngine::Vector2 spawn2{1460.0f, 500.0f};
	KamataEngine::Vector2 spawn3{2740.0f, 500.0f};
	KamataEngine::Vector2 spawnBoss{4020.0f, 500.0f};

	std::vector<EnemySpawn> fodder;
	std::vector<EnemySpawn> flyer;
	std::vector<EnemySpawn> ranged;
	std::vector<EnemySpawn> heavy;

	std::vector<DoorDesc> doors;
};

/// <summary>
/// 部屋番号(1始まり)から、その部屋の左端ワールドXを返す。
/// </summary>
inline float RoomLeft(int section) { return static_cast<float>(section - 1) * 2560.0f; }

/// <summary>
/// 部屋番号から右端ワールドXを返す。
/// </summary>
inline float RoomRight(int section) { return static_cast<float>(section) * 2560.0f; }

/// <summary>
/// 仮マップデータ。座標を変えれば配置を差し替えられる。
/// </summary>
inline WorldDesc GetWorldDesc() {
	WorldDesc world;
	world.fodder = {};
	world.flyer = {};
	world.ranged = {};
	world.heavy = {};
	world.doors = {};
	return world;
}