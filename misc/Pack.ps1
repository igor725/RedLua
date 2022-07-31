$CODES = @{
	"RDR2" = "rdr3"
	"V" = "gta5"
}

Foreach($item in @("RDR2", "V")) {
	New-Item -Path ".\objs\output$($item)\" -Name "RedLua" -ItemType "Directory" -ErrorAction "SilentlyContinue"
	Copy-Item -Path ".\misc\Langs\" -Destination ".\objs\output$($item)\RedLua\" -Container -Recurse -Force
	Copy-Item .\README.md .\objs\output$($item)\
	Copy-Item .\LICENSE .\objs\output$($item)\
	CURL.EXE -o.\objs\output$($item)\RedLua\natives.json https://raw.githubusercontent.com/alloc8or/$($CODES[$item])-nativedb-data/master/natives.json
	7Z.EXE a -tzip .\objs\out$($item).zip .\objs\output$($item)\* -r
}
