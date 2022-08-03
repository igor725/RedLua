$ErrorActionPreference = "Stop";

$SCR_DIR = $(Split-Path $MyInvocation.MyCommand.Path -Parent)
if ($args[0] -eq "gtav") {
	$VARIANT = "V"
	$LINK = "/gtav/scripthookv/"
} else {
	$VARIANT = "RDR2"
	$LINK = "/rdr2/scripthookrdr2/"
}

$SHOOK_DIR = "$SCR_DIR/../src/thirdparty/ScriptHook$($VARIANT)"
$DOMAIN = "http://www.dev-c.com"
$FILES = @(
	"inc/", "lib/",
	"inc/enums.h", "inc/main.h", "inc/nativeCaller.h",
	"inc/types.h", "readme.txt", "lib/ScriptHook$($VARIANT).lib"
)

if (Test-Path -PathType Container $SHOOK_DIR) {
	$corrupted = $false

	Foreach($item in $FILES) {
		if (!(Test-Path "$SHOOK_DIR/$item")) {
			$corrupted = $true;
			break
		}
	}
	if ($corrupted -eq $false) {
		Exit 0
	}

	$title = "Corrupted ScriptHook$($VARIANT) installation"
	$text = "Looks like you have a broken ScriptHook$($VARIANT), do you want to redownload it?"
	$choices = New-Object Collections.ObjectModel.Collection[Management.Automation.Host.ChoiceDescription]
	$choices.Add((New-Object Management.Automation.Host.ChoiceDescription -ArgumentList "&Yes", "ScriptHook$($VARIANT) will be redownloaded"))
	$choices.Add((New-Object Management.Automation.Host.ChoiceDescription -ArgumentList "&No", "This script will close"))
	switch ($Host.UI.PromptForChoice($title, $text, $choices, 1)) {
		0 {
			Remove-Item -Recurse -Force $SHOOK_DIR
		}
		1 {
			Write-Host "Operation cancelled"
			Exit 0
		}
	}
}


$HEADERS = @{
	"Referer" = ($DOMAIN + $LINK);
	"User-Agent" = "RL/1.0"
}

Foreach($elem in (Invoke-WebRequest -URI ($DOMAIN + $LINK)).Links.Href) {
	if ($elem.IndexOf("ScriptHook$($VARIANT)_SDK") -ige 0) {
		$outFile = "$SCR_DIR/temp.zip";
		Invoke-WebRequest -Uri ($DOMAIN + $elem) -OutFile $outFile -Headers $HEADERS
		Add-Type -Assembly System.IO.Compression.FileSystem
		$zip = [IO.Compression.ZipFile]::OpenRead($outFile)
		New-Item -ItemType Directory -Path $SHOOK_DIR
		Foreach($ent in $zip.Entries) {
			$fileName = $ent.FullName
			if ($FILES.Contains($fileName)) {
				if ($fileName.Substring($fileName.Length - 1) -eq '/') {
					New-Item -ItemType Directory -Path "$SHOOK_DIR\$filename"
					Continue
				}
				[IO.Compression.ZipFileExtensions]::ExtractToFile($ent, "$SHOOK_DIR\$fileName", $true)
			}
		}
		$zip.Dispose()
		Remove-Item -Path $outFile
		Exit 0
	}
}

Write-Error "SDK link not found"
