<?php 
	session_start();
	$db = new MongoClient("mongodb://terminator:hrsyqrzjcxfuuagwxrheksvyzcsfmqjg@localhost:27017");


	if(isset($_SESSION["captcha"])) if($_SESSION['captcha'] != "CODE_NO_MATCH" || $_SESSION['captcha'] != "CODE_NO_MATCH") {
		$_SESSION["first_name"] = "";
		$_SESSION["last_name"] = "";
		$_SESSION["team_name"] = "";
		$_SESSION["email"] = "";
		$_SESSION["password"] = "";
	}
	if(isset($_POST["email"])) {
		var_dump($_SESSION['6_letters_code']);
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
			$email_valid = filter_var($_POST["email"], FILTER_VALIDATE_EMAIL);
			if($email_valid) {
				echo "THANKYOU WORKED";
				$_SESSION['captcha'] = "CODE_MATCH";
				$team_exists = $db->go_server->teams->find(array("team_name"=>$_POST["team_name"]));
				$team_exists = iterator_to_array($team_exists);
				if(count($team_exists)) { echo "SORRY BUT TEAM ALREADY EXISTS!"; }
				else {
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

