#!/bin/bash

PACKAGE_NAME="dping-package"
DEB_DIR="$PACKAGE_NAME"
BIN_DIR="$DEB_DIR/usr/local/bin"

echo "Компиляция проекта..."
make || { echo "Ошибка компиляции"; exit 1; }

echo "Перемещение файла в директорию пакета..."
cp "dping" "$BIN_DIR/"

echo "Создание .deb пакета..."
dpkg-deb --build "$DEB_DIR" || { echo "Ошибка при создании .deb пакета"; exit 1; }

echo "Пакет создан успешно: $PACKAGE_NAME.deb"
