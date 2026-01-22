#pragma once
#include <cstdint>

// シーンインタフェース
class IScene {
public:
	IScene() = default;
	virtual ~IScene() = default;
	virtual void update(float delta) = 0;
	//virtual void update(float delta) = 0;
	virtual void draw(float delta) = 0;
	virtual void init() = 0;
	virtual void loadAsync()=0;
	virtual void dispose() = 0;
};
