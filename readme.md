# Proin &mdash; [Pro]cess [In]formation

Сборка модуля ядра:
```sh
make proin
```

Сборка клиентской программы:
```sh
make proinya
```

Выполнение модуля:
```sh
indmod proin.ko identifier_process=n
```

Выгрузка модуля:
```sh
rmmod proin
```

Запуск клиентской программы:
```sh
./proinya n_1 ... n_k
```

Модуль записывает ответ в файл "/proc/proin_response". Клиентская программа считывает ответ из этого файла.
