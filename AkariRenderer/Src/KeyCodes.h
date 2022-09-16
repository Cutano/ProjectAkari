#pragma once

namespace Akari
{
	typedef enum class KeyCode : uint16_t
	{
		// From glfw3.h
		Space = 32,
		Apostrophe = 39, /* ' */
		Comma = 44, /* , */
		Minus = 45, /* - */
		Period = 46, /* . */
		Slash = 47, /* / */

		D0 = 48, /* 0 */ 
		D1 = 49, /* 1 */
		D2 = 50, /* 2 */
		D3 = 51, /* 3 */
		D4 = 52, /* 4 */
		D5 = 53, /* 5 */
		D6 = 54, /* 6 */
		D7 = 55, /* 7 */
		D8 = 56, /* 8 */
		D9 = 57, /* 9 */

		Semicolon = 59, /* ; */
		Equal = 61, /* = */

		A = 65,
		B = 66,
		C = 67,
		D = 68,
		E = 69,
		F = 70,
		G = 71,
		H = 72,
		I = 73,
		J = 74,
		K = 75,
		L = 76,
		M = 77,
		N = 78,
		O = 79,
		P = 80,
		Q = 81,
		R = 82,
		S = 83,
		T = 84,
		U = 85,
		V = 86,
		W = 87,
		X = 88,
		Y = 89,
		Z = 90,

		LeftBracket = 91,  /* [ */
		Backslash = 92,  /* \ */
		RightBracket = 93,  /* ] */
		GraveAccent = 96,  /* ` */

		World1 = 161, /* non-US #1 */
		World2 = 162, /* non-US #2 */

		/* Function keys */
		Escape = 256,
		Enter = 257,
		Tab = 258,
		Backspace = 259,
		Insert = 260,
		Delete = 261,
		Right = 262,
		Left = 263,
		Down = 264,
		Up = 265,
		PageUp = 266,
		PageDown = 267,
		Home = 268,
		End = 269,
		CapsLock = 280,
		ScrollLock = 281,
		NumLock = 282,
		PrintScreen = 283,
		Pause = 284,
		F1 = 290,
		F2 = 291,
		F3 = 292,
		F4 = 293,
		F5 = 294,
		F6 = 295,
		F7 = 296,
		F8 = 297,
		F9 = 298,
		F10 = 299,
		F11 = 300,
		F12 = 301,
		F13 = 302,
		F14 = 303,
		F15 = 304,
		F16 = 305,
		F17 = 306,
		F18 = 307,
		F19 = 308,
		F20 = 309,
		F21 = 310,
		F22 = 311,
		F23 = 312,
		F24 = 313,
		F25 = 314,

		/* Keypad */
		KP0 = 320,
		KP1 = 321,
		KP2 = 322,
		KP3 = 323,
		KP4 = 324,
		KP5 = 325,
		KP6 = 326,
		KP7 = 327,
		KP8 = 328,
		KP9 = 329,
		KPDecimal = 330,
		KPDivide = 331,
		KPMultiply = 332,
		KPSubtract = 333,
		KPAdd = 334,
		KPEnter = 335,
		KPEqual = 336,

		LeftShift = 340,
		LeftControl = 341,
		LeftAlt = 342,
		LeftSuper = 343,
		RightShift = 344,
		RightControl = 345,
		RightAlt = 346,
		RightSuper = 347,
		Menu = 348
	} Key;

	enum class CursorMode
	{
		Normal = 0,
		Hidden = 1,
		Locked = 2
	};

	typedef enum class MouseButton : uint16_t
	{
		Button0 = 0,
		Button1 = 1,
		Button2 = 2,
		Button3 = 3,
		Button4 = 4,
		Button5 = 5,
		Left = Button0,
		Right = Button1,
		Middle = Button2
	} Button;


	inline std::ostream& operator<<(std::ostream& os, KeyCode keyCode)
	{
		os << static_cast<int32_t>(keyCode);
		return os;
	}

	inline std::ostream& operator<<(std::ostream& os, MouseButton button)
	{
		os << static_cast<int32_t>(button);
		return os;
	}
}

