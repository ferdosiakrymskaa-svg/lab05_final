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
Если нужен сбор покрытия, добавляем флаги `--coverage` и линкуем `gcov`.`-O0 -g` — без оптимизаций, с отладочной информацией.<br>
`add_test(NAME check COMMAND check)`<br>
Регистрируем тест с именем check, который будет запускать исполняемый файл check. Это позволяет использовать ctest.<br>
`endif()`<br>
Закрываем условие `BUILD_TESTS`.<br>

## 3`banking/CMakeLists.txt`
Содержимое файла:
```sh
cmake_minimum_required(VERSION 3.10)
project(banking)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(banking STATIC Account.cpp Transaction.cpp)
target_include_directories(banking PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

if(COLLECT_COVERAGE)
  target_compile_options(banking PRIVATE -O0 -g --coverage)
  target_link_options(banking PRIVATE --coverage)
endif()
```
Все команды по стандарту, кроме:
```sh
if(COLLECT_COVERAGE)
  target_compile_options(banking PRIVATE -O0 -g --coverage)
  target_link_options(banking PRIVATE --coverage)
endif()
```
Аналогично корневому CMakeLists: добавляем флаги покрытия для самой библиотеки.

## 4 `banking/Account.h`
```sh
#pragma once
```
Защита от повторного включения файла.

```sh
class Account {
 public:
  Account(int id, int balance);
  virtual ~Account();
```
Конструктор с параметрами, виртуальный деструктор (чтобы корректно удалять наследников).

```sh
  // Virtual to test.
  virtual int GetBalance() const;
  virtual void ChangeBalance(int diff);
  virtual void Lock();
  virtual void Unlock();
```
Объявление виртуальных методов — это позволяет создавать Mock-объекты (переопределять их в тестах).

```sh
  int id() const { return id_; }
```
Невиртуальный метод — возвращает идентификатор счёта. Определён прямо в заголовке (inline).

```sh
 private:
  int id_;
  int balance_;
  bool is_locked_;
};
```
Приватные поля: номер счёта, баланс, флаг блокировки.

## 5 `banking/Account.cpp`
```sh
#include "Account.h"
#include <stdexcept>
```
Подключаем свой заголовок и стандартное исключение.
```sh
Account::Account(int id, int balance)
    : id_(id), balance_(balance), is_locked_(false) {}
```
Конструктор инициализирует поля. Счёт изначально не заблокирован.
```sh
Account::~Account() {}
```
Деструктор (пустой, но виртуальность важна).
```sh
int Account::GetBalance() const { return balance_; }
```
Просто возвращает баланс.
```sh
void Account::ChangeBalance(int diff) {
  if (!is_locked_) throw std::runtime_error("at first lock the account");
  balance_ += diff;
}
```
Изменить баланс можно только после блокировки (`Lock`). Иначе — исключение.
```sh
void Account::Lock() {
  if (is_locked_) throw std::runtime_error("already locked");
  is_locked_ = true;
}
```
Блокировка: если уже заблокирован — ошибка, иначе ставим `true`.
```sh
void Account::Unlock() { is_locked_ = false; }
```
Просто снимаем блокировку.

## 6 `banking/Transaction.h`
```sh
#pragma once
class Account;
```
Предварительное объявление класса Account (чтобы не подключать весь заголовок).
```csh
class Transaction {
 public:
  Transaction();
  virtual ~Transaction();
```
Конструктор и виртуальный деструктор.
```sh
  bool Make(Account& from, Account& to, int sum);
```
Основной метод перевода денег.
```sh
  int fee() const { return fee_; }
  void set_fee(int fee) { fee_ = fee; }
```
Геттер и сеттер для комиссии.
```sh
 private:
  void Credit(Account& accout, int sum);
  bool Debit(Account& accout, int sum);
```
Внутренние методы: зачисление и списание. Не виртуальные.

```sh
  // Virtual to test.
  virtual void SaveToDataBase(Account& from, Account& to, int sum);
```
Виртуальный метод сохранения транзакции. Его будем мокать.

