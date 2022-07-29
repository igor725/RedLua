local t = {}

function t.OnLoad()
	menu.set{
		title = 'My menu',
		items = {
			{
				type = 1,
				title = 'My inactive button'
			},
			{
				type = 1,
				title = 'My button',
				onclick = function()
					print('Hello!')
				end
			},
			{
				type = 2,
				title = 'My switchable',
				onclick = function(state)
					return not state
				end
			},
			{
				type = 0,
				islist = true,
				title = 'My sublist',
				menu = {
					title = 'My submenu',
					items = {
						{
							type = 1,
							title = 'My subbutton',
							onclick = function() end
						}
					}
				}
			}
		}
	}
end

function t.OnStop()
	menu.remove()
end

t.OnReload = t.OnStop

return t
