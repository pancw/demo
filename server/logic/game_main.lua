function Init()
	print("Lua Game Main Init.")

	local str = "haha"
	print(lmd5(str))

	local function onRet(ret, code)
		print("onRet", ret, code)
		if tonumber(ret) ~= 200 then
			return
		end
	end
	
	local API_KEY = "YZlVOyVTlpopG13A8cmnN2eu"
	local SECRET_KEY = "c989613febf535b24e0387a4fcf2ec95"
	local dnsName = "openapi.baidu.com"
	local url = string.format("/oauth/2.0/token?grant_type=client_credentials&client_id=%s&client_secret=%s", API_KEY, SECRET_KEY)
	local function getIp(ip)
		print("ip:", ip)
		HTTP.httpGet(ip, dnsName, 80, url, onRet)
	end
	HTTP.resolve(dnsName, getIp)
end
