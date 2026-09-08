#include "SoundManager.h"
#include "KamataEngine.h"

SoundManager* SoundManager::GetInstance() {
	static SoundManager instance;
	return &instance;
}

void SoundManager::Initialize() {
	currentBgmVoiceHandle_ = static_cast<uint32_t>(-1);
	bgmHandles_.clear();
	seHandles_.clear();
}

void SoundManager::Finalize() {
	StopBGM();
	bgmHandles_.clear();
	seHandles_.clear();
}

// --- BGM実装 ---
void SoundManager::LoadBGM(const std::string& key, const std::string& filePath) {
	if (bgmHandles_.find(key) == bgmHandles_.end()) {
		uint32_t handle = KamataEngine::Audio::GetInstance()->LoadWave(filePath);
		bgmHandles_[key] = handle;
	}
}

void SoundManager::PlayBGM(const std::string& key, bool isLoop, float volume) {
	StopBGM();

	if (bgmHandles_.find(key) != bgmHandles_.end()) {
		currentBgmVoiceHandle_ = KamataEngine::Audio::GetInstance()->PlayWave(bgmHandles_[key], isLoop, volume);
	}
}

void SoundManager::StopBGM() {
	if (currentBgmVoiceHandle_ != static_cast<uint32_t>(-1)) {
		KamataEngine::Audio::GetInstance()->StopWave(currentBgmVoiceHandle_);
		currentBgmVoiceHandle_ = static_cast<uint32_t>(-1);
	}
}

// --- SE実装 ---
void SoundManager::LoadSE(const std::string& key, const std::string& filePath) {
	if (seHandles_.find(key) == seHandles_.end()) {
		uint32_t handle = KamataEngine::Audio::GetInstance()->LoadWave(filePath);
		seHandles_[key] = handle;
	}
}

void SoundManager::PlaySE(const std::string& key, float volume) {
	if (seHandles_.find(key) != seHandles_.end()) {
		// PlayWave を呼ぶだけで返り値のボイスハンドルは保持しない（鳴り終わりはエンジンに任せる）
		KamataEngine::Audio::GetInstance()->PlayWave(seHandles_[key], false, volume);
	}
}