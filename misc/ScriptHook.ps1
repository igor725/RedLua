$SCR_DIR = $(Split-Path $MyInvocation.MyCommand.Path -Parent)
$SHOOK_DIR = "$SCR_DIR/../src/thirdparty/ScriptHook"
$DOMAIN = "http://www.dev-c.com"
$LINK = "/rdr2/scripthookrdr2/"
$FILES = @(
	"inc/", "lib/",
	"inc/enums.h", "inc/main.h", "inc/nativeCaller.h",
	"inc/types.h", "lib/ScriptHookRDR2.lib", "readme.txt"
)
$PATCHES = @{
	"inc/main.h" = @(
		,@(
			"VER_1_0_1207_69_RGS,`r`n`r`n`tVER_SIZE",
			("`VER_1_0_1207_69_RGS,`r`n" +
			"`r`n`t//Added by patcher`r`n" +
			"`tVER_1_0_1207_73_RGS,`r`n" +
			"`tVER_1_0_1207_77_RGS,`r`n" +
			"`tVER_1_0_1207_80_RGS,`r`n" +
			"`tVER_1_0_1232_13_RGS,`r`n" +
			"`tVER_1_0_1232_17_RGS,`r`n" +
			"`tVER_1_0_1311_12_RGS,`r`n" +
			"`tVER_1_0_1436_25_RGS,`r`n" +
			"`tVER_1_0_1436_31_RGS,`r`n" +
			"`r`n`tVER_SIZE")
		)
	)
}

if(Test-Path -PathType Container $SHOOK_DIR) {
	$corrupted = $false

	Foreach($item in $FILES) {
		if(!(Test-Path "$SHOOK_DIR/$item")) {
			$corrupted = $true;
			break
		}
	}

	if($corrupted) {
		$title = "Corrupted ScriptHookRDR2 installation"
		$text = "Looks like you have a broken ScriptHookRDR2, do you want to redownload it?"
		$choices = New-Object Collections.ObjectModel.Collection[Management.Automation.Host.ChoiceDescription]
		$choices.Add((New-Object Management.Automation.Host.ChoiceDescription -ArgumentList "&Yes", "ScriptHookRDR2 will be redownloaded"))
		$choices.Add((New-Object Management.Automation.Host.ChoiceDescription -ArgumentList "&No", "This script will close"))
		switch($Host.UI.PromptForChoice($title, $text, $choices, 1)) {
			0 {
				Remove-Item -Recurse -Force $SHOOK_DIR
			}
			1 {
				Write-Host "Operation cancelled"
				Exit 0
			}
		}
	} else {
		Exit 0
	}
} else {
	New-Item -ItemType Directory -Path $SHOOK_DIR
}


$HEADERS = @{
	"Referer" = ($DOMAIN + $LINK);
	"User-Agent" = "RL/1.0"
}

Foreach($elem in (Invoke-WebRequest -URI ($DOMAIN + $LINK)).Links.Href) {
	if($elem.IndexOf("ScriptHookRDR2_SDK") -ige 0) {
		$outFile = "$SCR_DIR/temp.zip";
		Invoke-WebRequest -Uri ($DOMAIN + $elem) -OutFile $outFile -Headers $HEADERS
		Add-Type -Assembly System.IO.Compression.FileSystem
		$zip = [IO.Compression.ZipFile]::OpenRead($outFile)
		Foreach($ent in $zip.Entries) {
			$fileName = $ent.FullName
			if($FILES.Contains($fileName)) {
				if($fileName.Substring($fileName.Length - 1) -eq '/') {
					New-Item -ItemType Directory -Path "$SHOOK_DIR\$filename"
					Continue
				}
				[IO.Compression.ZipFileExtensions]::ExtractToFile($ent, "$SHOOK_DIR\$fileName", $true)
			}
		}
		$zip.Dispose()
		Foreach($fpatches in $PATCHES.GetEnumerator()) {
			$fileName = "$SHOOK_DIR/$($fpatches.Name)"
			$content = (Get-Content $fileName -Raw)
			Foreach($patch in $fpatches.Value) {
				$content = $content -replace $patch[0], $patch[1]
			}
			$content | Set-Content $fileName -Force
		}
		Remove-Item -Path $outFile
		Exit 0
	}
}

Write-Error "SDK link not found"
