$claxHome = "C:\Clarive\Clax"
$claxReleasesUrl = "https://api.github.com/repos/clarive/clax/releases/latest"

$DebugPreference = "Continue"

function prepareClaxHome([string]$claxHome)
{
    If (-Not (Test-Path $claxHome)) {
        Write-Debug "Creating Clax Home $claxHome"
        New-Item -ItemType directory -Path $claxHome
    }
    else {
        Write-Debug "Clax Home $claxHome already exists"
    }
}

function download([string]$url, [string]$output)
{
    Write-Debug "Downloading $url to $output"
    Invoke-WebRequest -Uri $url -OutFile $output
}

function detectArch
{
    if ([IntPtr]::Size -eq 8) {
        Write-Debug "Detected 64bits"
	return "x86_64"
    }
    else {
        Write-Debug "Detected 32bits"
	return "x86"
    }
}

function getLatestRelease
{
    $arch = detectArch
    $js = Invoke-WebRequest -Uri $claxReleasesUrl | ConvertFrom-Json

    [hashtable]$release = @{}

    $downloadUrl = $null
    $js.assets | ForEach-Object {
	$name = $_.name
	if ($name -match "clax_.*_windows-$arch.zip") {
		Write-Debug "Found release $name"
		$release.name = $name;
		$release.downloadUrl = $_.browser_download_url
	}
    }

    if (-Not $release.name) {
       Write-Debug "Release not found"
    }

    return $release
}

function unzip([string]$path, [string]$outdir)
{
	$shell = new-object -com shell.application

	Write-Debug "Unzipping $path to $outdir"

	Get-ChildItem $path |
	Foreach-Object {
	    $zip = $shell.NameSpace($_.FullName)
	    foreach($item in $zip.items()) {
		$shell.Namespace($outdir).copyhere($item, 0x14)
	    }
	}
}

function backupPreviousInstallation
{
    If (Test-Path "$claxHome\clax") {
        $date = Get-Date -uformat "%s"
	$from = "$claxHome\clax"
	$to = "$claxHome\clax.backup.$date"

	Write-Debug "Backing up previous installation $from to $to"
        Move-Item -path "$from" -destination "$to"
    }
}

prepareClaxHome $claxHome

$release = getLatestRelease
$outputPath = "{0}\{1}" -f $claxHome, $release.name

download $release.downloadUrl $outputPath

backupPreviousInstallation

unzip $outputPath $claxHome

Write-Debug "Preparing clax directory"
Rename-Item -path $outputPath -newName "$claxHome\clax"

#Write-Debug "Preparing wininetd configuration"
#$wininetdConfig = "11801 none $claxHome\clax\clax.exe -l $claxHome\clax\clax.log"
#$wininetdConfig | Out-File -FilePath $claxHome\clax\wininetd.conf

#Write-Debug "Installing wininetd service"
#Invoke-Item "$claxHome\clax\wininetd.exe --install cfgfile $claxHome\clax\wininetd.conf"
