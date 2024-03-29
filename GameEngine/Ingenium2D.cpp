#include "Ingenium2D.h"

// Callum Mackenzie

using namespace ingenium2D;

Ingenium2D* Ingenium2D::engine;

Ingenium2D* Ingenium2D::getEngine()
{
	if (engine == nullptr) {
		engine = new Ingenium2D();
	}
	return engine;
}
Ingenium2D::Ingenium2D()
{
	running = true;
}
Ingenium2D::~Ingenium2D() {
	if (running) {
		stop();
	}
#if defined(_DEBUG) && defined(INGENIUM_WND_GUI)
	Debug::destroyDebugWindow();
#endif
	memory::safe_delete(drwn);
#if RENDERER == RENDERER_2D_WINDOWS
	memory::safe_delete(primeClass);
#endif
}
void Ingenium2D::stop()
{
	running = false;
	onClose();
	memory::safe_delete(Input::input);
	memory::safe_delete(Physics2D::physics2D);
}
void Ingenium2D::createWindow(const char* name, int width, int height)
{
	Ingenium2D::engine = this;
#if RENDERER == RENDERER_2D_WINDOWS
	primeClass->setWindowProc(DEFAULT_WND_PROC);
	primeClass->wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
	primeClass->registerClass();

	RootWindow* win = new RootWindow(primeClass->hInst, primeClass, L"Ingenium", CW_USEDEFAULT, CW_USEDEFAULT, height, width);
	win->style = WS_SYSMENU | WS_SIZEBOX;
	win->create();
	win->show();

	drwn = new Direct2DWindow(win);
	drwn->clear();
#endif
#if RENDERER == RENDERER_3D
	drwn = new OpenGLWindow(width, height);
#endif
}
void Ingenium2D::start()
{
	init();
	if (drwn) {
#if RENDERER == RENDERER_3D
		if (!drwn->initSuccess) {
			abort();
			return;
		}
#endif
	}
	else 
		return;
#if RENDERER == RENDERER_2D_WINDOWS
	MSG msg;
#endif
	while (running)
	{
		if (Ingenium2D::getEngine()->running) 
		{
#if RENDERER == RENDERER_3D
			if (glfwWindowShouldClose(drwn->window))
			{
				running = false;
				break;
			}
#endif
#if RENDERER == RENDERER_2D_WINDOWS
			if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
#endif
			if (Time::nextFixedFrameReady()) {
				onFixedUpdate();
			}
			if (Time::nextFrameReady()) {
#if RENDERER == RENDERER_3D
				glfwPollEvents();
#endif
				onUpdate();
#if defined(_DEBUG) && defined(INGENIUM_WND_GUI)
				if (Debug::windowWriteReady()) {
					Debug::oss << "FPS: " << (1000.f / Time::getTime()->deltaTime) << "\n";
					Debug::writeToWindow();
				}
				Debug::oss.str("");
				Debug::oss.clear();
#endif
			}
		}
	}
}
void Ingenium2D::init()
{
#if defined(SCRIPT_LUA)
	ingenium_lua::initLua();
#endif
#if defined(_DEBUG) && defined(INGENIUM_WND_GUI)
	HINSTANCE hInstance = GetModuleHandle(NULL);
	HINSTANCE hPrevInstance = NULL;
	Debug::createDebugWindow(hInstance);
#endif

#if RENDERER == RENDERER_2D_WINDOWS
	primeClass = new WindowClass(L"Ingenium WC", hInstance);
#endif

	onCreate();

#if defined(SCRIPT_LUA)
	loadToLua();
	ingenium_lua::loadFileA(windows::fileAbsolutePathA(LUA_ENGINE_ENTRY));
	ingenium_lua::executeChunk();
	ingenium_lua::executeFunc(LUA_ENGINE_INIT);
#endif
}
void Ingenium2D::onCreate()
{
}
void Ingenium2D::onUpdate() {
#if defined(SCRIPT_LUA)
	ingenium_lua::executeFunc(LUA_ENGINE_UPDATE);
#endif
}
void Ingenium2D::onFixedUpdate() {
#if defined(SCRIPT_LUA)
	ingenium_lua::executeFunc(LUA_ENGINE_FIXED_UPDATE);
	luaL_dostring(ingenium_lua::state, "collectgarbage(\"collect\")");
#endif
}
void Ingenium2D::onClose() {
#if defined(SCRIPT_LUA)
	ingenium_lua::executeFunc(LUA_ENGINE_CLOSE);
	ingenium_lua::stopLua();
#endif
}
std::string Ingenium2D::getFileAsString(std::string fPath)
{
	std::ifstream f;
	std::stringstream ss;
	std::string ln;
	f.open(fPath);
	while (!f.eof()) {
		getline(f, ln);
		ss << ln << "\n";
	}
	return ss.str();
}
#if RENDERER == RENDERER_2D_WINDOWS
LRESULT CALLBACK Ingenium2D::DEFAULT_WND_PROC(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_SIZE:
		if (Ingenium2D::getEngine()->drwn) {

#if RENDERER == RENDERER_2D_WINDOWS
			Ingenium2D::getEngine()->drwn->resizePRT(LOWORD(lParam), HIWORD(lParam));
			Ingenium2D::getEngine()->drwn->calculateRPR();
#endif
		}
		break;
	case WM_CLOSE:
		Ingenium2D::getEngine()->stop();
		PostQuitMessage(0);
		break;
	case WM_LBUTTONDOWN:
		Input::getInput()->MouseClick[0] = true;
		break;
	case WM_LBUTTONUP:
		Input::getInput()->MouseClick[0] = false;
		break;
	case WM_RBUTTONDOWN:
		Input::getInput()->MouseClick[1] = true;
		break;
	case WM_RBUTTONUP:
		Input::getInput()->MouseClick[1] = false;
		break;
	case WM_MBUTTONDOWN:
		Input::getInput()->MouseClick[2] = true;
		break;
	case WM_MBUTTONUP:
		Input::getInput()->MouseClick[2] = false;
		break;
	case WM_MOUSELEAVE || WM_NCMOUSELEAVE || WM_SETFOCUS:
		Input::getInput()->MouseClick[0] = false;
		Input::getInput()->MouseClick[1] = false;
		Input::getInput()->MouseClick[2] = false;
		break;
	default:
		break;
	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}
#endif

#if defined(SCRIPT_LUA)
namespace lua_funcs_2D
{
	template <typename T> T* uDataToPtr(void* data) {
		return *(T**)data;
	}
	template <typename T> T* getSelfAsUData(lua_State* L, int index, const char* inheritName) {
		using namespace ingenium_lua;
		void* ud = 0;
		luaL_checktype(L, index, LUA_TTABLE);
		lua_getfield(L, index, "__self");
		ud = lua_touserdata(L, index + 1);
		T* ret = uDataToPtr<T>(ud);
		// luaL_argcheck(L, ud != 0, "Inheritable class type expected but not found.");
		return ret;
	}
	namespace d2d
	{
#if RENDERER == RENDERER_2D_WINDOWS
		ingenium_lua::LuaClass<Direct2DWindow> iClass = ingenium_lua::LuaClass<Direct2DWindow>("D2D");
#endif
#if RENDERER == RENDERER_3D
		ingenium_lua::LuaClass<OpenGLWindow> iClass = ingenium_lua::LuaClass<OpenGLWindow>("D2D");
#endif

