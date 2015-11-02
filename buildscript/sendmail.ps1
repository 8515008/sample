
param([string]$flag)

ssh-agent -k
ssh-agent -k

 function sendMail{

     Write-Host "Sending Email"

     #SMTP server name
     $smtpServer = "smtp.sina.com"

     #Creating a Mail object
     $msg = new-object Net.Mail.MailMessage

     #Creating SMTP server object
     #$smtp = new-object Net.Mail.SmtpClient($smtpServer)

	 $file = $(get-location).Path + '\script.log'
	 $att = new-object Net.Mail.Attachment($file)

     #Email structure 
     $msg.From = "shareitbuild@sina.com"
     $msg.To.Add("8515008@qq.com")
	 $msg.To.Add("32121236@qq.com")
	 $msg.To.Add("80677408@qq.com")
	 $msg.To.Add("wxwangf@isoftstone.com")
	 $msg.To.Add("ginaliu@lenovo.com")
	 
	 $comp = $flag.CompareTo("pass")
	 
	 if($comp -eq 0)
	 {
	    $msg.subject = "Build pass, enjoy it."
		Write-Host "hello, pass"
	 }else
	 {
	    $msg.subject = "Build fail, please read the log."
		Write-Host "OMG, fail"	 
	}
	 
     $msg.body = "This is an automatic email from build server, don't reply!"
	 $msg.Attachments.Add($att)
	 
	$SMTPClient = New-Object Net.Mail.SmtpClient($smtpServer) 
	$SMTPClient.EnableSsl = $true 
	$SMTPClient.Credentials = New-Object System.Net.NetworkCredential("shareitbuild@sina.com", "Aa123456"); 
	$SMTPClient.Send($msg)  
	
	$att.Dispose()
}

#Calling function
#sendMail