```sh
  int fee_;
};
```
Хранит размер комиссии.
## 7 `banking/Transaction.cpp`
```sh
#include "Transaction.h"
#include <cassert>
#include <iostream>
#include <stdexcept>
#include "Account.h"
```
Подключение необходимых заголовков.
```sh
namespace {
struct Guard {
  Guard(Account& account) : account_(&account) { account_->Lock(); }
  ~Guard() { account_->Unlock(); }
 private:
  Account* account_;
};
}
```
Внутренний класс `Guard` — реализует идиому RAII. При создании блокирует счёт, при уничтожении разблокирует. Это гарантирует разблокировку даже при исключениях.

```sh
Transaction::Transaction() : fee_(1) {}
Transaction::~Transaction() {}
```
Конструктор устанавливает комиссию по умолчанию = 1.
```sh
bool Transaction::Make(Account& from, Account& to, int sum) {
  if (from.id() == to.id()) throw std::logic_error("invalid action");
  if (sum < 0) throw std::invalid_argument("sum can't be negative");
  if (sum < 100) throw std::logic_error("too small");
  if (fee_ * 2 > sum) return false;
```
Проверки: нельзя переводить самому себе, сумма не отрицательна, минимальная сумма 100, если комиссия слишком велика — транзакция невозможна (возвращает false, исключения нет).

```sh
  Guard guard_from(from);
  Guard guard_to(to);
```
Блокируем оба счёта (через Guard). Теперь они защищены от параллельных изменений.
```sh
  Credit(to, sum);
```
Зачисляем сумму получателю.

```sh
  bool success = Debit(from, sum + fee_);
```
Пытаемся списать с отправителя сумму + комиссию. Возвращает true, если хватило средств.

```sh
  if (!success) to.ChangeBalance(-sum);
```
Если не хватило, откатываем зачисленную сумму у получателя.

```sh
  SaveToDataBase(from, to, sum);
```
Сохраняем информацию о транзакции (в нашем случае — вывод в консоль).

```sh
  return success;
}
```
Возвращаем результат (true — успех, false — неудача из-за недостатка средств).

```sh
void Transaction::Credit(Account& accout, int sum) {
  assert(sum > 0); // LCOV_EXCL_LINE
  accout.ChangeBalance(sum);
}
```
Зачисление: проверяем, что сумма > 0 (assert активен только в отладочной сборке), меняем баланс.

```sh
bool Transaction::Debit(Account& accout, int sum) {
  assert(sum > 0); // LCOV_EXCL_LINE
  if (accout.GetBalance() > sum) {
    accout.ChangeBalance(-sum);
    return true;
  }
  return false;
}
```
Списание: если баланс больше списываемой суммы, уменьшаем баланс и возвращаем true. Иначе false.

```sh
void Transaction::SaveToDataBase(Account& from, Account& to, int sum) {
  std::cout << from.id() << " send to " << to.id() << " $" << sum << std::endl;
  ...
}
```
Вывод информации о транзакции. Строки `assert` исключены из покрытия с помощью `// LCOV_EXCL_LINE`, потому что они не выполняются при обычной работе (assert всегда истинен, но lcov считает строку не покрытой из-за того, что при успехе она не «проходится» в бинарном смысле).

## 8 `tests/mock_classes.h`
```sh
#pragma once
#include <gmock/gmock.h>
#include "Account.h"
#include "Transaction.h"
```
Подключаем Google Mock и наши классы.

```sh
class MockAccount : public Account {
public:
    MockAccount(int id, int balance) : Account(id, balance) {}
    MOCK_METHOD(int, GetBalance, (), (const, override));
    MOCK_METHOD(void, ChangeBalance, (int diff), (override));
    MOCK_METHOD(void, Lock, (), (override));
    MOCK_METHOD(void, Unlock, (), (override));
};
```
Мок-класс для `Account`.  
- `MOCK_METHOD` — макрос, создающий заглушку с заданной сигнатурой.  
- Теперь в тестах можно управлять поведением этих методов и проверять, сколько раз они вызваны.

