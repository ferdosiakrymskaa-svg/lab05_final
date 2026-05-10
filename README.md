# Лабораторная работа 5
[![Coverage Status](https://coveralls.io/repos/github/ferdosiakrymskaa-svg/lab05_final/badge.svg?branch=main)](https://coveralls.io/github/ferdosiakrymskaa-svg/lab05_final?branch=main)

## В данной лабораторной работе нам предстояло изучить фреймворки для тестирования на примере `GTest`.

###  Разберем, что в себя включает каждый файл данной лабораторной работы: 

## 1 `.github/workflows/ci.yml`
Полное содержимое файла:
```sh
name: CI

on: [push, pull_request]

jobs:
  build-and-test:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v3
        with:
          submodules: true
      - name: Install lcov
        run: sudo apt-get install -y lcov
      - name: Configure
        run: cmake -H. -B build -DBUILD_TESTS=ON -DCOLLECT_COVERAGE=ON
      - name: Build
        run: cmake --build build
      - name: Test
        run: ./build/check
      - name: Coverage
        run: |
          lcov --capture --directory build --output-file coverage_raw.info --ignore-errors mismatch,unexecuted
          lcov --extract coverage_raw.info '*/banking/Account.cpp' '*/banking/Transaction.cpp' --output-file coverage.info
          lcov --remove coverage.info '/usr/*' --output-file coverage.info --ignore-errors unused
          lcov --list coverage.info
      
      - name: Coveralls
        uses: coverallsapp/github-action@v2
        with:
          github-token: ${{ secrets.GITHUB_TOKEN }}
          file: coverage.info
```
`name: CI` задает имя исполняемого процесса<br>
`on: [push, pull_request]` триггерится, когда совершает пуш или пулл-реквест в ветку
```sh
jobs:
  build-and-test:
    runs-on: ubuntu-latest
```
Имя сборки на последней версии убунту `ubuntu-latest` - `build-and-test`
```sh
steps:
      - uses: actions/checkout@v3
        with:
          submodules: true
      - name: Install lcov
        run: sudo apt-get install -y lcov
      - name: Configure
        run: cmake -H. -B build -DBUILD_TESTS=ON -DCOLLECT_COVERAGE=ON
      - name: Build
        run: cmake --build build
      - name: Test
        run: ./build/check
```
Шаги: используем `actions/checkout@v3` с сабмодулями, далее утснавливаем `lcov` командой `sudo apt-get install -y lcov`, далее выпоняем компиляцию проекта командой `cmake -H. -B build -DBUILD_TESTS=ON -DCOLLECT_COVERAGE=ON`, где `-DBUILD_TESTS=ON -DCOLLECT_COVERAGE=ON` способ включить возможности, которые по умолчанию отключены.
Потом билдим и запускаем файл командами `cmake --build build` и `./build/check`
```sh
 - name: Coverage
        run: |
          lcov --capture --directory build --output-file coverage_raw.info --ignore-errors mismatch,unexecuted
          lcov --extract coverage_raw.info '*/banking/Account.cpp' '*/banking/Transaction.cpp' --output-file coverage.info
          lcov --remove coverage.info '/usr/*' --output-file coverage.info --ignore-errors unused
          lcov --list coverage.info
      
      - name: Coveralls
        uses: coverallsapp/github-action@v2
        with:
          github-token: ${{ secrets.GITHUB_TOKEN }}
          file: coverage.info
```
`lcov --capture` собирает данные о выполнении кода из папки build. Игнорируем ошибки `mismatch` и `unexecuted` (они возникают в тестовых макросах, не влияют на наши файлы).<br>
`lcov --extract` оставляет в `coverage.info` только информацию о `Account.cpp` и `Transaction.cpp`.<br>
`lcov --remove` убирает системные заголовки `(/usr/*)`, если вдруг попали.<br>
`lcov --list` печатает краткий отчёт — видишь итоговые проценты.<br>
`github-token: ${{ secrets.GITHUB_TOKEN }}` использует секретный токен, выдаваемый на сайте `Coveralls`. Далее идет отправка

## 2 Корневой `CMakeLists.txt`

Содержимое целиком:
```sh
cmake_minimum_required(VERSION 3.10)
project(lab05)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(BUILD_TESTS "Build tests" OFF)
option(COLLECT_COVERAGE "Collect coverage" OFF)


add_subdirectory(banking)

if(BUILD_TESTS)
  enable_testing()
  add_subdirectory(third-party/gtest)

  file(GLOB TEST_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/tests/*.cpp")
  add_executable(check ${TEST_SOURCES})
  target_link_libraries(check banking gtest_main gmock_main)

  if(COLLECT_COVERAGE)
    target_link_libraries(check gcov)
    target_compile_options(check PRIVATE -O0 -g --coverage)
    target_link_options(check PRIVATE --coverage)
  endif()

  add_test(NAME check COMMAND check)
endif()
```
Базовые состовляющие `CMakeLists.txt`:
```sh
cmake_minimum_required(VERSION 3.10)
project(lab05)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
```

```sh
option(BUILD_TESTS "Build tests" OFF)
option(COLLECT_COVERAGE "Collect coverage" OFF)
```
Данные опции изначально выключаются, чтобы затем включить их в командной строке во время сборки
```sh
if(BUILD_TESTS)
  enable_testing()
  add_subdirectory(third-party/gtest)

  file(GLOB TEST_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/tests/*.cpp")
  add_executable(check ${TEST_SOURCES})
  target_link_libraries(check banking gtest_main gmock_main)

  if(COLLECT_COVERAGE)
    target_link_libraries(check gcov)
    target_compile_options(check PRIVATE -O0 -g --coverage)
    target_link_options(check PRIVATE --coverage)
  endif()

  add_test(NAME check COMMAND check)
endif()
```
Если тесты включены, разрешаем CTest (`enable_testing()`) и подключаем `Google Test` (подмодуль).
```sh
  file(GLOB TEST_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/tests/*.cpp")
  add_executable(check ${TEST_SOURCES})
```
Собирает все файлы *.cpp в исполняемый файл `check`
```sh
  if(COLLECT_COVERAGE)
    target_link_libraries(check gcov)
    target_compile_options(check PRIVATE -O0 -g --coverage)
    target_link_options(check PRIVATE --coverage)
  endif()
```
Если нужен сбор покрытия, добавляем флаги `--coverage` и линкуем `gcov`.`-O0 -g` — без оптимизаций, с отладочной информацией.
``
``
``
``
``
``
