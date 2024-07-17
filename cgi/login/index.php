<?php
/**
 * index.php
 * @since 2018/09/18
 */
ini_set('display_errors', true);
error_reporting(E_ALL);

session_start();

require 'database.php';

// エラーを格納する変数
$err = [];

// 「ログイン」ボタンが押されて、POST通信のとき
if (filter_input(INPUT_SERVER, 'REQUEST_METHOD') === 'POST') {
    $user_name = filter_input(INPUT_POST, 'user_name');
    $password = filter_input(INPUT_POST, 'password');

    if ($user_name === '') {
        $err['user_name'] = 'ユーザー名は入力必須です。';
    }
    if ($password === '') {
        $err['password'] = 'パスワードは入力必須です。';
    }

    // エラーがないとき
    if (count($err) === 0) {

        // DB接続
        $pdo = connect();

        // ステートメント
        $stmt = $pdo->prepare('SELECT * FROM User WHERE user_name = ?');

        // パラメータ設定
        $params = [];
        $params[] = $user_name;

        // SQL実行
        $stmt->execute($params);

        // レコードセットを取得
        $rows = $stmt->fetchAll();

        // パスワード検証
        foreach ($rows as $row) {
            $password_hash = $row['password'];

            // パスワード一致
            if (password_verify($password, $password_hash)) {
                session_regenerate_id(true);
                $_SESSION['login_user'] = $row;
                header('Location:main.php');
                return;
            }
        }
        $err['login'] = 'ログインに失敗しました。';
    }
}
?>
<!DOCTYPE HTML>
<html lang="ja">
    <head>
        <meta charset="UTF-8">
        <title>ログイン</title>
        <style type="text/css">
            .error {
                color: red;
            }
        </style>
    </head>
    <body>
        <div id="wrapper">
            <form action="" method="post">
                <?php if (isset($err['login'])) : ?>
                    <p class="error"><?php echo h($err['login']); ?></p>
                <?php endif; ?>
                <p>
                    <label for="user_name">ユーザー名</label>
                    <input id="user_id" name="user_name" type="text" />
                    <?php if (isset($err['user_name'])) : ?>
                        <p class="error"><?php echo h($err['user_name']); ?></p>
                    <?php endif; ?>
                </p>
                <p>
                    <label for="">パスワード</label>
                    <input id="password" name="password" type="password" />
                    <?php if (isset($err['password'])) : ?>
                        <p class="error"><?php echo h($err['password']); ?></p>
                    <?php endif; ?>
                </p>
                <p>
                    <button type="submit">ログイン</button>
                </p>
                <p>
                    <a href="adduser.php">新規ユーザー登録</a>
                </p>
            </form>
        </div>
    </body>
</html>
