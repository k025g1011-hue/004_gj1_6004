#pragma once
#include <cstdint>
#include <string>
#include <unordered_map>

class SoundManager {
public:
	static SoundManager* GetInstance();

	void Initialize();
	void Finalize();

	// --- BGM機能 ---
	void LoadBGM(const std::string& key, const std::string& filePath);
	void PlayBGM(const std::string& key, bool isLoop = true, float volume = 0.3f);
	void StopBGM();

	// --- SE（効果音）機能 ---
	void LoadSE(const std::string& key, const std::string& filePath);
	// SEは基本的にループしない(isLoop = false)、重ねて鳴らすためボイスハンドルは保持しない
	void PlaySE(const std::string& key, float volume = 0.5f);

private:
	SoundManager() = default;
	~SoundManager() = default;
	SoundManager(const SoundManager&) = delete;
	SoundManager& operator=(const SoundManager&) = delete;

	// BGM用・SE用それぞれでデータ（Waveデータ）を保持
	std::unordered_map<std::string, uint32_t> bgmHandles_;
	std::unordered_map<std::string, uint32_t> seHandles_;

	// 再生中のBGMボイスハンドル（無効時は static_cast<uint32_t>(-1)）
	uint32_t currentBgmVoiceHandle_ = static_cast<uint32_t>(-1);
};