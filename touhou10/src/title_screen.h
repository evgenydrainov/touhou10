#pragma once

#include "common.h"

struct Title_Screen {

	enum State {
		ANIM_IN,
		NORMAL,
		ANIM_OUT,
	};

	State state;
	float animation;
	int cursor;

	void init();
	void destroy();
	void update(float delta);
	void draw(float delta);
};
