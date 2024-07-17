<?php
/**
 * adduser.php
 *
 * @since 2018/09/21
 */
ini_set('display_errors', true);
error_reporting(E_ALL);

session_start();

require 'database.php';

$err = [];

// 「ログイン」ボタンが押されて、POST通信のとき
if (filter_input(INPUT_SERVER, 'REQUEST_METHOD') === 'POST') {
    $user_name = filter_input(INPUT_POST, 'user_name');
    $password = filter_input(INPUT_POST, 'password');
    $password_conf = filter_input(INPUT_POST, 'password_conf');

    if ($user_name === '') {
        $err['user_name'] = 'ユーザー名は入力必須です。';
    }
    if ($password === '') {
        $err['password'] = 'パスワードは入力必須です。';
    }
    if ($password !== $password_conf) {
        $err['password_conf'] = 'パスワードが一致しません。';
    }

    // エラーがないとき
    if (count($err) === 0) {

        // DB接続
        $pdo = connect();

        // ステートメント
        $stmt = $pdo->prepare('INSERT INTO `User` (`id`, `user_name`, `password`) VALUES (null, ?, ?)');

        // パラメータ設定
        $params = [];
        $params[] = $user_name;
        $params[] = password_hash($password, PASSWORD_DEFAULT);

        // SQL実行
        $success = $stmt->execute($params);
    }
}
?>
<!DOCTYPE HTML>
<html lang="ja">
    <head>
        <meta charset="UTF-8">
        <title></title>
        <style type="text/css">
            .error {
                color: red;
            }
        </style>
    </head>
    <body>
        <?php if (count($err) > 0) : ?>
            <?php foreach ($err as $e): ?>
                <p class="error"><?php echo h($e); ?></p>
            <?php endforeach; ?>
        <?php endif; ?>

        <?php if (isset($success) && $success) : ?>
            <p>登録に成功しました。</p>
            <p><a href="index.php">こちらからログインしてください。</a></p>
        <?php else: ?>
            <form action="" method="post">
                <p>
                    <label for="user_name">ユーザー名</label>
                    <input id="user_id" name="user_name" type="text" />
                </p>
                <p>
                    <label for="">パスワード</label>
                    <input id="password" name="password" type="password" />
                </p>
                <p>
                    <label for="">確認用パスワード</label>
                    <input id="password_conf" name="password_conf" type="password" />
                </p>
                <p>
                    <button type="submit">登録</button>
                </p>
            </form>
        <?php endif; ?>
    </body>
</html>
