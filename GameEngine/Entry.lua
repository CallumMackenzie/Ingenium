
v2 = Vector2:new(30, 212)

function printMethods (o)
	D2D.write("Printing table methods:\n")
	for key,value in pairs(o) do
		D2D.write("found member " .. key .. "\n");
	end
end

function init ()
	D2D.show()
	local v1 = Vector2:new(10, 2)
	local derived = Vector2:new(30, 1)

	local h2d = Hitbox2D:new(47, Vector2:new(50, 50))

	sprt = Sprite:new("SPRITE_1", "./Planet1.png")
	sprt:setSize(100, 100)
	sprt:setHitbox2D(h2d)
	sprt:setXY(500, 100)
	sprt:setRotationCenter(50, 50)
	sprt:renderHitbox(false)

	runner = Sprite:new("Runner", "./RunSpriteSheet.png", SpriteFrameData:new(12, false, 40, 64, 0.1))
	runner:setHitbox2D(h2d)
	runner:setSize(80, 128)
	runner:setXY(200, 200)
	runner:setRotationCenter(40, 64)

	function derived:someMethod (slf)
		return "wow it works\n"
	end

	v1:setX(100)
	v1:setY(5)
	v1:getX()
	v1:getY()
	v1:normalize()

	spriteHitbox = sprt:getHitbox2D()

	D2D.write(
	"Info from methods: " .. 
	derived:someMethod() .. 
	"v1: " .. tostring(v1) .. 
	"\nv2: " .. tostring(v2) .. 
	"\nadded: " .. tostring(v1 + v2) .. 
	"\nv1: " .. tostring(v1) .. 
	"\nv2: " .. tostring(v2) .. 
	"\nv1 magnitude: " .. v1:magnitude() ..
	"\nh2d: " .. tostring(h2d) .. 
	"\nh2d type: " .. h2d:type() .. 
	"\nh2d size: (" .. h2d:width() .. ", " .. h2d:height() .. ")" ..
	"\nsprt: " .. tostring(sprt) .. 
	"\nsprt pos(" .. sprt:getX() .. ", " .. sprt:getY() .. ")" ..
	"\nsprt hitbox method call: " .. tostring(spriteHitbox) .. 
	"\nrunner: " .. tostring(runner) .. "\n")
	D2D.setClearColour(0xffffff)
	Time.setFPS(200)
	-- D2D.setFullscreen()
end

addValue = Vector2:new()
deceleration = Vector2:new(0.9, 0.9)

function onFixedUpdate() 
	addValue = addValue * deceleration
end

accel = 0.5

function onUpdate () 
	if D2D.keyPressed(87) then
		addValue:setY(-accel)
	end
	if D2D.keyPressed(83) then
		addValue:setY(accel)
	end
	if D2D.keyPressed(68) then
		addValue:setX(accel)
	end
	if D2D.keyPressed(65) then
		addValue:setX(-accel)
	end

	sprt:addXY(addValue:getX() * Time.deltaTime(), addValue:getY() * Time.deltaTime())
	local coll = sprt:getCollision(runner)

	if coll:getDirection() > 0 then
		sprt:addXY(coll:getHitVectorX() / 2, coll:getHitVectorY() / 2)
		runner:addXY(-coll:getHitVectorX() / 2, -coll:getHitVectorY() / 2)
	end

	sprt:render()
	runner:calculateFrame()
	sprt:addRotation(0, 0, 0.1 * Time.deltaTime())
	runner:render()
	D2D.render(false)
end

function onClose()

end