		int setDRWNSize(lua_State* lua) {
			int nargs = lua_gettop(lua);
			if (nargs != 2)
				return luaL_error(lua, "Got %d arguments, expected 2: (number, number).", nargs);
			UINT width = luaL_checkinteger(lua, 1);
			UINT height = luaL_checkinteger(lua, 2);

			lua_pop(lua, 2);

			Ingenium2D::getEngine()->drwn->setSize(width, height);
			return 0;
		}
		int getMouseXDRWN(lua_State* lua) {
			int nargs = lua_gettop(lua);
			if (nargs != 0)
				return luaL_error(lua, "Got %d arguments, expected 0.", nargs);
			lua_pushnumber(lua, Ingenium2D::getEngine()->drwn->getMouseX());
			return 1;
		}
		int getMouseYDRWN(lua_State* lua) {
			int nargs = lua_gettop(lua);
			if (nargs != 0)
				return luaL_error(lua, "Got %d arguments, expected 0.", nargs);
			lua_pushnumber(lua, Ingenium2D::getEngine()->drwn->getMouseY());
			return 1;
		}
		int printDRWN(lua_State* lua) {
			int nargs = lua_gettop(lua);
			for (int i = 0; i < nargs; i++) {
				Debug::oss << luaL_checkstring(lua, i + 1);
				Debug::write();
			}
			lua_pop(lua, nargs);
			return 0;
		}
		int renderDRWN(lua_State* lua) {
			int nargs = lua_gettop(lua);
			if (nargs != 1)
				return luaL_error(lua, "Got %d arguments, expected 1: (boolean).", nargs);
			bool clear = lua_toboolean(lua, 1);
			Ingenium2D::getEngine()->drwn->beginRender();
			Ingenium2D::getEngine()->drwn->drawQueue(clear);
			Ingenium2D::getEngine()->drwn->endRender();
			lua_pop(lua, 1);
			return 0;
		}
		int setDRWNFullScreen(lua_State* lua) {
			int nargs = lua_gettop(lua);
			if (nargs != 0)
				return luaL_error(lua, "Got %d arguments, expected 0.", nargs);
			Ingenium2D::getEngine()->drwn->window->setFullscreen();
			return 0;
		}
		int setDRWNClearColour(lua_State* lua) {
			int nargs = lua_gettop(lua);
			if (nargs != 1)
				return luaL_error(lua, "Got %d arguments, expected 1: (int).", nargs);
			UINT32 colour = luaL_checkinteger(lua, 1);
			Ingenium2D::getEngine()->drwn->clearColour = D2D1::ColorF(colour);
			return 0;
		}
		int isKeyPressed(lua_State* lua) {
			int nargs = lua_gettop(lua);
			if (nargs != 1)
				return luaL_error(lua, "Got %d arguments, expected 1: (integer).", nargs);
			int key = luaL_checkinteger(lua, 1);
			lua_pop(lua, 1);
			bool keyPressed = Input::getInput()->getKeyState(key);
			lua_pushboolean(lua, keyPressed);
			return 1;
		}
		int showWindow(lua_State* lua) {
			int nargs = lua_gettop(lua);
			if (nargs != 0)
				return luaL_error(lua, "Got %d arguments, expected 0.", nargs);

			Ingenium2D::getEngine()->drwn->window->show();

			return 0;
		}
		int setWallpaper(lua_State* lua) {
			int nargs = lua_gettop(lua);
			if (nargs != 0)
				return luaL_error(lua, "Got %d arguments, expected 0.", nargs);

			Ingenium2D::getEngine()->drwn->getWindow()->embedWallpaper();

			return 0;
		}
		int setCameraPos(lua_State* lua) {
			int nargs = lua_gettop(lua);
			if (nargs != 2)
				return luaL_error(lua, "Got %d arguments, expected 2: (number, number).", nargs);

			Ingenium2D::getEngine()->drwn->offset = D2D1::Point2F(-lua_tonumber(lua, 1), -lua_tonumber(lua, 2));
			lua_pop(lua, 2);

			return 0;
		}
		int setCameraX(lua_State* lua) {
			int nargs = lua_gettop(lua);
			if (nargs != 1)
				return luaL_error(lua, "Got %d arguments, expected 1: (number).", nargs);

			lua_pushnumber(lua, Ingenium2D::getEngine()->drwn->offset.y);

			return setCameraPos(lua);
		}
		int setCameraY(lua_State* lua) {
			int nargs = lua_gettop(lua);
			if (nargs != 1)
				return luaL_error(lua, "Got %d arguments, expected 1: (number).", nargs);

			float y = lua_tonumber(lua, 1);
			lua_pop(lua, 1);
			lua_pushnumber(lua, Ingenium2D::getEngine()->drwn->offset.x);
			lua_pushnumber(lua, y);

			return setCameraPos(lua);
		}
		int addCameraPos(lua_State* lua) {
			int nargs = lua_gettop(lua);
			if (nargs != 2)
				return luaL_error(lua, "Got %d arguments, expected 2: (number, number).", nargs);

			Ingenium2D::getEngine()->drwn->offset.x += lua_tonumber(lua, 1);
			Ingenium2D::getEngine()->drwn->offset.y += lua_tonumber(lua, 2);
			lua_pop(lua, 2);

			return 0;
		}
		int getCameraX(lua_State* lua) {
			int nargs = lua_gettop(lua);
			if (nargs != 2)
				return luaL_error(lua, "Got %d arguments, expected 0.", nargs);

			lua_pushnumber(lua, Ingenium2D::getEngine()->drwn->offset.x);

			return 1;
		}
		int getCameraY(lua_State* lua) {
			int nargs = lua_gettop(lua);
			if (nargs != 2)
				return luaL_error(lua, "Got %d arguments, expected 0.", nargs);

			lua_pushnumber(lua, Ingenium2D::getEngine()->drwn->offset.y);

			return 1;
		}
		int setCameraZoom(lua_State* lua) {
			int nargs = lua_gettop(lua);
			if (nargs != 2)
				return luaL_error(lua, "Got %d arguments, expected 2: (number, number).", nargs);

			Ingenium2D::getEngine()->drwn->zoom = D2D1::SizeF(lua_tonumber(lua, 1), lua_tonumber(lua, 2));
			lua_pop(lua, 2);

			return 0;
		}

		void registerDRWN(lua_State* lua)
		{
			using namespace ingenium_lua;
			
			iClass.addMetaMethod(lua, lua_func("setSize", setDRWNSize));
			iClass.addMetaMethod(lua, lua_func("render", renderDRWN));
			iClass.addMetaMethod(lua, lua_func("getMouseX", getMouseXDRWN));
			iClass.addMetaMethod(lua, lua_func("getMouseY", getMouseYDRWN));
			iClass.addMetaMethod(lua, lua_func("write", printDRWN));
			iClass.addMetaMethod(lua, lua_func("setFullscreen", setDRWNFullScreen));
			iClass.addMetaMethod(lua, lua_func("setClearColour", setDRWNClearColour));
			iClass.addMetaMethod(lua, lua_func("keyPressed", isKeyPressed));
			iClass.addMetaMethod(lua, lua_func("show", showWindow));
			iClass.addMetaMethod(lua, lua_func("setWallpaper", setWallpaper));
			iClass.addMetaMethod(lua, lua_func("setCameraPos", setCameraPos));
			iClass.addMetaMethod(lua, lua_func("setCameraX", setCameraX));
			iClass.addMetaMethod(lua, lua_func("setCameraY", setCameraY));
			iClass.addMetaMethod(lua, lua_func("addCameraPos", addCameraPos));
			iClass.addMetaMethod(lua, lua_func("getCameraX", getCameraX));
			iClass.addMetaMethod(lua, lua_func("getCameraY", getCameraY));
			iClass.addMetaMethod(lua, lua_func("setCameraZoom", setCameraZoom));

			iClass.registerClass(lua);
		}
	}
	namespace time
	{
		ingenium_lua::LuaClass<Direct2DWindow> iClass = ingenium_lua::LuaClass<Direct2DWindow>("Time");