```sh
class MockTransaction : public Transaction {
public:
    MOCK_METHOD(void, SaveToDataBase, (Account& from, Account& to, int sum), (override));
};
```
Мок для `Transaction` — подменяет только виртуальный `SaveToDataBase`, чтобы не печатать в консоль и проверять вызовы.

---

## 9 `tests/test_account.cpp`
Разберем по блокам.
```sh
#include <gtest/gtest.h>
#include "Account.h"
```
Заголовки Google Test и наш класс.

Параметризованный тест:
```sh
class AccountBalanceTest : public testing::TestWithParam<int> {
public:
    Account* acc;
    void SetUp() override { acc = new Account(1, GetParam()); }
    void TearDown() override { delete acc; }
};
```
Фикстура-класс, параметризованный значением баланса (int).  
- `SetUp` создаёт счёт с переданным параметром.  
- `TearDown` удаляет.

```sh
TEST_P(AccountBalanceTest, InitialBalance) {
    EXPECT_EQ(acc->GetBalance(), GetParam());
}
```
Тело теста: проверяем, что баланс соответствует параметру.

```sh
INSTANTIATE_TEST_SUITE_P(
    DifferentBalances,
    AccountBalanceTest,
    testing::Values(100, 0, 1000, 5000)
);
```
Создаём четыре экземпляра теста с разными балансами.

Обычная фикстура:
```sh
class AccountBehaviourTest : public testing::Test {
public:
    Account* acc;
    void SetUp() override { acc = new Account(123, 1000); }
    void TearDown() override { delete acc; }
};
```

Тест `LockUnlock`:
```sh
TEST_F(AccountBehaviourTest, LockUnlock) {
    EXPECT_NO_THROW(acc->Lock());          // должно пройти без исключений
    EXPECT_THROW(acc->Lock(), std::runtime_error); // вторая блокировка → ошибка
    acc->Unlock();                         // разблокируем
    EXPECT_NO_THROW(acc->Lock());          // теперь снова можно
}
```

`ChangeBalanceWhenUnlocked`:
```sh
TEST_F(AccountBehaviourTest, ChangeBalanceWhenUnlocked) {
    EXPECT_THROW(acc->ChangeBalance(50), std::runtime_error);
}
```

`ChangeBalanceWhenLocked`:
```sh
TEST_F(AccountBehaviourTest, ChangeBalanceWhenLocked) {
    acc->Lock();
    EXPECT_NO_THROW(acc->ChangeBalance(200));
    EXPECT_EQ(acc->GetBalance(), 1200);
}
```

`IdIsCorrect`:
```sh
TEST_F(AccountBehaviourTest, IdIsCorrect) {
    EXPECT_EQ(acc->id(), 123);
}
```

## 10 `tests/test_transaction.cpp`
```sh
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "Transaction.h"
#include "mock_classes.h"
```
Подключаем Google Test, Google Mock, наш класс и моки.

```sh
using ::testing::StrictMock;
using ::testing::Return;
using ::testing::_;
```
Удобные псевдонимы: `StrictMock` — строгая проверка, `Return` — задаём возвращаемое значение, `_` — любой аргумент.

```sh
class TransactionTest : public testing::Test {
public:
    StrictMock<MockTransaction>* trans;
    StrictMock<MockAccount>* from;
    StrictMock<MockAccount>* to;
    void SetUp() override {
        trans = new StrictMock<MockTransaction>();
        from = new StrictMock<MockAccount>(1, 1000);
        to = new StrictMock<MockAccount>(2, 1000);
    }
    void TearDown() override {
        delete trans;
        delete from;
        delete to;
    }
};
```
Фикстура: создаём экземпляры с `StrictMock` — если вызов метода не будет запланирован через `EXPECT_CALL`, тест упадёт.


Далее идут тесты:

`DefaultFee` — проверяет, что комиссия по умолчанию = 1.  
`SetFee` — задаём 10 и проверяем.  

`SameAccount`: `trans->Make(*from, *from, 200)` — ожидаем `std::logic_error`.

