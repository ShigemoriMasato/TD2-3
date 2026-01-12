#pragma once
#include <Scene/IScene.h>
#include <Camera/DebugCamera.h>
#include <Test/Sphere.h>
#include <Test/SimpleRooms.h>
#include <Test/SneakWalk.h>
#include <Test/SamplerTest.h>
#include <Test/AnimationTest.h>
#include <Test/AnimationMiscTest.h>
#include <Test/SpotLightTest.h>

class TestScene : public IScene {
public:

	TestScene() = default;
	~TestScene() = default;

	void Initialize() override;
	std::unique_ptr<IScene> Update() override;
	void Draw() override;

private:

	std::unique_ptr<DebugCamera> debugCamera_;

	std::unique_ptr<SimpleRooms> simpleRooms_;
	std::unique_ptr<SneakWalk> sneakWalk_;
	std::unique_ptr<Sphere> sphere_;
	std::unique_ptr<SamplerTest> samplerTest_;
	std::unique_ptr<AnimationTest> animationTest_;
	std::unique_ptr<AnimationMiscTest> animationMiscTest_;
	std::unique_ptr<SpotLightTest> spotLightTest_;

	std::vector<std::string> labels_;
	int editIndex_ = 0;
};
