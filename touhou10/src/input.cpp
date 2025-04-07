#include "input.h"

#include "window_creation.h"

Input input;

struct InputBinding {
	SDL_Scancode scancode0;
	SDL_Scancode scancode1;
	SDL_GameControllerButton controller_button0;
	SDL_GameControllerButton controller_button1;
};

static const InputBinding g_InputBindings[] = {
	/* INPUT_MOVE_RIGHT    */ { SDL_SCANCODE_RIGHT, (SDL_Scancode)0, SDL_CONTROLLER_BUTTON_DPAD_RIGHT },
	/* INPUT_MOVE_UP       */ { SDL_SCANCODE_UP,    (SDL_Scancode)0, SDL_CONTROLLER_BUTTON_DPAD_UP },
	/* INPUT_MOVE_LEFT     */ { SDL_SCANCODE_LEFT,  (SDL_Scancode)0, SDL_CONTROLLER_BUTTON_DPAD_LEFT },
	/* INPUT_MOVE_DOWN     */ { SDL_SCANCODE_DOWN,  (SDL_Scancode)0, SDL_CONTROLLER_BUTTON_DPAD_DOWN },

	/* INPUT_FIRE          */ { SDL_SCANCODE_Z,      (SDL_Scancode)0,     SDL_CONTROLLER_BUTTON_A },
	/* INPUT_BOMB          */ { SDL_SCANCODE_X,      (SDL_Scancode)0,     SDL_CONTROLLER_BUTTON_B },
	/* INPUT_FOCUS         */ { SDL_SCANCODE_LSHIFT, SDL_SCANCODE_RSHIFT, SDL_CONTROLLER_BUTTON_X },
	/* INPUT_SKIP_DIALOGUE */ { SDL_SCANCODE_LCTRL,  SDL_SCANCODE_RCTRL,  SDL_CONTROLLER_BUTTON_Y },

	/* INPUT_PAUSE         */ { SDL_SCANCODE_ESCAPE, (SDL_Scancode)0, SDL_CONTROLLER_BUTTON_START },

	/* INPUT_UI_RIGHT      */ { SDL_SCANCODE_RIGHT, (SDL_Scancode)0, SDL_CONTROLLER_BUTTON_DPAD_RIGHT, (SDL_GameControllerButton)0 },
	/* INPUT_UI_UP         */ { SDL_SCANCODE_UP,    (SDL_Scancode)0, SDL_CONTROLLER_BUTTON_DPAD_UP,    (SDL_GameControllerButton)0 },
	/* INPUT_UI_LEFT       */ { SDL_SCANCODE_LEFT,  (SDL_Scancode)0, SDL_CONTROLLER_BUTTON_DPAD_LEFT,  (SDL_GameControllerButton)0 },
	/* INPUT_UI_DOWN       */ { SDL_SCANCODE_DOWN,  (SDL_Scancode)0, SDL_CONTROLLER_BUTTON_DPAD_DOWN,  (SDL_GameControllerButton)0 },

	/* INPUT_UI_CONFIRM    */ { SDL_SCANCODE_Z, SDL_SCANCODE_RETURN, SDL_CONTROLLER_BUTTON_A, SDL_CONTROLLER_BUTTON_START },
	/* INPUT_UI_CANCEL     */ { SDL_SCANCODE_X, SDL_SCANCODE_ESCAPE, SDL_CONTROLLER_BUTTON_B },
};

void Input::init() {}

void Input::deinit() {}

bool Input::handle_event(const SDL_Event& ev) {
	switch (ev.type) {
		case SDL_KEYDOWN: {
			SDL_Scancode scancode = ev.key.keysym.scancode;

			if (scancode == 0) break;

			u32 mask = 1;
			for (auto& it : g_InputBindings) {
				if (it.scancode0 == scancode || it.scancode1 == scancode) {
					if (ev.key.repeat) {
						state_repeat |= mask;
					} else {
						state_press |= mask;
						state |= mask;
					}
				}

				mask <<= 1;
			}
			break;
		}

		case SDL_KEYUP: {
			SDL_Scancode scancode = ev.key.keysym.scancode;

			if (scancode == 0) break;

			u32 mask = 1;
			for (auto& it : g_InputBindings) {
				if (it.scancode0 == scancode || it.scancode1 == scancode) {
					state_release |= mask;
					state &= ~mask;
				}

				mask <<= 1;
			}
			break;
		}
	}

	return false;
}

void Input::update(float delta) {}

void Input::clear() {
	// `state` is not cleared
	state_press = 0;
	state_release = 0;
	state_repeat = 0;
}

bool is_input_held(InputKey key) {
	return (input.state & key) != 0;
}

bool is_input_pressed(InputKey key, bool repeat) {
	bool result = (input.state_press & key) != 0;

	if (repeat) {
		result |= (input.state_repeat & key) != 0;
	}

	return result;
}

bool is_input_released(InputKey key) {
	return (input.state_release & key) != 0;
}
