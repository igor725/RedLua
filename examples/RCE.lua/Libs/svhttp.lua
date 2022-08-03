local ffi = require'ffi'
local C = ffi.C
ffi.cdef[[
	void *malloc(size_t);
	void *realloc(void *, size_t);
	void free(void *);

	void Sleep(unsigned long);

	struct WSADATA {
		short wLoVer, wHiVer;
		unsigned short iMaxSock;
		long dont_care[128];
	};

	int WSAStartup(short, struct WSADATA *);
	void WSACleanup();
	unsigned long WSAGetLastError();

	struct sockaddr {
		unsigned short sa_family;
		char           sa_data[14];
	};

	struct in_addr {
		unsigned long s_addr;
	};

	struct sockaddr_in {
		short sin_family;
		unsigned short sin_port;
		struct in_addr sin_addr;
		char sin_zero[8];
	};

	unsigned short htons(unsigned short);
	unsigned long htonl(unsigned long);
	int inet_pton(int, const char *, void *);

	int socket(int, int, int);
	int bind(int, const struct sockaddr *, int);
	int listen(int, int);
	int accept(int);
	int recv(int, char *, int, int);
	int send(int, const char *, int, int);
	int setsockopt(int, int, int, const void *, int);
	int ioctlsocket(int, long, unsigned long *);
	void closesocket(int);

	struct sockbuf {
		unsigned int size, pos;
		char *ptr;
	};
]]
local lib = ffi.load'ws2_32'
local wdata = ffi.new('struct WSADATA')
if lib.WSAStartup(0x0202, wdata) ~= 0 then
	error('WSAStartup failed')
end

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

local function ensure(buf, need)
	if buf.size - buf.pos < need then
		local newsz = buf.size + need + 256
		buf.ptr = C.realloc(buf.ptr, newsz)
		assert(buf.ptr ~= nil, 'realloc() failed')
		buf.size = newsz
	end
end

local sock_meta = {
	init = function(self, fd)
		self.fd = fd
		local val = ffi.new('long[1]', 0)
		if lib.setsockopt(fd, 0xffff, 0x0004, val, 4) ~= 0 then -- REUSEADDR
			error('setsockopt failed')
		end
		val[0] = 1
		if lib.ioctlsocket(fd, -2147195266, val) ~= 0 then -- FIONBIO
			print(lib.WSAGetLastError())
			error('ioctlsocket failed')
		end
	end,
	bind = function(self, addr)
		if lib.bind(self.fd, ffi.cast('const struct sockaddr *', addr), ffi.sizeof(addr)) ~= 0 then
			print(lib.WSAGetLastError())
			error('bind() failed')
		end
		if lib.listen(self.fd, 128) ~= 0 then
			print(lib.WSAGetLastError())
			error('listen() failed')
		end
	end,
	receive = function(self, t)
		if self._sockbuf == nil then
			self._sockbuf = ffi.new('struct sockbuf', {
				32, 0, C.malloc(32)
			})
		end

		if t == '*l' then
			local len
			repeat
				ensure(self._sockbuf, 1)
				len = lib.recv(self.fd, self._sockbuf.ptr + self._sockbuf.pos, 1, 0)
				if len > 0 then
					local ch = self._sockbuf.ptr[self._sockbuf.pos]
					if ch == 10 then
						local e = self._sockbuf.pos
						self._sockbuf.pos = 0
						return ffi.string(self._sockbuf.ptr, e)
					elseif ch ~= 13 then
						self._sockbuf.pos = self._sockbuf.pos + 1
					end
				end
			until len == -1 or len == 0

			return nil, 'timeout'
		elseif t == '*a' then
			ensure(self._sockbuf, 128)
			local ret
			repeat
				ret = lib.recv(self.fd, self._sockbuf.ptr + self._sockbuf.pos, 128, 0)
				if ret > 0 then self._sockbuf.pos = self._sockbuf.pos + ret end
			until ret == -1 or ret == 0

			return self._sockbuf.pos
		elseif type(t) == 'number' then
			ensure(self._sockbuf, t)
			local len = lib.recv(self.fd, self._sockbuf.ptr, t, 0)
			return ffi.string(self._sockbuf.ptr, len)
		end

		return nil, 'fuck'
	end,
	send = function(self, data, from)
		from = from or 0
		local bs = #data - from
		local sent = from
		if bs > 0 then
			data = ffi.cast('char *', data) + from
			local ret = lib.send(self.fd, data, bs, 0)
			if ret < 0 then return 0, 'timeout', from end
			from = from + ret
			bs = bs - ret
		end

		local err
		if bs ~= 0 then
			err = 'timeout'
		else
			from = nil
		end

		return sent, err, from
	end,
	close = function(self)
		lib.closesocket(self.fd)
		if self._sockbuf then
			C.free(self._sockbuf.ptr)
			self._sockbuf.ptr = nil
		end
	end
}
sock_meta.__index = sock_meta
sock_meta.accept = function(self)
	local cfd = lib.accept(self.fd)
	if cfd == -1 then return nil, true end
	local cl = setmetatable({}, sock_meta)
	cl:init(cfd)
	return cl
end

local function binder(ip, port)
	if ip == '*' then ip = '0.0.0.0' end
	local addr = ffi.new('struct sockaddr_in', {
		sin_family = 2, -- AF_INET
		sin_port = lib.htons(port),
	})
	if lib.inet_pton(addr.sin_family, ip, addr.sin_addr) ~= 1 then
		error('Invalid IP specified')
	end
	local sock = setmetatable({}, sock_meta)
	sock:init(lib.socket(addr.sin_family, 1, 6))
	sock:bind(addr)
	return sock
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
		C.Sleep(10)
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