`NegativeSum`: сумма -100 → `std::invalid_argument`.

`TooSmall`: 70 → `std::logic_error`.

`FeeExceeds`: устанавливаем комиссию 60, при сумме 100 транзакция не должна вызывать `SaveToDataBase` и возвращает false.

`Successful`: здесь самая важная логика с `InSequence`.  
```sh
testing::InSequence seq;
```
Задаём, что вызовы должны идти строго в том порядке, в котором мы их ожидаем.
```sh
EXPECT_CALL(*from, Lock());
EXPECT_CALL(*to, Lock());
EXPECT_CALL(*to, ChangeBalance(sum));                // Credit
EXPECT_CALL(*from, GetBalance()).WillOnce(Return(1000));
EXPECT_CALL(*from, ChangeBalance(-(sum + 1)));       // Debit sum+fee
EXPECT_CALL(*trans, SaveToDataBase(_, _, sum));
EXPECT_CALL(*to, Unlock());
EXPECT_CALL(*from, Unlock());
```
Проверяем, что счета блокируются, затем начисление, затем запрос баланса отправителя, затем списание, затем сохранение, затем разблокировка. Итог — `EXPECT_TRUE`.
`NotEnoughFunds`: аналогично, но баланс отправителя меньше, поэтому `Debit` не списывает, а потом вызывается `ChangeBalance(-sum)` у получателя (откат). Результат — `EXPECT_FALSE`.
`RealTransaction`: тест без моков, чтобы реальный `SaveToDataBase` выполнился и код покрылся. Создаются настоящие объекты `Account` и `Transaction`, вызывается `Make` с проверкой на успех.

## Результат работы команд по компиляции и тестрироавнию работы программы:

`cmake -H. -B build -DBUILD_TESTS=ON -DCOLLECT_COVERAGE=ON`:
```sh
-- The C compiler identification is GNU 13.3.0
-- The CXX compiler identification is GNU 13.3.0
-- Detecting C compiler ABI info
-- Detecting C compiler ABI info - done
-- Check for working C compiler: /usr/bin/cc - skipped
-- Detecting C compile features
-- Detecting C compile features - done
-- Detecting CXX compiler ABI info
-- Detecting CXX compiler ABI info - done
-- Check for working CXX compiler: /usr/bin/c++ - skipped
-- Detecting CXX compile features
-- Detecting CXX compile features - done
-- Performing Test CMAKE_HAVE_LIBC_PTHREAD
-- Performing Test CMAKE_HAVE_LIBC_PTHREAD - Success
-- Found Threads: TRUE  
-- Configuring done (1.3s)
-- Generating done (0.0s)
-- Build files have been written to: /home/ilyasov_ilya/ferdosiakrymskaa-svg/workspace/projects/lab05/lab05_final_edit/lab05/build

```

`cmake --build build`:
```sh
[  7%] Building CXX object third-party/gtest/googletest/CMakeFiles/gtest.dir/src/gtest-all.cc.o
[ 14%] Linking CXX static library ../../../lib/libgtest.a
[ 14%] Built target gtest
[ 21%] Building CXX object third-party/gtest/googletest/CMakeFiles/gtest_main.dir/src/gtest_main.cc.o
[ 28%] Linking CXX static library ../../../lib/libgtest_main.a
[ 28%] Built target gtest_main
[ 35%] Building CXX object banking/CMakeFiles/banking.dir/Account.cpp.o
[ 42%] Building CXX object banking/CMakeFiles/banking.dir/Transaction.cpp.o
[ 50%] Linking CXX static library libbanking.a
[ 50%] Built target banking
[ 57%] Building CXX object third-party/gtest/googlemock/CMakeFiles/gmock.dir/src/gmock-all.cc.o
[ 64%] Linking CXX static library ../../../lib/libgmock.a
[ 64%] Built target gmock
[ 71%] Building CXX object third-party/gtest/googlemock/CMakeFiles/gmock_main.dir/src/gmock_main.cc.o
[ 78%] Linking CXX static library ../../../lib/libgmock_main.a
[ 78%] Built target gmock_main
[ 85%] Building CXX object CMakeFiles/check.dir/tests/test_account.cpp.o
[ 92%] Building CXX object CMakeFiles/check.dir/tests/test_transaction.cpp.o
[100%] Linking CXX executable check
[100%] Built target check
```

