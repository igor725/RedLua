local t = {}
local index_page = [[
<!DOCTYPE html>
<html>
	<head>
		<title>Remote code execution</title>
		<meta name="viewport" content="width=device-width; initial-scale=1.0,user-scalable=no">
		<style type="text/css" media="screen">
			.ace_scrollbar {
				scrollbar-width: thin;
			}

			#editor {
				position: absolute;
				resize: vertical;
				top: 0;
				right: 0;
				bottom: 50%;
				left: 0;
			}

			#output {
				position: absolute;
				min-height: 5%;
				height: auto;
				top: 50%;
				right: 0;
				bottom: 0;
				left: 0;
			}
		</style>
		<script src="https://cdnjs.cloudflare.com/ajax/libs/ace/1.8.1/ace.min.js" integrity="sha512-IunksvjFi1CZJ59SN0Fw0dSkjMgLrY1PQ0WVPv1L3er6z1zW0AVLXs9nM2ZoEisoRo8eHDQn8FOs2KsZPwrUww==" crossorigin="anonymous" referrerpolicy="no-referrer"></script>
		<script src="https://cdnjs.cloudflare.com/ajax/libs/ace/1.8.1/ext-language_tools.min.js" integrity="sha512-ljsVE/eL9Wo8jiHXmrgTO4NZwcBLr7pzhHkeq38BnzYujF5T+NnKbXHh5bS0kCaZgEYfBrQqOiGK0q917lo3BQ==" crossorigin="anonymous" referrerpolicy="no-referrer"></script>
		<script src="https://cdnjs.cloudflare.com/ajax/libs/ace/1.8.1/snippets/lua.min.js" integrity="sha512-Q2rANYT60bpljXiPx2Gc02A+Mhwx9vXFcDkOFqEh8VoYUj2Av4X3B0K783TxNq4EvIpqewhzbSJ8dbU81sbrQQ==" crossorigin="anonymous" referrerpolicy="no-referrer"></script>
		<script src="https://cdnjs.cloudflare.com/ajax/libs/ace/1.8.1/mode-lua.min.js" integrity="sha512-1FRYEFZJW+iKT+sbGRqj+GKs/BXfhFPMmEaOyksw5S4VcZEDY6tSn4/f9CsQ9fX+KYXWhPRqvuHzCLk8B+k8Yg==" crossorigin="anonymous" referrerpolicy="no-referrer"></script>
		<script src="https://cdnjs.cloudflare.com/ajax/libs/ace/1.8.1/worker-lua.min.js" integrity="sha512-5/kMTTUZCTXQtU+SBaYUjK8uETzt4FcpPHVIOO5oF4wNpWNyGBRoxJdpPngEcsb6+aJlq+afQjYvbUT4gYDLSw==" crossorigin="anonymous" referrerpolicy="no-referrer"></script>
		<script src="https://cdnjs.cloudflare.com/ajax/libs/ace/1.8.1/theme-tomorrow_night.min.js" integrity="sha512-rpnNB4FBZTLJy1Vjk5/2A3nfJCBUOCFznS4Gth9qJIRZC7Ndx53XKbNDSmwpLZGvoOZoZEVQ1AkAj8KGnw1KKw==" crossorigin="anonymous" referrerpolicy="no-referrer"></script>
		<script src="https://cdnjs.cloudflare.com/ajax/libs/ace/1.8.1/theme-twilight.min.js" integrity="sha512-dFk7kcyT/ImbfIW3JcvxjjJ1ZqIxNfUbqn4QoYkkeZsp0h298RuPwy81fF1ZguJp4HbCz4ToDUhl3ajkCVAIBw==" crossorigin="anonymous" referrerpolicy="no-referrer"></script>
		<script src="https://cdnjs.cloudflare.com/ajax/libs/keyboardjs/2.6.2/keyboard.min.js" integrity="sha512-Q9aijJKP9BeTXgQHmb/j8AZTQ15//k9QvGXCbKMf1bt289s75awi/3SBFZ3M3J27NtD7JyU3d9d1eRPuO4BbhQ==" crossorigin="anonymous" referrerpolicy="no-referrer"></script>
		<script src="https://cdnjs.cloudflare.com/ajax/libs/toastify-js/1.12.0/toastify.min.js" integrity="sha512-0Yc4Jv5wX4+mjDuLxmHFGqgDtMFAEBLpPq/0nPVmAOwHPMkYXiS1YVYWTcrVQztftk/32089DDTyrCJO8hBCZw==" crossorigin="anonymous" referrerpolicy="no-referrer"></script>
		<link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/toastify-js/1.12.0/toastify.min.css" integrity="sha512-k+xZuzf4IaGQK9sSDjaNyrfwgxBfoF++7u6Q0ZVUs2rDczx9doNZkYXyyQbnJQcMR4o+IjvAcIj69hHxiOZEig==" crossorigin="anonymous" referrerpolicy="no-referrer" />
		<script>
			window.onload = () => {
				let sessions = [];
				for (let i = 0; i < 10; i++) {
					let name = 'sess' + i;
					let session = JSON.parse(localStorage.getItem(name));
					if (session == null) {
						session = {
							annotations: [],
							breakpoints: [],
							folds: [],
							history: {
								undo: [],
								redo: []
							},
							scrollLeft: 0,
							scrollTop: 0,
							selection: {
								start: {row: 0, column: 0},
								end: {row: 0, column: 0},
								isBackwards: false
							},
							value: ''
						}
						localStorage.setItem(name, JSON.stringify(session));
					}
					sessions.push(session);
				}

				let restoreSession = (current, id) => {
					if (id >= sessions.length) throw "wtf dude?";
					let saved = sessions[id];

					current.removeFolds(current.getAllFolds());
					current.setValue(saved.value);
					saved.folds.forEach((fold) => {
						current.addFold('...', ace.Range.fromPoints(fold.start, fold.end));
					});
					current.setAnnotations(saved.annotations);
					current.setBreakpoints(saved.breakpoints);
					let um = new ace.UndoManager();
					current.setUndoManager(um);
					um.$undoStack = saved.history.undo;
					um.$redoStack = saved.history.redo;
					current.setMode('ace/mode/lua');
					current.setScrollLeft(saved.scrollLeft);
					current.setScrollTop(saved.scrollTop);
					current.selection.fromJSON(saved.selection);
				}

				let saveSession = (session, id) => {
					sessions[id] = {
						annotations: session.getAnnotations(),
						breakpoints: session.getBreakpoints(),
						folds: session.getAllFolds().map(function(fold) {
							return fold.range;
						}),
						history: {
							undo: session.getUndoManager().$undoStack,
							redo: session.getUndoManager().$redoStack
						},
						scrollLeft: session.getScrollLeft(),
						scrollTop: session.getScrollTop(),
						selection: session.getSelection().toJSON(),
						value: session.getValue()
					};

					localStorage.setItem('sess' + id, JSON.stringify(sessions[id]));
				}

				let currentSession = parseInt(localStorage.getItem('sess'));
				if (isNaN(currentSession)) currentSession = 0;

				let editor = ace.edit('editor');
				editor.focus();
				editor.setOptions({
					tabSize: 4,
					wrap: true,
					useSoftTabs: false,
					newLineMode: 'unix',
					scrollPastEnd: true,
					enableSnippets: true,
					animatedScroll: true,
					indentedSoftWrap: false,
					enableLiveAutocompletion: true,
					enableBasicAutocompletion: true,
					theme: 'ace/theme/tomorrow_night'
				});

				restoreSession(editor.getSession(), currentSession);
				window.onbeforeunload = () => saveSession(editor.getSession(), currentSession);

				let output = ace.edit('output');
				output.setOptions({
					readOnly: true,
					showGutter: false,
					showPrintMargin: false,
					theme: 'ace/theme/twilight'
				});

				let output_el = document.getElementById('output'),
				editor_el = document.getElementById('editor');

				let observer = new MutationObserver((muts) => {
					muts.forEach((mut) => {
						output_el.style.top = editor_el.style.height;
						editor.resize(true);
					});
				});

				observer.observe(editor_el, {
					attributes: true,
					attributeFilter: ['style']
				});

				let add_output = (text) => {
					let osess = output.getSession();
					osess.insert({
						row: osess.getLength(),
						column: 0
					}, "\n" + text);
					output.scrollToRow(osess.getLength());
				}

				let push_toast = (text, dur = 1000) => {
					Toastify({
						text: text,
						duration: dur,
						close: true,
						gravity: "top",
						position: "right",
						style: {
							background: "linear-gradient(to right, #1D1F21, #232527)"
						}
					}).showToast();
				}

				keyboardJS.watch(editor_el);

				for (let i = 0; i < 10; i++) {
					keyboardJS.bind('ctrl+num' + i, (e) => {
						if (currentSession != i) {
							let session = editor.getSession();
							saveSession(session, currentSession);
							restoreSession(session, i);
							localStorage.setItem('sess', i);
							currentSession = i;
							push_toast("Switched to Tab#" + i);
						}
						e.preventDefault();
					});
				}

				keyboardJS.bind('ctrl+s', (e) => {
					saveSession(editor.getSession(), currentSession);
					push_toast("Tab#" + currentSession + " saved!");
					e.preventDefault();
				});

				keyboardJS.bind('ctrl+r', (e) => {
					output.getSession().setValue('');
					e.preventDefault();
				});

				keyboardJS.bind('f5', (e) => {
					let code = editor.getSession().getValue();
					if (code.length > 1) {
						let xhr = new XMLHttpRequest();
						xhr.open('POST', '/api/execute', true);
						xhr.setRequestHeader('Content-Type', 'text/x-lua');
						xhr.setRequestHeader('Accept', 'text/plain');
						xhr.onload = (e) => add_output("Execution result: " + xhr.responseText);
						xhr.onerror = (e) => add_output(`Request failed: ${xhr.status} ${xhr.responseText}`);
						xhr.send(code);
					}
					e.preventDefault();
				});
			}
		</script>
	</head>
	<body>
		<div id="editor"></div>
		<div id="output">Exectuion output will appear here</div>
	</body>
