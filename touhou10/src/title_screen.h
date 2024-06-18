#pragma once

#include "common.h"

struct Title_Screen {
	int cursor;

	void init();
	void destroy();
	void update(float delta);
	void draw(float delta);
};
