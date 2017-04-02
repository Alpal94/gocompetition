<?php
	require_once("helpers/class.user.php");
	require_once("helpers/powerhelper.php");
	
	$user_id = $_SESSION['user_session'];
	$thankyou = false;
	
	$auth_user = new USER();
	if(isset($_SESSION["captcha"])) if($_SESSION['captcha'] != "CODE_NO_MATCH" || $_SESSION['captcha'] != "CODE_NO_MATCH") {
		$_SESSION["email"] = "";
		$_SESSION["name"] = "";
		$_SESSION["phone"] = "";
		$_SESSION["message"] = "";
	}
	if(isset($_POST["email"])) {
		if(empty($_SESSION['6_letters_code'] ) ||
		    strcasecmp($_SESSION['6_letters_code'], $_POST['6_letters_code']) != 0)
		  {
			$_SESSION['captcha'] = "CODE_NO_MATCH";
			$_SESSION["email"] = $_POST["email"];
			$_SESSION["name"] = $_POST["name"];
			$_SESSION["phone"] = $_POST["phone"];
			$_SESSION["message"] = $_POST["message"];
		  }
		else {
			$email_valid = filter_var($_POST["email"], FILTER_VALIDATE_EMAIL);
			$phone_valid = true; //preg_match("/^[0-9]{11}$|^[0-9]{8}$|^[0-9]{10}$/", preg_replace("/\+|\ |\-/", "", $_POST["phone"]));
			if($email_valid && $phone_valid && $_POST["message"] != "" && $_POST["name"] != "") {
				$auth_user->sendContactEmail($_POST["email"], "NAME: ".$_POST["name"]."   PHONE: ".$_POST["phone"], $_POST["message"]);
				$_SESSION['captcha'] = "CODE_MATCH";
				$thankyou = true;
			} else { 
				if(!$email_valid) $_SESSION['valid_email'] = "EMAIL_INVALID";
				if(!$phone_valid) $_SESSION['valid_phone'] = "PHONE_INVALID";
				if($_POST["message"] == "") $_SESSION['valid_message'] = "MESSAGE_INVALID";
				if($_POST["name"] == "") $_SESSION['valid_name'] = "NAME_INVALID";
			}

			$_SESSION["phone"] = $_POST["phone"];
			$_SESSION["email"] = $_POST["email"];
			$_SESSION["name"] = $_POST["name"];
			$_SESSION["message"] = $_POST["message"];
		}
	}

?>

<!DOCTYPE html>
<html lang="en">
<head>        
    <title>PowerLedger - Transactions</title>
    
    <meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
	
    <meta charset="utf-8">
    <meta http-equiv="X-UA-Compatible" content="IE=edge">
    <meta name="viewport" content="width=device-width, initial-scale=1">
    
    <link rel="icon" type="image/ico" href="favicon.ico"/>
    
   <link href="css/stylesheets.css" rel="stylesheet" type="text/css" />
	<link href="css/style_custom.css" rel="stylesheet" type="text/css" />
	<link href="css/font-awesome.min.css" rel="stylesheet" type="text/css">
   
    <script type='text/javascript' src='js/plugins/jquery/jquery.min.js'></script>
    <script type='text/javascript' src='js/plugins/jquery/jquery-ui.min.js'></script>
    <script type='text/javascript' src='js/plugins/jquery/jquery-migrate.min.js'></script>
    <script type='text/javascript' src='js/plugins/jquery/globalize.js'></script>    
    <script type='text/javascript' src='js/plugins/bootstrap/bootstrap.min.js'></script>
    
    <script type='text/javascript' src='js/plugins/uniform/jquery.uniform.min.js'></script>
	<script type='text/javascript' src='js/plugins/datatables/jquery.dataTables.min.js'></script>
	<script type='text/javascript' src='js/moment.min.js'></script>
    <script type='text/javascript' src='js/ts/dataTables.responsive.min.js'></script> 
    
    <script type='text/javascript' src='js/plugins/knob/jquery.knob.js'></script>
	
	<script src="js/plugins/amcharts/amcharts.js" type="text/javascript"></script>
	<script src="js/plugins/amcharts/serial.js" type="text/javascript"></script>
	<script src="js/plugins/amcharts/pie.js" type="text/javascript"></script>
	<script src="js/plugins/amcharts/themes/dark.js" type="text/javascript"></script>

	<script type='text/javascript' src='js/time.js'></script>
	
	<script type='text/javascript' src='js/settings_handler.js'></script>
	<script type='text/javascript' src='js/settings.js'></script>
    
</head>
<style>
html {
  height: 100%;
  box-sizing: border-box;
}

*,
*:before,
*:after {
  box-sizing: inherit;
}

body {
  position: relative;
  margin: 0;
  padding-bottom: 6rem;
  min-height: 100%;
  font-family: "Helvetica Neue", Arial, sans-serif;
}

.demo {
  margin: 0 auto;
  padding-top: 64px;
  max-width: 640px;
  width: 94%;
}

.demo h1 {
  margin-top: 0;
}

/**
 * Footer Styles
 */