		int getDeltaTime(lua_State* lua) {
			int nargs = lua_gettop(lua);
			if (nargs != 0)
				return luaL_error(lua, "Got %d arguments, expected 0.", nargs);

			lua_pushnumber(lua, Time::deltaTime);
			return 1;
		}
		int getFixedDeltaTime(lua_State* lua) {
			int nargs = lua_gettop(lua);
			if (nargs != 0)
				return luaL_error(lua, "Got %d arguments, expected 0.", nargs);
		
			lua_pushnumber(lua, Time::fixedDeltaTime);

			return 1;
		}
		int setFPS(lua_State* lua) {
			int nargs = lua_gettop(lua);
			if (nargs != 1)
				return luaL_error(lua, "Got %d arguments, expected 1: (number).", nargs);

			float newFPS = luaL_checknumber(lua, 1);
			lua_pop(lua, 1);
			Time::setFPS(newFPS);

			return 0;
		}

		void registerTime(lua_State* lua) {
			using namespace ingenium_lua;

			iClass.addMetaMethod(lua, lua_func("fixedDeltaTime", getFixedDeltaTime));
			iClass.addMetaMethod(lua, lua_func("deltaTime", getDeltaTime));
			iClass.addMetaMethod(lua, lua_func("setFPS", setFPS));

			iClass.registerClass(lua);
		}
	}
	namespace vec2
	{
		ingenium_lua::LuaClass<Vector2> iClass = ingenium_lua::LuaClass<Vector2>("Vector2");

		constexpr int exargs = 1;

		int free(lua_State* lua) {
			return ingenium_lua::free<Vector2>(lua);
		}
		int newVector2(lua_State* lua);
		int magnitude(lua_State* lua) {
			int nargs = lua_gettop(lua);
			if (nargs != 1)
				return luaL_error(lua, "Got %d arguments, expected 1: (self).", nargs);
			Vector2* v2 = getSelfAsUData<Vector2>(lua, 1, iClass.metaName);
			lua_pushnumber(lua, Vector2::hypotenuse(*v2));
			return 1;
		}
		int normalize(lua_State* lua) {
			int nargs = lua_gettop(lua);
			if (nargs != 1)
				return luaL_error(lua, "Got %d arguments, expected 1: (self).", nargs);
			Vector2* v2 = getSelfAsUData<Vector2>(lua, 1, iClass.metaName);
			v2->normalize();
			return 0;
		}
		int getX(lua_State* lua) {
			int nargs = lua_gettop(lua);
			if (nargs != 1)
				return luaL_error(lua, "Got %d arguments, expected 1: (self).", nargs);

			Vector2* v2 = getSelfAsUData<Vector2>(lua, 1, iClass.metaName);
			lua_pushnumber(lua, v2->x);
			return 1;
		}
		int getY(lua_State* lua) {
			int nargs = lua_gettop(lua);
			if (nargs != 1)
				return luaL_error(lua, "Got %d arguments, expected 1: (self).", nargs);
			Vector2* v2 = getSelfAsUData<Vector2>(lua, 1, iClass.metaName);
			lua_pushnumber(lua, v2->y);
			return 1;
		}
		int setX(lua_State* lua) {
			int nargs = lua_gettop(lua);
			if (nargs != 2)
				return luaL_error(lua, "Got %d arguments, expected 2: (self, number).", nargs);
			float nv = luaL_checknumber(lua, 2);
			lua_pop(lua, 1);
			Vector2* v2 = getSelfAsUData<Vector2>(lua, 1, iClass.metaName);
			lua_pop(lua, 1);
			v2->x = nv;
			return 0;
		}
		int setY(lua_State* lua) {
			int nargs = lua_gettop(lua);
			if (nargs != 2)
				return luaL_error(lua, "Got %d arguments, expected 2: (self, number).", nargs);
			float nv = luaL_checknumber(lua, 2);
			lua_pop(lua, 1);
			Vector2* v2 = getSelfAsUData<Vector2>(lua, 1, iClass.metaName);
			lua_pop(lua, 1);
			v2->y = nv;
			return 0;
		}
		int add(lua_State* lua) {
			Vector2* v2 = getSelfAsUData<Vector2>(lua, 2, iClass.metaName);
			lua_pop(lua, 2);
			Vector2* v1 = getSelfAsUData<Vector2>(lua, 1, iClass.metaName);
			lua_pop(lua, 2);
			lua_getglobal(lua, iClass.name);
			lua_pushnumber(lua, v1->x + v2->x);
			lua_pushnumber(lua, v1->y + v2->y);
			return newVector2(lua);
		}
		int subtract(lua_State* lua) {
			Vector2* v2 = getSelfAsUData<Vector2>(lua, 2, iClass.metaName);
			lua_pop(lua, 2);
			Vector2* v1 = getSelfAsUData<Vector2>(lua, 1, iClass.metaName);
			lua_pop(lua, 2);
			lua_getglobal(lua, iClass.name);
			lua_pushnumber(lua, v1->x - v2->x);
			lua_pushnumber(lua, v1->y - v2->y);
			return newVector2(lua);
		}
		int multiply(lua_State* lua) {
			Vector2* v2 = getSelfAsUData<Vector2>(lua, 2, iClass.metaName);
			lua_pop(lua, 2);
			Vector2* v1 = getSelfAsUData<Vector2>(lua, 1, iClass.metaName);
			lua_pop(lua, 2);
			lua_getglobal(lua, iClass.name);
			lua_pushnumber(lua, v1->x * v2->x);
			lua_pushnumber(lua, v1->y * v2->y);
			return newVector2(lua);
		}
		int divide(lua_State* lua) {
			Vector2* v2 = getSelfAsUData<Vector2>(lua, 2, iClass.metaName);
			lua_pop(lua, 2);
			Vector2* v1 = getSelfAsUData<Vector2>(lua, 1, iClass.metaName);
			lua_pop(lua, 2);
			lua_getglobal(lua, iClass.name);
			lua_pushnumber(lua, v1->x / v2->x);
			lua_pushnumber(lua, v1->y / v2->y);
			return newVector2(lua);
		}
		int unaryMinus(lua_State* lua) {
			Vector2* v1 = getSelfAsUData<Vector2>(lua, 2, iClass.metaName);
			lua_pop(lua, 3);
			lua_getglobal(lua, iClass.name);
			lua_pushnumber(lua, -v1->x);
			lua_pushnumber(lua, -v1->y);
			return newVector2(lua);
		}
		int floorDiv(lua_State* lua) {
			Vector2* v2 = getSelfAsUData<Vector2>(lua, 2, iClass.metaName);
			lua_pop(lua, 2);
			Vector2* v1 = getSelfAsUData<Vector2>(lua, 1, iClass.metaName);
			lua_pop(lua, 2);
			lua_getglobal(lua, iClass.name);
			lua_pushnumber(lua, (int)(v1->x / v2->x));
			lua_pushnumber(lua, (int)(v1->y / v2->y));
			return newVector2(lua);
		}
		int toString(lua_State* lua) {
			Vector2* v2 = getSelfAsUData<Vector2>(lua, 1, iClass.metaName);
			std::string v2s("vec2(");
			v2s = v2s.append(std::to_string(v2->x)).append(", ").append(std::to_string(v2->y)).append(")");
			lua_pushstring(lua, v2s.c_str());
			return 1;
		}
		int getNormalized(lua_State* lua) {
			int nargs = lua_gettop(lua);
			if (nargs != 1)
				return luaL_error(lua, "Got %d arguments, expected 1: (self).", nargs);
			Vector2 v2 = *getSelfAsUData<Vector2>(lua, 1, iClass.metaName);
			lua_pop(lua, 1);

			v2.normalize();
			lua_pushnumber(lua, v2.x);
			lua_pushnumber(lua, v2.y);
			return newVector2(lua);
		}

