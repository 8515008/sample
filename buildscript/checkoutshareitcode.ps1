
$rungitresult = -1
if (test-path .\Shareit.Foundation)
{
	echo "git pull"
	cd .\Shareit.Foundation\
	git pull
	$rungitresult = $lastexitcode
}else
{
	echo "git clone"
	git clone git@github.com:kiddliu/Shareit.Foundation.git
	$rungitresult = $lastexitcode
	cd .\Shareit.Foundation\
}

ssh-agent -k
ssh-agent -k

$findstring = select-string .\installer\script.log -pattern "up-to-date" -quiet
if(!$findstring)
{
	if($rungitresult -eq 0)
	{
		
		$today = Get-Date
		$todaydate = $today.ToString("yyyy-MM-dd")
		$thedaybeforeyesterday = $today.AddDays(-2)
		$sincedate = $thedaybeforeyesterday.ToString("yyyy-MM-dd")
		$outputfile = -Join(".\installer\fixedbugs_", $todaydate, ".sh")
		$outputfile
		git log --since $sincedate --pretty=%B > $outputfile
		
	ssh-agent -k
	ssh-agent -k
		
		echo "start run build.bat"
		echo "test" > .\installer\isonbuildsever.tag
		pushd .\installer
		.\build.bat
		popd

		cd ..\cricket
		
		git add --all
		git commit -m "upload nightly build to server." 
		git pull
		git push -u origin master
		
			
	ssh-agent -k
	ssh-agent -k
	}else
	{
		echo "git command return failed!"
	}
}