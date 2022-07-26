local socket = require('socket.core')
local httpcodes = require('httpcodes')
local no_body = {
	['GET'] = true, ['DELETE'] = true,
	['TRACE'] = true, ['OPTIONS'] = true,
	['HEAD'] = true
}

local ctx_meta = {
	__metatable = 'none of your business',

	isPOST = function(self)
		return self.request.method == 'POST'
	end,
	isGET = function(self)
		return self.request.method == 'GET'
	end,
	isMethod = function(self, meth)
		return self.request.method == meth
	end,

	getUserAgent = function(self)
		return self.reqheaders['user-agent']
	end,
	getRequestAddress = function(self)
		return self.client:getpeername()
	end,
	getRequestHeader = function(self, hdr)
		return self.reqheaders[hdr:lower()]
	end,
	getRequestBody = function(self)
		return self.reqheaders['content-type'], self.request.body
	end,

	setCode = function(self, code)
		self.respcode = code or 200
		return self
	end,
	setContentType = function(self, t)
		self.respheaders['Content-Type'] = t
		return self
	end,

	unpack = function(self, data)
		return self.respcode, self.respheaders, data
	end
}
ctx_meta.__index = ctx_meta

local _M = {
	_VERSION = 1
}
_M.__index = _M

local function sassert(fd, ret, err)
	if not ret then
		fd:close()
	end
	assert(ret, err)
end

local function binder(ip, port)
	if ip == '*' then ip = '0.0.0.0' end
	local fd = assert(socket.tcp())
	fd:setoption('reuseaddr', true)
	sassert(fd, fd:bind(ip, port))
	sassert(fd, fd:listen())
	fd:settimeout(0)
	return fd
end

local function waitForLine(cl)
	local line, err
	local mark = os.clock()
	while not line do
		line, err = cl:receive('*l')
		if not line then
			if os.clock() - mark > 4 then
				error(err)
			end
			coroutine.yield()
		end
	end
	return line
end

local function reliableSend(cl, text)
	local err, from
	local mark = os.clock()

	repeat
		err, from = select(2, cl:send(text, from))
		if err == 'closed' then
			return false
		elseif err == 'timeout' then
			if os.clock() - mark > 4 then
				error(err)
			end
		end
		if from then
			coroutine.yield()
		end
	until from == nil

	return true
end

function _M:newServer(ip, port, root)
	return setmetatable({
		server = assert(binder(ip, port), 'Failed to bind port'),
		root = root or './www',
		no_io = false,
		clients = {},
		vhs = {}
	}, self)
end

local valid = {["nil"] = true, ["function"] = true, ["string"] = true}
function _M:registerVirtualHandler(path, func)
	assert(valid[type(func)])
	self.vhs[path] = func
end

function _M:generateResponse(cl, code, headers, body)
	cl:receive('*a') -- Читаем остатки данных, если такие имеются
	local hint = httpcodes[code] or error(('Invalid HTTP code: %d'):format(code))
	body = body ~= nil and tostring(body) or hint
	headers = headers or {}

	if not reliableSend(cl, ('HTTP/1.1 %d %s\r\n'):format(code, hint)) then
		return false
	end

	headers['Server'] = self.serverName
	headers['Content-Length'] = #body
	if not headers['Content-Type'] then
		headers['Content-Type'] = 'text/html'
	end
	for key, value in pairs(headers) do
		if not reliableSend(cl, ('%s: %s\r\n'):format(key, value)) then
			return false
		end
	end
	if not reliableSend(cl, '\r\n') then
		return false
	end

	return body and reliableSend(cl, body) or true
end

function _M:pushError(cl, err)
	return self:generateResponse(cl, 500, nil, err)
end

_M.processLuaBlock = function(block)
	local succ, ret = xpcall(load(block, 'LuaBlock', nil, getfenv()), debug.traceback)
	if not succ then error(ret)end
	return __BUFFER .. (ret ~= nil and tostring(ret) or "")
end