		int newVector2(lua_State* lua) {
			int nargs = lua_gettop(lua);
			if (nargs != 0 + exargs && nargs != 2 + exargs)
				return luaL_error(lua, "Got %d arguments, expected 0 or 2: (number, number).", nargs);

			float xComp = 0;
			float yComp = 0;
			if (nargs == 2 + exargs) {
				xComp = luaL_checknumber(lua, 1 + exargs);
				yComp = luaL_checknumber(lua, 2 + exargs);
				lua_pop(lua, 2);
			}
			Vector2* vec2 = new Vector2(xComp, yComp);

			iClass.createInstance(lua, vec2);
			return 1;
		}

		void registerVector2(lua_State* lua) {
			using namespace ingenium_lua;
			iClass.addMetaMethod(lua, lua_func("new", newVector2));

			iClass.addMetaMethod(lua, lua_func("__gc", free));
			iClass.addMetaMethod(lua, lua_func("__tostring", toString));
			iClass.addMetaMethod(lua, lua_func("__add", add));
			iClass.addMetaMethod(lua, lua_func("__sub", subtract));
			iClass.addMetaMethod(lua, lua_func("__mul", multiply));
			iClass.addMetaMethod(lua, lua_func("__div", divide));
			iClass.addMetaMethod(lua, lua_func("__unm", unaryMinus));
			iClass.addMetaMethod(lua, lua_func("__idiv", floorDiv));

			iClass.addMethod(lua, lua_func("getX", getX));
			iClass.addMethod(lua, lua_func("getY", getY));
			iClass.addMethod(lua, lua_func("setX", setX));
			iClass.addMethod(lua, lua_func("setY", setY));
			iClass.addMethod(lua, lua_func("normalize", normalize));
			iClass.addMethod(lua, lua_func("normalized", getNormalized));
			iClass.addMethod(lua, lua_func("magnitude", magnitude));

			iClass.registerClass(lua);
		}
#ifdef CONSTRUCTOR_METHOD_NAME
#undef CONSTRUCTOR_METHOD_NAME
#endif
	}
	namespace collision_data_2D {
		ingenium_lua::LuaClass<Physics2D::CollisionData> iClass = ingenium_lua::LuaClass<Physics2D::CollisionData>("CollisionData2D");

		int free(lua_State* lua) {
			return ingenium_lua::free<Physics2D::CollisionData>(lua);
		}
		int toString(lua_State* lua) {
			Physics2D::CollisionData* v2 = getSelfAsUData<Physics2D::CollisionData>(lua, 1, iClass.metaName);
			std::string v2s("");
			v2s = v2s.append(iClass.name).append("(").append("dir=").append(std::to_string(v2->direction)).append(")");
			lua_pushstring(lua, v2s.c_str());
			return 1;
		}
		int hitVectorX(lua_State* lua) {
			int nargs = lua_gettop(lua);
			if (nargs != 1)
				return luaL_error(lua, "Got %d arguments, expected 1: (self).", nargs);

			Physics2D::CollisionData* cd = getSelfAsUData<Physics2D::CollisionData>(lua, 1, iClass.metaName);
			lua_pop(lua, 2);

			lua_pushnumber(lua, cd->hitVector.x);

			return 1;
		}
		int hitVectorY(lua_State* lua) {
			int nargs = lua_gettop(lua);
			if (nargs != 1)
				return luaL_error(lua, "Got %d arguments, expected 1: (self).", nargs);

			Physics2D::CollisionData* cd = getSelfAsUData<Physics2D::CollisionData>(lua, 1, iClass.metaName);
			lua_pop(lua, 2);

			lua_pushnumber(lua, cd->hitVector.y);

			return 1;
		}
		int getDirection(lua_State* lua) {
			int nargs = lua_gettop(lua);
			if (nargs != 1)
				return luaL_error(lua, "Got %d arguments, expected 1: (self).", nargs);

			Physics2D::CollisionData* cd = getSelfAsUData<Physics2D::CollisionData>(lua, 1, iClass.metaName);
			lua_pop(lua, 2);

			lua_pushinteger(lua, cd->direction);

			return 1;
		}

		void registerCollision2DData(lua_State* lua) {
			using namespace ingenium_lua;

			iClass.addMetaMethod(lua, lua_func("__gc", free));
			iClass.addMetaMethod(lua, lua_func("__tostring", toString));

			iClass.addMetaMethod(lua, lua_func("getHitVectorX", hitVectorX));
			iClass.addMetaMethod(lua, lua_func("getHitVectorY", hitVectorY));
			iClass.addMetaMethod(lua, lua_func("getDirection", getDirection));

			iClass.registerClass(lua);
		}
	}
	namespace hitbox2D
	{
		ingenium_lua::LuaClass<Hitbox2D> iClass = ingenium_lua::LuaClass<Hitbox2D>("Hitbox2D");

