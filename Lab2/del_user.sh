#!/bin/bash

# Запуск: sudo ./del_user.sh <имя_пользователя>

if [ "$EUID" -ne 0 ]; then
    echo "Запусти от root: sudo $0"
    exit 1
fi

if [ -z "$1" ]; then
    echo "Использование: sudo $0 <имя_пользователя>"
    exit 1
fi

USERNAME=$1
HOME_DIR=/var/www/$USERNAME
DOMAIN="${USERNAME}.ru"

if [ ! -d "$HOME_DIR" ]; then
    echo "Ошибка: пользователь $USERNAME не найден в системе хостинга"
    exit 1
fi

# Виртуальный хост
a2dissite $DOMAIN.conf 2>/dev/null
rm -f /etc/apache2/sites-available/$DOMAIN.conf
systemctl reload apache2

# Файлы сайта
rm -rf $HOME_DIR

# База данных и пользователь MySQL
mysql -u root <<EOF
DROP DATABASE IF EXISTS \`$USERNAME\`;
DROP USER IF EXISTS '$USERNAME'@'localhost';
FLUSH PRIVILEGES;
EOF

# Системный пользователь
userdel -r $USERNAME 2>/dev/null
groupdel $USERNAME 2>/dev/null

echo ""
echo "Готово! Пользователь $USERNAME полностью удалён"