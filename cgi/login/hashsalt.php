<?php
$options = [
    'cost' => 10,
    'salt' => 'AFHFBDX6pVTdX2CZ6IO0IO',
];
echo password_hash("password", PASSWORD_BCRYPT, $options);
?>
