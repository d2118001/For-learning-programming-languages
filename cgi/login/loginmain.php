<?php
session_start();
$login_user = $_SESSION['login_user'];
?>
<!DOCTYPE HTML>
<html lang="ja">
    <head>
        <meta charset="UTF-8">
        <title></title>
    </head>
    <body>
        <p><?php echo $login_user; ?>さんこんにちは</p>
    </body>
</html>
