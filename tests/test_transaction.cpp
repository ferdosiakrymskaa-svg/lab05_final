#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "Transaction.h"
#include "mock_classes.h"

using ::testing::StrictMock;
using ::testing::Return;
using ::testing::_;

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

// Проверка значений fee
TEST_F(TransactionTest, DefaultFee) {
    EXPECT_EQ(trans->fee(), 1);
}

TEST_F(TransactionTest, SetFee) {
    trans->set_fee(10);
    EXPECT_EQ(trans->fee(), 10);
}

// Перевод самому себе
TEST_F(TransactionTest, SameAccount) {
    EXPECT_THROW(trans->Make(*from, *from, 200), std::logic_error);
}

// Отрицательная сумма
TEST_F(TransactionTest, NegativeSum) {
    EXPECT_THROW(trans->Make(*from, *to, -100), std::invalid_argument);
}

// Слишком маленькая сумма
TEST_F(TransactionTest, TooSmall) {
    EXPECT_THROW(trans->Make(*from, *to, 70), std::logic_error);
}

// Слишком большая комиссия
TEST_F(TransactionTest, FeeExceeds) {
    trans->set_fee(60);
    // Метод не должен вызываться
    EXPECT_CALL(*trans, SaveToDataBase(_, _, _)).Times(0);
    EXPECT_FALSE(trans->Make(*from, *to, 100));
}

// Успешная транзакция
TEST_F(TransactionTest, Successful) {
    const int sum = 200;
    trans->set_fee(1);

    testing::InSequence seq;
    EXPECT_CALL(*from, Lock());
    EXPECT_CALL(*to, Lock());

    EXPECT_CALL(*to, ChangeBalance(sum));                // Credit
    EXPECT_CALL(*from, GetBalance()).WillOnce(Return(1000));
    EXPECT_CALL(*from, ChangeBalance(-(sum + 1)));       // Debit sum+fee

    EXPECT_CALL(*trans, SaveToDataBase(_, _, sum));      // любые Account, сумма = sum

    EXPECT_CALL(*to, Unlock());
    EXPECT_CALL(*from, Unlock());

    EXPECT_TRUE(trans->Make(*from, *to, sum));
}

// Недостаток средств – откат
TEST_F(TransactionTest, NotEnoughFunds) {
    const int sum = 1200;
    trans->set_fee(1);

    testing::InSequence seq;
    EXPECT_CALL(*from, Lock());
    EXPECT_CALL(*to, Lock());

    EXPECT_CALL(*to, ChangeBalance(sum));                // Credit
    EXPECT_CALL(*from, GetBalance()).WillOnce(Return(1000));
    EXPECT_CALL(*to, ChangeBalance(-sum));               // откат

    EXPECT_CALL(*trans, SaveToDataBase(_, _, sum));      // любые Account, сумма = sum

    EXPECT_CALL(*to, Unlock());
    EXPECT_CALL(*from, Unlock());

    EXPECT_FALSE(trans->Make(*from, *to, sum));
}

// Отдельный тест для покрытия реального SaveToDataBase
TEST(RealTransaction, SaveToDataBaseExecution) {
    Account from(1, 1000);
    Account to(2, 1000);
    Transaction trans;
    trans.set_fee(1);
    EXPECT_TRUE(trans.Make(from, to, 200));
    SUCCEED();
}