		int free(lua_State* lua) {
			return ingenium_lua::free<Hitbox2D>(lua);
		}
		int toString(lua_State* lua) {
			Hitbox2D* v2 = getSelfAsUData<Hitbox2D>(lua, 1, iClass.metaName);
			std::string v2s("Hitbox2D(");
			switch (v2->type) {
			case Hitbox2D::TYPE_RECTANGLE:
				v2s = v2s.append("TYPE_RECTANGLE (").append(std::to_string(Hitbox2D::TYPE_RECTANGLE)).append("), pos=(").append(std::to_string(v2->rectPos().x)).append(", ").append(std::to_string(v2->rectPos().y)).
					append("), width=").append(std::to_string(v2->rectSize().x)).append(", height=").append(std::to_string(v2->rectSize().y));
				break;
			case Hitbox2D::TYPE_CIRCLE:
				v2s = v2s.append("TYPE_CIRCLE (").append(std::to_string(Hitbox2D::TYPE_CIRCLE)).append("), r=").append(std::to_string(v2->circleRadius()).append(", ").
					append("centre=(").append(std::to_string(v2->circleCentre().x))).append(", ").append(std::to_string(v2->circleCentre().y)).append(")");
				break;
			default:
				v2s = v2s.append("TYPE_UNKNOWN (").append(std::to_string(Hitbox2D::TYPE_UNDEFINED)).append(")");
				break;
			}
			v2s = v2s.append(")");
			lua_pushstring(lua, v2s.c_str());
			return 1;
		}
		int getType(lua_State* lua) {
			int nargs = lua_gettop(lua);
			if (nargs != 1)
				return luaL_error(lua, "Got %d arguments, expected 1: (self).", nargs);
			Hitbox2D* v2 = getSelfAsUData<Hitbox2D>(lua, 1, iClass.metaName);
			lua_pushnumber(lua, v2->type);
			return 1;
		}
		int sizeX(lua_State* lua) {
			int nargs = lua_gettop(lua);
			if (nargs != 1)
				return luaL_error(lua, "Got %d arguments, expected 1: (self).", nargs);

			Hitbox2D* v2 = getSelfAsUData<Hitbox2D>(lua, 1, iClass.metaName);
			switch (v2->type) {
			case Hitbox2D::TYPE_RECTANGLE:
				lua_pushnumber(lua, v2->rectSize().x);
				break;
			case Hitbox2D::TYPE_CIRCLE:
				lua_pushnumber(lua, v2->circleRadius());
				break;
			default:
				return luaL_error(lua, "No size for HITBOX2D_TYPE_UNDEFINED.");
			}
			return 1;
		}
		int sizeY(lua_State* lua) {
			int nargs = lua_gettop(lua);
			if (nargs != 1)
				return luaL_error(lua, "Got %d arguments, expected 1: (self).", nargs);
			Hitbox2D* v2 = getSelfAsUData<Hitbox2D>(lua, 1, iClass.metaName);
			switch (v2->type) {
			case Hitbox2D::TYPE_RECTANGLE:
				lua_pushnumber(lua, v2->rectSize().y);
				break;
			case Hitbox2D::TYPE_CIRCLE:
				lua_pushnumber(lua, v2->circleRadius());
				break;
			default:
				return luaL_error(lua, "No size for HITBOX2D_TYPE_UNDEFINED.");
			}
			return 1;
		}
		int xPosition(lua_State* lua) {
			int nargs = lua_gettop(lua);
			if (nargs != 1)
				return luaL_error(lua, "Got %d arguments, expected 1: (self).", nargs);
			Hitbox2D* v2 = getSelfAsUData<Hitbox2D>(lua, 1, iClass.metaName);
			switch (v2->type) {
			case Hitbox2D::TYPE_RECTANGLE:
				lua_pushnumber(lua, v2->rectPos().x);
				break;
			case Hitbox2D::TYPE_CIRCLE:
				lua_pushnumber(lua, v2->circleCentre().x);
				break;
			default:
				return luaL_error(lua, "No position for HITBOX2D_TYPE_UNDEFINED.");
			}
			return 1;
		}
		int yPosition(lua_State* lua) {
			int nargs = lua_gettop(lua);
			if (nargs != 1)
				return luaL_error(lua, "Got %d arguments, expected 1: (self).", nargs);
			Hitbox2D* v2 = getSelfAsUData<Hitbox2D>(lua, 1, iClass.metaName);
			switch (v2->type) {
			case Hitbox2D::TYPE_RECTANGLE:
				lua_pushnumber(lua, v2->rectPos().y);
				break;
			case Hitbox2D::TYPE_CIRCLE:
				lua_pushnumber(lua, v2->circleCentre().y);
				break;
			default:
				return luaL_error(lua, "No position for HITBOX2D_TYPE_UNDEFINED.");
			}
			return 1;
		}
		int radius(lua_State* lua) {
			int nargs = lua_gettop(lua);
			if (nargs != 1)
				return luaL_error(lua, "Got %d arguments, expected 1: (self).", nargs);

			Hitbox2D* v2 = getSelfAsUData<Hitbox2D>(lua, 1, iClass.metaName);
			lua_pop(lua, 2);

			switch (v2->type) {
			case Hitbox2D::TYPE_CIRCLE:
				lua_pushnumber(lua, v2->circleRadius());
				break;
			case Hitbox2D::TYPE_RECTANGLE:
				return luaL_error(lua, "No radius for HITBOX2D_TYPE_RECTANGLE.");
			default:
				return luaL_error(lua, "No radius for HITBOX2D_TYPE_UNDEFINED.");
			}

			return 1;
		}
		int colliding(lua_State* lua) {
			int nargs = lua_gettop(lua);
			if (nargs != 2)
				return luaL_error(lua, "Got %d arguments, expected 2: (Hitbox2D, Hitbox2D).", nargs);

			Hitbox2D* hb2 = getSelfAsUData<Hitbox2D>(lua, 2, iClass.metaName);
			lua_pop(lua, 2);
			Hitbox2D* hb1 = getSelfAsUData<Hitbox2D>(lua, 1, iClass.metaName);
			lua_pop(lua, 2);

			Physics2D::CollisionData cd = Physics2D::getPhysics2D()->colliding(*hb1, *hb2);

			lua_pushboolean(lua, !(cd.direction == Physics2D::COLLISION_NONE));

			return 1;
		}
		int getCollisionData(lua_State* lua) {
			int nargs = lua_gettop(lua);
			if (nargs != 2)
				return luaL_error(lua, "Got %d arguments, expected 2: (Hitbox2D, Hitbox2D).", nargs);

			Hitbox2D* hb2 = getSelfAsUData<Hitbox2D>(lua, 2, iClass.metaName);
			lua_pop(lua, 2);
			Hitbox2D* hb1 = getSelfAsUData<Hitbox2D>(lua, 1, iClass.metaName);
			lua_pop(lua, 2);

			Physics2D::CollisionData* cd = new Physics2D::CollisionData(-1, Vector2());
			*cd = Physics2D::getPhysics2D()->colliding(*hb1, *hb2);

			lua_getglobal(lua, collision_data_2D::iClass.name);
			collision_data_2D::iClass.createInstance(lua, cd);

			return 1;
		}

		int newHitbox2D(lua_State* lua) {
			int nargs = lua_gettop(lua);
			if (nargs != 1 && nargs != 3)
				return luaL_error(lua, "Got %d arguments, expected 0 or 2: (Vector2, Vector2) or (number, Vector2).", nargs);

			Hitbox2D* hit = nullptr;

			if (nargs == 1) {
				hit = Hitbox2D::createUndefinedHitboxPtr();
			}
			else {
				switch (lua_type(lua, 2)) {
				case LUA_TNUMBER:
				{
					Vector2* v2 = getSelfAsUData<Vector2>(lua, 3, vec2::iClass.metaName);
					lua_pop(lua, 2);
					float diameter = luaL_checknumber(lua, 2);
					lua_pop(lua, 1);
					hit = Hitbox2D::createCircleHitboxPtr(diameter, *v2);
				}
				break;
				default:
					Vector2* v2 = getSelfAsUData<Vector2>(lua, 3, iClass.metaName);
					lua_pop(lua, 2);
					Vector2* v1 = getSelfAsUData<Vector2>(lua, 2, iClass.metaName);
					lua_pop(lua, 2);
					hit = Hitbox2D::createRectHitboxPtr(*v1, *v2);
					break;
				}
			}

			iClass.createInstance(lua, hit);

			return 1;
		}