function _M:executeLuaBlocks(data, ctx)
	local env = setmetatable({
		_CONTEXT = ctx,
		__BUFFER = '',

		os = {},
		io = {}
	}, {__index = _G})
	env.print = function(...)
		for i = 1, select('#', ...) do
			env.__BUFFER = env.__BUFFER .. tostring(select(i, ...))
		end
	end
	env.io.write = env.print
	setfenv(self.processLuaBlock, env)
	data = data:gsub('%<%?lua(.-)%?%>', self.processLuaBlock)
	setfenv(self.processLuaBlock, _G)
	return data
end

function _M:processRequest(ctx)
	local req = ctx.request
	local vh = self.vhs[req.uri]
	if vh ~= nil then
		local data
		if type(vh) == 'function' then
			data = vh(self, ctx)
		else
			data = tostring(vh)
		end
		return ctx:unpack(data ~= ctx and data or nil)
	end

	if self.no_io == false then
		local path = self.root .. req.uri
		if path:find('/$') then
			path = path .. 'index.html'
		end

		local mime = require('mimetypes').guess(path)
		local file = io.open(path, 'rb')
		if file then
			local data = file:read('*a')
			if mime:find('^text/') then
				data = self:executeLuaBlocks(data, ctx)
			end
			file:close()
			ctx:setContentType(mime)
			return ctx:unpack(data)
		end
	end

	return 404
end

function _M:clientHandler(cl)
	local line
	local ctx = setmetatable({
		client = cl,
		request = {},
		reqheaders = {},
		respcode = 200,
		respheaders = {}
	}, ctx_meta)

	-- Получаем заголовки
	line = waitForLine(cl)
	local r = ctx.request
	r.method, r.uri, r.version = line:match('(%w+)%s(/.*)%sHTTP/(.+)')
	if not r.method then
		return self:generateResponse(cl, 400, nil, 'Failed to parse request line')
	else
		r.method = r.method:upper()
	end

	if r.version ~= '1.1' and r.version ~= '1.0' then
		return self:generateResponse(cl, 505)
	end

	local twodots = r.uri:find('%.%.')
	if twodots then
		return self:generateResponse(cl, 400, nil, 'Prohibited URL')
	end

	local query_start = r.uri:find('%?')
	if query_start then
		local qstr = r.uri:sub(query_start + 1)
		r.uri = r.uri:sub(0, query_start - 1)
		if #qstr > 0 then
			local query
			for qprm in qstr:gmatch('([^&]+)') do
				local key, value = qprm:match('(.+)=(.*)')
				if not key then
					return self:generateResponse(cl, 400, nil, 'Malformed query string')
				end
				query = query or {}
				query[key] = tonumber(value)or value
			end
			r.query = query
		end
	end

	local h = ctx.reqheaders
	repeat
		line = waitForLine(cl)
		if #line > 0 then
			local key, value = line:match('(.+): (.+)')
			h[key:lower()] = tonumber(value) or value
		end
	until #line == 0

	local dleft = h['content-length']
	if dleft then
		if no_body[r.method] == true then
			return self:generateResponse(cl, 400)
		end

		repeat
			local data = cl:receive(dleft)
			r.body = r.body and r.body..data or data
			dleft = dleft - #data
		until dleft == 0
	end

	return self:generateResponse(cl, self:processRequest(ctx))
end

function _M:doStep()
	while true do
		local client, err = self.server:accept()
		if err ~= nil then
			if err == 'closed' then
				return false
			end

			break
		end
		client:settimeout(0)
		self.clients[client] = coroutine.create(self.clientHandler)
	end

	for client, coro in pairs(self.clients) do
		local status = coroutine.status(coro)
		if status == 'suspended' then
			local succ, err = coroutine.resume(coro, self, client)
			if not succ then
				if not tostring(err):find('timeout$') then
					coro = coroutine.create(self.pushError)
					succ, err = coroutine.resume(coro, self, client, err)
					if not succ then print('Omfg, error in error handler', err)end
					self.clients[client] = coro
				end
			end
		elseif status == 'dead' then
			self.clients[client] = nil
			client:close()
		end
	end

	return true
end

function _M:startLoop()
	while self:doStep() do
		socket.sleep(0.01)
	end
end

function _M:disableIO(status)
	self.no_io = (status == true)
end

function _M:close()
	for cl in pairs(self.clients) do
		self.clients[cl] = nil
		cl:close()
	end
	self.server:close()
	collectgarbage()
end

function _M:getVersion()
	return self._VERSION
end

return _M
