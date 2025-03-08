function getName()
	return "Jungle8 Zig zag"
end

function onMapLoaded()
end

function initMap()
	-- get the current map context
	local map = Map.get()

	math.randomseed(os.time())

	for y = 0,13
	do
	for x = 0,13
	do
		map:addTile("tile-background-jungle-0" .. tostring( math.random(1, 4) ), x, y)
	end
	end

	map:setSetting("width", "14")
	map:setSetting("height", "14")
	map:setSetting("fishnpc", "false")
	map:setSetting("flyingnpc", "false")
	map:setSetting("gravity", "9.81")
	map:setSetting("introwindow", "")
	map:setSetting("npcs", "4")
	map:setSetting("npctransfercount", "3")  -- 1
	map:setSetting("packagetransfercount", "0")
	map:addStartPosition("4.000000", "3.000000")
	map:setSetting("points", "100")
	map:setSetting("referencetime", "30")
	map:setSetting("sideborderfail", "false")
	map:setSetting("theme", "jungle")
	map:setSetting("tutorial", "false")

	map:setSetting("waterheight", "0.99")
	map:setSetting("waterchangespeed", "0")
	map:setSetting("waterrisingdelay", "0")
	map:setSetting("waterchangespeed", "0.000000")
	map:setSetting("waterfallingdelay", "4000")
	map:setSetting("wind", "0")

end
