<?php
/**
 * main.php
 *
 * @since 2018/09/18
 */
session_start();

require 'database.php';
$login_user = $_SESSION['login_user'];
?>
<!DOCTYPE HTML>
<html lang="ja">
    <head>
        <meta charset="UTF-8">
        <title></title>
    </head>
    <body>
        <?php foreach ($login_user as $key => $val) : ?>
            <p><?php echo h($key); ?> : <?php echo h($val); ?></p>
        <?php endforeach; ?>
    </body>
</html>
