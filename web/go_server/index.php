<?php 
	session_start();
	$db = new MongoClient("mongodb://terminator:hrsyqrzjcxfuuagwxrheksvyzcsfmqjg@localhost:27017");


	$valid_captcha = false; $_team_exists = false;$thankyou = false; $team_null = false;
	if(isset($_SESSION["captcha"])) if($_SESSION['captcha'] != "CODE_NO_MATCH" || $_SESSION['captcha'] != "CODE_NO_MATCH") {
		$_SESSION["first_name"] = "";
		$_SESSION["last_name"] = "";
		$_SESSION["team_name"] = "";
		$_SESSION["email"] = "";
		$_SESSION["password"] = "";
	}
	if(isset($_POST["email"])) {
		if(empty($_SESSION['6_letters_code'] ) ||
		    strcasecmp($_SESSION['6_letters_code'], $_POST['6_letters_code']) != 0)
		  {
			$_SESSION['captcha'] = "CODE_NO_MATCH";
			$_SESSION["names"] = $_POST["names"];
			$_SESSION["student_ids"] = $_POST["student_ids"];
			$_SESSION["team_name"] = $_POST["team_name"];
			$_SESSION["email"] = $_POST["email"];
		  }
		else {
			$valid_captcha = true;
			$email_valid = filter_var($_POST["email"], FILTER_VALIDATE_EMAIL);
			if($email_valid) {
				$_SESSION['captcha'] = "CODE_MATCH";
				$team_exists = $db->go_server->teams->find(array("team_name"=>$_POST["team_name"]));
				$team_exists = iterator_to_array($team_exists);
				if(count($team_exists)) { $_team_exists = true; }
				else if(!count($_POST["team_name"])) $team_null = true;
				else {
					$_team_exists = false;
					$thankyou = true;
					$db->go_server->teams->insert(array(
						"team_name"=>$_POST["team_name"], 
						"names"=>$_POST["names"], "student_ids"=>$_POST["student_ids"],
						"email"=>$_POST["email"],
						"password"=>hash('sha256', $_POST["password"])
					));
				}
			} else { 
				if(!$email_valid) $_SESSION['valid_email'] = "EMAIL_INVALID";
				if($_POST["name"] == "") $_SESSION['valid_name'] = "NAME_INVALID";
			}

			$_SESSION["email"] = $_POST["email"];
			$_SESSION["names"] = $_POST["names"];
			$_SESSION["student_ids"] = $_POST["student_ids"];
		}
	}
?>

<html>
<head>
<meta charset="utf-8">
<link href="css/style.css" rel='stylesheet' type='text/css' />
<meta name="viewport" content="width=device-width, initial-scale=1">
<script type="application/x-javascript"> addEventListener("load", function() { setTimeout(hideURLbar, 0); }, false); function hideURLbar(){ window.scrollTo(0,1); } </script>
<!--webfonts-->
<link href='http://fonts.googleapis.com/css?family=Oxygen:400,300,700' rel='stylesheet' type='text/css'>
<!--//webfonts-->
</head>
<body>
<div class="main">
		<?php if(!$thankyou) { ?>
			<h1 style="text-align:center;color:white;">Welcome to the Go Server registration page </h1>
		<?php } ?>
		<br>
		<?php if($_team_exists) { ?>
			<h1 style="text-align:center; color:white;">We are sorry, but this team already exists!</h1> 
		<?php } ?>
		<?php if($_team_null) { ?>
			<h1 style="text-align:center; color:white;">Please enter a valid team name.</h1> 
		<?php } ?>
		<?php if(!$email_valid) { ?>
			<h1 style="text-align:center; color:white;">Please enter a valid email</h1>
		<?php } ?>
		<?php if(!$valid_captcha) { ?>
			<h1 style="text-align:center; color:white;">Captcha is invalid!  Please try again.</h1> 
		<?php } ?>
		<?php if($thankyou) { ?>
			<h1 style="text-align:center; color:white;">Thankyou for registering your team!  Your details are recorded, and we look forward to seeing you</h1> 
		<?php } else {?>

		<form action="<?php echo $_SERVER['SELF'];?>" method="post">
		   <div class="lable">
		        <div class="col_1_of_2 span_1_of_2">	<input type="text" class="text" value="Each name" onfocus="this.value = '';" onblur="if (this.value == '') {this.value = 'Each name';}" id="" name="names"></div>
                <div class="col_1_of_2 span_1_of_2"><input type="text" class="text" value="Student IDs" onfocus="this.value = '';" onblur="if (this.value == '') {this.value = 'Student IDs';}" name="student_ids"></div>
                <div class="clear"> </div>
		   </div>
		   <div class="lable-2">

			<input name="team_name" type="text" class="text" value="Team Name " onfocus="this.value = '';" onblur="if (this.value == '') {this.value = 'Team Name ';}">

		   </div>
		   <div class="lable-2">

		        <input name="email" type="text" class="text" value="your@email.com " onfocus="this.value = '';" onblur="if (this.value == '') {this.value = 'your@email.com ';}">
		        <input name="password" type="password" class="text" value="Password " onfocus="this.value = '';" onblur="if (this.value == '') {this.value = 'Password ';}">

		   </div>
		<div class="">
			<div align="center">
			<div style="display:inline-block; position:relative; padding-right:10px;padding-top:-20px;">
				<h3 class="control-label">Enter the captcha code here :</h3>
				<input style="width:200px;padding-bottom:20px;" id="6_letters_code" class="form-control" name="6_letters_code" type="text">
			</div>
			<div style="display:inline-block;position:relative"><br><img style="height:70px;width:200px;border-style:dotted;border-color:black;" src="captcha_code_file.php?rand=<?php echo rand(); ?>" id="captchaimg"></div>
			</div>
		</div>
		<?php if(isset($_SESSION["captcha"]) && $_SESSION["captcha"] == "CODE_NO_MATCH") { $_SESSION["captcha"]  = "N/A"; ?>
				<div class="alert alert-danger">
					<br><h1 style="padding-left:50px;color:white"> ERROR!  Captcha codes do not match, please try again.</h1>
				</div>
		<?php } ?>
		   <div style="display:inline-block;padding-left:70px;" align="center"><h3 style="text-align:center">By registering, you agree to our <span class="term"><a href="terms.html">Terms & Conditions</a></span></h3></div>
		   <div style="padding-left:-20px;" class="submit">
			  <input type="submit" onclick="myFunction()" value="Create account" >
		   </div>
		   <div class="clear"> </div>
		</form>
		<?php } ?>
		<!-----//end-main---->
		</div>
</body>
</html>
