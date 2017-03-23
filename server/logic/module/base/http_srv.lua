
function onHttpRequest(reqInfo, paramTbl)	
	print("onHttpRequest", sys.dump(reqInfo), sys.dump(paramTbl))
	return 200, "suc", "body"
end