		void registerHitbox2D(lua_State* lua) {
			using namespace ingenium_lua;
			iClass.addMetaMethod(lua, lua_func("new", newHitbox2D));
			iClass.addMetaMethod(lua, lua_func("newUndefined", newHitbox2D));
			iClass.addMetaMethod(lua, lua_func("newCircle", newHitbox2D));
			iClass.addMetaMethod(lua, lua_func("newRect", newHitbox2D));
			iClass.addMetaMethod(lua, lua_func("__gc", free));
			iClass.addMetaMethod(lua, lua_func("__tostring", toString));

			iClass.addMethod(lua, lua_func("type", getType));
			iClass.addMethod(lua, lua_func("yPos", yPosition));
			iClass.addMethod(lua, lua_func("xPos", xPosition));
			iClass.addMethod(lua, lua_func("radius", radius));
			iClass.addMethod(lua, lua_func("width", sizeX));
			iClass.addMethod(lua, lua_func("height", sizeY));
			iClass.addMethod(lua, lua_func("colliding", colliding));
			iClass.addMethod(lua, lua_func("getCollision", getCollisionData));

			iClass.registerClass(lua);

			lua_pushnumber(lua, Hitbox2D::TYPE_UNDEFINED);
			lua_setglobal(lua, "HITBOX2D_TYPE_UNDEFINED");
			lua_pushnumber(lua, Hitbox2D::TYPE_CIRCLE);
			lua_setglobal(lua, "HITBOX2D_TYPE_CIRCLE");
			lua_pushnumber(lua, Hitbox2D::TYPE_RECTANGLE);
			lua_setglobal(lua, "HITBOX2D_TYPE_RECTANGLE");
		}
#ifdef CONSTRUCTOR_METHOD_NAME
#undef CONSTRUCTOR_METHOD_NAME
#endif
	}
	namespace sprite_frame_data {
		ingenium_lua::LuaClass<Sprite::FrameData> iClass = ingenium_lua::LuaClass<Sprite::FrameData>("SpriteFrameData");

		int free(lua_State* lua) {
			return ingenium_lua::free<Sprite::FrameData>(lua);
		}
		int toString(lua_State* lua) {
			Sprite::FrameData* fd = getSelfAsUData<Sprite::FrameData>(lua, 1, iClass.metaName);
			lua_pop(lua, 2);
			std::string str("SFrameData(");
			str = str.append("frames=").append(std::to_string(fd->frames)).append(", fdelta=").append(std::to_string(fd->frameTime)).append("s").append(")");
			lua_pushstring(lua, str.c_str());
			return 1;
		}
		int setStartFrame(lua_State* lua) {
			int nargs = lua_gettop(lua);
			if (nargs != 3)
				return luaL_error(lua, "Got %d arguments, expected 3: (self, number, number).", nargs);

			int newX = lua_tointeger(lua, 2);
			int newY = lua_tointeger(lua, 3);
			lua_pop(lua, 2);

			Sprite::FrameData* fd = getSelfAsUData<Sprite::FrameData>(lua, 1, iClass.metaName);
			lua_pop(lua, 2);

			fd->startFrame[0] = newX;
			fd->startFrame[1] = newY;

			return 0;
		}

		int newFrameData(lua_State* lua) {

			// frames, spritesheetDirection, frameWidth, frameHeight, frameDelta

			int nargs = lua_gettop(lua);
			if (nargs != 6)
				return luaL_error(lua, "Got %d arguments, expected 6.", nargs);

			Sprite::FrameData* fd = nullptr;

			fd = new Sprite::FrameData();
			fd->frameTime = lua_tonumber(lua, 6);
			fd->frameHeight = lua_tonumber(lua, 5);
			fd->frameWidth = lua_tonumber(lua, 4);
			fd->spriteSheetDirection = lua_toboolean(lua, 3);
			fd->frames = lua_tointeger(lua, 2);
			lua_pop(lua, 5);

			iClass.createInstance(lua, fd);

			return 1;
		}

		void registerSpriteFrameData(lua_State* lua) {
			using namespace ingenium_lua;

			iClass.addMetaMethod(lua, lua_func("new", newFrameData));
			iClass.addMetaMethod(lua, lua_func("__gc", free));
			iClass.addMetaMethod(lua, lua_func("__tostring", toString));

			iClass.addMethod(lua, lua_func("setStartFrame", setStartFrame));

			iClass.registerClass(lua);
		}
	};
	namespace sprite
	{
		ingenium_lua::LuaClass<Sprite> iClass = ingenium_lua::LuaClass<Sprite>("Sprite");

