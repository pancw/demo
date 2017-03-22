msgpack = require "common.MessagePack"

CONFIG_TBL = {
	port = 1228,
}

function __G__TRACKBACK__(msg)
	print_error("----------------------------------------\n")
	print_error("DEBUG ERROR: " .. tostring(msg) .. "\n")
	print_error(debug.traceback())
	print_error("\n----------------------------------------")
end 

function BeforeShutDown()
end

function OnUserCloseConnect(vfd)
end

function Tick()
end

function handle_event(vfd, msg)
	print("====",vfd ,msg)
	lnet.send(vfd, "haha")
end

local DOFILELIST = 
{
	"base/common_class.lua",
	"base/class.lua",
	"base/import.lua",
	"base/extend.lua",
	"base/global.lua",
}
for _,file in ipairs(DOFILELIST) do
	dofile(file)
end

--MONGO_SL.initMongo()

function main()
end

GAME_MAIN.Init()
--xpcall(main, __G__TRACKBACK__)
