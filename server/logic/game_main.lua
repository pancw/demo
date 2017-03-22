
function Init()
	print("Lua Game Main Init.")

	local str = "haha"
	print(lmd5(str))
	
	local dnsName = "openapi.baidu.com"
	local function getIp(ip)
		print("ip:", ip)
	end
	HTTP.resolve(dnsName, getIp)
	--print(debug.traceback())
end