		int free(lua_State* lua) {
			return ingenium_lua::free<Sprite>(lua);
		}
		int toString(lua_State* lua) {
			Sprite* sp = getSelfAsUData<Sprite>(lua, 1, iClass.metaName);
			lua_pop(lua, 2);
			std::string str("sprite(");
			str = str.append(sp->name).append(")");
			lua_pushstring(lua, str.c_str());
			return 1;
		}
		int render(lua_State* lua) {
			int nargs = lua_gettop(lua);
			if (nargs != 1)
				return luaL_error(lua, "Got %d arguments, expected 1: (self).", nargs);
			Sprite* sp = getSelfAsUData<Sprite>(lua, 1, iClass.metaName);
			Ingenium2D::getEngine()->drwn->addToRenderQueue(sp, Direct2DWindow::RenderLinkedList::TYPE_RENDER_SPRITE);
			return 0;
		}
		int setHitbox2D(lua_State* lua) {
			int nargs = lua_gettop(lua);
			if (nargs != 2)
				return luaL_error(lua, "Got %d arguments, expected 2: (self, Hitbox2D).", nargs);

			Hitbox2D* h2d = getSelfAsUData<Hitbox2D>(lua, 2, hitbox2D::iClass.metaName);
			lua_pop(lua, 2);
			Sprite* sp = getSelfAsUData<Sprite>(lua, 1, iClass.metaName);
			lua_pop(lua, 2);

			sp->hitbox2D = *h2d;
			return 0;
		}
		int setX(lua_State* lua) {
			int nargs = lua_gettop(lua);
			if (nargs != 2)
				return luaL_error(lua, "Got %d arguments, expected 2: (self, number).", nargs);

			float xV = lua_tonumber(lua, 2);
			lua_pop(lua, 1);
			Sprite* sp = getSelfAsUData<Sprite>(lua, 1, iClass.metaName);
			lua_pop(lua, 2);

			sp->setX(xV);
			return 0;
		}
		int setY(lua_State* lua) {
			int nargs = lua_gettop(lua);
			if (nargs != 2)
				return luaL_error(lua, "Got %d arguments, expected 2: (self, number).", nargs);

			float yV = lua_tonumber(lua, 2);
			lua_pop(lua, 1);
			Sprite* sp = getSelfAsUData<Sprite>(lua, 1, iClass.metaName);
			lua_pop(lua, 2);

			sp->setY(yV);
			return 0;
		}
		int setXY(lua_State* lua) {
			int nargs = lua_gettop(lua);
			if (nargs != 3)
				return luaL_error(lua, "Got %d arguments, expected 3: (self, number, number).", nargs);

			float yV = lua_tonumber(lua, 3);
			lua_pop(lua, 1);
			float xV = lua_tonumber(lua, 2);
			lua_pop(lua, 1);
			Sprite* sp = getSelfAsUData<Sprite>(lua, 1, iClass.metaName);
			lua_pop(lua, 2);

			sp->setXY(xV, yV);
			return 0;
		}
		int getX(lua_State* lua) {
			int nargs = lua_gettop(lua);
			if (nargs != 1)
				return luaL_error(lua, "Got %d arguments, expected 1: (self).", nargs);

			Sprite* sp = getSelfAsUData<Sprite>(lua, 1, iClass.metaName);
			lua_pop(lua, 2);

			lua_pushnumber(lua, sp->position.x);
			return 1;
		}
		int getY(lua_State* lua) {
			int nargs = lua_gettop(lua);
			if (nargs != 1)
				return luaL_error(lua, "Got %d arguments, expected 1: (self).", nargs);

			Sprite* sp = getSelfAsUData<Sprite>(lua, 1, iClass.metaName);
			lua_pop(lua, 2);

			lua_pushnumber(lua, sp->position.y);
			return 1;
		}
		int addX(lua_State* lua) {
			int nargs = lua_gettop(lua);
			if (nargs != 2)
				return luaL_error(lua, "Got %d arguments, expected 2: (self, number).", nargs);

			float xV = lua_tonumber(lua, 2);
			lua_pop(lua, 1);
			Sprite* sp = getSelfAsUData<Sprite>(lua, 1, iClass.metaName);
			lua_pop(lua, 2);

			sp->addXY(xV, 0);
			return 0;
		}
		int addY(lua_State* lua) {
			int nargs = lua_gettop(lua);
			if (nargs != 2)
				return luaL_error(lua, "Got %d arguments, expected 2: (self, number).", nargs);

			float yV = lua_tonumber(lua, 2);
			lua_pop(lua, 1);
			Sprite* sp = getSelfAsUData<Sprite>(lua, 1, iClass.metaName);
			lua_pop(lua, 2);

			sp->addXY(0, yV);
			return 0;
		}
		int addXY(lua_State* lua) {
			int nargs = lua_gettop(lua);
			if (nargs != 3)
				return luaL_error(lua, "Got %d arguments, expected 3: (self, number, number).", nargs);

			float yV = lua_tonumber(lua, 3);
			lua_pop(lua, 1);
			float xV = lua_tonumber(lua, 2);
			lua_pop(lua, 1);
			Sprite* sp = getSelfAsUData<Sprite>(lua, 1, iClass.metaName);
			lua_pop(lua, 2);

			sp->addXY(xV, yV);
			return 0;
		}
		int getHitbox(lua_State* lua) {
			int nargs = lua_gettop(lua);
			if (nargs != 1)
				return luaL_error(lua, "Got %d arguments, expected 1: (self).", nargs);

			Sprite* sp = getSelfAsUData<Sprite>(lua, 1, iClass.metaName);
			lua_pop(lua, 2);

			Hitbox2D* hb2d = Hitbox2D::createUndefinedHitboxPtr();
			*hb2d = sp->hitbox2D;

			lua_getglobal(lua, hitbox2D::iClass.name);
			hitbox2D::iClass.createInstance(lua, hb2d);
			lua_remove(lua, -2);
			return 1;
		}
		int setSize(lua_State* lua) {
			int nargs = lua_gettop(lua);
			if (nargs != 3)
				return luaL_error(lua, "Got %d arguments, expected 3: (self, number, number).", nargs);

			float width = lua_tonumber(lua, 2);
			float height = lua_tonumber(lua, 3);
			lua_pop(lua, 2);

			Sprite* sp = getSelfAsUData<Sprite>(lua, 1, iClass.metaName);
			lua_pop(lua, 2);
			sp->size.x = (width);
			sp->size.y = (height);
			return 0;
		}
		int calculateFrame(lua_State* lua) {
			int nargs = lua_gettop(lua);
			if (nargs != 1)
				return luaL_error(lua, "Got %d arguments, expected 1: (self).", nargs);

			Sprite* sp = getSelfAsUData<Sprite>(lua, 1, iClass.metaName);
			lua_pop(lua, 2);
			sp->frameData.calculateFrame();
			return 0;
		}
		int setRotation(lua_State* lua) {
			int nargs = lua_gettop(lua);
			if (nargs != 4)
				return luaL_error(lua, "Got %d arguments, expected 4: (self, number, number, number).", nargs);

			float xRot = lua_tonumber(lua, 2), yRot = lua_tonumber(lua, 3), zRot = lua_tonumber(lua, 4);
			lua_pop(lua, 3);
			Sprite* sp = getSelfAsUData<Sprite>(lua, 1, iClass.metaName);
			lua_pop(lua, 2);

			sp->rotation.x = xRot;
			sp->rotation.y = yRot;
			sp->rotation.z = zRot;

			return 0;
		}
		int addRotation(lua_State* lua) {
			int nargs = lua_gettop(lua);
			if (nargs != 4)
				return luaL_error(lua, "Got %d arguments, expected 4: (self, number, number, number).", nargs);

			float xRot = lua_tonumber(lua, 2), yRot = lua_tonumber(lua, 3), zRot = lua_tonumber(lua, 4);
			lua_pop(lua, 3);
			Sprite* sp = getSelfAsUData<Sprite>(lua, 1, iClass.metaName);
			lua_pop(lua, 2);

			sp->rotation.x += xRot;
			sp->rotation.y += yRot;
			sp->rotation.z += zRot;

			return 0;
		}
		int setRotationCenter(lua_State* lua) {
			int nargs = lua_gettop(lua);
			if (nargs != 3)
				return luaL_error(lua, "Got %d arguments, expected 3: (self, number, number).", nargs);

			float xRot = lua_tonumber(lua, 2), yRot = lua_tonumber(lua, 3);
			lua_pop(lua, 2);
			Sprite* sp = getSelfAsUData<Sprite>(lua, 1, iClass.metaName);
			lua_pop(lua, 2);

			sp->rotation.centre[0] = xRot;
			sp->rotation.centre[1] = yRot;

			return 0;
		}
		int toggleHitboxRender(lua_State* lua) {
			int nargs = lua_gettop(lua);
			if (nargs != 2)
				return luaL_error(lua, "Got %d arguments, expected 3: (self, boolean).", nargs);

			bool renderHitboxFlag = lua_toboolean(lua, 2);
			lua_pop(lua, 1);
			Sprite* sp = getSelfAsUData<Sprite>(lua, 1, iClass.metaName);
			lua_pop(lua, 2);
			sp->renderHitbox = renderHitboxFlag;
			return 0;
		}
		int colliding(lua_State* lua) {
			int nargs = lua_gettop(lua);
			if (nargs != 2)
				return luaL_error(lua, "Got %d arguments, expected 2: (Sprite, Sprite).", nargs);

			Sprite* hb2 = getSelfAsUData<Sprite>(lua, 2, iClass.metaName);
			lua_pop(lua, 2);
			Sprite* hb1 = getSelfAsUData<Sprite>(lua, 1, iClass.metaName);
			lua_pop(lua, 2);

			Physics2D::CollisionData cd = Physics2D::getPhysics2D()->colliding(hb1->hitbox2D, hb2->hitbox2D);

			lua_pushboolean(lua, !(cd.direction == Physics2D::COLLISION_NONE));

			return 1;
		}
		int getCollisionData(lua_State* lua) {
			int nargs = lua_gettop(lua);
			if (nargs != 2)
				return luaL_error(lua, "Got %d arguments, expected 2: (Hitbox2D, Hitbox2D).", nargs);

			Sprite* hb2 = getSelfAsUData<Sprite>(lua, 2, iClass.metaName);
			lua_pop(lua, 2);
			Sprite* hb1 = getSelfAsUData<Sprite>(lua, 1, iClass.metaName);
			lua_pop(lua, 2);

			Physics2D::CollisionData* cd = new Physics2D::CollisionData(-1, Vector2());
			*cd = Physics2D::getPhysics2D()->colliding(hb1->hitbox2D, hb2->hitbox2D);

			lua_getglobal(lua, collision_data_2D::iClass.name);
			collision_data_2D::iClass.createInstance(lua, cd);

			return 1;
		}

