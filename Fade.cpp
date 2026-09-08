#define NOMINMAX
#include "Fade.h"
#include <algorithm>

void Fade::Initialize() {
	sprite_ = Sprite::Create(0, {0.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 1.0f});

	sprite_->SetSize(Vector2(kWindowWidth, kWindowHeight));

	sprite_->SetColor(Vector4(0.0f, 0.0f, 0.0f, 1.0f));
}

void Fade::Update() {
	// フェード状態による分岐
	switch (status_) {
	case Status::None:
		// なにもしない
		break;
	case Status::FadeIn:
		// 1フレーム分の秒数を加算
		counter_ += 1.0f / 60.0f;

		// フェード継続時間で打ち止め
		counter_ = std::min(counter_, duration_);

		// αを1→0へ変化
		sprite_->SetColor(Vector4(0.0f, 0.0f, 0.0f, std::clamp(1.0f - counter_ / duration_, 0.0f, 1.0f)));
		break;
	case Status::FadeOut:
		// 1フレーム分の秒数をカウントアップ
		counter_ += 1.0f / 60.0f;
		// フェード継続時間に達したら打ち止め
		counter_ = std::min(counter_, duration_);
		// 0.0fから1.0fの間で、経過時間がフェード継続時間に近づくほどアルファ値を大きくする
		sprite_->SetColor(Vector4(0, 0, 0, std::clamp(counter_ / duration_, 0.0f, 1.0f)));
		break;
	default:
		break;
	}
}

void Fade::Draw() {
	if (status_ == Status::None) {
		return;
	}
	Sprite::PreDraw();
	sprite_->Draw();
	Sprite::PostDraw();
}

void Fade::Start(Status status, float duration) {
	status_ = status;
	duration_ = duration;
	counter_ = 0.0f;

	if (status_ == Status::FadeIn) {
		sprite_->SetColor(Vector4(0.0f, 0.0f, 0.0f, 1.0f));
	} else if (status_ == Status::FadeOut) {
		sprite_->SetColor(Vector4(0.0f, 0.0f, 0.0f, 0.0f));
	}
}

void Fade::Stop() { status_ = Status::None; }

bool Fade::IsFinished() const {

	switch (status_) {

	case Status::FadeIn:
	case Status::FadeOut:
		return counter_ >= duration_;

	case Status::None:
		return true;
	}

	return true;
}