#!/bin/bash

# Запуск: sudo ./add_user.sh <имя_пользователя> <пароль>

if [ "$EUID" -ne 0 ]; then
    echo "Запусти от root: sudo $0"
    exit 1
fi

if [ -z "$1" ] || [ -z "$2" ]; then
    echo "Использование: sudo $0 <имя_пользователя> <пароль>"
    exit 1
fi

USERNAME=$1
PASSWORD=$2
HOME_DIR=/var/www/$USERNAME
DOMAIN="${USERNAME}.ru"

if id "$USERNAME" &>/dev/null; then
    echo "Ошибка: пользователь $USERNAME уже существует"
    exit 1
fi

# Системный пользователь
useradd -m -d $HOME_DIR -s /bin/bash $USERNAME
echo "$USERNAME:$PASSWORD" | chpasswd

# Папка сайта и тестовая страница
mkdir -p $HOME_DIR/public_html
echo "<?php echo '<h1>Добро пожаловать, $USERNAME!</h1>'; ?>" > $HOME_DIR/public_html/index.php
chmod 755 $HOME_DIR
chmod 755 $HOME_DIR/public_html
chmod 644 $HOME_DIR/public_html/index.php
chown -R $USERNAME:$USERNAME $HOME_DIR

# База данных, пользователь MySQL, таблица
mysql -u root <<EOF
CREATE DATABASE IF NOT EXISTS \`$USERNAME\`;
CREATE USER IF NOT EXISTS '$USERNAME'@'localhost' IDENTIFIED BY '$PASSWORD';
GRANT ALL PRIVILEGES ON \`$USERNAME\`.* TO '$USERNAME'@'localhost';
USE \`$USERNAME\`;
CREATE TABLE IF NOT EXISTS users (
    id INT AUTO_INCREMENT PRIMARY KEY,
    username VARCHAR(50) NOT NULL,
    email VARCHAR(100),
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);
FLUSH PRIVILEGES;
EOF

# Виртуальный хост
tee /etc/apache2/sites-available/$DOMAIN.conf > /dev/null <<EOF
<VirtualHost *:80>
    ServerName $DOMAIN
    DocumentRoot $HOME_DIR/public_html

    <Directory $HOME_DIR/public_html>
        Options Indexes FollowSymLinks
        AllowOverride All
        Require all granted
    </Directory>

    ErrorLog \${APACHE_LOG_DIR}/${USERNAME}_error.log
    CustomLog \${APACHE_LOG_DIR}/${USERNAME}_access.log combined
</VirtualHost>
EOF

a2ensite $DOMAIN.conf
apache2ctl configtest
systemctl reload apache2

echo ""
echo "Готово!"
echo "Пользователь:      $USERNAME"
echo "Директория сайта:  $HOME_DIR/public_html"
echo "Виртуальный хост:  http://$DOMAIN"
echo "База данных MySQL: $USERNAME"