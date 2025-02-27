function getName()
	return "Taxi 04"
end

function onMapLoaded()
end

function initMap()
	-- get the current map context
	local map = Map.get()
	map:addTile("tile-background-ice-06", 0.000000, 0.000000)
	map:addTile("tile-background-03", 0.000000, 1.000000)
	map:addTile("tile-background-02", 0.000000, 2.000000)
	map:addTile("tile-background-03", 0.000000, 3.000000)
	map:addTile("tile-background-01", 0.000000, 4.000000)
	map:addTile("tile-background-01", 0.000000, 5.000000)
	map:addTile("tile-background-ice-05", 0.000000, 6.000000)
	map:addTile("tile-background-03", 0.000000, 7.000000)
	map:addTile("tile-background-ice-05", 0.000000, 8.000000)
	map:addTile("tile-background-big-01", 0.000000, 9.000000)
	map:addTile("tile-background-02", 0.000000, 11.000000)
	map:addTile("tile-background-04", 1.000000, 0.000000)
	map:addTile("tile-background-big-01", 1.000000, 1.000000)
	map:addTile("tile-background-04", 1.000000, 3.000000)
	map:addTile("tile-background-big-01", 1.000000, 4.000000)
	map:addTile("tile-background-02", 1.000000, 6.000000)
	map:addTile("tile-background-03", 1.000000, 7.000000)
	map:addTile("tile-background-window-02", 1.000000, 8.000000)
	map:addTile("tile-background-02", 1.000000, 11.000000)
	map:addTile("tile-background-ice-06", 2.000000, 0.000000)
	map:addTile("tile-background-02", 2.000000, 3.000000)
	map:addTile("tile-background-big-01", 2.000000, 6.000000)
	map:addTile("tile-ground-ledge-left-01", 2.000000, 9.000000)
	map:addTile("tile-background-02", 2.000000, 10.000000)
	map:addTile("tile-background-02", 2.000000, 11.000000)
	map:addTile("tile-background-02", 3.000000, 0.000000)
	map:addTile("tile-ground-01", 3.000000, 2.000000)
	map:addTile("tile-rock-03", 3.000000, 3.000000)
	map:addTile("tile-ground-ledge-left-01", 3.000000, 4.000000)
	map:addTile("tile-background-02", 3.000000, 5.000000)
	map:addTile("tile-background-cave-art-01", 3.000000, 8.000000)
	map:addTile("tile-ground-02", 3.000000, 9.000000)
	map:addTile("tile-rock-03", 3.000000, 10.000000)
	map:addTile("tile-rock-03", 3.000000, 11.000000)
	map:addTile("tile-background-03", 4.000000, 0.000000)
	map:addTile("tile-background-window-02", 4.000000, 1.000000)
	map:addTile("tile-ground-03", 4.000000, 2.000000)
	map:addTile("tile-rock-big-01", 4.000000, 3.000000)
	map:addTile("tile-background-big-01", 4.000000, 5.000000)
	map:addTile("tile-ground-ledge-left-02", 4.000000, 7.000000)
	map:addTile("tile-background-02", 4.000000, 8.000000)
	map:addTile("tile-ground-01", 4.000000, 9.000000)
	map:addTile("tile-rock-03", 4.000000, 10.000000)
	map:addTile("tile-rock-03", 4.000000, 11.000000)
	map:addTile("tile-background-04", 5.000000, 0.000000)
	map:addTile("tile-background-01", 5.000000, 1.000000)
	map:addTile("tile-ground-03", 5.000000, 2.000000)
	map:addTile("tile-ground-03", 5.000000, 7.000000)
	map:addTile("tile-rock-big-01", 5.000000, 8.000000)
	map:addTile("tile-rock-02", 5.000000, 10.000000)
	map:addTile("tile-rock-03", 5.000000, 11.000000)
	map:addTile("tile-background-04", 6.000000, 0.000000)
	map:addTile("tile-background-big-01", 6.000000, 1.000000)
	map:addTile("tile-background-02", 6.000000, 3.000000)
	map:addTile("tile-ground-03", 6.000000, 4.000000)
	map:addTile("tile-ground-ledge-left-01", 6.000000, 5.000000)
	map:addTile("tile-background-02", 6.000000, 6.000000)
	map:addTile("tile-ground-03", 6.000000, 7.000000)
	map:addTile("tile-rock-big-01", 6.000000, 10.000000)
	map:addTile("tile-background-01", 7.000000, 0.000000)
	map:addTile("tile-background-window-01", 7.000000, 3.000000)
	map:addTile("tile-ground-01", 7.000000, 4.000000)
	map:addTile("tile-ground-02", 7.000000, 5.000000)
	map:addTile("tile-rock-01", 7.000000, 6.000000)
	map:addTile("tile-rock-01", 7.000000, 7.000000)
	map:addTile("tile-rock-big-01", 7.000000, 8.000000)
	map:addTile("tile-background-02", 8.000000, 0.000000)
	map:addTile("tile-background-02", 8.000000, 1.000000)
	map:addTile("tile-background-02", 8.000000, 2.000000)
	map:addTile("tile-ground-ledge-right-01", 8.000000, 4.000000)
	map:addTile("tile-background-02", 8.000000, 5.000000)
	map:addTile("tile-background-ice-05", 8.000000, 6.000000)
	map:addTile("tile-ground-03", 8.000000, 7.000000)
	map:addTile("tile-rock-03", 8.000000, 10.000000)
	map:addTile("tile-rock-02", 8.000000, 11.000000)
	map:addTile("tile-background-03", 9.000000, 0.000000)
	map:addTile("tile-background-big-01", 9.000000, 1.000000)
	map:addTile("tile-background-01", 9.000000, 3.000000)
	map:addTile("tile-ground-ledge-right-02", 9.000000, 4.000000)
	map:addTile("tile-background-ice-06", 9.000000, 5.000000)
	map:addTile("tile-background-04", 9.000000, 6.000000)
	map:addTile("tile-ground-01", 9.000000, 7.000000)
	map:addTile("tile-rock-03", 9.000000, 8.000000)
	map:addTile("tile-ground-ledge-right-01", 9.000000, 9.000000)
	map:addTile("tile-background-big-01", 9.000000, 10.000000)
	map:addTile("tile-background-02", 10.000000, 0.000000)
	map:addTile("tile-background-04", 10.000000, 3.000000)
	map:addTile("tile-background-02", 10.000000, 4.000000)
	map:addTile("tile-background-02", 10.000000, 5.000000)
	map:addTile("tile-background-cave-art-01", 10.000000, 6.000000)
	map:addTile("tile-ground-03", 10.000000, 7.000000)
	map:addTile("tile-rock-03", 10.000000, 8.000000)
	map:addTile("tile-background-04", 10.000000, 9.000000)
	map:addTile("tile-background-02", 11.000000, 0.000000)
	map:addTile("tile-background-02", 11.000000, 1.000000)
	map:addTile("tile-background-02", 11.000000, 2.000000)
	map:addTile("tile-background-03", 11.000000, 3.000000)
	map:addTile("tile-background-big-01", 11.000000, 4.000000)
	map:addTile("tile-background-window-02", 11.000000, 6.000000)
	map:addTile("tile-ground-01", 11.000000, 7.000000)
	map:addTile("tile-ground-ledge-ice-right-02", 11.000000, 8.000000)
	map:addTile("tile-background-01", 11.000000, 9.000000)
	map:addTile("tile-background-ice-big-01", 11.000000, 10.000000)
	map:addTile("tile-background-01", 12.000000, 0.000000)
	map:addTile("tile-background-04", 12.000000, 1.000000)
	map:addTile("tile-background-02", 12.000000, 2.000000)
	map:addTile("tile-background-02", 12.000000, 3.000000)
	map:addTile("tile-ground-03", 12.000000, 7.000000)
	map:addTile("tile-background-03", 12.000000, 8.000000)
	map:addTile("tile-background-04", 12.000000, 9.000000)
	map:addTile("tile-background-01", 13.000000, 0.000000)
	map:addTile("tile-background-ice-06", 13.000000, 1.000000)
	map:addTile("tile-background-ice-05", 13.000000, 2.000000)
	map:addTile("tile-background-ice-05", 13.000000, 3.000000)
	map:addTile("tile-background-ice-06", 13.000000, 4.000000)
	map:addTile("tile-background-03", 13.000000, 5.000000)
	map:addTile("tile-background-03", 13.000000, 6.000000)
	map:addTile("tile-ground-ledge-ice-right-01", 13.000000, 7.000000)
	map:addTile("tile-background-03", 13.000000, 8.000000)
	map:addTile("tile-background-ice-05", 13.000000, 9.000000)
	map:addTile("tile-background-01", 13.000000, 10.000000)
	map:addTile("tile-background-ice-05", 13.000000, 11.000000)
	map:addTile("tile-background-02", 14.000000, 0.000000)
	map:addTile("tile-background-ice-05", 14.000000, 1.000000)
	map:addTile("tile-background-03", 14.000000, 2.000000)
	map:addTile("tile-background-ice-05", 14.000000, 3.000000)
	map:addTile("tile-background-ice-05", 14.000000, 4.000000)
	map:addTile("tile-background-big-01", 14.000000, 5.000000)
	map:addTile("tile-background-03", 14.000000, 7.000000)
	map:addTile("tile-background-ice-06", 14.000000, 8.000000)
	map:addTile("tile-background-01", 14.000000, 9.000000)
	map:addTile("tile-background-01", 14.000000, 10.000000)
	map:addTile("tile-background-ice-06", 15.000000, 0.000000)
	map:addTile("tile-background-03", 15.000000, 1.000000)
	map:addTile("tile-background-02", 15.000000, 2.000000)
	map:addTile("tile-background-01", 15.000000, 3.000000)
	map:addTile("tile-background-03", 15.000000, 4.000000)
	map:addTile("tile-background-02", 15.000000, 7.000000)
	map:addTile("tile-background-03", 15.000000, 8.000000)
	map:addTile("tile-background-03", 15.000000, 9.000000)


	map:addCave("tile-cave-01", 2.000000, 8.000000, "", 1000)
	map:addCave("tile-cave-01", 3.000000, 1.000000, "", 500000)
	map:addCave("tile-cave-01", 8.000000, 3.000000, "", 5000)
	map:addCave("tile-cave-01", 12.000000, 6.000000, "", 3000)

	map:setSetting("width", "16")
	map:setSetting("height", "12")
	map:setSetting("fishnpc", "false")
	map:setSetting("flyingnpc", "true")
	map:setSetting("gravity", "9.81")
	map:setSetting("npcs", "3")
	map:setSetting("npctransfercount", "3")
	map:setSetting("packages", "0")
	map:setSetting("packagetransfercount", "0")
	map:setSetting("points", "100")
	map:setSetting("referencetime", "50")
	map:setSetting("sideborderfail", "false")
	map:setSetting("theme", "rock")
	map:setSetting("waterchangespeed", "0.200000")
	map:setSetting("waterfallingdelay", "0")
	map:setSetting("waterheight", "1.000000")
	map:setSetting("waterrising", "0")
	map:setSetting("waterrisingdelay", "0")
	map:setSetting("wind", "0")

	map:addStartPosition("9.000000", "6.000000")
end
