<?php
$hostname ="localhost";
$username ="root";
$password ="";
$database ="CMUProject";
$conn = mysqli_connect($hostname,$username,$password,$database);
if (!$conn){
    die("Connection Failed: " .mysqli_connect_error());

}
echo "database Connection OKAY";
if(isset($_POST["temperature"]) && isset($_POST["humidity"])) {

    $t = $_POST["temperature"];
    $h = $_POST["humidity"]; 

    $sql = "INSERT INTO first_table (Temperature, humidity) VALUES (".$t.", ".$h.")";

    if (mysqli_query($conn, $sql)) {
        echo "\nNew record created successfully";
    } else {
        echo "Error: " . $sql . "<br>" . mysqli_error($conn);
    }
}

?>