</html>
]]

function t.OnLoad()
	local server = require('svhttp'):newServer('*', 1337)
	local exec_env = setmetatable({
		_SELF = server,
		io = {
			open = io.open
		},
		os = {}
	}, {
		__index = _G
	})
	exec_env.print = function(...)
		for i = 1, select('#', ...) do
			exec_env._BUF = exec_env._BUF .. tostring(select(i, ...))
		end
	end
	exec_env.io.write = exec_env.print

	server:disableIO(true)
	server:registerVirtualHandler('/', index_page)
	server:registerVirtualHandler('/api', function(_, ctx) return ctx:setCode(403) end)
	server:registerVirtualHandler('/api/execute', function(_, ctx)
		if not ctx:isPOST() then
			return ctx:setCode(405)
		end

		local mime, data = ctx:getRequestBody()
		if mime == 'text/x-lua' then
			exec_env._BUF = ''
			local succ, ret = pcall(load(data, 'api.execute', nil, exec_env))
			ctx:setCode(succ and 200 or 500)
			return succ and (ret ~= nil and (exec_env._BUF .. tostring(ret)) or exec_env._BUF) or ret
		end

		return ctx:setCode(415)
	end)

	t.server = server
end

function t.OnTick()
	t.server:doStep()
end

function t.OnStop()
	t.server:close()
end

t.OnReload = t.OnStop

return t
