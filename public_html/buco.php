<? 
// Author : Rakusai
// Print all environment value

?>
 <html>
<head><title>ファイルアップロード</title></head>
<body><form enctype="multipart/form-data" action="" method="post">
    Name:<input type="text" name="name"><br>
    Speed:<input type="text" name="speed"><br>
    Image:<input type="file" name="data">
    <input type="submit" value="Upload">
</form>
 </body>
</html>
<?


if (@$_FILES['data']['tmp_name']){

	$name = $_POST["name"];

	$filename = "/home/rakusai/public_html/buco/$name.txt";
	$fp=fopen($filename, "w");
	fputs($fp, $_POST["speed"]);
	fclose($fp);
	
	$filename = "/home/rakusai/public_html/buco/$name.png";
	
	move_uploaded_file($_FILES['data']['tmp_name'], $filename);
	
?>	success

<?
}

print_r($_SERVER);
print_r($_POST);
print_r($_GET);


?>