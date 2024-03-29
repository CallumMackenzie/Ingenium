#pragma once

#include "Renderable.h"
#include "Physics2D.h"

namespace ingenium2D {

	class Sprite : public Renderable<ID2D1Bitmap>
	{
	public:
		static Sprite* createSpriteFromName(const char* name_, LPCWSTR bitmapPath, FrameData fd, ID2D1RenderTarget* pRT, Hitbox2D hb2d = Hitbox2D::createUndefinedHitbox(), Vector2 pos = Vector2(), Rotation rot = Rotation());

		Sprite(const char* name_, ID2D1Bitmap* bitmap_);
		Sprite(const char* name_, ID2D1Bitmap* bitmap_, Hitbox2D hb2d);
		Sprite(const char* name_, ID2D1Bitmap* bitmap_, Hitbox2D hb2d, Vector2 pos);
		Sprite(const char* name_, ID2D1Bitmap* bitmap_, Hitbox2D hb2d, Vector2 pos, Rotation rot);
		~Sprite();

		Hitbox2D hitbox2D = Hitbox2D::createUndefinedHitbox();
		Vector2 size = Vector2(1, 1);
		Vector2 velocity = Vector2();
		ID2D1Bitmap* bitmap;

#if defined(_DEBUG)
		bool renderHitbox = true;
#else
		bool renderHitbox = false;
#endif

		void setXY(float x, float y);
		void setXY(Vector2 pos);
		void addXY(float x, float y);
		void addXY(Vector2 pos);
		void subtractXY(float x, float y);
		void subtractXY(Vector2 pos);
		void setY(float y);
		void setX(float x);
	};
};