.footer {
  position: absolute;
  right: 0;
  bottom: 0;
  left: 0;
  padding: 1rem;
  background-color: #efefef;
  text-align: center;
}
.image-cropper {
    width: 100px;
    height: 100px;
    position: relative;
    overflow: hidden;
    -webkit-border-radius: 50%;
    -moz-border-radius: 50%;
    -ms-border-radius: 50%;
    -o-border-radius: 50%;
    border-radius: 50%;
}
img {
    display: inline;
    margin: 0 auto;
    height: 100%;
    width: auto;
}
#overflow2 {
	white-space:nowrap;
	width: 12em;
	overflow: hidden;
	text-overflow:ellipsis;
	width:80%;
}
</style>
<body class="bg-img-num1">
    
    <div class="container">
            
	<?php
		require_once("header.php");
		date_default_timezone_set("Australia/Perth");
	?>
	<div class="col-md-12 col-xs-12 top-msg-div" style="text-align:center;height:23px;">
		<h6 id="top-msg" class="text-custom" style="font-size:12px;"></h6>
	</div>
	<div class="col-md-6" style="margin:auto;float:none;">
			<?php if($thankyou) { ?>
			<br><br><br>
			<div class="block"><div class="content"><h4>Message has been sent successfully. Thank you for mailing Power Ledger.<br><br> Our hard working team will get back to you as soon as possible.</h4></div></div>
	</div>
    </div>
</body></html>
			<!--
			<?php } if(false) { ?>
			-->
			<?php } ?>
			<div class="header">
				<h2>Contact us</h2>
			</div>
			<form method="post" action="<?php echo $_SERVER['PHP_SELF']; ?>">
			<div class="content">
				<div class="form-group">
					<label class="control-label">Your name:</label>
					<input name="name" type="text" class="form-control" value="<?php if(isset($_SESSION["name"])) echo htmlspecialchars(strip_tags($_SESSION["name"])); ?>">
				</div>
				<div class="form-group">
					<label class="control-label">Your phone:</label>
					<input name="phone" type="text" class="form-control" value="<?php if(isset($_SESSION["phone"])) echo htmlspecialchars(strip_tags($_SESSION["phone"])); ?>">
				</div>                        
				<div class="form-group">
					<label class="control-label">Your E-mail:</label>
					<input name="email" type="text" class="form-control" value="<?php if(isset($_SESSION["email"])) echo htmlspecialchars(strip_tags($_SESSION["email"])); ?>">
				</div>
				<div class="form-group">
					<label class="control-label">Message:</label>
					<textarea name="message" class="control-label" style="width:100%;height:200px;"><?php if(isset($_SESSION["message"])) echo htmlspecialchars(strip_tags($_SESSION["message"])); ?></textarea>
				</div>
				<div class="form-group">
					<div align="center">
					<div style="display:inline-block; padding-right:10px;">
						<label style="display:block" class="control-label" for="message">Enter the captcha code here :</label>
						<input id="6_letters_code" class="form-control" name="6_letters_code" type="text">
					</div>
					<div style="display:inline-block;height:100%;"><br><img style="height:70px;width:200px;padding-top:20px;border-style:dotted;border-color:black;" src="captcha_code_file.php?rand=<?php echo rand(); ?>" id="captchaimg"></div>
					<br><br>
					</div>
				</div>
				<?php if(isset($_SESSION["captcha"]) && $_SESSION["captcha"] == "CODE_NO_MATCH") { $_SESSION["captcha"]  = "N/A"; ?>
				<div class="alert alert-danger">
					<label class="control-label"> ERROR!  Captcha codes do not match, please try again.</label>
				</div>
				<?php } if(isset($_SESSION["valid_email"]) && $_SESSION["valid_email"] == "EMAIL_INVALID") { $_SESSION["valid_email"] = "N/A"; ?>
				<div class="alert alert-danger">
					<label class="control-label"> Email is invalid.  Please enter valid email! </label>
				</div>
				<?php } if(isset($_SESSION["valid_phone"]) && $_SESSION["valid_phone"] == "PHONE_INVALID") { $_SESSION["valid_phone"] = "N/A";  ?>
				<div class="alert alert-danger">
					<label class="control-label"> Phone is invalid.  Please enter valid phone! </label>
				</div>
				<?php } if(isset($_SESSION["valid_message"]) && $_SESSION["valid_message"] == "MESSAGE_INVALID") { $_SESSION["valid_message"] = "N/A";  ?>
				<div class="alert alert-danger">
					<label class="control-label"> Please enter a message! </label>
				</div>
				<?php } if(isset($_SESSION["valid_name"]) && $_SESSION["valid_name"] == "NAME_INVALID") { $_SESSION["valid_name"] = "N/A";  ?>
				<div class="alert alert-danger">
					<label class="control-label"> Please provide your name. </label>
				</div>
				<?php } ?>
				<div class="form-group nmb tar">
					<button class="btn btn-default btn-clean">Send it</button>
				</div>
			</div>
			</form>
		</div>	
	</div>
</div>

</div>
<div id="scriptfooter" class="container"><a href="https://powerledger.io"><?php require_once("footer.php"); ?></a></div>
<script>
$(document).ready(function() {
			if($(document).height() >= $(window).height()) {
				var d = document.getElementById('scriptfooter');
				d.style.position = "relative"; 
			}
			else {
				var d = document.getElementById('scriptfooter');
				d.style.position = "absolute";
				d.style.bottom = "0px";
			}
		});
		$(window).resize(function() {
			if($(document).height() >= $(window).height()) {
				var d = document.getElementById('scriptfooter');
				d.style.position = "relative"; 
			}
			else {
				var d = document.getElementById('scriptfooter');
				d.style.position = "absolute";
				d.style.bottom = "0px";
			}
		});
</script>
</body>
</html>