// From glfw3.h
#define AKR_KEY_SPACE           ::Akari::Key::Space
#define AKR_KEY_APOSTROPHE      ::Akari::Key::Apostrophe    /* ' */
#define AKR_KEY_COMMA           ::Akari::Key::Comma         /* , */
#define AKR_KEY_MINUS           ::Akari::Key::Minus         /* - */
#define AKR_KEY_PERIOD          ::Akari::Key::Period        /* . */
#define AKR_KEY_SLASH           ::Akari::Key::Slash         /* / */
#define AKR_KEY_0               ::Akari::Key::D0
#define AKR_KEY_1               ::Akari::Key::D1
#define AKR_KEY_2               ::Akari::Key::D2
#define AKR_KEY_3               ::Akari::Key::D3
#define AKR_KEY_4               ::Akari::Key::D4
#define AKR_KEY_5               ::Akari::Key::D5
#define AKR_KEY_6               ::Akari::Key::D6
#define AKR_KEY_7               ::Akari::Key::D7
#define AKR_KEY_8               ::Akari::Key::D8
#define AKR_KEY_9               ::Akari::Key::D9
#define AKR_KEY_SEMICOLON       ::Akari::Key::Semicolon     /* ; */
#define AKR_KEY_EQUAL           ::Akari::Key::Equal         /* = */
#define AKR_KEY_A               ::Akari::Key::A
#define AKR_KEY_B               ::Akari::Key::B
#define AKR_KEY_C               ::Akari::Key::C
#define AKR_KEY_D               ::Akari::Key::D
#define AKR_KEY_E               ::Akari::Key::E
#define AKR_KEY_F               ::Akari::Key::F
#define AKR_KEY_G               ::Akari::Key::G
#define AKR_KEY_H               ::Akari::Key::H
#define AKR_KEY_I               ::Akari::Key::I
#define AKR_KEY_J               ::Akari::Key::J
#define AKR_KEY_K               ::Akari::Key::K
#define AKR_KEY_L               ::Akari::Key::L
#define AKR_KEY_M               ::Akari::Key::M
#define AKR_KEY_N               ::Akari::Key::N
#define AKR_KEY_O               ::Akari::Key::O
#define AKR_KEY_P               ::Akari::Key::P
#define AKR_KEY_Q               ::Akari::Key::Q
#define AKR_KEY_R               ::Akari::Key::R
#define AKR_KEY_S               ::Akari::Key::S
#define AKR_KEY_T               ::Akari::Key::T
#define AKR_KEY_U               ::Akari::Key::U
#define AKR_KEY_V               ::Akari::Key::V
#define AKR_KEY_W               ::Akari::Key::W
#define AKR_KEY_X               ::Akari::Key::X
#define AKR_KEY_Y               ::Akari::Key::Y
#define AKR_KEY_Z               ::Akari::Key::Z
#define AKR_KEY_LEFT_BRACKET    ::Akari::Key::LeftBracket   /* [ */
#define AKR_KEY_BACKSLASH       ::Akari::Key::Backslash     /* \ */
#define AKR_KEY_RIGHT_BRACKET   ::Akari::Key::RightBracket  /* ] */
#define AKR_KEY_GRAVE_ACCENT    ::Akari::Key::GraveAccent   /* ` */
#define AKR_KEY_WORLD_1         ::Akari::Key::World1        /* non-US #1 */
#define AKR_KEY_WORLD_2         ::Akari::Key::World2        /* non-US #2 */

