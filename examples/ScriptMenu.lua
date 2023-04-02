local t = {}

function t.OnLoad()
	lang.install {
		en = {
			['example.mystr'] = 'My localized button'
		},
		ru = {
			['example.mystr'] = 'Моя локализированная кнопка'
		}
	}
end

function t.OnTick(buildMenu)
	-- Menu builder MUST be here
	if buildMenu then
		menu.set {
			title = 'My menu',
			items = {
				{
					type = 1,
					title = lang.get 'example.mystr'
				},
				{
					type = LUAMENU_ITEM_BUTTON,
					title = 'My button',
					onclick = function()
						print('Hello!')
					end
				},
				{
					type = LUAMENU_ITEM_SWITCHABLE,
					title = 'My switchable',
					onclick = function(state)
						return not state
					end
				},
				{
					type = LUAMENU_ITEM_MENU,
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
end

function t.OnStop()
	menu.remove()
end

t.OnReload = t.OnStop

return t
