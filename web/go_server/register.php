<html>

<?php 
	$db = new MongoClient("mongodb://terminator:hrsyqrzjcxfuuagwxrheksvyzcsfmqjg@localhost:27017");

	$db->go_server->teams->insert(array("THIS IS A TEST"=>"HELLO WORLD"));
?>

</html>
