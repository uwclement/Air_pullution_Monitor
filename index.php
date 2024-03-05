
<html>
<head>
    <title>
        CMUProject
    </title>
</head>
<body>
<h4>Temperature : <span id="temperature" >0 C</span></h4>
<h4> Humidity :<span id="humidity"> 0</span></h4>
<button id="ledButton" onclick="toggle()">OFF</button>
<script>
   var socket = new WebSocket('ws://192.168.1.94:81');
   socket.onmessage = function (event){
       console.log(event.data);
       const data = event.data.split (":");
       const msg =data [0] || "";
       const sensor =data[1] || "";
       if (sensor == "led"){
           var button =document.getElementById("ledButton");
           button.innerHTML = msg =="1" ? "ON" : "OFF";
       }
      // for DHT temperature and Humidity
       else if(sensor == "dht"){
        var parts = msg.split(",");
        var temperature = parts[0];
        var humidity = parts[1];

        document.getElementById("temperature").innerHTML = temperature;
        document.getElementById("humidity").innerHTML = humidity + "%";
       }
   }
   function toggle(){
       var button = document.getElementById("ledButton");
       var status = button.innerHTML == "OFF" ? "1" : "0";
        socket.send(status + ":led:esp:localhost")
   }
</script>
</body>
</html>
<?php
$hostname = "localhost";
$username = "root";
$password = "";
$database = "CMUProject";

$conn = mysqli_connect($hostname, $username, $password, $database);
if (!$conn) {
    die("Connection failed: " . mysqli_connect_error());
}
echo "Database Connection OKAY";

$dataPoints1 = array();
$dataPoints2 = array();

$sql_select = "SELECT Temperature, humidity, time FROM first_table";
$result = mysqli_query($conn, $sql_select);

if (mysqli_num_rows($result) > 0) {
    while ($row = mysqli_fetch_assoc($result)) {
        $dataPoints1[] = array("label" => $row["time"], "y" => $row["Temperature"]);
        $dataPoints2[] = array("label" => $row["time"], "y" => $row["humidity"]);
    }
} else {
    echo "0 results";
}

mysqli_close($conn);
?>

<!DOCTYPE HTML>
<html>
<head>
    <script src="https://canvasjs.com/assets/script/canvasjs.min.js"></script>
    <script>
        window.onload = function () {
            var chart = new CanvasJS.Chart("chartContainer", {
                animationEnabled: true,
                theme: "light2",
                title: {
                    text: "Temperature & Humidity Statistics"
                },
                axisY: {
                    includeZero: true
                },
                legend: {
                    cursor: "pointer",
                    verticalAlign: "center",
                    horizontalAlign: "right",
                    itemclick: toggleDataSeries
                },
                data: [
                    {
                        type: "column",
                        name: "Temperature",
                        indexLabel: "{y}",
                        yValueFormatString: "#0.##°C",
                        showInLegend: true,
                        dataPoints: <?php echo json_encode($dataPoints1, JSON_NUMERIC_CHECK); ?>
                    },
                    {
                        type: "column",
                        name: "Humidity",
                        indexLabel: "{y}",
                        yValueFormatString: "#0.## g.m-3",
                        showInLegend: true,
                        dataPoints: <?php echo json_encode($dataPoints2, JSON_NUMERIC_CHECK); ?>
                    }
                ]
            });
            chart.render();

            function toggleDataSeries(e) {
                if (typeof (e.dataSeries.visible) === "undefined" || e.dataSeries.visible) {
                    e.dataSeries.visible = false;
                } else {
                    e.dataSeries.visible = true;
                }
                chart.render();
            }
        }
    </script>
</head>
<body>
<div id="chartContainer" style="height: 370px; width: 100%;"></div>
</body>
</html>
