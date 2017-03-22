msgpack = require "MessagePack"
lnet = require "lnetlib"

CONFIG_TBL = {
	port = 1228,
}

function handle_event(vfd, msg)
	print("====",vfd ,msg)
end


function Tick()
	--print("lua tick")
	--lnet.send(1, "lua tick")
end