		int newSprite(lua_State* lua) {
			// name, path --- 3
			// name, path, frameData --- 4
			// name, path, frameData, callback_function, callback_function_frame --- 6

			int nargs = lua_gettop(lua);
			if (nargs != 3 && nargs != 4 && nargs != 6)
				return luaL_error(lua, "Got %d arguments, expected 3, 4, or 6.", nargs);

			Sprite* sp = nullptr;

			switch (nargs) {
			case 3:
			{
				Sprite::FrameData fd = Sprite::FrameData();
				const char* name = lua_tostring(lua, 2);
				std::wstring path = string_conversion::widen(lua_tostring(lua, 3));
				lua_pop(lua, 2);

				sp = Sprite::createSpriteFromName(name, path.c_str(), fd, Ingenium2D::getEngine()->drwn->pRT);
				break;
			}
			case 4:
			{
				Sprite::FrameData* fd = getSelfAsUData<Sprite::FrameData>(lua, 4, sprite_frame_data::iClass.metaName);
				lua_pop(lua, 2);
				const char* name = lua_tostring(lua, 2);
				std::wstring path = string_conversion::widen(lua_tostring(lua, 3));
				lua_pop(lua, 2);
				sp = Sprite::createSpriteFromName(name, path.c_str(), *fd, Ingenium2D::getEngine()->drwn->pRT);
				break;
			}
			default:
				break;
			}

			iClass.createInstance(lua, sp);

			return 1;
		}

		void registerSprite(lua_State* lua) {
			using namespace ingenium_lua;

			iClass.addMetaMethod(lua, lua_func("new", newSprite));
			iClass.addMetaMethod(lua, lua_func("__gc", free));
			iClass.addMetaMethod(lua, lua_func("__tostring", toString));

			iClass.addMethod(lua, lua_func("render", render));
			iClass.addMethod(lua, lua_func("setHitbox2D", setHitbox2D));
			iClass.addMethod(lua, lua_func("setX", setX));
			iClass.addMethod(lua, lua_func("setY", setY));
			iClass.addMethod(lua, lua_func("setXY", setXY));
			iClass.addMethod(lua, lua_func("getX", getX));
			iClass.addMethod(lua, lua_func("getY", getY));
			iClass.addMethod(lua, lua_func("addX", addX));
			iClass.addMethod(lua, lua_func("addY", addY));
			iClass.addMethod(lua, lua_func("addXY", addXY));
			iClass.addMethod(lua, lua_func("getHitbox2D", getHitbox));
			iClass.addMethod(lua, lua_func("setSize", setSize));
			iClass.addMethod(lua, lua_func("calculateFrame", calculateFrame));
			iClass.addMethod(lua, lua_func("setRotation", setRotation));
			iClass.addMethod(lua, lua_func("addRotation", addRotation));
			iClass.addMethod(lua, lua_func("setRotationCenter", setRotationCenter));
			iClass.addMethod(lua, lua_func("renderHitbox", toggleHitboxRender));
			iClass.addMethod(lua, lua_func("colliding", colliding));
			iClass.addMethod(lua, lua_func("getCollision", getCollisionData));

			iClass.registerClass(lua);
		}
	}
	namespace line {
		ingenium_lua::LuaClass<Line> iClass = ingenium_lua::LuaClass<Line>("Line");

		int free(lua_State* lua) {
			return ingenium_lua::free<Line>(lua);
		}
		int render(lua_State* lua) {
			int nargs = lua_gettop(lua);
			if (nargs != 1)
				return luaL_error(lua, "Got %d arguments, expected 1: (self).", nargs);
			Line* sp = getSelfAsUData<Line>(lua, 1, iClass.metaName);
			Ingenium2D::getEngine()->drwn->addToRenderQueue(sp, Direct2DWindow::RenderLinkedList::TYPE_RENDER_ID2D1LINE);
			return 0;
		}

		int newLine(lua_State* lua) {
			// start, end -- 3
			// start, end, size -- 4
			// start, end, size, colour, transparecy -- 6
			
			int nargs = lua_gettop(lua);
			if ((nargs < 3 || nargs > 6))
				return luaL_error(lua, "Got %d arguments, expected 3 - 6", nargs);

			Line* line = nullptr;

			switch (nargs) {
			case 3:
			{
				Vector2* end = getSelfAsUData<Vector2>(lua, 3, vec2::iClass.metaName);
				lua_pop(lua, 2);
				Vector2* start = getSelfAsUData<Vector2>(lua, 2, vec2::iClass.metaName);
				line = new Line(*start, *end);
				lua_pop(lua, 2);
			}
			break;
			case 4:
			{
				float size = lua_tonumber(lua, 4);
				lua_pop(lua, 1);
				Vector2* end = getSelfAsUData<Vector2>(lua, 3, vec2::iClass.metaName);
				lua_pop(lua, 2);
				Vector2* start = getSelfAsUData<Vector2>(lua, 2, vec2::iClass.metaName);
				line = new Line(*start, *end);
				line->lineSize = size;
				lua_pop(lua, 2);
			}
			break;
			case 5: case 6:
			{
				float alpha = nargs == 6 ? lua_tonumber(lua, 6) : 1;
				unsigned long colour = lua_tointeger(lua, 5);
				float size = lua_tonumber(lua, 4);
				lua_pop(lua, nargs == 6 ? 3 : 2);
				Vector2* end = getSelfAsUData<Vector2>(lua, 3, vec2::iClass.metaName);
				lua_pop(lua, 2);
				Vector2* start = getSelfAsUData<Vector2>(lua, 2, vec2::iClass.metaName);
				line = new Line(*start, *end);
				line->lineSize = size;
				Ingenium2D::getEngine()->drwn->pRT->CreateSolidColorBrush(D2D1::ColorF(colour, alpha), &line->brush);
				lua_pop(lua, 2);
			}
			break;
			default:
				break;
			}

			iClass.createInstance(lua, line);

			return 1;
		}

		void registerLine(lua_State* lua) {
			using namespace ingenium_lua;

			iClass.addMetaMethod(lua, lua_func("new", newLine));
			iClass.addMetaMethod(lua, lua_func("__gc", free));

			iClass.addMethod(lua, lua_func("render", render));

			iClass.registerClass(lua);
		}
	}
}

void Ingenium2D::loadToLua()
{
	if (!ingenium_lua::state) {
		ingenium_lua::initLua();
	}
	lua_funcs_2D::d2d::registerDRWN(ingenium_lua::state);
	lua_funcs_2D::time::registerTime(ingenium_lua::state);
	lua_funcs_2D::vec2::registerVector2(ingenium_lua::state);
	lua_funcs_2D::collision_data_2D::registerCollision2DData(ingenium_lua::state);
	lua_funcs_2D::hitbox2D::registerHitbox2D(ingenium_lua::state);
	lua_funcs_2D::sprite_frame_data::registerSpriteFrameData(ingenium_lua::state);
	lua_funcs_2D::sprite::registerSprite(ingenium_lua::state);
	lua_funcs_2D::line::registerLine(ingenium_lua::state);
}
#endif