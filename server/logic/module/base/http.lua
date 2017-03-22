
function onHttpCB(id, ret, code)
	local info = CB_MGR.getInfoById(id)
	info.func(ret, code)
	CB_MGR.clearCBId(id)
end

function httpGet(ip, dnsName, port, url, cbFunc)
	local id = CB_MGR.fetchCBId(cbFunc)
	lhttp.http_get(ip, dnsName, port, url, id)
end

function httpPost(ip, dnsName, port, url, cbFunc)
	local id = CB_MGR.fetchCBId(cbFunc)
	lhttp.http_post(ip, dnsName, port, url, id)
end

function base64Encode(data, len)
	return lhttp.base64_encode(data, len)
end

function encodeURI(uri)
	return lhttp.encode_uri(uri)
end

function decodeURI(uri)
	return lhttp.decode_uri(uri)
end

function onDnsCB(id, ip)
	local info = CB_MGR.getInfoById(id)
	info.func(ip)
	CB_MGR.clearCBId(id)
end

function resolve(dns, funcCB)
	local id = CB_MGR.fetchCBId(funcCB)
	ldns.resolve(dns, id)
end