`./build/check`:
```sh
Running main() from /home/ilyasov_ilya/ferdosiakrymskaa-svg/workspace/projects/lab05/lab05_final_edit/lab05/third-party/gtest/googletest/src/gtest_main.cc
[==========] Running 17 tests from 4 test suites.
[----------] Global test environment set-up.
[----------] 4 tests from AccountBehaviourTest
[ RUN      ] AccountBehaviourTest.LockUnlock
[       OK ] AccountBehaviourTest.LockUnlock (0 ms)
[ RUN      ] AccountBehaviourTest.ChangeBalanceWhenUnlocked
[       OK ] AccountBehaviourTest.ChangeBalanceWhenUnlocked (0 ms)
[ RUN      ] AccountBehaviourTest.ChangeBalanceWhenLocked
[       OK ] AccountBehaviourTest.ChangeBalanceWhenLocked (0 ms)
[ RUN      ] AccountBehaviourTest.IdIsCorrect
[       OK ] AccountBehaviourTest.IdIsCorrect (0 ms)
[----------] 4 tests from AccountBehaviourTest (0 ms total)

[----------] 8 tests from TransactionTest
[ RUN      ] TransactionTest.DefaultFee
[       OK ] TransactionTest.DefaultFee (0 ms)
[ RUN      ] TransactionTest.SetFee
[       OK ] TransactionTest.SetFee (0 ms)
[ RUN      ] TransactionTest.SameAccount
[       OK ] TransactionTest.SameAccount (0 ms)
[ RUN      ] TransactionTest.NegativeSum
[       OK ] TransactionTest.NegativeSum (0 ms)
[ RUN      ] TransactionTest.TooSmall
[       OK ] TransactionTest.TooSmall (0 ms)
[ RUN      ] TransactionTest.FeeExceeds
[       OK ] TransactionTest.FeeExceeds (0 ms)
[ RUN      ] TransactionTest.Successful
[       OK ] TransactionTest.Successful (0 ms)
[ RUN      ] TransactionTest.NotEnoughFunds
[       OK ] TransactionTest.NotEnoughFunds (0 ms)
[----------] 8 tests from TransactionTest (0 ms total)

[----------] 1 test from RealTransaction
[ RUN      ] RealTransaction.SaveToDataBaseExecution
1 send to 2 $200
Balance 1 is 799
Balance 2 is 1200
[       OK ] RealTransaction.SaveToDataBaseExecution (0 ms)
[----------] 1 test from RealTransaction (0 ms total)

[----------] 4 tests from DifferentBalances/AccountBalanceTest
[ RUN      ] DifferentBalances/AccountBalanceTest.InitialBalance/0
[       OK ] DifferentBalances/AccountBalanceTest.InitialBalance/0 (0 ms)
[ RUN      ] DifferentBalances/AccountBalanceTest.InitialBalance/1
[       OK ] DifferentBalances/AccountBalanceTest.InitialBalance/1 (0 ms)
[ RUN      ] DifferentBalances/AccountBalanceTest.InitialBalance/2
[       OK ] DifferentBalances/AccountBalanceTest.InitialBalance/2 (0 ms)
[ RUN      ] DifferentBalances/AccountBalanceTest.InitialBalance/3
[       OK ] DifferentBalances/AccountBalanceTest.InitialBalance/3 (0 ms)
[----------] 4 tests from DifferentBalances/AccountBalanceTest (0 ms total)

[----------] Global test environment tear-down
[==========] 17 tests from 4 test suites ran. (0 ms total)
[  PASSED  ] 17 tests.

```




```

## Вывод
В результате проделанной лабораторной работы мы научились реализовывать тесты используя мок-объекты, кроме того проверяли покрываемость кода на сайте `Coveralls.io`.
