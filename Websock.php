<?php
require 'vendor/autoload.php';
use Ratchet\MessageComponentInterface;
use Ratchet\ConnectionInterface;
class  MyWebsocket implements MessageComponentInterface{
 public $clients;
 private $connectedClients;
    public function __construct()
    {
        $this->clients = new \SplObjectStorage();
        $this->connectedClients = [];
    }
    public function onOpen(ConnectionInterface $con)
    {
      $this ->clients ->attach($con);
      $this -> connectedClients [$con -> resourceId] = $con;
      echo "New connection ({$con->resourceId})";
      $con->send("Welcome to the server\n");
    }

    function onClose(ConnectionInterface $conn)
    {
    echo "Connection Closed ({$conn->resourceId})\n";
    $conn->close();
    }

    function onError(ConnectionInterface $conn, \Exception $e)
    {
        echo "An error occurred: ". $e->getMessage() . "\n";
        $conn->close();
    }

    function onMessage(ConnectionInterface $from, $msg)
    {
        echo $msg . "\n";
      foreach ($this->connectedClients as $client){
        $client->send($msg);
      }
    }
}
$app = new Ratchet\App("192.168.1.94", 81, '0.0.0.0');
$app ->route('/',new MyWebsocket, array('*'));
$app ->run();
?>
