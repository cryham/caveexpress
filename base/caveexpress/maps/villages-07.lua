function getName()
	return "Village7 New"
end

function onMapLoaded()
end

function initMap()
	-- get the current map context
	local map = Map.get()

	math.randomseed(os.time())

	for y = 0,15
	do
	for x = 0,15
	do
		map:addTile("tile-background-0" .. tostring( math.random(1, 4) ), x, y)
	end
	end

	map:setSetting("width", "16")
	map:setSetting("height", "16")
	map:setSetting("fishnpc", "false")
	map:setSetting("flyingnpc", "false")
	map:setSetting("gravity", "9.81")
	map:setSetting("introwindow", "")
	map:setSetting("npcs", "4")
	map:setSetting("npctransfercount", "1")  -- 1
	map:setSetting("packagetransfercount", "2")
	map:addStartPosition("4.000000", "3.000000")
	map:setSetting("points", "100")
	map:setSetting("referencetime", "30")
	map:setSetting("sideborderfail", "false")
	map:setSetting("theme", "rock")
	map:setSetting("tutorial", "false")

	map:setSetting("waterheight", "0.9")
	map:setSetting("waterchangespeed", "0")
	map:setSetting("waterrisingdelay", "0")
	map:setSetting("waterchangespeed", "0.000000")
	map:setSetting("waterfallingdelay", "4000")
	map:setSetting("wind", "0")

end
