
local globalId = 0

local cbTbl = {}
--[[
	[id] = {
		func = *,
		paramTbl = *,
	},
--]]

function fetchCBId(func, ...)
	globalId = globalId + 1

	cbTbl[globalId] = {
		func = func,
		paramTbl = {...},
	}

	return globalId
end

function getInfoById(id)
	return cbTbl[id]
end

function clearCBId(cbId)
	if cbTbl[cbId] then
		cbTbl[cbId] = nil
	end
end