/* Function keys */
#define AKR_KEY_ESCAPE          ::Akari::Key::Escape
#define AKR_KEY_ENTER           ::Akari::Key::Enter
#define AKR_KEY_TAB             ::Akari::Key::Tab
#define AKR_KEY_BACKSPACE       ::Akari::Key::Backspace
#define AKR_KEY_INSERT          ::Akari::Key::Insert
#define AKR_KEY_DELETE          ::Akari::Key::Delete
#define AKR_KEY_RIGHT           ::Akari::Key::Right
#define AKR_KEY_LEFT            ::Akari::Key::Left
#define AKR_KEY_DOWN            ::Akari::Key::Down
#define AKR_KEY_UP              ::Akari::Key::Up
#define AKR_KEY_PAGE_UP         ::Akari::Key::PageUp
#define AKR_KEY_PAGE_DOWN       ::Akari::Key::PageDown
#define AKR_KEY_HOME            ::Akari::Key::Home
#define AKR_KEY_END             ::Akari::Key::End
#define AKR_KEY_CAPS_LOCK       ::Akari::Key::CapsLock
#define AKR_KEY_SCROLL_LOCK     ::Akari::Key::ScrollLock
#define AKR_KEY_NUM_LOCK        ::Akari::Key::NumLock
#define AKR_KEY_PRINT_SCREEN    ::Akari::Key::PrintScreen
#define AKR_KEY_PAUSE           ::Akari::Key::Pause
#define AKR_KEY_F1              ::Akari::Key::F1
#define AKR_KEY_F2              ::Akari::Key::F2
#define AKR_KEY_F3              ::Akari::Key::F3
#define AKR_KEY_F4              ::Akari::Key::F4
#define AKR_KEY_F5              ::Akari::Key::F5
#define AKR_KEY_F6              ::Akari::Key::F6
#define AKR_KEY_F7              ::Akari::Key::F7
#define AKR_KEY_F8              ::Akari::Key::F8
#define AKR_KEY_F9              ::Akari::Key::F9
#define AKR_KEY_F10             ::Akari::Key::F10
#define AKR_KEY_F11             ::Akari::Key::F11
#define AKR_KEY_F12             ::Akari::Key::F12
#define AKR_KEY_F13             ::Akari::Key::F13
#define AKR_KEY_F14             ::Akari::Key::F14
#define AKR_KEY_F15             ::Akari::Key::F15
#define AKR_KEY_F16             ::Akari::Key::F16
#define AKR_KEY_F17             ::Akari::Key::F17
#define AKR_KEY_F18             ::Akari::Key::F18
#define AKR_KEY_F19             ::Akari::Key::F19
#define AKR_KEY_F20             ::Akari::Key::F20
#define AKR_KEY_F21             ::Akari::Key::F21
#define AKR_KEY_F22             ::Akari::Key::F22
#define AKR_KEY_F23             ::Akari::Key::F23
#define AKR_KEY_F24             ::Akari::Key::F24
#define AKR_KEY_F25             ::Akari::Key::F25

/* Keypad */
#define AKR_KEY_KP_0            ::Akari::Key::KP0
#define AKR_KEY_KP_1            ::Akari::Key::KP1
#define AKR_KEY_KP_2            ::Akari::Key::KP2
#define AKR_KEY_KP_3            ::Akari::Key::KP3
#define AKR_KEY_KP_4            ::Akari::Key::KP4
#define AKR_KEY_KP_5            ::Akari::Key::KP5
#define AKR_KEY_KP_6            ::Akari::Key::KP6
#define AKR_KEY_KP_7            ::Akari::Key::KP7
#define AKR_KEY_KP_8            ::Akari::Key::KP8
#define AKR_KEY_KP_9            ::Akari::Key::KP9
#define AKR_KEY_KP_DECIMAL      ::Akari::Key::KPDecimal
#define AKR_KEY_KP_DIVIDE       ::Akari::Key::KPDivide
#define AKR_KEY_KP_MULTIPLY     ::Akari::Key::KPMultiply
#define AKR_KEY_KP_SUBTRACT     ::Akari::Key::KPSubtract
#define AKR_KEY_KP_ADD          ::Akari::Key::KPAdd
#define AKR_KEY_KP_ENTER        ::Akari::Key::KPEnter
#define AKR_KEY_KP_EQUAL        ::Akari::Key::KPEqual

#define AKR_KEY_LEFT_SHIFT      ::Akari::Key::LeftShift
#define AKR_KEY_LEFT_CONTROL    ::Akari::Key::LeftControl
#define AKR_KEY_LEFT_ALT        ::Akari::Key::LeftAlt
#define AKR_KEY_LEFT_SUPER      ::Akari::Key::LeftSuper
#define AKR_KEY_RIGHT_SHIFT     ::Akari::Key::RightShift
#define AKR_KEY_RIGHT_CONTROL   ::Akari::Key::RightControl
#define AKR_KEY_RIGHT_ALT       ::Akari::Key::RightAlt
#define AKR_KEY_RIGHT_SUPER     ::Akari::Key::RightSuper
#define AKR_KEY_MENU            ::Akari::Key::Menu

// Mouse
#define AKR_MOUSE_BUTTON_LEFT    ::Akari::Button::Left
#define AKR_MOUSE_BUTTON_RIGHT   ::Akari::Button::Right
#define AKR_MOUSE_BUTTON_MIDDLE  ::Akari::Button::Middle
